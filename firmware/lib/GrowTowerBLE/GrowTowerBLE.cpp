/*
 * ESP32 BLE GATT Server für GrowTower
 * Nutzt den offiziellen ESP-IDF Bluedroid Stack
 */

#include "GrowTowerBLE.h"

// ESP-IDF BLE Includes
#include <esp_bt.h>
#include <esp_bt_defs.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>

// Service UUID (128-bit): 4fafc201-1fb5-459e-8fcc-c5c9c331914b
static uint8_t service_uuid128[16] = {
    // 4fafc201-1fb5-459e-8fcc-c5c9c331914b (Little Endian / Wire Order)
    0x4b, 0x91, 0x31, 0xc3, 0xc9, 0xc5, 0xcc, 0x8f,
    0x9e, 0x45, 0xb5, 0x1f, 0x01, 0xc2, 0xaf, 0x4f};

// Characteristic UUIDs (16-bit for simplicity)
#define CHAR_LIGHT_UUID 0xABC1
#define CHAR_FAN_UUID 0xABC2
#define CHAR_FAN_MIN_UUID 0xABC3
#define CHAR_FAN_MAX_UUID 0xABC4
#define CHAR_LIGHT_ON_UUID 0xABC5
#define CHAR_LIGHT_OFF_UUID 0xABC6

// GATT Interface
static esp_gatt_if_t g_gatts_if = 0;
static uint16_t g_conn_id = 0;
static bool g_connected = false;
static uint8_t g_connection_count = 0;

// Service Handle
static uint16_t g_service_handle = 0;

// Characteristic Handles
static uint16_t g_char_handles[6] = {0};
static uint16_t g_char_val_handles[6] = {0};

#define CHAR_IDX_LIGHT 0
#define CHAR_IDX_FAN 1
#define CHAR_IDX_FAN_MIN 2
#define CHAR_IDX_FAN_MAX 3
#define CHAR_IDX_LIGHT_ON 4
#define CHAR_IDX_LIGHT_OFF 5

// Characteristic UUIDs array
static uint16_t char_uuids[6] = {CHAR_LIGHT_UUID,    CHAR_FAN_UUID,
                                 CHAR_FAN_MIN_UUID,  CHAR_FAN_MAX_UUID,
                                 CHAR_LIGHT_ON_UUID, CHAR_LIGHT_OFF_UUID};

// Aktuelle Werte
static uint8_t g_values[6] = {0, 15, 0, 100, 18, 14};

// Callbacks
static LightCallback g_light_cb = NULL;
static FanCallback g_fan_cb = NULL;
static ConfigCallback g_config_cbs[4] = {NULL, NULL, NULL, NULL};

// Current characteristic being added
static int g_current_char_idx = 0;

// Advertising Konfiguration
static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 16,
    .p_service_uuid = service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 0,
    .p_service_uuid = NULL,
    .flag = 0,
};

// GAP Event Handler
static void gap_event_handler(esp_gap_ble_cb_event_t event,
                              esp_ble_gap_cb_param_t *param) {
  switch (event) {
  case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    Serial.println("[BLE] Advertising data set");
    // Setze den Gerätenamen EXPLIZIT
    esp_ble_gap_set_device_name("TOWER");
    esp_ble_gap_config_adv_data(&scan_rsp_data);
    break;

  case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
    Serial.println("[BLE] Scan response set");
    esp_ble_gap_start_advertising(&adv_params);
    break;

  case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
    if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
      Serial.println("[BLE] Advertising started! Device 'TOWER' is visible.");
    } else {
      Serial.printf("[BLE] Advertising failed: %d\n",
                    param->adv_start_cmpl.status);
    }
    break;

  default:
    break;
  }
}

// Add next characteristic
static void add_next_characteristic() {
  if (g_current_char_idx >= 6) {
    Serial.println("[BLE] All characteristics created");
    return;
  }

  uint16_t uuid = char_uuids[g_current_char_idx];
  uint8_t val = g_values[g_current_char_idx];

  esp_bt_uuid_t char_uuid = {
      .len = ESP_UUID_LEN_16,
      .uuid = {.uuid16 = uuid},
  };

  esp_attr_value_t attr_val = {
      .attr_max_len = 1,
      .attr_len = 1,
      .attr_value = &val,
  };

  esp_err_t ret = esp_ble_gatts_add_char(
      g_service_handle, &char_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE, &attr_val,
      NULL);

  if (ret == ESP_OK) {
    Serial.printf("[BLE] Adding characteristic 0x%04X...\n", uuid);
  } else {
    Serial.printf("[BLE] Error adding characteristic: %d\n", ret);
  }
}

