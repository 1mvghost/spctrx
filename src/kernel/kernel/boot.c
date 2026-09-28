#include <boot.h>

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

__attribute__((used, section(".limine_requests_start"))) static volatile u64
    limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile u64
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

u64 getHHDM() {
  return hhdmRequest.response->offset;
}