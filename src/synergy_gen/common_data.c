/* generated common source file - do not edit */
#include "common_data.h"
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer2) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_SCI2_RXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_SCI2_RXI
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_0) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_0);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0
#endif
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_1) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_1);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1
#endif
#endif
#endif

dtc_instance_ctrl_t g_transfer2_ctrl;
transfer_info_t g_transfer2_info =
{ .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
  .repeat_area = TRANSFER_REPEAT_AREA_DESTINATION,
  .irq = TRANSFER_IRQ_END,
  .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
  .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
  .size = TRANSFER_SIZE_1_BYTE,
  .mode = TRANSFER_MODE_NORMAL,
  .p_dest = (void *) NULL,
  .p_src = (void const *) NULL,
  .num_blocks = 0,
  .length = 0, };
const transfer_cfg_t g_transfer2_cfg =
{ .p_info = &g_transfer2_info,
  .activation_source = ELC_EVENT_SCI2_RXI,
  .auto_enable = false,
  .p_callback = NULL,
  .p_context = &g_transfer2,
  .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer2 =
{ .p_ctrl = &g_transfer2_ctrl, .p_cfg = &g_transfer2_cfg, .p_api = &g_transfer_on_dtc };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer1) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_SCI2_TXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_SCI2_TXI
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_0) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_0);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0
#endif
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_1) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_1);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1
#endif
#endif
#endif

dtc_instance_ctrl_t g_transfer1_ctrl;
transfer_info_t g_transfer1_info =
{ .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
  .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
  .irq = TRANSFER_IRQ_END,
  .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
  .src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
  .size = TRANSFER_SIZE_1_BYTE,
  .mode = TRANSFER_MODE_NORMAL,
  .p_dest = (void *) NULL,
  .p_src = (void const *) NULL,
  .num_blocks = 0,
  .length = 0, };
const transfer_cfg_t g_transfer1_cfg =
{ .p_info = &g_transfer1_info,
  .activation_source = ELC_EVENT_SCI2_TXI,
  .auto_enable = false,
  .p_callback = NULL,
  .p_context = &g_transfer1,
  .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer1 =
{ .p_ctrl = &g_transfer1_ctrl, .p_cfg = &g_transfer1_cfg, .p_api = &g_transfer_on_dtc };
#if !defined(SSP_SUPPRESS_ISR_g_i2c0) && !defined(SSP_SUPPRESS_ISR_SCI2)
SSP_VECTOR_DEFINE_CHAN(sci_i2c_rxi_isr, SCI, RXI, 2);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_i2c0) && !defined(SSP_SUPPRESS_ISR_SCI2)
SSP_VECTOR_DEFINE_CHAN(sci_i2c_txi_isr, SCI, TXI, 2);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_i2c0) && !defined(SSP_SUPPRESS_ISR_SCI2)
SSP_VECTOR_DEFINE_CHAN(sci_i2c_tei_isr, SCI, TEI, 2);
#endif
sci_i2c_instance_ctrl_t g_i2c0_ctrl;

/** I2C extended configuration */
const sci_i2c_extended_cfg g_i2c0_cfg_extend =
{ .bitrate_modulation = true };

const i2c_cfg_t g_i2c0_cfg =
{ .channel = 2,
  .rate = I2C_RATE_STANDARD,
  .slave = 0,
  .addr_mode = I2C_ADDR_MODE_7BIT,
  .sda_delay = 300,
#define SYNERGY_NOT_DEFINED (1)            
#if (SYNERGY_NOT_DEFINED == g_transfer1)
  .p_transfer_tx = NULL,
#else
  .p_transfer_tx = &g_transfer1,
#endif
#if (SYNERGY_NOT_DEFINED == g_transfer2)
  .p_transfer_rx = NULL,
#else
  .p_transfer_rx = &g_transfer2,
#endif
#undef SYNERGY_NOT_DEFINED
  .p_callback = NULL,
  .p_context = (void *) &g_i2c0,
  .rxi_ipl = (3),
  .txi_ipl = (3),
  .tei_ipl = (3),
  .eri_ipl = BSP_IRQ_DISABLED,
  .p_extend = &g_i2c0_cfg_extend, };
/* Instance structure to use this module. */
const i2c_master_instance_t g_i2c0 =
{ .p_ctrl = &g_i2c0_ctrl, .p_cfg = &g_i2c0_cfg, .p_api = &g_i2c_master_on_sci };
static TX_MUTEX sf_bus_mutex_g_sf_i2c_bus0;
static TX_EVENT_FLAGS_GROUP sf_bus_eventflag_g_sf_i2c_bus0;
static sf_i2c_instance_ctrl_t *sf_curr_ctrl_g_sf_i2c_bus0;
static sf_i2c_instance_ctrl_t *sf_curr_bus_ctrl_g_sf_i2c_bus0;
sf_i2c_bus_t g_sf_i2c_bus0 =
{ .p_bus_name = (uint8_t *) "g_sf_i2c_bus0",
  .channel = 2,
  .p_lock_mutex = &sf_bus_mutex_g_sf_i2c_bus0,
  .p_sync_eventflag = &sf_bus_eventflag_g_sf_i2c_bus0,
  .pp_curr_ctrl = (sf_i2c_ctrl_t **) &sf_curr_ctrl_g_sf_i2c_bus0,
  .p_lower_lvl_api = (i2c_api_master_t *) &g_i2c_master_on_sci,
  .device_count = 0,
  .pp_curr_bus_ctrl = (sf_i2c_ctrl_t **) &sf_curr_bus_ctrl_g_sf_i2c_bus0, };
