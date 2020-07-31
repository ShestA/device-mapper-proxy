obj-m += dmp.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
	@sudo rm -r ../dist
	@mkdir ../dist
	@mv dmp.ko ../dist/dmp.ko
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
	@rm *.o
	@rm *.mod
	@rm *.mod.c
	@rm *.symvers
	@rm *.order
	@rm .dmp.ko.cmd
	@rm .dmp.mod.cmd
	@rm .dmp.mod.o.cmd
	@rm .dmp.o.cmd
	@sudo rm -r -f ../dist
	@mkdir ../dist
	@mv dmp.ko ../dist/dmp.ko
cleanup:
	rm *.o
	rm *.mod
	rm *.mod.c
	rm *.symvers
	rm *.order
	rm .dmp.ko.cmd
	rm .dmp.mod.cmd
	rm .dmp.mod.o.cmd
	rm .dmp.o.cmd
all-clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
