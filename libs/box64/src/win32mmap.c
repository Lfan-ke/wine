#include "win32mmap.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(box64);
 
ULONG_PTR default_zero_bits32 = 0x7fffffff;

static uint32_t prot_unix_to_win32(uint32_t unx)
{
    if ((unx & (PROT_READ | PROT_WRITE | PROT_EXEC)) == (PROT_READ | PROT_WRITE | PROT_EXEC))
        return PAGE_EXECUTE_READWRITE;
    if ((unx & (PROT_READ | PROT_EXEC)) == (PROT_READ | PROT_EXEC))
        return PAGE_EXECUTE_READ;
    if ((unx & PROT_EXEC) == PROT_EXEC)
        return PAGE_EXECUTE_READ;
    if ((unx & (PROT_READ | PROT_WRITE)) == (PROT_READ | PROT_WRITE))
        return PAGE_READWRITE;
    if ((unx & PROT_READ) == PROT_READ)
        return PAGE_READONLY;
    printf_log(LOG_INFO, "Cannot map prot %x\n", unx);
    return 0; // or PAGE_NOACCESS ?
}

#define GETCURRENTPROCESS ((HANDLE)~(ULONG_PTR)0)
int mprotect(void *addr, size_t len, int prot)
{
    NTSTATUS ntstatus;
    ULONG old_prot;
    SIZE_T allocsize = len;
    ntstatus = NtProtectVirtualMemory( GETCURRENTPROCESS, &addr, &allocsize, prot_unix_to_win32(prot), &old_prot );
    if (BOX64ENV(dynarec_log)) printf_log(LOG_DEBUG, "ntstatus %08lx %p %ld -> %d\n", ntstatus, addr, old_prot, prot_unix_to_win32(prot));
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    NTSTATUS ntstatus;
    SIZE_T sz = length;
    ULONG_PTR limit;
    void *ret = NULL;

    if (BOX64ENV(dynarec_log)) printf_log(LOG_INFO, "%p %x %i %i\n", addr, length, prot, flags);

    if (addr != NULL)
    {
        printf_log(LOG_INFO, "Fixed address requested: %p\n", addr);
        return MAP_FAILED;
    }
    if (fd && fd != -1)
    {
        printf_log(LOG_INFO, "File descriptor requested: %i\n", fd);
        return MAP_FAILED;
    }
    if (offset)
    {
        printf_log(LOG_INFO, "Offset requested: %x\n", offset);
        return MAP_FAILED;
    }

    if (flags & MAP_32BIT)
        limit = default_zero_bits32;
    else
        limit = 0;

    ntstatus = NtAllocateVirtualMemory(NtCurrentProcess(), &ret, limit, &sz, MEM_COMMIT | MEM_RESERVE, prot_unix_to_win32(prot));
    if (BOX64ENV(dynarec_log)) printf_log(LOG_DEBUG, "ntstatus %08lx %p\n", ntstatus, ret);
    return ret;
}

int munmap(void *addr, size_t length)
{
    int ret = 0;
    if (BOX64ENV(dynarec_log)) printf_log(LOG_INFO, "%p %x\n", addr, length);
    if (NtFreeVirtualMemory(NtCurrentProcess(), &addr, &length, MEM_RELEASE))
        ret = -1;
    return ret;
}


void* internal_mmap(void *addr, unsigned long length, int prot, int flags, int fd, ssize_t offset)
{
    if (BOX64ENV(dynarec_log)) printf_log(LOG_INFO, "%p %x %i %i\n", addr, length, prot, flags);
    return mmap(addr, length, prot, flags, fd, offset);
}

int internal_munmap(void* addr, unsigned long length)
{
    if (BOX64ENV(dynarec_log)) printf_log(LOG_INFO, "%p %x\n", addr, length);
    return munmap(addr, length);
}


char *strerror(int errnum)
{
    if (BOX64ENV(dynarec_log)) printf_log(LOG_INFO, "error number: %i\n", errnum);
    return "see above";
}
