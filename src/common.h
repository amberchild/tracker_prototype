/*
 * common.h
 *
 *  Created on: 7 Sep 2018
 *      Author: GDR
 */

#ifndef COMMON_H_
#define COMMON_H_

#include "hal_data.h"

#define STRING_LEN 33
#define UNUSED(x) (void)(x)

typedef enum {
    PUSH_BUTTON = 1,
}EVENT_TYPE;

typedef struct
{
    EVENT_TYPE hdr_val;
} msg_hdr_t;


typedef struct
{   msg_hdr_t msg_hdr;
    uint8_t pb_switch_num;
    uint8_t level;
} pb_switch_payload_t;

typedef struct
{
    double bme280_temperature;
    double bme280_humidity;
    double bme280_pressure;
    int32_t bmx055_accx;
    int32_t bmx055_accy;
    int32_t bmx055_accz;
    int32_t bmx055_gyrx;
    int32_t bmx055_gyry;
    int32_t bmx055_gyrz;
    int32_t bmx055_magx;
    int32_t bmx055_magy;
    int32_t bmx055_magz;
    int32_t bmx055_xy_angle;
    time_t last_movement;

} sensors_data_storage_t;

typedef struct
{
    int32_t coef_T1;
    int32_t coef_T2;
    int32_t coef_T3;
    int32_t coef_P1;
    int32_t coef_P2;
    int32_t coef_P3;
    int32_t coef_P4;
    int32_t coef_P5;
    int32_t coef_P6;
    int32_t coef_P7;
    int32_t coef_P8;
    int32_t coef_P9;
    int32_t coef_H1;
    int32_t coef_H2;
    int32_t coef_H3;
    int32_t coef_H4;
    int32_t coef_H5;
    int32_t coef_H6;

} sensors_coefficients_t;

typedef struct
{
    _Bool   gsm_pwr_status;
    _Bool   gnss_pwr_status;
    _Bool   charger_pwr_status;
    _Bool   data_request;
    _Bool   agps_request;
    int32_t network_status;
    int32_t   gprs_status;
    int32_t batt_voltage_mv;
    int32_t signal;
    char operator[17];
    char imei[16];
    char ip_address[16];
    char device_name[21];
    char fw_version[16];
    _Bool   gnss_online;
    char gps_sw_version[50];
    char gps_utc[16];
    char gps_date[16];
    char gps_latitude[16];
    char gps_longitude[16];
    char gps_altitude[16];
    char gps_precision[16];
    char gps_course[16];
    char gps_speed[16];
    char gps_satt_in_use[16];


} modem_data_storage_t;

typedef struct
{
    uint8_t mode;
    uint32_t interval;
    uint32_t repeat;
    char phone_number[20];
    char telit_appID[100];
    char telit_appToken[100];
    char APN[100];
    uint32_t RTC_interval;
    uint8_t ACC_threshold;
    uint32_t CRC_data; /*CRC info, must stay last*/

} tracker_settings_t;

#define BME_READ        0xEF
#define BME_WRITE       0xEE
#define IIC_TIMEOUT     100
#define ACC_READ        0x33
#define ACC_WRITE       0x32
#define GYR_READ        0xD3
#define GYR_WRITE       0xD2
#define MAG_READ        0x27
#define MAG_WRITE       0x26

#define CLOCK_A         0
#define CLOCK_B         1
#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923

#define CRC_SEED    0x04C11DB7

/*Operation modes*/
#define NORMAL_MODE                 0   /*Always on and ready to execute received commands*/
#define POWER_SAVING_RTC            1   /*System wakes up in intervals. RTC is used to wake up the system*/
#define POWER_SAVING_ACC            2   /*System wakes up if accelerometer detects movement*/
#define CLOUD_MODE                  3   /*Always on, sends data to the Telit cloud. Interval and repeat times are adjustable*/
#define DEFAULT_MODE                NORMAL_MODE

/*LED blink modes*/
#define CONSTANT_ON                 1
#define CONSTANT_OFF                0
#define LED_SYSTEM_INIT             2
#define ONLINE_IDLE                 3

int OpenDriver( void );
void CloseDriver( void );
ssp_err_t ReadSettings( void *buff, unsigned int size );
ssp_err_t WriteSettings( void *buff, unsigned int size );
void TrackerSettingsSave(void);

#endif /* COMMON_H_ */
