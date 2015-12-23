/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Definitions for the TCP protocol sk_state field.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef _LINUX_TCP_STATES_H
#define _LINUX_TCP_STATES_H

enum {
	TCP_ESTABLISHED = 1,
	TCP_SYN_SENT,
	TCP_SYN_RECV,
	TCP_FIN_WAIT1,
	TCP_FIN_WAIT2,
	TCP_TIME_WAIT,
	TCP_CLOSE,
	TCP_CLOSE_WAIT,
	TCP_LAST_ACK,
	TCP_LISTEN,
	TCP_CLOSING,	/* Now a valid state */

	TCP_MAX_STATES	/* Leave at the end! */
};

#define TCP_STATE_MASK	0xF

#define TCP_ACTION_FIN	(1 << 7)

// TCPF_XXX 是辅助enum 。如，如果希望判断一个socket是否处于某几种状态中的一种
// 可以这么写： ((1 << sk->sk_state) & (TCPF_CLOSE | TCPF_LISTEN))
enum {
	TCPF_ESTABLISHED = (1 << 1), // (1 << TCP_ESTABLISHED)
	TCPF_SYN_SENT	 = (1 << 2), // (1 << TCP_SYN_SENT)
	TCPF_SYN_RECV	 = (1 << 3), // (1 << TCP_SYN_RECV)
	TCPF_FIN_WAIT1	 = (1 << 4), // (1 << TCP_FIN_WAIT1)
	TCPF_FIN_WAIT2	 = (1 << 5), // (1 << TCP_FIN_WAIT2)
	TCPF_TIME_WAIT	 = (1 << 6), // (1 << TCP_TIME_WAIT)
	TCPF_CLOSE	 = (1 << 7),     // (1 << TCP_CLOSE)
	TCPF_CLOSE_WAIT	 = (1 << 8), // (1 << TCP_CLOSE_WAIT)
	TCPF_LAST_ACK	 = (1 << 9), // (1 << TCP_LAST_ACK)
	TCPF_LISTEN	 = (1 << 10),    // (1 << TCP_LISTEN)
	TCPF_CLOSING	 = (1 << 11) // (1 << TCP_CLOSING)
};

#endif	/* _LINUX_TCP_STATES_H */
