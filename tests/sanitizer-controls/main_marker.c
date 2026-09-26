#include <unistd.h>

int __real_main(int argc, char **argv, char **envp);

int __wrap_main(int argc, char **argv, char **envp) {
  static const char marker[] = "KERN_SANITIZER_MAIN_ENTERED target=host_test\n";
  (void)write(STDERR_FILENO, marker, sizeof(marker) - 1);
  return __real_main(argc, argv, envp);
}
