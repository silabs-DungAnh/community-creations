#include "ota_client.h"
#include "uuid_number.h"
#include "flash_store.h"
// ====== Static biến nội bộ ======
#define INVALID_CONNECTION_HANDLE 0xFF
#define INVALID_CHARACTERISTIC_HANDLE 0xFFFF
static time_t   dfu_start_time;


// ====== Helper ======

static ota_state_t ota_state = OTA_IDLE;
static uint8_t ble_connection = INVALID_CONNECTION_HANDLE;
static uint16_t ota_control_characteristic         = INVALID_CHARACTERISTIC_HANDLE;
static uint16_t ota_data_characteristic            = INVALID_CHARACTERISTIC_HANDLE;
static uint16_t application_version_characteristic = INVALID_CHARACTERISTIC_HANDLE;
static uint32_t g_total    = 0; 
//for OTA


#define MAX_DFU_PACKET 256
static uint8_t dfu_data[MAX_DFU_PACKET];
static bool dfu_resync = false;
static size_t dfu_toload = 0;
static size_t dfu_total = 0;
static size_t dfu_current_pos = 0;
static time_t dfu_start_time;
static uint32_t offset = 0;
#define MAX_MTU 256
#define MIN_MTU 23
static uint32_t mtu = MIN_MTU;
static uint16_t max_mtu = MAX_MTU;
void ota_change_state(ota_state_t new_state)
{
  ota_state = new_state;
  switch (ota_state) {
    case OTA_END: {
      sl_status_t sc = sl_bt_gatt_write_characteristic_value(
          ble_connection, ota_control_characteristic, 1, (uint8_t*)"\x03");
      if (sc) ERROR_EXIT("Error, write END failed,0x%x", sc);
      app_log("DFU completed");
      reset_ota_params();
    } break;

    case OTA_UPLOAD_WITHOUT_RSP: send_dfu_block(); break;
    case OTA_UPLOAD_WITH_RSP:    send_dfu_packet_with_confirmation(); break;

    case OTA_BEGIN: {
      sl_status_t sc = sl_bt_gatt_write_characteristic_value(
          ble_connection, ota_control_characteristic, 1, (uint8_t*)"\x00");
      if (sc) ERROR_EXIT("Error, write BEGIN failed,0x%x", sc);
      app_log("DFU mode..."); dfu_start_time = time(NULL);
    } break;

    case OTA_READ_OTA_DATA_PROPERTIES: {
      if (ota_data_characteristic == 0xFFFF) ERROR_EXIT("Invalid data handle");
      sl_status_t sc = sl_bt_gatt_read_characteristic_value_from_offset(
          ble_connection, (ota_data_characteristic - 1), 0, 1);
      if (sc) ERROR_EXIT("Error, read data properties failed,0x%x", sc);
    } break;

    case OTA_INIT: {
      app_log("OTA init...");

      if (dfu_set_size_from_slot() != 0) ERROR_EXIT("Error, DFU file read failed\n");
      ota_change_state(OTA_READ_APPLICATION_VERSION);
    } break;

    case OTA_READ_APPLICATION_VERSION:
      sl_bt_gatt_read_characteristic_value(ble_connection, application_version_characteristic);
      break;

    default: break;
  }
}

// API for initialization
void init_ota_client(uint8_t connection, uint16_t control_char, uint16_t data_char, uint16_t app_ver_char) {
  ble_connection = connection;
  ota_control_characteristic = control_char;
  ota_data_characteristic = data_char;
  application_version_characteristic = app_ver_char;
}
//send with rsp
void send_dfu_packet_with_confirmation()
{
  time_t ti;
  size_t dfu_size;

  if (dfu_toload > 0) {
    sl_status_t sc;
    
    dfu_size = dfu_toload > (mtu - 3) ? (mtu - 3) : dfu_toload;
    size_t got = ota_store_read((uint32_t)offset, dfu_data, dfu_size);
  if (got != dfu_size) {
    ERROR_EXIT("slot read failure off=%u want=%u got=%u",
               (unsigned)offset, (unsigned)dfu_size, (unsigned)got);
  }
    sc = sl_bt_gatt_write_characteristic_value(ble_connection, ota_data_characteristic, dfu_size, dfu_data);
    if (sc) {
      ERROR_EXIT("Error,%s, characteristic write failed:0x%x", __FUNCTION__, sc);
    }
    offset += dfu_size;
    dfu_current_pos += dfu_size;
    dfu_toload -= dfu_size;

    ti = time(NULL);
    if (ti != dfu_start_time && dfu_total > 0) {
      app_log("\r%d%% %.2fkbit/s                   ",
              (int)(100 * dfu_current_pos / dfu_total),
              dfu_current_pos * 8.0 / 1000.0 / difftime(ti, dfu_start_time));
    }
  } else {
    ti = time(NULL);
    app_log("\n");
    app_log("time: %.2fs", difftime(ti, dfu_start_time));

    app_log("\n");
    ota_change_state(OTA_END);
  }
}

//send wo rsp
void send_dfu_block()
{
  time_t ti;
  size_t dfu_size;
  while (dfu_toload > 0) {
    sl_status_t sc;
    uint16_t sent_len;

    dfu_size = dfu_toload > (mtu - 3) ? (mtu - 3) : dfu_toload;
    size_t got = ota_store_read((uint32_t)offset, dfu_data, dfu_size);
  if (got != dfu_size) {
    ERROR_EXIT("slot read failure off=%u want=%u got=%u",
               (unsigned)offset, (unsigned)dfu_size, (unsigned)got);
  }

    do {
      sc = sl_bt_gatt_write_characteristic_value_without_response(ble_connection, ota_data_characteristic, dfu_size, dfu_data, &sent_len);
    } while (sc != SL_STATUS_OK);

    if (sc) {
      ERROR_EXIT("Error,%s, characteristic write failed:0x%x", __FUNCTION__, sc);
    }
    offset += dfu_size;
    dfu_current_pos += dfu_size;
    dfu_toload -= dfu_size;

    ti = time(NULL);
    if (ti != dfu_start_time && dfu_total > 0) {
      app_log("\r%d%% %.2fkbit/s                   ",
              (int)(100 * dfu_current_pos / dfu_total),
              dfu_current_pos * 8.0 / 1000.0 / difftime(ti, dfu_start_time));
    }
  }

  app_log("\n");
  app_log("time: %.2fs", difftime(ti, dfu_start_time));

  app_log("\n");
  ota_change_state(OTA_END);
}
// API for getting status
uint8_t ota_client_get_connection(void) {
  return ble_connection;
}
ota_state_t ota_client_get_state(void) {
  return ota_state;
}
// API for setting DFU size
int dfu_set_size_from_slot(void)
{
  dfu_total  = ota_store_get_payload_size();
  dfu_toload = dfu_total;
  if (dfu_total == 0) return -1;

  app_log("Bytes to send: %lu\n", (unsigned long)dfu_total);
  dfu_current_pos = 0;
  dfu_start_time  = time(NULL);
  return 0;
}
// ====== Main dispatcher ======
