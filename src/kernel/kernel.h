#ifndef KERNEL_H
#define KERNEL_H

#include <std/std.h>
#include <kernel/drivers/kb/kb.h>
#include <kernel/drivers/vga/vga.h>

void kernel_begin_critical();
void kernel_end_critical();

#endif