// GATT Event Handler
static void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param) {

  esp_gatt_srvc_id_t service_id;
  esp_ble_conn_update_params_t conn_params;
  uint16_t handle;
  uint8_t *data;
  uint16_t len;
  int i;

  switch (event) {
  case ESP_GATTS_REG_EVT:
    Serial.println("[BLE] GATT Server registered");
    g_gatts_if = gatts_if;

    // Create service with proper ID structure
    memset(&service_id, 0, sizeof(service_id));
    service_id.is_primary = true;
    service_id.id.inst_id = 0;
    service_id.id.uuid.len = ESP_UUID_LEN_128;
    memcpy(service_id.id.uuid.uuid.uuid128, service_uuid128, 16);

    esp_ble_gatts_create_service(gatts_if, &service_id, 20);

    // Add advertising configuration here
    esp_ble_gap_set_device_name("TOWER");
    esp_ble_gap_config_adv_data(&adv_data);
    break;

  case ESP_GATTS_CREATE_EVT:
    Serial.println("[BLE] Service created");
    g_service_handle = param->create.service_handle;
    esp_ble_gatts_start_service(g_service_handle);

    // Start adding characteristics
    g_current_char_idx = 0;
    add_next_characteristic();
    break;

  case ESP_GATTS_START_EVT:
    Serial.println("[BLE] Service started");
    break;

  case ESP_GATTS_ADD_CHAR_EVT:
    if (param->add_char.status == ESP_GATT_OK) {
      g_char_handles[g_current_char_idx] = param->add_char.attr_handle;
      g_char_val_handles[g_current_char_idx] = param->add_char.attr_handle;

      Serial.printf("[BLE] Characteristic 0x%04X added (handle=%d)\n",
                    char_uuids[g_current_char_idx],
                    param->add_char.attr_handle);

      // Move to next characteristic
      g_current_char_idx++;
      add_next_characteristic();
    } else {
      Serial.printf("[BLE] Failed to add characteristic: %d\n",
                    param->add_char.status);
    }
    break;

  case ESP_GATTS_CONNECT_EVT:
    Serial.printf("[BLE] Client connected (conn_id=%d)\n",
                  param->connect.conn_id);
    g_conn_id = param->connect.conn_id;
    g_connected = true;
    g_connection_count++;

    // Update connection parameters
    memset(&conn_params, 0, sizeof(conn_params));
    memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
    conn_params.latency = 0;
    conn_params.max_int = 0x30;
    conn_params.min_int = 0x18;
    conn_params.timeout = 400;
    esp_ble_gap_update_conn_params(&conn_params);
    break;

  case ESP_GATTS_DISCONNECT_EVT:
    Serial.println("[BLE] Client disconnected");
    g_connected = false;
    g_connection_count--;
    g_conn_id = 0;

    // Restart advertising
    delay(100);
    esp_ble_gap_start_advertising(&adv_params);
    break;

  case ESP_GATTS_WRITE_EVT:
    if (!param->write.is_prep) {
      handle = param->write.handle;
      data = param->write.value;
      len = param->write.len;

      if (len > 0) {
        // Find which characteristic was written
        for (i = 0; i < 6; i++) {
          if (handle == g_char_val_handles[i]) {
            uint8_t value = data[0];
            g_values[i] = value;

            switch (i) {
            case CHAR_IDX_LIGHT:
              Serial.printf("[BLE] WRITE Light: %s\n", value ? "ON" : "OFF");
              if (g_light_cb)
                g_light_cb(value != 0);
              break;
            case CHAR_IDX_FAN:
              Serial.printf("[BLE] WRITE Fan: %d%%\n", value);
              if (g_fan_cb)
                g_fan_cb(value);
              break;
            case CHAR_IDX_FAN_MIN:
              Serial.printf("[BLE] WRITE FanMin: %d%%\n", value);
              if (g_config_cbs[0])
                g_config_cbs[0](value);
              break;
            case CHAR_IDX_FAN_MAX:
              Serial.printf("[BLE] WRITE FanMax: %d%%\n", value);
              if (g_config_cbs[1])
                g_config_cbs[1](value);
              break;
            case CHAR_IDX_LIGHT_ON:
              Serial.printf("[BLE] WRITE LightOn: %d:00\n", value);
              if (g_config_cbs[2])
                g_config_cbs[2](value);
              break;
            case CHAR_IDX_LIGHT_OFF:
              Serial.printf("[BLE] WRITE LightOff: %d:00\n", value);
              if (g_config_cbs[3])
                g_config_cbs[3](value);
              break;
            }
            break;
          }
        }
      }

      if (param->write.need_rsp) {
        esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                                    param->write.trans_id, ESP_GATT_OK, NULL);
      }
    }
    break;

  case ESP_GATTS_READ_EVT:
    // Values are read directly from attribute table
    break;

  default:
    break;
  }
}

