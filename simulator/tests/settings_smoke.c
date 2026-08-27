#include "core/settings.h"
#include "sim_nvs.h"

#include <nvs_flash.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define CHECK(condition, message)                                             \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "settings_smoke failed: %s\n", message);               \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main(void) {
  char root[256];
  snprintf(root, sizeof(root), "/tmp/kern-sim-settings-smoke-%ld",
           (long)getpid());
  CHECK(mkdir(root, 0700) == 0, "create isolated NVS directory");

  sim_nvs_set_data_dir(root);
  CHECK(nvs_flash_init() == ESP_OK, "initialize simulated NVS");
  CHECK(settings_init() == ESP_OK, "initialize settings");
  CHECK(!settings_get_anti_exfil_signing(),
        "anti-exfil signing defaults off");

  CHECK(settings_set_anti_exfil_signing(true) == ESP_OK,
        "persist enabled state");
  CHECK(settings_get_anti_exfil_signing(), "enabled state is immediately live");

  settings_deinit();
  CHECK(settings_init() == ESP_OK, "reopen settings");
  CHECK(settings_get_anti_exfil_signing(), "enabled state survives reopen");

  CHECK(settings_set_anti_exfil_signing(false) == ESP_OK,
        "persist disabled state");
  CHECK(!settings_get_anti_exfil_signing(), "disabled state is immediately live");

  CHECK(settings_reset_all() == ESP_OK, "reset settings");
  CHECK(!settings_get_anti_exfil_signing(), "reset restores default-off state");

  settings_deinit();
  CHECK(nvs_flash_deinit() == ESP_OK, "deinitialize simulated NVS");
  sim_nvs_set_data_dir(NULL);
  char settings_file[320];
  snprintf(settings_file, sizeof(settings_file), "%s/settings.nvs", root);
  CHECK(unlink(settings_file) == 0, "remove simulated settings file");
  CHECK(rmdir(root) == 0, "remove isolated NVS directory");

  puts("settings_smoke ok");
  return 0;
}
