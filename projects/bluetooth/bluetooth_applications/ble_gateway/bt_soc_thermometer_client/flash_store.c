// ota_store_slot.c
#include "btl_interface.h"              // bootloader_init/deinit
#include "btl_interface_storage.h"      // bootloader_*Storage*
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef OTA_SLOT_ID
#define OTA_SLOT_ID   0u
#endif

static uint32_t g_written = 0;       // tổng byte đã ghi
static uint32_t g_total    = 0;      // tổng byte dự kiến
static bool     g_opened   = false;

int pad4_write(uint32_t slot, uint32_t off, const uint8_t* data, size_t len) {
  // bootloader_writeStorage: length phải %4 == 0
  if ((len & 3u) == 0) {
    return bootloader_eraseWriteStorage(slot, off, (uint8_t*)data, len);
  }
  // pad 0xFF để đủ bội số 4
  uint8_t tmp[256 + 4];
  if (len > sizeof(tmp) - 4) return -1; // block quá lớn cho buffer tạm cục bộ
  memcpy(tmp, data, len);
  size_t pad = 4 - (len & 3u);
  memset(tmp + len, 0xFF, pad);
  return bootloader_eraseWriteStorage(slot, off, tmp, len + pad);
}

// === API bạn gọi từ app ===
int ota_store_begin(uint32_t total_size /*, uint32_t crc_expect nếu bạn muốn giữ*/ )
{
  if (bootloader_init() != BOOTLOADER_OK) {
    return -1;
  }
  g_written = 0;
  g_total   = total_size;
  g_opened  = true;
  return 0;
}

int ota_store_write(const uint8_t* data, size_t len)
{
  if (!g_opened) return -1;
  // Ghi block vào slot, auto pad 4B nếu cần
  int32_t rc = pad4_write(OTA_SLOT_ID, g_written, data, len);
  if (rc != BOOTLOADER_OK) return -2;

  g_written += (uint32_t)len;   // chỉ tính “payload thật” (không tính byte pad)
  return 0;
}

int ota_store_finalize(void)
{
  if (!g_opened) return -1;
  int32_t rc = bootloader_verifyImage(OTA_SLOT_ID, NULL);
  if (rc != BOOTLOADER_OK) { bootloader_deinit(); return -2; }
  
  g_opened = false;
  bootloader_deinit();
  return 0;
}

// ==== Phục vụ luồng “đọc để phát BLE” ===

// Lấy tổng size vừa ghi (payload thật) – dùng lại trong phiên hiện tại
uint32_t ota_store_get_payload_size(void) {
  return g_total;
}

// Đọc từ slot để đẩy BLE (không cần bội số 4 khi đọc)
size_t ota_store_read(uint32_t off, uint8_t* out, size_t max)
{
  // bootloader_readStorage đọc theo (slot, offset)
  int32_t rc = bootloader_readStorage(OTA_SLOT_ID, off, out, max);
  if (rc != BOOTLOADER_OK) return 0;

  return max;
}

void reset_ota_params()
{
g_written = 0;       // tổng byte đã ghi
g_total    = 0;      // tổng byte dự kiến
g_opened   = false;
}