// Initialisierung
void growtower_ble_init(LightCallback light_cb, FanCallback fan_cb,
                        ConfigCallback fan_min_cb, ConfigCallback fan_max_cb,
                        ConfigCallback light_on_cb, ConfigCallback light_off_cb,
                        bool initial_light_on, int initial_fan_speed,
                        int initial_fan_min, int initial_fan_max,
                        int initial_light_on_hour, int initial_light_off_hour) {

  // Callbacks speichern
  g_light_cb = light_cb;
  g_fan_cb = fan_cb;
  g_config_cbs[0] = fan_min_cb;
  g_config_cbs[1] = fan_max_cb;
  g_config_cbs[2] = light_on_cb;
  g_config_cbs[3] = light_off_cb;

  // Initialwerte setzen
  g_values[CHAR_IDX_LIGHT] = initial_light_on ? 1 : 0;
  g_values[CHAR_IDX_FAN] = (uint8_t)initial_fan_speed;
  g_values[CHAR_IDX_FAN_MIN] = (uint8_t)initial_fan_min;
  g_values[CHAR_IDX_FAN_MAX] = (uint8_t)initial_fan_max;
  g_values[CHAR_IDX_LIGHT_ON] = (uint8_t)initial_light_on_hour;
  g_values[CHAR_IDX_LIGHT_OFF] = (uint8_t)initial_light_off_hour;

  Serial.println("[BLE] Initializing ESP32 Bluetooth...");

  // Bluetooth Controller initialisieren
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_err_t ret = esp_bt_controller_init(&bt_cfg);
  if (ret != ESP_OK) {
    Serial.printf("[BLE] Controller init failed: %d\n", ret);
    return;
  }

  // Bluetooth Controller aktivieren (BLE Mode)
  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret != ESP_OK) {
    Serial.printf("[BLE] Controller enable failed: %d\n", ret);
    return;
  }

  // Bluedroid Stack initialisieren
  ret = esp_bluedroid_init();
  if (ret != ESP_OK) {
    Serial.printf("[BLE] Bluedroid init failed: %d\n", ret);
    return;
  }

  // Bluedroid aktivieren
  ret = esp_bluedroid_enable();
  if (ret != ESP_OK) {
    Serial.printf("[BLE] Bluedroid enable failed: %d\n", ret);
    return;
  }

  // GAP Event Handler registrieren
  esp_ble_gap_register_callback(gap_event_handler);

  // Set device name early
  esp_ble_gap_set_device_name("TOWER");

  // GATT Server Event Handler registrieren
  esp_ble_gatts_register_callback(gatts_event_handler);

  // GATT App registrieren (Profile ID 0)
  esp_ble_gatts_app_register(0);

  // TX Power auf Maximum setzen
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);

  Serial.println("[BLE] Initialization complete");
}

// Update Funktionen
void growtower_ble_update_light(bool on) {
  g_values[CHAR_IDX_LIGHT] = on ? 1 : 0;
  if (g_char_val_handles[CHAR_IDX_LIGHT] != 0) {
    esp_ble_gatts_set_attr_value(g_char_val_handles[CHAR_IDX_LIGHT], 1,
                                 &g_values[CHAR_IDX_LIGHT]);
  }
}

void growtower_ble_update_fan(int speed) {
  g_values[CHAR_IDX_FAN] = (uint8_t)speed;
  if (g_char_val_handles[CHAR_IDX_FAN] != 0) {
    esp_ble_gatts_set_attr_value(g_char_val_handles[CHAR_IDX_FAN], 1,
                                 &g_values[CHAR_IDX_FAN]);
  }
}

void growtower_ble_update_fan_min(int min) {
  g_values[CHAR_IDX_FAN_MIN] = (uint8_t)min;
  if (g_char_val_handles[CHAR_IDX_FAN_MIN] != 0) {
    esp_ble_gatts_set_attr_value(g_char_val_handles[CHAR_IDX_FAN_MIN], 1,
                                 &g_values[CHAR_IDX_FAN_MIN]);
  }
}

void growtower_ble_update_fan_max(int max_val) {
  g_values[CHAR_IDX_FAN_MAX] = (uint8_t)max_val;
  if (g_char_val_handles[CHAR_IDX_FAN_MAX] != 0) {
    esp_ble_gatts_set_attr_value(g_char_val_handles[CHAR_IDX_FAN_MAX], 1,
                                 &g_values[CHAR_IDX_FAN_MAX]);
  }
}

void growtower_ble_update_light_on_hour(int hour) {
  g_values[CHAR_IDX_LIGHT_ON] = (uint8_t)hour;
  if (g_char_val_handles[CHAR_IDX_LIGHT_ON] != 0) {
    esp_ble_gatts_set_attr_value(g_char_val_handles[CHAR_IDX_LIGHT_ON], 1,
                                 &g_values[CHAR_IDX_LIGHT_ON]);
  }
}

void growtower_ble_update_light_off_hour(int hour) {
  g_values[CHAR_IDX_LIGHT_OFF] = (uint8_t)hour;
  if (g_char_val_handles[CHAR_IDX_LIGHT_OFF] != 0) {
    esp_ble_gatts_set_attr_value(g_char_val_handles[CHAR_IDX_LIGHT_OFF], 1,
                                 &g_values[CHAR_IDX_LIGHT_OFF]);
  }
}

// Status Funktionen
bool growtower_ble_is_connected(void) { return g_connected; }

int growtower_ble_get_connected_count(void) { return g_connection_count; }

void growtower_ble_restart_advertising(void) {
  if (!g_connected) {
    Serial.println("[BLE] Restarting advertising...");
    esp_ble_gap_start_advertising(&adv_params);
  }
}