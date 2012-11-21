OBJCOPY := objcopy

ARCH     = $(shell uname -m | sed s,i[3456789]86,ia32,)
EFIROOT  = /usr
HDRROOT  = $(EFIROOT)/include/efi
INCLUDES = -I. -I$(HDRROOT) -I$(HDRROOT)/$(ARCH) -I$(HDRROOT)/protocol

CFLAGS = -O2 -fpic -Wall -fshort-wchar -fno-strict-aliasing -mno-red-zone \
		 -fno-merge-constants -fno-stack-protector
CPPFLAGS = $(INCLUDES) -DCONFIG_$(ARCH)
ifeq ($(ARCH),x86_64)
	CPPFLAGS += -DEFI_FUNCTION_WRAPPER
endif

FORMAT   = efi-app-$(ARCH)

CRTOBJS  = $(EFIROOT)/lib/crt0-efi-$(ARCH).o
LDSCRIPT = $(EFIROOT)/lib/elf_$(ARCH)_efi.lds
LDFLAGS  = -nostdlib -T $(LDSCRIPT) -shared -Bsymbolic -L$(EFIROOT)/lib $(CRTOBJS)
LOADLIBS = -lefi -lgnuefi $(shell $(CC) -print-libgcc-file-name)

%.efi: %.so
	$(OBJCOPY) \
		-j .text \
		-j .sdata \
		-j .data \
		-j .dynamic \
		-j .dynsym \
		-j .rel \
		-j .rela \
		-j .reloc \
		--target=efi-app-$(ARCH) $^ $@

%.so: %.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LOADLIBS)

SOURCES = $(wildcard *.c)
TARGETS = $(SOURCES:.c=.efi)

all: $(TARGETS)

clean:
	rm -f ./*.so
	rm -f ./*.o
	rm -f ./*.efi

.DEFAULT_GOAL: all
.PHONY: clean
