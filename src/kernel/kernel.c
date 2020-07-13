//freestanding headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//kernel stdlib headers
#include <std/printf.h>
#include <std/string.h>

//kernel headers
#include <kernel/multiboot.h>
#include <kernel/boot.h>
#include <kernel/elf.h>
#include <kernel/assert.h>
#include <kernel/boot_info.h>
#include <kernel/segmentation/gdt.h>
#include <kernel/interrupts/interrupts.h>

//kernel drivers
#include <kernel/drivers/pit/pit.h>
#include <kernel/drivers/serial/serial.h>
#include <kernel/drivers/text_mode/text_mode.h>

//higher-level kernel features
#include <kernel/pmm/pmm.h>
#include <kernel/vmm/vmm.h>
#include <std/kheap.h>
#include <kernel/syscall/syscall.h>

//testing!
#include <kernel/multitasking/tasks/task_small.h>
#include <kernel/util/amc/amc.h>
#include <kernel/util/vfs/fs.h>

#define SPIN while (1) {sys_yield(RUNNABLE);}
#define SPIN_NOMULTI do {} while (1);

void print_os_name() {
    NotImplemented();
}

void system_mem() {
    NotImplemented();
}

void drivers_init(void) {
    pit_timer_init(PIT_TICK_GRANULARITY_1MS);
    serial_init();
    mouse_install();
    kb_install();
}

static void kernel_spinloop() {
    printf("\nBoot complete, kernel spinlooping.\n");
    asm("cli");
    asm("hlt");
}

static void kernel_idle() {
    while (1) {
        //nothing to do!
        //put the CPU to sleep until the next interrupt
        asm volatile("hlt");
    }
}

static void exec() {
    const char* program_name = "cat";

    FILE* fp = initrd_fopen(program_name, "rb");
    char* filename = kmalloc(32);
    snprintf(filename, 32, "test-%d.txt", getpid());
    char* argv[] = {program_name, filename, NULL};

    elf_load_file(program_name, fp, argv);
}

uint32_t initial_esp = 0;
void kernel_main(struct multiboot_info* mboot_ptr, uint32_t initial_stack) {
    initial_esp = initial_stack;
    //set up this driver first so we can output to framebuffer
    text_mode_init();

    //environment info
    boot_info_read(mboot_ptr);
    boot_info_dump();

    //x86 descriptor tables
    gdt_init();
    interrupt_init();

    //external device drivers
    drivers_init();

    //kernel features
    timer_init();
    pmm_init();
    pmm_dump();
    vmm_init();
    kheap_init();
    syscall_init();
    initrd_init();
    
    vmm_dump(boot_info_get()->vmm_kernel);
    vmm_notify_shared_kernel_memory_allocated();
    tasking_init();

    for (int i = 0; i < 512; i++) {
        task_spawn(exec);
    }

    kernel_idle();

    while (1) {}
    //the above call should never return, but just in case...
    kernel_spinloop();
}
