/* generated thread header file - do not edit */
#ifndef TRACKER_TASK_H_
#define TRACKER_TASK_H_
#include "bsp_api.h"
#include "tx_api.h"
#include "hal_data.h"
#ifdef __cplusplus
extern "C" void tracker_task_entry(void);
#else
extern void tracker_task_entry(void);
#endif
#include "sf_i2c.h"
#include "sf_i2c_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
/* SF I2C on SF I2C Instance. */
extern const sf_i2c_instance_t g_sf_i2c_bmx055_mag;
/* SF I2C on SF I2C Instance. */
extern const sf_i2c_instance_t g_sf_i2c_bmx055_gyr;
/* SF I2C on SF I2C Instance. */
extern const sf_i2c_instance_t g_sf_i2c_bmx055_acc;
/* SF I2C on SF I2C Instance. */
extern const sf_i2c_instance_t g_sf_i2c_bme280;
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* TRACKER_TASK_H_ */
