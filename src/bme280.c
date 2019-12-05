/*
 * bme280.c
 *
 *  Created on: 10 Dec 2018
 *      Author: GDR
 */


#include "bme280.h"
#include "config_s3a7.h"
#include "math.h"
#include "util.h"
#include "common.h"
#include "tracker_task.h"

/*Sensors data structure */
extern sensors_data_storage_t sensors_data;

/*Sensors coefficients storage*/
extern sensors_coefficients_t sensors_coeff;

_Bool bme280_init(void)
{
    uint8_t data[32];
    ssp_err_t i2c_status = SSP_SUCCESS;

    /* Open SCI2 IIC BME280 device */
    i2c_status = g_sf_i2c_bme280.p_api->open(g_sf_i2c_bme280.p_ctrl, g_sf_i2c_bme280.p_cfg);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    tx_thread_sleep (10);

    /*Reset the BME280*/
    data[0] = 0xE0;
    data[0] = 0xB6;
    i2c_status = g_sf_i2c_bme280.p_api->write(g_sf_i2c_bme280.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    tx_thread_sleep (10);

    /*Read 24 bytes of data from address 0x88*/
    data[0] = 0x88;
    i2c_status = g_sf_i2c_bme280.p_api->write(g_sf_i2c_bme280.p_ctrl, data, 1, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    i2c_status = g_sf_i2c_bme280.p_api->read(g_sf_i2c_bme280.p_ctrl, data, 24, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /* Convert the data*/
    /* temp coefficients*/
    sensors_coeff.coef_T1 = (data[0] & 0xFF) + ((data[1] & 0xFF) * 256);
    sensors_coeff.coef_T2 = (data[2] & 0xFF) + ((data[3] & 0xFF) * 256);
    if(sensors_coeff.coef_T2 > 32767)
    {
        sensors_coeff.coef_T2 -= 65536;
    }
    sensors_coeff.coef_T3 = (data[4] & 0xFF) + ((data[5] & 0xFF) * 256);
    if(sensors_coeff.coef_T3 > 32767)
    {
        sensors_coeff.coef_T3 -= 65536;
    }

    /* pressure coefficients */
    sensors_coeff.coef_P1 = (data[6] & 0xFF) + ((data[7] & 0xFF) * 256);
    sensors_coeff.coef_P2 = (data[8] & 0xFF) + ((data[9] & 0xFF) * 256);
    if(sensors_coeff.coef_P2 > 32767)
    {
        sensors_coeff.coef_P2 -= 65536;
    }
    sensors_coeff.coef_P3 = (data[10] & 0xFF) + ((data[11] & 0xFF) * 256);
    if(sensors_coeff.coef_P3 > 32767)
    {
        sensors_coeff.coef_P3 -= 65536;
    }
    sensors_coeff.coef_P4 = (data[12] & 0xFF) + ((data[13] & 0xFF) * 256);
    if(sensors_coeff.coef_P4 > 32767)
    {
        sensors_coeff.coef_P4 -= 65536;
    }
    sensors_coeff.coef_P5 = (data[14] & 0xFF) + ((data[15] & 0xFF) * 256);
    if(sensors_coeff.coef_P5 > 32767)
    {
        sensors_coeff.coef_P5 -= 65536;
    }
    sensors_coeff.coef_P6 = (data[16] & 0xFF) + ((data[17] & 0xFF) * 256);
    if(sensors_coeff.coef_P6 > 32767)
    {
        sensors_coeff.coef_P6 -= 65536;
    }
    sensors_coeff.coef_P7 = (data[18] & 0xFF) + ((data[19] & 0xFF) * 256);
    if(sensors_coeff.coef_P7 > 32767)
    {
        sensors_coeff.coef_P7 -= 65536;
    }
    sensors_coeff.coef_P8 = (data[20] & 0xFF) + ((data[21] & 0xFF) * 256);
    if(sensors_coeff.coef_P8 > 32767)
    {
        sensors_coeff.coef_P8 -= 65536;
    }
    sensors_coeff.coef_P9 = (data[22] & 0xFF) + ((data[23] & 0xFF) * 256);
    if(sensors_coeff.coef_P9 > 32767)
    {
        sensors_coeff.coef_P9 -= 65536;
    }

    /*Read 1 byte of data from address 0xA1*/
    data[0] = 0xA1;
    i2c_status = g_sf_i2c_bme280.p_api->write(g_sf_i2c_bme280.p_ctrl, data, 1, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    i2c_status = g_sf_i2c_bme280.p_api->read(g_sf_i2c_bme280.p_ctrl, data, 1, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    sensors_coeff.coef_H1 = data[0];

    /*Read 7 bytes of data from address 0xE1*/
    data[0] = 0xE1;
    i2c_status = g_sf_i2c_bme280.p_api->write(g_sf_i2c_bme280.p_ctrl, data, 1, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    i2c_status = g_sf_i2c_bme280.p_api->read(g_sf_i2c_bme280.p_ctrl, data, 7, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /* Convert the data */
    /* humidity coefficients */
    sensors_coeff.coef_H2 = (data[0] & 0xFF) + (data[1] * 256);
    if(sensors_coeff.coef_H2 > 32767)
    {
        sensors_coeff.coef_H2 -= 65536;
    }
    sensors_coeff.coef_H3 = data[2] & 0xFF ;
    sensors_coeff.coef_H4 = ((data[3] & 0xFF) * 16) + (data[4] & 0xF);
    if(sensors_coeff.coef_H4 > 32767)
    {
        sensors_coeff.coef_H4 -= 65536;
    }
    sensors_coeff.coef_H5 = ((data[4] & 0xFF) / 16) + ((data[5] & 0xFF) * 16);
    if(sensors_coeff.coef_H5 > 32767)
    {
        sensors_coeff.coef_H5 -= 65536;
    }
    sensors_coeff.coef_H6 = data[6] & 0xFF;
    if(sensors_coeff.coef_H6 > 127)
    {
        sensors_coeff.coef_H6 -= 256;
    }

    /*Humidity over sampling rate = 1*/
    data[0] = 0xF2;
    data[1] = 0x01;
    i2c_status = g_sf_i2c_bme280.p_api->write(g_sf_i2c_bme280.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Normal mode, temp and pressure over sampling rate = 1*/
    data[0] = 0xF4;
    data[1] = 0x27;
    i2c_status = g_sf_i2c_bme280.p_api->write(g_sf_i2c_bme280.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Time standby 0.5 ms*/
    data[0] = 0xF5;
    data[1] = 0x00;
    i2c_status = g_sf_i2c_bme280.p_api->write(g_sf_i2c_bme280.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    tx_thread_sleep (10);

    return true;

    exit_with_error:
    g_sf_i2c_bme280.p_api->reset(g_sf_i2c_bme280.p_ctrl, TX_NO_WAIT);
    g_sf_i2c_bme280.p_api->close(g_sf_i2c_bme280.p_ctrl);
    return false;
}

 _Bool bme280_deinit(void)
{
    uint8_t data[32];
    ssp_err_t i2c_status = SSP_SUCCESS;

    /*Reset the BME280*/
    data[0] = 0xE0;
    data[0] = 0xB6;
    i2c_status = g_sf_i2c_bme280.p_api->write(g_sf_i2c_bme280.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS){ return false;}

    /*Close the framework*/
    i2c_status =  g_sf_i2c_bme280.p_api->close(g_sf_i2c_bme280.p_ctrl);
    if(i2c_status != SSP_SUCCESS){ return false;}

    return true;
}

ssp_err_t bme280_measure(void)
{
    uint8_t data[32];
    ssp_err_t i2c_status = SSP_SUCCESS;

    /*Read 8 bytes of data from address 0xF7*/
    data[0] = 0xF7;
    i2c_status = g_sf_i2c_bme280.p_api->write(g_sf_i2c_bme280.p_ctrl, data, 1, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    i2c_status = g_sf_i2c_bme280.p_api->read(g_sf_i2c_bme280.p_ctrl, data, 8, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /* Convert pressure and temperature data to 19-bits */
    long adc_p = (((long)(data[0] & 0xFF) * 65536) + ((long)(data[1] & 0xFF) * 256) + (long)(data[2] & 0xF0)) / 16;
    long adc_t = (((long)(data[3] & 0xFF) * 65536) + ((long)(data[4] & 0xFF) * 256) + (long)(data[5] & 0xF0)) / 16;
    /* Convert the humidity data */
    long adc_h = ((long)(data[6] & 0xFF) * 256 + (long)(data[7] & 0xFF));

    /* Temperature offset calculations*/
    double var1 = (((double)adc_t) / 16384.0 - ((double)sensors_coeff.coef_T1) / 1024.0) * ((double)sensors_coeff.coef_T2);
    double var2 = ((((double)adc_t) / 131072.0 - ((double)sensors_coeff.coef_T1) / 8192.0) *
                   (((double)adc_t)/131072.0 - ((double)sensors_coeff.coef_T1)/8192.0)) * ((double)sensors_coeff.coef_T3);
    double t_fine = (long)(var1 + var2);
    sensors_data.bme280_temperature = (var1 + var2) / 5120.0;

    /* Pressure offset calculations */
    var1 = ((double)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)sensors_coeff.coef_P6) / 32768.0;
    var2 = var2 + var1 * ((double)sensors_coeff.coef_P5) * 2.0;
    var2 = (var2 / 4.0) + (((double)sensors_coeff.coef_P4) * 65536.0);
    var1 = (((double) sensors_coeff.coef_P3) * var1 * var1 / 524288.0 + ((double) sensors_coeff.coef_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)sensors_coeff.coef_P1);
    double p = 1048576.0 - (double)adc_p;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double) sensors_coeff.coef_P9) * p * p / 2147483648.0;
    var2 = p * ((double) sensors_coeff.coef_P8) / 32768.0;
    sensors_data.bme280_pressure = (p + (var1 + var2 + ((double)sensors_coeff.coef_P7)) / 16.0) / 100;

    // Humidity offset calculations
    double var_H = (((double)t_fine) - 76800.0);
    var_H = (adc_h - (sensors_coeff.coef_H4 * 64.0 + sensors_coeff.coef_H5 / 16384.0 * var_H)) * (sensors_coeff.coef_H2 / 65536.0 * (1.0 + sensors_coeff.coef_H6 / 67108864.0 * var_H * (1.0 + sensors_coeff.coef_H3 / 67108864.0 * var_H)));
    sensors_data.bme280_humidity = var_H * (1.0 -  sensors_coeff.coef_H1 * var_H / 524288.0);
    if(sensors_data.bme280_humidity > 100.0)
    {
        sensors_data.bme280_humidity = 100.0;
    }
    else
        if(sensors_data.bme280_humidity < 0.0)
        {
            sensors_data.bme280_humidity = 0.0;
        }


    /*No errors occurred*/
    return i2c_status;

    exit_with_error:
    /*reset IIC interface*/
    g_sf_i2c_bme280.p_api->reset(g_sf_i2c_bme280.p_ctrl, TX_NO_WAIT);
    g_sf_i2c_bme280.p_api->close(g_sf_i2c_bme280.p_ctrl);
    return i2c_status;
}
