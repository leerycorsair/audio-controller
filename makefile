CONFIG_MODULE_SIG=n
CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)
TARGET = audio_module
obj-m := $(TARGET).o


default: audio_controller
	$(MAKE) -C$(KDIR) M=$(PWD) modules
  
audio_controller: audio_controller.c
	gcc -o $@ $@.c -lasound