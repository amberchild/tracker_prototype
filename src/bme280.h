/*
 * bme280.h
 *
 *  Created on: 10 Dec 2018
 *      Author: GDR
 */

#ifndef BME280_H_
#define BME280_H_

#include "hal_data.h"

_Bool bme280_init(void);
_Bool bme280_deinit(void);
ssp_err_t bme280_measure(void);

#endif /* BME280_H_ */
