#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  fprintf(stderr, "KERN_SANITIZER_MAIN_ENTERED control=heap_oob\n");
  fflush(stderr);
  size_t size = argc > 1 ? strtoul(argv[1], NULL, 10) : 64;
  char *buffer = malloc(size);
  if (buffer == NULL) {
    return 2;
  }
  memset(buffer, 0x41, size + 8);
  fprintf(stderr, "%d\n", buffer[0]);
  free(buffer);
  return 0;
}
