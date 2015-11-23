## 搭建linux kernel 调试环境
* [利用qemu进行内核源码级调试](http://blog.csdn.net/gdt_a20/article/details/7231652)

* [制作镜像](http://minimal.linux-bg.org/)

* [网络配置](http://i.huaixiaoz.com/linux/kvm_qemu.html)
```bash
sudo qemu-system-x86_64 -S -kernel arch/x86/boot/bzImage  -initrd /home/clouder/example/minimal_linux_live/work/rootfs.cpio.gz -append "root=/dev/ram rdinit=/init noapic" -net nic,model=virtio -net tap -m 1024
```  
