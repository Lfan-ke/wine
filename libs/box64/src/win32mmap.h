#include <stdarg.h>
#include <stdint.h>
#define WIN32_NO_STATUS
#include "ntstatus.h"
#include "windef.h"
#include "winnt.h"
#include "winternl.h"
#include "debug.h"

typedef int off_t;
typedef __int64 ssize_t;

#define PROT_READ       0x1
#define PROT_WRITE      0x2
#define PROT_EXEC       0x4

#define MAP_FAILED      ((void *) -1)
#define MAP_PRIVATE     0x02
#define MAP_FIXED       0x10
#define MAP_ANONYMOUS   0x20
#define MAP_32BIT       0x40

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
int mprotect(void *addr, size_t len, int prot);

void* internal_mmap(void *addr, unsigned long length, int prot, int flags, int fd, ssize_t offset);
int internal_munmap(void* addr, unsigned long length);

char *strerror(int errnum);
