#include "ble_init.h"

#include <stdint.h>

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"

#define BLE_CONN_CFG_TAG            1                                               /**< A tag identifying the SoftDevice BLE configuration. */
#define BLE_OBSERVER_PRIO           3  

#define GAP_DEFAULT_CONN_INTERVAL	MSEC_TO_UNITS(15, UNIT_1_25_MS)
#define GAP_MIN_CONN_INTERVAL       MSEC_TO_UNITS(15, UNIT_1_25_MS)              /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define GAP_MAX_CONN_INTERVAL       MSEC_TO_UNITS(15, UNIT_1_25_MS)             		/**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define GAP_SLAVE_LATENCY           0                                           		/**< Slave latency. */
#define GAP_CONN_SUP_TIMEOUT        MSEC_TO_UNITS(4000, UNIT_10_MS)             		/**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */

#define BLE_GAP_DATA_LENGTH_DEFAULT     27          //!< The stack's default data length.
#define BLE_GAP_DATA_LENGTH_MAX         251         //!< Maximum data length.

#define BLE_ADV_INTERVAL                		64                                          		/**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define BLE_ADV_DURATION                		6000                                       		/**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                               /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                              /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3

#define BLE_MORETHINGS_OBSERVER_PRIO     3

#define OPCODE_LENGTH 1 /**< Length of opcode inside a notification. */
#define HANDLE_LENGTH 2 /**< Length of handle inside a notification. */

static void init_ble_gap(void);
static void init_ble_gatt(void);
static void init_ble_service(void);

static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context);

void init_ble(void)
{
    ret_code_t err_code;

    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    ble_cfg_t ble_cfg;
    // Configure the GATTS attribute table.
    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count = NRF_SDH_BLE_PERIPHERAL_LINK_COUNT;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count = NRF_SDH_BLE_CENTRAL_LINK_COUNT;
    // ble_cfg.gap_cfg.role_count_cfg.qos_channel_survey_role_available = true; /* Enable channel survey role */

    err_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, &ram_start);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("sd_ble_cfg_set() returned %s when attempting to set BLE_GAP_CFG_ROLE_COUNT.",
                        nrf_strerror_get(err_code));
    }

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Set the Power mode to Low power mode
    err_code = sd_power_mode_set(NRF_POWER_MODE_CONSTLAT);//(NRF_POWER_MODE_LOWPWR);
    APP_ERROR_CHECK(err_code);

    // Enaable the DCDC Power Mode
    err_code = sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, BLE_OBSERVER_PRIO, ble_evt_handler, NULL);

    ret_code_t err_code = radio_notification_init(BLE_OBSERVER_PRIO, NRF_RADIO_NOTIFICATION_TYPE_INT_ON_BOTH, NRF_RADIO_NOTIFICATION_DISTANCE_800US);
    APP_ERROR_CHECK(err_code);
}

static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    
}

volatile bool flag = false;
void SWI1_IRQHandler(bool radio_evt)
{
    flag = !flag;
    if (flag)
    {

    }
    else
    {

    }
}

static void init_ble_gap(void)
{
	ble_gap_conn_sec_mode_t sec_mode;
	
  	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
	
	ble_gap_addr_t macAddress;
	uint32_t err_code = sd_ble_gap_addr_get(&macAddress);
	APP_ERROR_CHECK(err_code);
	
	char temp_name[50];
	sprintf(temp_name, "moreMat_%02X%02X%02X", macAddress.addr[2], macAddress.addr[1], macAddress.addr[0]);
	uint32_t temp_length = strlen(temp_name); 	
	
	err_code = sd_ble_gap_device_name_set(&sec_mode,(const uint8_t *)temp_name, temp_length);	
	APP_ERROR_CHECK(err_code);

	ble_gap_conn_params_t   gap_conn_params;
	memset(&gap_conn_params, 0, sizeof(gap_conn_params)); 

	gap_conn_params.min_conn_interval = GAP_MIN_CONN_INTERVAL;//GAP_DEFAULT_CONN_INTERVAL;
	gap_conn_params.max_conn_interval = GAP_MAX_CONN_INTERVAL;//GAP_DEFAULT_CONN_INTERVAL;
	gap_conn_params.slave_latency     = GAP_SLAVE_LATENCY;
	gap_conn_params.conn_sup_timeout  = GAP_CONN_SUP_TIMEOUT;

	err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
	APP_ERROR_CHECK(err_code);
}

static void init_ble_gatt(void)
{
    
}

static void init_ble_service(void)
{

}

uint32_t radio_notification_init(uint32_t irq_priority, uint8_t notification_type, uint8_t notification_distance)
{
    uint32_t err_code;

    err_code = sd_nvic_ClearPendingIRQ(SWI1_IRQn);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = sd_nvic_SetPriority(SWI1_IRQn, irq_priority);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = sd_nvic_EnableIRQ(SWI1_IRQn);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Configure the event
    return sd_radio_notification_cfg_set(notification_type, notification_distance);
}