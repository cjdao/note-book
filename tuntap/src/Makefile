tunclient:tunclient.o tun_alloc.o
	gcc $^ -o $@

tunclient.o:tunclient.c
	gcc $^ -c

tun_alloc.o:tun_alloc.c tun_alloc.h
	gcc tun_alloc.c -c
