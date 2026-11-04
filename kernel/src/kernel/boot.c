#include <boot.h>
#include <printf.h>

__attribute__((
    used,
    section(
        ".limine_requests"))) static volatile struct limine_paging_mode_request
    pagingModeRequest = {.id = LIMINE_PAGING_MODE_REQUEST_ID,
                         .revision = 4,
                         .mode = LIMINE_PAGING_MODE_X86_64_4LVL};

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_hhdm_request
    hhdmRequest = {.id = LIMINE_HHDM_REQUEST_ID, .revision = 4};

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_module_request
    moduleRequest = {.id = LIMINE_MODULE_REQUEST_ID, .revision = 4};

__attribute__((used, section(".limine_requests_start"))) static volatile u64
    limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile u64
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

u64 getHHDM() {
  return hhdmRequest.response->offset;
}

void parseBootInfo() {
  if (moduleRequest.response == 0) {
    printf("boot: no modules found\n");
    return;
  }

  struct limine_file** modules = moduleRequest.response->modules;

  for (u64 i = 0; i < moduleRequest.response->module_count; i++) {
    printf("boot: found module %s addr %llx\n", modules[i]->path,
           modules[i]->address);
  }
}