const cgc_clocks_cfg_t g_cgc_cfg0 =
{ .system_clock = CGC_CLOCK_PLL,
  .pll_cfg =
  { .source_clock = CGC_CLOCK_MAIN_OSC, .divider = CGC_PLL_DIV_2, .multiplier = (float) (8.0), },
  .sys_cfg =
  { .pclka_div = CGC_SYS_CLOCK_DIV_1, .pclkb_div = CGC_SYS_CLOCK_DIV_2, .pclkc_div = CGC_SYS_CLOCK_DIV_1, .pclkd_div =
            CGC_SYS_CLOCK_DIV_1,
    .bclk_div = CGC_SYS_CLOCK_DIV_2, .fclk_div = CGC_SYS_CLOCK_DIV_2, .iclk_div = CGC_SYS_CLOCK_DIV_1, },
  .loco_state = CGC_CLOCK_CHANGE_NONE,
  .moco_state = CGC_CLOCK_CHANGE_NONE,
  .hoco_state = CGC_CLOCK_CHANGE_NONE,
  .subosc_state = CGC_CLOCK_CHANGE_NONE,
  .mainosc_state = CGC_CLOCK_CHANGE_NONE,
  .pll_state = CGC_CLOCK_CHANGE_NONE,

};
/************************************************************/
/** Begin PPM V2 Common Configurations **********************/
/************************************************************/
sf_power_profiles_v2_ctrl_t g_sf_power_profiles_v2_common_ctrl;
sf_power_profiles_v2_cfg_t g_sf_power_profiles_v2_common_cfg =
{ .p_extend = NULL, };
sf_power_profiles_v2_instance_t g_sf_power_profiles_v2_common =
{ .p_ctrl = &g_sf_power_profiles_v2_common_ctrl, .p_cfg = &g_sf_power_profiles_v2_common_cfg, .p_api =
          &g_sf_power_profiles_v2_on_sf_power_profiles_v2, };
/************************************************************/
/** End PPM V2 Common Configurations ************************/
/************************************************************/

/*******************************************************************************************************************//**
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.

 *            - void sf_power_profiles_v2_init0(void)
 **********************************************************************************************************************/
void sf_power_profiles_v2_init0(void)
{
    ssp_err_t err;
    err = g_sf_power_profiles_v2_common.p_api->open (g_sf_power_profiles_v2_common.p_ctrl,
                                                     g_sf_power_profiles_v2_common.p_cfg);
    if (SSP_SUCCESS != err)
    {
        BSP_CFG_HANDLE_UNRECOVERABLE_ERROR (0);
    }
}
/************************************************************/
/** Begin LPM V2 Configurations *****************************/
/************************************************************/
#if defined(BSP_MCU_GROUP_S3A7)
const lpmv2_mcu_cfg_t g_lpmv2_standby0_mcu_cfg =
{
    .standby_wake_sources = (lpmv2_standby_wake_source_bits_t)LPMV2_STANDBY_WAKE_SOURCE_IRQ0 |
    0 |
    0 |
    0 |
    0 |
    0 |
    0 |
    0 |
    0 |
    LPMV2_STANDBY_WAKE_SOURCE_IRQ9 |
    0 |
    0 |
    0 |
    LPMV2_STANDBY_WAKE_SOURCE_IRQ13 |
    0 |
    0 |
    0 |
    0 |
    0 |
    0 |
    0 |
    0 |
    LPMV2_STANDBY_WAKE_SOURCE_RTCALM |
    0 |
    0 |
    0 |
    0 |
    0 |
    0,
    .snooze_request_source = LPMV2_SNOOZE_REQUEST_RXD0_FALLING,
    .snooze_end_sources = (lpmv2_snooze_end_bits_t)0 |
    0 |
    0 |
    0 |
    0 |
    0,
    .dtc_state_in_snooze = LPMV2_SNOOZE_DTC_DISABLE,
    .output_port_enable = LPMV2_OUTPUT_PORT_ENABLE_RETAIN,
    .snooze_cancel_sources = LPMV2_SNOOZE_CANCEL_SOURCE_SCI0_RXI_OR_ERI,
};
const lpmv2_cfg_t g_lpmv2_standby0_cfg =
{
    .low_power_mode = LPMV2_LOW_POWER_MODE_STANDBY,
    .p_extend = &g_lpmv2_standby0_mcu_cfg,
};
const lpmv2_instance_t g_lpmv2_standby0 =
{
    .p_api = &g_lpmv2_on_lpmv2,
    .p_cfg = &g_lpmv2_standby0_cfg
};
#endif
/************************************************************/
/** End LPM V2 Configurations *******************************/
/************************************************************/
const elc_instance_t g_elc =
{ .p_api = &g_elc_on_elc, .p_cfg = NULL };
/* Instance structure to use this module. */
const fmi_instance_t g_fmi =
{ .p_api = &g_fmi_on_fmi };
const ioport_instance_t g_ioport =
{ .p_api = &g_ioport_on_ioport, .p_cfg = NULL };
const cgc_instance_t g_cgc =
{ .p_api = &g_cgc_on_cgc, .p_cfg = NULL };
void g_common_init(void)
{
    /** Initialize PPM V2 Common */
    /** Call initialization function if user has selected to do so. */
#if (1)
    sf_power_profiles_v2_init0 ();
#endif
}
