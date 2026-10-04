#include <stdio.h>

int __TRK_write_console(__file_handle file, unsigned char *buffer, size_t *count,
                        __idle_proc idle_proc) {
  return 0;
}

int __read_console(__file_handle file, unsigned char *buffer, size_t *count,
                   __idle_proc idle_proc) {
  return 0;
}

void InitMetroTRK(void) {}

void EnableMetroTRKInterrupts(void) {}
