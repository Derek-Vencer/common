# NTT BES1306P Firmware - Source-Verified Flowcharts

**Repository:** `Derek-Vencer/common`  
**Commit:** `c326e79351cbe6ce49b9b6e7ddcca80a75a2b020`  
**Scope:** Verified source paths and functions below. These diagrams are **not** a complete SDK call graph. Solid arrows indicate source-observed control flow; high-level module arrows are marked as logical relationships.

## 01 - Verified module relationships

```mermaid
flowchart LR
    CASE["Charging Case / 1-Wire"] --> UART["singleWiredUart.cpp"]
    UART --> UI["app_ui_set_local_box_state / app_ui_sync_box_state"]
    UART --> PMU["pmu_ntt_case_closed_latch_set"]
    UART --> BAT["wired_uart_get_battery_level"]
    BLE["ble_sparrow_server.cpp"] --> EQ["ntt_keymap_apply_eq_preset"]
    BLE --> BLOCK["ntt_key_role_switch_block_start"]
    BLOCK --> RSSI["app_rssi.cpp"]
    SYNC["app_ibrt_customif_cmd.cpp"] --> PEER["TWS peer settings sync"]
    BUILD["build_pro_1306p_0015.sh"] --> TARGET["best1306p target"]
```

## 02 - 1-Wire receive and validation

**Source:** `services/aiwang_services/singleWiredUart.cpp`, lines 1598-1665, 2479.

```mermaid
flowchart TD
    A["UART data received"] --> B["wired_uart_communication_cmd_handle_process"]
    B --> C{"Length >= 4 and header 55 AA?"}
    C -->|No| X["Return"]
    C -->|Yes| D["Read cmd_event = data[2]"]
    D --> E{"CRC8 valid?"}
    E -->|No| X
    E -->|Yes| F["Reset / initialize UART idle timer"]
    F --> G{"factory_mode and cmd not ship-mode?"}
    G -->|Yes| X
    G -->|No| H["switch(cmd_event)"]
    H --> I["CMD_POWER_OFF 0x06"]
    H --> J["CMD_OPEN_CASE 0x07"]
    H --> K["Other supported commands"]
```

## 03 - Case-close power-off (0x06)

**Source:** `services/aiwang_services/singleWiredUart.cpp`, lines 1831-1912. 300/3000 ms branches are documented in source comments; timer implementation must be inspected separately for exact runtime branching.

```mermaid
flowchart TD
    A["CMD_POWER_OFF 0x06"] --> B{"g_ntt_case_poweroff_received?"}
    B -->|Yes| Z["Ignore duplicate"]
    B -->|No| C["Set g_ntt_case_poweroff_received = true"]
    C --> D["pmu_ntt_case_closed_latch_set(true)"]
    D --> E["Set case closed and disable open reconnect authorization"]
    E --> F["app_ui_set_local_box_state(IN_BOX_CLOSED)"]
    F --> G["app_ui_sync_box_state(IN_BOX_CLOSED)"]
    G --> H["ntt_case_poweroff_enable()"]
    H --> I["wired_uart_get_battery_level()"]
    I --> J{"already_pending?"}
    J -->|Yes| K["Keep existing timer"]
    J -->|No| L["ntt_case_poweroff_start_timer()"]
    L --> M["Source comment: both in case 300 ms; otherwise 3000 ms"]
```

## 04 - Case open (0x07) latch release

**Source:** `services/aiwang_services/singleWiredUart.cpp`, lines 1913-1940.

```mermaid
flowchart TD
    A["CMD_OPEN_CASE 0x07"] --> B["g_ntt_case_poweroff_received = false"]
    B --> C["pmu_ntt_case_closed_latch_set(false)"]
    C --> D{"s_ntt_pair_case_open_latched?"}
    D -->|No| E["Accept CLOSED to OPEN edge; set latch"]
    D -->|Yes| F["Do not rearm pairing on repeated 0x07"]
```

## 05 - Key event and RSSI auto-role-switch block

**Sources:** `services/aiwang_services/ble_sparrow_server.cpp`, lines 3490-3525; `apps/btapp/app_rssi/app_rssi.cpp`, lines 120-146.

