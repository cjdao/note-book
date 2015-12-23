/*
 * NET		Generic infrastructure for Network protocols.
 *
 * Authors:	Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 *
 * 		From code originally in include/net/tcp.h
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

#include <net/request_sock.h>

/*
 * Maximum number of SYN_RECV sockets in queue per LISTEN socket.
 * One SYN_RECV socket costs about 80bytes on a 32bit machine.
 * It would be better to replace it with a global counter for all sockets
 * but then some measure against one socket starving all other sockets
 * would be needed.
 *
 * The minimum value of it is 128. Experiments with real servers show that
 * it is absolutely not enough even at 100conn/sec. 256 cures most
 * of problems.
 * This value is adjusted to 128 for low memory machines,
 * and it will increase in proportion to the memory of machine.
 * Note : Dont forget somaxconn that may limit backlog too.
 */
int sysctl_max_syn_backlog = 256;
EXPORT_SYMBOL(sysctl_max_syn_backlog);

// 该函数主要是对request_sock_queue里的listen_opt域进行初始化
int reqsk_queue_alloc(struct request_sock_queue *queue,
		      unsigned int nr_table_entries)
{
	size_t lopt_size = sizeof(struct listen_sock);
	struct listen_sock *lopt;

    // 我们在用户态可以通过命令'sysctl -w net.core.somaxconn=xxx' 来限制我们调用
    // listen时所能传递的最大backlog的backlog参数
    // 从这里的实现看,如果socket是tcp类型'sysctl -w net.core.somaxconn=xxx'
    // 赋值的有效范围为: 8~sysctl_max_syn_backlog  ($sysctl net.ipv4.tcp_max_syn_backlog)
	nr_table_entries = min_t(u32, nr_table_entries, sysctl_max_syn_backlog);
	nr_table_entries = max_t(u32, nr_table_entries, 8);
    // nr_table_entries 最终的值 总是比backlog指定的大，且向上对齐到2的N次方
	nr_table_entries = roundup_pow_of_two(nr_table_entries + 1);
     
	lopt_size += nr_table_entries * sizeof(struct request_sock *);
	if (lopt_size > PAGE_SIZE)
		lopt = vzalloc(lopt_size);
	else
		lopt = kzalloc(lopt_size, GFP_KERNEL);
	if (lopt == NULL)
		return -ENOMEM;

    // nr_table_entries的最终值总是2的N次方，max_qlen_log就是N的值
	for (lopt->max_qlen_log = 3; // nr_table_entries的最小值是8，即2的3次方
	     (1 << lopt->max_qlen_log) < nr_table_entries;
	     lopt->max_qlen_log++);

	get_random_bytes(&lopt->hash_rnd, sizeof(lopt->hash_rnd));
	rwlock_init(&queue->syn_wait_lock);
	queue->rskq_accept_head = NULL;
	lopt->nr_table_entries = nr_table_entries;

	write_lock_bh(&queue->syn_wait_lock);
    // 本函数的终极目标就是这里了!!!
	queue->listen_opt = lopt;
	write_unlock_bh(&queue->syn_wait_lock);

	return 0;
}

void __reqsk_queue_destroy(struct request_sock_queue *queue)
{
	struct listen_sock *lopt;
	size_t lopt_size;

	/*
	 * this is an error recovery path only
	 * no locking needed and the lopt is not NULL
	 */

	lopt = queue->listen_opt;
	lopt_size = sizeof(struct listen_sock) +
		lopt->nr_table_entries * sizeof(struct request_sock *);

	if (lopt_size > PAGE_SIZE)
		vfree(lopt);
	else
		kfree(lopt);
}

static inline struct listen_sock *reqsk_queue_yank_listen_sk(
		struct request_sock_queue *queue)
{
	struct listen_sock *lopt;

	write_lock_bh(&queue->syn_wait_lock);
	lopt = queue->listen_opt;
	queue->listen_opt = NULL;
	write_unlock_bh(&queue->syn_wait_lock);

	return lopt;
}

void reqsk_queue_destroy(struct request_sock_queue *queue)
{
	/* make all the listen_opt local to us */
	struct listen_sock *lopt = reqsk_queue_yank_listen_sk(queue);
	size_t lopt_size = sizeof(struct listen_sock) +
		lopt->nr_table_entries * sizeof(struct request_sock *);

	if (lopt->qlen != 0) {
		unsigned int i;

		for (i = 0; i < lopt->nr_table_entries; i++) {
			struct request_sock *req;

			while ((req = lopt->syn_table[i]) != NULL) {
				lopt->syn_table[i] = req->dl_next;
				lopt->qlen--;
				reqsk_free(req);
			}
		}
	}

	WARN_ON(lopt->qlen != 0);
	if (lopt_size > PAGE_SIZE)
		vfree(lopt);
	else
		kfree(lopt);
}

