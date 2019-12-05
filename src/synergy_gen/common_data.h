/* generated common header file - do not edit */
#ifndef COMMON_DATA_H_
#define COMMON_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "r_dtc.h"
#include "r_transfer_api.h"
#include "r_sci_i2c.h"
#include "r_i2c_api.h"
#include "r_i2c_api.h"
#include "sf_i2c.h"
#include "sf_i2c_api.h"
#include "r_cgc.h"
#include "r_cgc_api.h"
#include "sf_power_profiles_v2.h"
#include "r_lpmv2_s3a7.h"
#include "r_lpmv2_api.h"
#include "r_elc.h"
#include "r_elc_api.h"
#include "r_fmi.h"
#include "r_fmi_api.h"
#include "r_ioport.h"
#include "r_ioport_api.h"
#include "r_cgc.h"
#include "r_cgc_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer2;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer1;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
extern const i2c_cfg_t g_i2c0_cfg;
/** I2C on SCI Instance. */
extern const i2c_master_instance_t g_i2c0;
#ifndef NULL
void NULL(i2c_callback_args_t *p_args);
#endif

extern const sci_i2c_extended_cfg g_i2c0_cfg_extend;
extern sci_i2c_instance_ctrl_t g_i2c0_ctrl;
#define SYNERGY_NOT_DEFINED (1)            
#if (SYNERGY_NOT_DEFINED == g_transfer1)
#define g_i2c0_P_TRANSFER_TX (NULL)
#else
#define g_i2c0_P_TRANSFER_TX (&g_transfer1)
#endif
#if (SYNERGY_NOT_DEFINED == g_transfer2)
#define g_i2c0_P_TRANSFER_RX (NULL)
#else
#define g_i2c0_P_TRANSFER_RX (&g_transfer2)
#endif
#undef SYNERGY_NOT_DEFINED
#define g_i2c0_P_EXTEND (&g_i2c0_cfg_extend)
extern sf_i2c_bus_t g_sf_i2c_bus0;
extern i2c_api_master_t const g_i2c_master_on_sci;

#define g_sf_i2c_bus0_CHANNEL        (2)
#define g_sf_i2c_bus0_RATE           (I2C_RATE_STANDARD)
#define g_sf_i2c_bus0_SLAVE          (0)
#define g_sf_i2c_bus0_ADDR_MODE      (I2C_ADDR_MODE_7BIT)          
#define g_sf_i2c_bus0_SDA_DELAY      (300)  
#define g_sf_i2c_bus0_P_CALLBACK     (NULL)
#define g_sf_i2c_bus0_P_CONTEXT      (&g_i2c0)
#define g_sf_i2c_bus0_RXI_IPL        ((3))
#define g_sf_i2c_bus0_TXI_IPL        ((3))
#define g_sf_i2c_bus0_TEI_IPL        ((3))            
#define g_sf_i2c_bus0_ERI_IPL        (BSP_IRQ_DISABLED)

/** These are obtained by macros in the I2C driver XMLs. */
#define g_sf_i2c_bus0_P_TRANSFER_TX  (g_i2c0_P_TRANSFER_TX)
#define g_sf_i2c_bus0_P_TRANSFER_RX  (g_i2c0_P_TRANSFER_RX)            
#define g_sf_i2c_bus0_P_EXTEND       (g_i2c0_P_EXTEND)
/** CGC Clocks */
extern const cgc_clocks_cfg_t g_cgc_cfg0;
/** Power Profiles on Power Profiles instance */
extern sf_power_profiles_v2_instance_t g_sf_power_profiles_v2_common;

void sf_power_profiles_v2_init0(void);
/** lpmv2 Instance */
extern const lpmv2_instance_t g_lpmv2_standby0;
/** ELC Instance */
extern const elc_instance_t g_elc;
/** FMI on FMI Instance. */
extern const fmi_instance_t g_fmi;
/** IOPORT Instance */
extern const ioport_instance_t g_ioport;
/** CGC Instance */
extern const cgc_instance_t g_cgc;
void g_common_init(void);
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* COMMON_DATA_H_ */