```mermaid
flowchart TD
    A["Key event"] --> B["Determine local / peer origin"]
    B --> C{"Origin earbud IN_CASE?"}
    C -->|Yes| X["Ignore key event"]
    C -->|No| D["ntt_key_role_switch_block_start()"]
    D --> E["Set block_until = now + 30000 ms"]
    E --> F["Reset RSSI switch confirmation count"]
    F --> G["Continue key event handling"]
    H["ntt_key_role_switch_block_cancel()"] --> I["Clear block flag and confirmation count"]
    J["ntt_key_role_switch_is_blocked()"] --> K{"30 seconds elapsed?"}
    K -->|No| L["Blocked"]
    K -->|Yes| M["Clear block; allow RSSI evaluation"]
```

## 06 - Peer RSSI response and evaluation

**Source:** `apps/btapp/app_rssi/app_rssi.cpp`, lines 855-927.

```mermaid
flowchart TD
    A["app_rssi_get_peer_rssi_rsp_handle"] --> B{"UI role == TWS_UI_MASTER?"}
    B -->|No| X["Reset confirmation count; return"]
    B -->|Yes| C["Find valid mobile ACL and matching peer address"]
    C --> D{"Master RSSI sample exists?"}
    D -->|No| Z["Skip entry"]
    D -->|Yes| E["Read master local RSSI and peer snoop RSSI"]
    E --> F["ntt_rssi_role_switch_check(master, slave)"]
```

## 07 - BLE EQ preset command

**Source:** `services/aiwang_services/ble_sparrow_server.cpp`, lines 2053-2092 and 2629-2640.

```mermaid
flowchart TD
    A["SET_EQ_PRESET"] --> B["handleSetEqPresent(data, len)"]
    B --> C{"data != NULL and len >= 4?"}
    C -->|No| X["Send API invalid parameter error"]
    C -->|Yes| D{"data_len == 1?"}
    D -->|No| X
    D -->|Yes| E{"ntt_eq_preset_is_valid(presetId)?"}
    E -->|No| X
    E -->|Yes| F["Clear EQ6 active flag and previous Clear Voice preset"]
    F --> G["ntt_keymap_apply_eq_preset(presetId)"]
    G --> H["Optional response notification"]
```

## 08 - Master sync of user settings

**Source:** `apps/earbuds/conn/app_ibrt_customif_cmd.cpp`, lines 372-429.

```mermaid
flowchart TD
    A["ntt_master_sync_all_user_settings_to_peer()"] --> B{"TWS link connected?"}
    B -->|No| X["Skip sync"]
    B -->|Yes| C["ntt_master_sync_eq_to_peer()"]
    C --> D["ntt_master_sync_keymap_to_peer()"]
    D --> E["ntt_master_sync_bt_name_to_peer()"]
    C --> F["Read NV eq_index_data"]
    F --> G{"eq_index > 5?"}
    G -->|Yes| H["Force EQ index 0; save NV"]
    G -->|No| I["Keep EQ index"]
    H --> J["app_ibrt_customif_cmd_sync_music_eq"]
    I --> J
    J --> K["app_ibrt_customif_cmd_sync_eq6_mode(active, index)"]
```

## 09 - Build configuration

**Source:** `tools/com_sdk/cust/build_pro_1306p_0015.sh`, lines 1-38; `config/best1306p/target.mk`, lines 1-35.

```mermaid
flowchart TD
    A["build_pro_1306p_0015.sh command"] --> B{"Command?"}
    B -->|clean| C["make T=best1306p clean"]
    B -->|gen_lib| D["make GEN_LIB=1 then ota_copy"]
    B -->|lst| E["make lst all"]
    B -->|default| F["make best1306p then ota_copy"]
    G["Build configuration"] --> H["BES_OTA=1 / OTA_BIN_COMPRESSED=1"]
    G --> I["BLE=1 / GFPS_ENABLE=1 / BLE_BATT=1"]
    G --> J["CHG_OUT_PWRON=1 / AUDIO_DRC=1 / AUDIO_LIMITER=0"]
```

## Verification and limitations

The diagrams above were derived from **specific source files at the exact commit**, not from a generic TWS design. They are a **verified partial architecture**. Full boot sequence, A2DP pipeline, OTA state machine, exact timer implementation and complete cross-file call graph require additional source tracing and are intentionally not represented as confirmed execution paths.

**GitHub source reference:** `https://github.com/Derek-Vencer/common/tree/c326e79351cbe6ce49b9b6e7ddcca80a75a2b020`
