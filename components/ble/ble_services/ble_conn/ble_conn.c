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

#include "ble_conn.h"
#include "ble_srv_common.h"
#include "sdk_common.h"
#include "HC_command_analysis.h"
#include "pstorage.h"
#include "pstorage_platform.h"
#include "SEGGER_RTT_Conf.h"
#include "SEGGER_RTT.h"
#include "nrf_delay.h"

#define BLE_UUID_ID_down_CHARACTERISTIC         0xFF11                      /**< The UUID of the Down Characteristic. */
#define BLE_UUID_Shakehands_CHARACTERISTIC      0xFF12                      /**< The UUID of the Down Characteristic. */
#define BLE_UUID_State_up_CHARACTERISTIC        0xFF13                      /**< The UUID of the Up Characteristic. */

#define CON_BASE_UUID                  {{0x23, 0xD1, 0xBC, 0xEA, 0x6F, 0x78, 0x23, 0x15, 0xCD, 0xAB, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}} /**< Used vendor specific UUID. */

extern bool ID_is_receive;
extern uint8_t communocate_state[5];
extern ble_conn_t                         m_conn;                                      /**< Structure to identify the Nordic UART Service. */
extern uint16_t                           m_conn_handle;                               /**< Handle of the current connection. */

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_com     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_conn_t * p_conn, ble_evt_t * p_ble_evt)
{
    p_conn->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_com     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_conn_t * p_conn, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_conn->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S110 SoftDevice.
 *
 * @param[in] p_com     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_conn_t * p_conn, ble_evt_t * p_ble_evt)
{
		uint32_t err_code;
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if ((p_evt_write->handle == p_conn->Shakehands_handles.cccd_handle)
        &&(p_evt_write->len == 2))  
				{
						if (ble_srv_is_notification_enabled(p_evt_write->data))
						{
								p_conn->is_Shakehands_notification_enabled = true;
						}
						else
						{
								p_conn->is_Shakehands_notification_enabled = false;
						}
				}
		
    if ((p_evt_write->handle == p_conn->State_up_handles.cccd_handle)
        &&(p_evt_write->len == 2))   
				{
						if (ble_srv_is_notification_enabled(p_evt_write->data))
						{
								p_conn->is_state_notification_enabled = true;
						}
						else
						{
								p_conn->is_state_notification_enabled = false;
						}
				}
	
	  if (p_evt_write->handle == p_conn->ID_down_handles.value_handle)  
    {
				ble_Com_ID_Analysis(p_evt_write->data, p_evt_write->len);    //收到ID信息
		}
		
		if (p_evt_write->handle == p_conn->Shakehands_handles.value_handle)  
    {
			if( *p_evt_write->data == 0x01 || *p_evt_write->data == 0x03)
		  {
			   if(ID_is_receive)
			   {
			      ble_Com_Shakehands_Analysis(p_evt_write->data, p_evt_write->len);  //收到握手信息
			   }
      }
			else
			{
				 if(RTT_PRINT)
				 {
							SEGGER_RTT_printf(0,"0x04 \r\n");
				 }					 
		     communocate_state[4] = 0x04;                           //数据错误
		     ble_State_string_send(&m_conn,communocate_state,5);
				 nrf_delay_ms(500);		 
				if(m_conn_handle != BLE_CONN_HANDLE_INVALID)
				{
							err_code = sd_ble_gap_disconnect(m_conn_handle,
																							 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
				    	APP_ERROR_CHECK(err_code);
				}
			}
	 }
}

void ble_conn_on_ble_evt(ble_conn_t * p_conn, ble_evt_t * p_ble_evt)
{
    if ((p_conn == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_conn, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_conn, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_conn, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for adding Down characteristic.
 *
 * @param[in] p_com       Nordic UART Service structure.
 * @param[in] p_com_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t ID_down_char_add(ble_conn_t * p_conn, const ble_conn_init_t * p_conn_init)
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

    ble_uuid.type = p_conn->uuid_type;
    ble_uuid.uuid = BLE_UUID_ID_down_CHARACTERISTIC;

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

    return sd_ble_gatts_characteristic_add(p_conn->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_conn->ID_down_handles);
}

/**@brief Function for adding Down characteristic.
 *
 * @param[in] p_com       Nordic UART Service structure.
 * @param[in] p_com_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t Shakehands_char_add(ble_conn_t * p_conn, const ble_conn_init_t * p_conn_init)
{
	  ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_md_t cccd_md;

  	cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    memset(&char_md, 0, sizeof(char_md));

	  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
	
    char_md.char_props.write         = 1;
    char_md.char_props.notify        = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = &cccd_md;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_conn->uuid_type;
    ble_uuid.uuid = BLE_UUID_Shakehands_CHARACTERISTIC;

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

    return sd_ble_gatts_characteristic_add(p_conn->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_conn->Shakehands_handles);
}

/**@brief Function for adding Up characteristic.
 *
 * @param[in] p_com       Nordic UART Service structure.
 * @param[in] p_com_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t State_up_char_add(ble_conn_t * p_conn, const ble_conn_init_t * p_conn_init)
{
    /**@snippet [Adding proprietary characteristic to S132 SoftDevice] */
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

    ble_uuid.type = p_conn->uuid_type;
    ble_uuid.uuid = BLE_UUID_State_up_CHARACTERISTIC;

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

    return sd_ble_gatts_characteristic_add(p_conn->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_conn->State_up_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}

uint32_t ble_conn_init(ble_conn_t * p_conn, const ble_conn_init_t * p_conn_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t conn_base_uuid = CON_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_conn);
    VERIFY_PARAM_NOT_NULL(p_conn_init);

    // Initialize the service structure.
    p_conn->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_conn->data_handler            = p_conn_init->data_handler;
    p_conn->is_Shakehands_notification_enabled = false;
    p_conn->is_state_notification_enabled = false;

    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&conn_base_uuid, &p_conn->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_conn->uuid_type;
    ble_uuid.uuid = BLE_UUID_CON_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_conn->service_handle);
    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    VERIFY_SUCCESS(err_code);

    // Add the ID_down Characteristic.
    err_code = ID_down_char_add(p_conn, p_conn_init);
    VERIFY_SUCCESS(err_code);

    // Add the Shakehands Characteristic.
    err_code = Shakehands_char_add(p_conn, p_conn_init);
    VERIFY_SUCCESS(err_code);

    // Add the State_up Characteristic.
    err_code = State_up_char_add(p_conn, p_conn_init);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}

uint32_t ble_Shakehands_string_send(ble_conn_t * p_conn, uint8_t * p_string, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;
    VERIFY_PARAM_NOT_NULL(p_conn);

    if ((p_conn->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_conn->is_Shakehands_notification_enabled))
    {
			return NRF_ERROR_INVALID_STATE;
		}
		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_conn->Shakehands_handles.value_handle;
		hvx_params.p_data = p_string;
		hvx_params.p_len  = &length;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

		return sd_ble_gatts_hvx(p_conn->conn_handle, &hvx_params);
}

uint32_t ble_State_string_send(ble_conn_t * p_conn, uint8_t * p_string, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;
    VERIFY_PARAM_NOT_NULL(p_conn);

    if ((p_conn->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_conn->is_state_notification_enabled))
    {
			return NRF_ERROR_INVALID_STATE;
		}
		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_conn->State_up_handles.value_handle;
		hvx_params.p_data = p_string;
		hvx_params.p_len  = &length;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

		return sd_ble_gatts_hvx(p_conn->conn_handle, &hvx_params);
}

