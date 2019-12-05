/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_icu.h"
#include "r_external_irq_api.h"
#include "r_crc.h"
#include "r_crc_api.h"
#include "r_flash_lp.h"
#include "r_flash_api.h"
#include "sf_power_profiles_v2.h"
#include "sf_power_profiles_v2.h"
#include "r_agt.h"
#include "r_timer_api.h"
#include "r_rtc.h"
#include "r_rtc_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
/* External IRQ on ICU Instance. */
extern const external_irq_instance_t g_external_irq13;
#ifndef detect_charger_callback
void detect_charger_callback(external_irq_callback_args_t *p_args);
#endif
extern const crc_instance_t g_crc;
/* Flash on Flash LP Instance. */
extern const flash_instance_t flash_driver;
#ifndef flash_CallBack
void flash_CallBack(flash_callback_args_t *p_args);
#endif
/* External IRQ on ICU Instance. */
extern const external_irq_instance_t g_external_irq9;
#ifndef accelerometer_callback
void accelerometer_callback(external_irq_callback_args_t *p_args);
#endif
#ifdef g_bsp_pin_cfg
#define POWER_PROFILES_V2_RUN_PIN_CFG_TBL_USED_g_sf_power_profiles_v2_run_0 (0)
#else
#define POWER_PROFILES_V2_RUN_PIN_CFG_TBL_USED_g_sf_power_profiles_v2_run_0 (1)
#endif
#if POWER_PROFILES_V2_RUN_PIN_CFG_TBL_USED_g_sf_power_profiles_v2_run_0
extern const ioport_cfg_t g_bsp_pin_cfg;
#endif
extern const cgc_clocks_cfg_t g_cgc_cfg0;
/** Power Profiles run structure */
extern sf_power_profiles_v2_run_cfg_t g_sf_power_profiles_v2_run_0;
#ifdef low_pwr_config
#define POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_v2_low_power_0 (0)
#else
#define POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_v2_low_power_0 (1)
#endif
#if POWER_PROFILES_V2_ENTER_PIN_CFG_TBL_USED_g_sf_power_profiles_v2_low_power_0
extern const ioport_cfg_t low_pwr_config;
#endif
#ifdef g_bsp_pin_cfg
#define POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_v2_low_power_0 (0)
#else
#define POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_v2_low_power_0 (1)
#endif
#if POWER_PROFILES_V2_EXIT_PIN_CFG_TBL_USED_g_sf_power_profiles_v2_low_power_0
extern const ioport_cfg_t g_bsp_pin_cfg;
#endif
#ifdef lpm_callback
#define POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_v2_low_power_0 (0)
#else
#define POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_v2_low_power_0 (1)
#endif
#if POWER_PROFILES_V2_CALLBACK_USED_g_sf_power_profiles_v2_low_power_0
void lpm_callback(sf_power_profiles_v2_callback_args_t *p_args);
#endif
/** Power Profiles run structure */
extern sf_power_profiles_v2_low_power_cfg_t g_sf_power_profiles_v2_low_power_0;
/* External IRQ on ICU Instance. */
extern const external_irq_instance_t g_external_irq0;
#ifndef button_callback
void button_callback(external_irq_callback_args_t *p_args);
#endif
/** AGT Timer Instance */
extern const timer_instance_t g_modem_timer;
#ifndef g_modem_timer_callback
void g_modem_timer_callback(timer_callback_args_t *p_args);
#endif
/** RTC on RTC Instance. */
extern const rtc_instance_t g_rtc0;
#ifndef rtc_alarm_callback
void rtc_alarm_callback(rtc_callback_args_t *p_args);
#endif
void hal_entry(void);
void g_hal_init(void);
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* HAL_DATA_H_ */
