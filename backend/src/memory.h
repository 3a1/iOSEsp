#pragma once
#include "pch.h"

/* External functions from jailbreak lib */
extern uint64_t proc_find(int pid);
extern uint64_t proc_task(uint64_t proc);
extern uint64_t kread_ptr(uint64_t va);
extern int vreadbuf(uint64_t tte_p, const void *addr, void *outdata, size_t datalen);

uint64_t get_process_ttep(int pid);
uint64_t get_process_base(int pid);

bool read_buffer(uint64_t ttep, uint64_t vaddr, void* buffer, size_t size);
float read_float(uint64_t ttep, uint64_t vaddr);
uint8_t read_u8(uint64_t ttep, uint64_t vaddr);
uint16_t read_u16(uint64_t ttep, uint64_t vaddr);
uint32_t read_u32(uint64_t ttep, uint64_t vaddr);
uint64_t read_u64(uint64_t ttep, uint64_t vaddr);