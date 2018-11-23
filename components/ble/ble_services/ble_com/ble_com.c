/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "ble_com.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "sdk_common.h"
#include "app_error.h"
#include "HC_data_flash.h"
#include "HC_factory_test.h"
#include "HC_ads129x_driver.h"
#include "protocol_analysis.h"

extern bool Global_connected_state;

#define BLE_UUID_COM_Up_CHARACTERISTIC     0xFF22                      /**< The UUID of the Up Characteristic. */
#define BLE_UUID_COM_Down_CHARACTERISTIC   0xFF21                      /**< The UUID of the Down Characteristic. */

#define COM_BASE_UUID                  {{0x23, 0xD1, 0xBC, 0xEA, 0x6F, 0x78, 0x23, 0x15, 0xCD, 0xAB, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}} /**< Used vendor specific UUID. */

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_com     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_com_t * p_com, ble_evt_t * p_ble_evt)
{
    p_com->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_com     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_com_t * p_com, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_com->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S110 SoftDevice.
 *
 * @param[in] p_com     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_com_t * p_com, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
	  if ((p_evt_write->handle == p_com->Up_handles.cccd_handle)
        &&(p_evt_write->len == 2) && Global_connected_state)        //连接成功才能进行指令的交互 
				{
						if (ble_srv_is_notification_enabled(p_evt_write->data))
						{
								p_com->is_com_notification_enabled = true;
						}
						else
						{
								p_com->is_com_notification_enabled = false;
						}
				}
		if ((p_evt_write->handle == p_com->Down_handles.value_handle ) && Global_connected_state) 
		{
				App_Nap_data_Analysis(p_evt_write->data);	  
		} 
}


void ble_com_on_ble_evt(ble_com_t * p_com, ble_evt_t * p_ble_evt)
{
    if ((p_com == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_com, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_com, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_com, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}
/**@brief Function for adding Up characteristic.
 *
 * @param[in] p_com       Nordic UART Service structure.
 * @param[in] p_com_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t Up_char_add(ble_com_t * p_com, const ble_com_init_t * p_com_init)
{
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    memset(&char_md, 0, sizeof(char_md));
		
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_com->uuid_type;
    ble_uuid.uuid = BLE_UUID_COM_Up_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = 20;

    return sd_ble_gatts_characteristic_add(p_com->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_com->Up_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}


/**@brief Function for adding Down characteristic.
 *
 * @param[in] p_com       Nordic UART Service structure.
 * @param[in] p_com_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t Down_char_add(ble_com_t * p_com, const ble_com_init_t * p_com_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));
	
    char_md.char_props.write         = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_com->uuid_type;
    ble_uuid.uuid = BLE_UUID_COM_Down_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = 20;

    return sd_ble_gatts_characteristic_add(p_com->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_com->Down_handles);
}

uint32_t ble_com_init(ble_com_t * p_com, const ble_com_init_t * p_com_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t com_base_uuid = COM_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_com);
    VERIFY_PARAM_NOT_NULL(p_com_init);

    // Initialize the service structure.
    p_com->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_com->data_handler            = p_com_init->data_handler;
    p_com->is_com_notification_enabled = false;

    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&com_base_uuid, &p_com->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_com->uuid_type;
    ble_uuid.uuid = BLE_UUID_COM_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_com->service_handle);
    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    VERIFY_SUCCESS(err_code);

    // Add the Up Characteristic.
    err_code = Up_char_add(p_com, p_com_init);
    VERIFY_SUCCESS(err_code);

    // Add the Down Characteristic.
    err_code = Down_char_add(p_com, p_com_init);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}

uint32_t ble_com_string_send(ble_com_t * p_com, uint8_t * p_string, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;
    VERIFY_PARAM_NOT_NULL(p_com);

    if ((p_com->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_com->is_com_notification_enabled))
    {
			return NRF_ERROR_INVALID_STATE;
		}
		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_com->Up_handles.value_handle;
		hvx_params.p_data = p_string;
		hvx_params.p_len  = &length;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

		return sd_ble_gatts_hvx(p_com->conn_handle, &hvx_params);
}


