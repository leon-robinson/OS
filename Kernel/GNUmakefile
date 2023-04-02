
override PREFIX := [Kernel]

override CC := gcc
override LD := ld
override OBJCOPY := objcopy

override C_FLAGS += \
	-std=gnu11 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-fno-lto \
	-fno-pie \
	-fno-pic \
	-m64 \
	-march=x86-64 \
	-mabi=sysv \
	-mno-80387 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-mno-red-zone \
	-mcmodel=kernel \
	-O2 \
	-pipe \
	-Wall \
	-Wextra \
	-I Inc \
	-x c

override LD_FLAGS += \
	-nostdlib \
	-static \
	-m elf_x86_64 \
	-z max-page-size=0x1000 \
	-T LINKER.LD

override NASM_FLAGS += \
	-f elf64

ifeq ($(shell $(LD) --help 2>&1 | grep 'no-pie' >/dev/null 2>&1; echo $$?),0)
    override LDFLAGS += -no-pie
endif

override C_FILES := $(shell find -L . -type f -name '*.C')
override NASM_FILES := $(shell find -L . -type f -name '*.ASM')
override OBJ += $(patsubst ./Src/%.C,Obj/%.C.O,$(C_FILES))
override OBJ += $(patsubst ./Src/%.ASM,Obj/%.ASM.O,$(NASM_FILES))

.PHONY: all
.PHONY: link

all:
	@$(MAKE) link --quiet

link: $(OBJ)
	@$(LD) $(OBJ) $(LD_FLAGS) -o Obj/KERNEL.ELF
	@$(OBJCOPY) -O binary Obj/KERNEL.ELF Obj/KERNEL.BIN

Obj/%.C.O: Src/%.C
	@mkdir -p $(@D)
	@echo "  $(PREFIX) Compiling $< -> $@..."
	@$(CC) $(C_FLAGS) -c $< -o $@

Obj/%.ASM.O: Src/%.ASM
	@mkdir -p $(@D)
	@echo "  $(PREFIX) Assembling $< -> $@..."
	@nasm $(NASM_FLAGS) $< -o $@