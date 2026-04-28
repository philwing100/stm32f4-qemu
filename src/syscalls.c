#include <sys/stat.h>

int _write(int file, char *ptr, int len) {
    extern void uart_putchar(char c);
    for (int i = 0; i < len; i++) uart_putchar(ptr[i]);
    return len;
}

int _close(int file) { return -1; }
int _fstat(int file, struct stat *st) { return -1; }
int _isatty(int file) { return 1; }
int _lseek(int file, int ptr, int dir) { return 0; }
int _read(int file, char *ptr, int len) { return 0; }
int _getpid(void) { return 1; }
int _kill(int pid, int sig) { return -1; }

static char heap[4096];
static char *heap_ptr = heap;
char *_sbrk(int incr) {
    char *prev = heap_ptr;
    if (heap_ptr - heap + incr >= (int)sizeof(heap)) return (char *)-1;
    heap_ptr += incr;
    return prev;
}
