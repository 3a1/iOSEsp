#include "memory.h"

uint64_t get_process_ttep(int pid)
{
    /* Get proc_t proc */
    uint64_t proc = proc_find(pid);
    if (!proc) { ErrorPrint("[-] proc_find failed"); return 0; }

    /* Get task_t task */
    uint64_t task = proc_task(proc);
    if (!task) { ErrorPrint("[-] proc_task failed"); return 0; }

    /* Get vm_map_t map */
    uint64_t map = kread_ptr(task + TASK_map);
    if (!map) { ErrorPrint("[-] map failed"); return 0; }

    /* Get pmap_t pmap */
    uint64_t pmap = kread_ptr(map + VM_MAP_pmap);
    if (!pmap) { ErrorPrint("[-] pmap failed"); return 0; }

    /* Get pmap_paddr_t ttep */
    uint64_t ttep = kread_ptr(pmap + PMAP_ttep);
    if (!ttep) { ErrorPrint("[-] ttep failed"); return 0; }

    return ttep;
}

uint64_t get_process_base(int pid) 
{
    /* Get proc_t proc */
    uint64_t proc = proc_find(pid);
    if (!proc) { ErrorPrint("[-] proc_find failed"); return 0; }

    /* Get task_t task */
    uint64_t task = proc_task(proc);
    if(!task) { ErrorPrint("[-] proc_task failed"); return 0; }

    /* Get vm_map_t map */
    uint64_t map = kread_ptr(task + 0x28);
    if (!map) { ErrorPrint("[-] map failed"); return 0; }

    /* Get first vm_map_entry_t */
    uint64_t first_entry = kread_ptr(map + 0x18); 
    if (!first_entry) { ErrorPrint("[-] first_entry failed"); return 0; }

    /* Get start of vm_map_entry_t */
    uint64_t app_base_vaddr = kread_ptr(first_entry + 0x10);
    if (!app_base_vaddr) { ErrorPrint("[-] app_base_vaddr failed"); return 0; }

    return app_base_vaddr;
}

bool read_buffer(uint64_t ttep, uint64_t vaddr, void* buffer, size_t size) 
{
    int result = vreadbuf(ttep, (void*)vaddr, buffer, size);
    if (result == -1) { return false; }
    return true;
}

float read_float(uint64_t ttep, uint64_t vaddr) 
{
    float value = 0;
    read_buffer(ttep, vaddr, &value, sizeof(value));
    return value;
}

uint8_t read_u8(uint64_t ttep, uint64_t vaddr) 
{
    uint8_t value = 0;
    read_buffer(ttep, vaddr, &value, sizeof(value));
    return value;
}

uint16_t read_u16(uint64_t ttep, uint64_t vaddr) 
{
    uint16_t value = 0;
    read_buffer(ttep, vaddr, &value, sizeof(value));
    return value;
}

uint32_t read_u32(uint64_t ttep, uint64_t vaddr) 
{
    uint32_t value = 0;
    read_buffer(ttep, vaddr, &value, sizeof(value));
    return value;
}

uint64_t read_u64(uint64_t ttep, uint64_t vaddr) 
{
    uint64_t value = 0;
    read_buffer(ttep, vaddr, &value, sizeof(value));
    return value;
}
