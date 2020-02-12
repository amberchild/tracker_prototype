/* generated HAL source file - do not edit */
#include "hal_data.h"
#if (1) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_external_irq13) && !defined(SSP_SUPPRESS_ISR_ICU13)
SSP_VECTOR_DEFINE( icu_irq_isr, ICU, IRQ13);
#endif
#endif
static icu_instance_ctrl_t g_external_irq13_ctrl;
static const external_irq_cfg_t g_external_irq13_cfg =
{ .channel = 13,
  .trigger = EXTERNAL_IRQ_TRIG_RISING,
  .filter_enable = true,
  .pclk_div = EXTERNAL_IRQ_PCLK_DIV_BY_64,
  .autostart = true,
  .p_callback = detect_charger_callback,
  .p_context = &g_external_irq13,
  .p_extend = NULL,
  .irq_ipl = (1), };
/* Instance structure to use this module. */
const external_irq_instance_t g_external_irq13 =
{ .p_ctrl = &g_external_irq13_ctrl, .p_cfg = &g_external_irq13_cfg, .p_api = &g_external_irq_on_icu };
static crc_instance_ctrl_t g_crc_ctrl;
const crc_cfg_t g_crc_cfg =
{ .polynomial = CRC_POLYNOMIAL_CRC_32C, .bit_order = CRC_BIT_ORDER_LMS_MSB, .fifo_mode = false, };
/* Instance structure to use this module. */
const crc_instance_t g_crc =
{ .p_ctrl = &g_crc_ctrl, .p_cfg = &g_crc_cfg, .p_api = &g_crc_on_crc };
#if (0) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_flash_driver) && !defined(SSP_SUPPRESS_ISR_FLASH)
SSP_VECTOR_DEFINE(fcu_frdyi_isr, FCU, FRDYI);
#endif
#endif
flash_lp_instance_ctrl_t flash_driver_ctrl;
const flash_cfg_t flash_driver_cfg =
{ .data_flash_bgo = true, .p_callback = flash_CallBack, .p_context = &flash_driver, .irq_ipl = (0),

};
/* Instance structure to use this module. */
const flash_instance_t flash_driver =
{ .p_ctrl = &flash_driver_ctrl, .p_cfg = &flash_driver_cfg, .p_api = &g_flash_on_flash_lp };
#if (1) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_external_irq9) && !defined(SSP_SUPPRESS_ISR_ICU9)
SSP_VECTOR_DEFINE( icu_irq_isr, ICU, IRQ9);
#endif
#endif
static icu_instance_ctrl_t g_external_irq9_ctrl;
static const external_irq_cfg_t g_external_irq9_cfg =
{ .channel = 9,
  .trigger = EXTERNAL_IRQ_TRIG_RISING,
  .filter_enable = false,
  .pclk_div = EXTERNAL_IRQ_PCLK_DIV_BY_64,
  .autostart = true,
  .p_callback = accelerometer_callback,
  .p_context = &g_external_irq9,
  .p_extend = NULL,
  .irq_ipl = (1), };
/* Instance structure to use this module. */
const external_irq_instance_t g_external_irq9 =
{ .p_ctrl = &g_external_irq9_ctrl, .p_cfg = &g_external_irq9_cfg, .p_api = &g_external_irq_on_icu };
/************************************************************/
/** Begin PPM V2 Run Configurations *************************/
/************************************************************/
sf_power_profiles_v2_run_cfg_t g_sf_power_profiles_v2_run_0 =
{
#if POWER_PROFILES_V2_RUN_PIN_CFG_TBL_USED_g_sf_power_profiles_v2_run_0
  .p_ioport_pin_tbl = &g_bsp_pin_cfg,
#else
  .p_ioport_pin_tbl = NULL,
#endif
  .p_clock_cfg = &g_cgc_cfg0,
  .p_extend = NULL, };
