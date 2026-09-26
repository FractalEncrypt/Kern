#include <stdio.h>
#include <stdlib.h>

__attribute__((noinline)) static void leak_memory(size_t size) {
  volatile char *buffer = malloc(size);
  if (buffer != NULL) {
    buffer[0] = 'K';
  }
}

int main(int argc, char **argv) {
  fprintf(stderr, "KERN_SANITIZER_MAIN_ENTERED control=leak\n");
  fflush(stderr);
  size_t size = argc > 1 ? strtoul(argv[1], NULL, 10) : 64;
  leak_memory(size);
  return 0;
}
