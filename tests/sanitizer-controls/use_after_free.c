#include <stdio.h>
#include <stdlib.h>

__attribute__((noinline)) static void write_after_free(volatile char *buffer) {
  buffer[0] = 1;
  fprintf(stderr, "%d\n", buffer[0]);
}

int main(int argc, char **argv) {
  fprintf(stderr, "KERN_SANITIZER_MAIN_ENTERED control=use_after_free\n");
  fflush(stderr);
  size_t size = argc > 1 ? strtoul(argv[1], NULL, 10) : 64;
  char *buffer = malloc(size);
  if (buffer == NULL) {
    return 2;
  }
  free(buffer);
  write_after_free(buffer);
  return 0;
}
