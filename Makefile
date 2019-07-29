obj-m:=kernel_demo.o
	KERNNELBUILD:=/lib/modules/$(shell uname -r)/build
	CONFIG_MODULE_SIG=n
default:
	make -C $(KERNNELBUILD) M=$(shell pwd) modules
clean:
	rm -rf *.o *.ko *.mod.o *.mod.c .*.cmd *.markers *.order *.symvers .tmp_versions