/*
 * config_s3a7.h
 *
 *  Created on: 7 Sep 2018
 *      Author: GDR
 */

#ifndef CONFIG_S3A7_H_
#define CONFIG_S3A7_H_

#define LED_COUNT                   1
#define LED_1_PIN                   IOPORT_PORT_04_PIN_00
#define LED_1_ON_LEVEL              IOPORT_LEVEL_LOW
#define LED_1_OFF_LEVEL             IOPORT_LEVEL_HIGH
#define LED_1_DEFAULT               LED_1_OFF_LEVEL

#define CELL_RESET_CTRL_PIN         IOPORT_PORT_01_PIN_11
#define GNSS_RESET_CTRL_PIN         IOPORT_PORT_01_PIN_12
#define CHARGER_CTRL_PIN            IOPORT_PORT_01_PIN_13
#define GNSS_ON_OFF_PIN             IOPORT_PORT_05_PIN_00
#define SUPPLY_OK_PIN               IOPORT_PORT_02_PIN_06
#define CELL_POWER_CTRL_PIN         IOPORT_PORT_02_PIN_05
#define GNSS_POWER_CTRL_PIN         IOPORT_PORT_02_PIN_04
#define SENS_POWER_CTRL_PIN         IOPORT_PORT_04_PIN_01

#define PB_SWITCH_COUNT             1
#define PB_SWITCH_1_IRQ_INSTANCE    g_external_irq0
#define PB_SWITCH_1_IRQ_CALLBACK    external_irq0_callback
#define PB_SWITCH_1_PIN             IOPORT_PORT_02_PIN_06
#define PB_SWITCH                   PB_SWITCH_1_PIN



#endif /* CONFIG_S3A7_H_ */
