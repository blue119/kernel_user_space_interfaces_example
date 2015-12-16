obj-m += gnKernel.o
#obj-m += udpRecvCallback.o
#obj-m += udpSend.o
#obj-m += ana_packet.o
#obj-m += syscalltable.o
#obj-m += procfs.o
#obj-m += seqfile.o
#obj-m += sysfs_ex.o
#obj-m += debugfs.o
#obj-m += usermodehelper.o
#obj-m += configfs_ex.o
#obj-m += cdev.o
#obj-m += ioctl.o
#obj-m += signal_kernel.o
#obj-m += mmap_simple_kernel.o
#obj-m += sysctl.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
