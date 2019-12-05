/*
 * bmx055.h
 *
 *  Created on: 10 Dec 2018
 *      Author: GDR
 */

#ifndef BMX055_H_
#define BMX055_H_

#include "hal_data.h"

_Bool bmx055_init(void);
_Bool bmx055_deinit(void);
ssp_err_t bmx055_measure(void);
uint16_t angle_conversion(int32_t x, int32_t y);

#endif /* BMX055_H_ */
