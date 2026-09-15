#include "utils.h"

int get_pid_by_name(const char *name) 
{
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    size_t size;
    
    /* Get buffer size needed */
    sysctl(mib, 4, NULL, &size, NULL, 0);
    struct kinfo_proc* procs = malloc(size);
    
    /* Fill the buffer with process metrics */
    sysctl(mib, 4, procs, &size, NULL, 0);
    int procCount = size / sizeof(struct kinfo_proc);
    
    for (int i = 0; i < procCount; i++) 
    {
        if (strstr(procs[i].kp_proc.p_comm, name) != NULL) 
        {
            int pid = procs[i].kp_proc.p_pid;
            free(procs);
            return pid;
        }
    }
    
    /* Free allocated processes buffer */
    free(procs);
    return 0;
}

void sleep_ms(long ms) 
{
    /* Convert ms to nanoms */
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000; 
    
    /* Time to sleep */
    nanosleep(&ts, NULL);
    /* Zzz... */
}