/************************************************************/
/** End PPM V2 Run Configurations ***************************/
/************************************************************/
/************************************************************/
/** Begin PPM V2 Low Power Profile **************************/
/************************************************************/
#if POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_v2_low_power_0
void lpm_callback(sf_power_profiles_v2_callback_args_t * p_args);
#endif
sf_power_profiles_v2_low_power_cfg_t g_sf_power_profiles_v2_low_power_0 =
{
#if POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_v2_low_power_0
  .p_ioport_pin_tbl_exit = &g_bsp_pin_cfg,
#else
  .p_ioport_pin_tbl_exit = NULL,
#endif
#if POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_v2_low_power_0
  .p_ioport_pin_tbl_enter = &low_pwr_config,
#else
  .p_ioport_pin_tbl_enter = NULL,
#endif
  .p_lower_lvl_lpm = &g_lpmv2_standby0,
  .p_callback = lpm_callback, .p_context = &g_sf_power_profiles_v2_low_power_0, .p_extend = NULL, };
/************************************************************/
/** End PPM V2 Low Power Profile ****************************/
/************************************************************/
#if (1) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_external_irq0) && !defined(SSP_SUPPRESS_ISR_ICU0)
SSP_VECTOR_DEFINE( icu_irq_isr, ICU, IRQ0);
#endif
#endif
static icu_instance_ctrl_t g_external_irq0_ctrl;
static const external_irq_cfg_t g_external_irq0_cfg =
{ .channel = 0,
  .trigger = EXTERNAL_IRQ_TRIG_FALLING,
  .filter_enable = true,
  .pclk_div = EXTERNAL_IRQ_PCLK_DIV_BY_64,
  .autostart = true,
  .p_callback = button_callback,
  .p_context = &g_external_irq0,
  .p_extend = NULL,
  .irq_ipl = (1), };
/* Instance structure to use this module. */
const external_irq_instance_t g_external_irq0 =
{ .p_ctrl = &g_external_irq0_ctrl, .p_cfg = &g_external_irq0_cfg, .p_api = &g_external_irq_on_icu };
#if (1) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_modem_timer) && !defined(SSP_SUPPRESS_ISR_AGT0)
SSP_VECTOR_DEFINE_CHAN(agt_int_isr, AGT, INT, 0);
#endif
#endif
static agt_instance_ctrl_t g_modem_timer_ctrl;
static const timer_on_agt_cfg_t g_modem_timer_extend =
{ .count_source = AGT_CLOCK_PCLKB,
  .agto_output_enabled = false,
  .agtio_output_enabled = false,
  .output_inverted = false,
  .agtoa_output_enable = false,
  .agtob_output_enable = false, };
static const timer_cfg_t g_modem_timer_cfg =
{ .mode = TIMER_MODE_PERIODIC,
  .period = 10,
  .unit = TIMER_UNIT_PERIOD_MSEC,
  .channel = 0,
  .autostart = true,
  .p_callback = g_modem_timer_callback,
  .p_context = &g_modem_timer,
  .p_extend = &g_modem_timer_extend,
  .irq_ipl = (1), };
/* Instance structure to use this module. */
const timer_instance_t g_modem_timer =
{ .p_ctrl = &g_modem_timer_ctrl, .p_cfg = &g_modem_timer_cfg, .p_api = &g_timer_on_agt };
#if (1) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_rtc0) && !defined(SSP_SUPPRESS_ISR_RTC)
SSP_VECTOR_DEFINE( rtc_alarm_isr, RTC, ALARM);
#endif
#endif
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_rtc0) && !defined(SSP_SUPPRESS_ISR_RTC)
SSP_VECTOR_DEFINE(rtc_period_isr, RTC, PERIOD);
#endif
#endif
#if (1) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_rtc0) && !defined(SSP_SUPPRESS_ISR_RTC)
SSP_VECTOR_DEFINE( rtc_carry_isr, RTC, CARRY);
#endif
#endif
rtc_instance_ctrl_t g_rtc0_ctrl;
const rtc_cfg_t g_rtc0_cfg =
{ .clock_source = RTC_CLOCK_SOURCE_SUBCLK,
  .hw_cfg = true,
  .error_adjustment_value = 0,
  .error_adjustment_type = RTC_ERROR_ADJUSTMENT_NONE,
  .p_callback = rtc_alarm_callback,
  .p_context = &g_rtc0,
  .alarm_ipl = (1),
  .periodic_ipl = (BSP_IRQ_DISABLED),
  .carry_ipl = (1), };
/* Instance structure to use this module. */
const rtc_instance_t g_rtc0 =
{ .p_ctrl = &g_rtc0_ctrl, .p_cfg = &g_rtc0_cfg, .p_api = &g_rtc_on_rtc };
void g_hal_init(void)
{
    g_common_init ();
}
