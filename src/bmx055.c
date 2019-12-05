/*
 * bmx055.c
 *
 *  Created on: 10 Dec 2018
 *      Author: GDR
 */

#include "bmx055.h"
#include "config_s3a7.h"
#include "math.h"
#include "util.h"
#include "common.h"
#include "tracker_task.h"

/*Sensors data structure */
extern sensors_data_storage_t sensors_data;
extern tracker_settings_t tracker_settings;

_Bool bmx055_init(void)
{
    uint8_t data[32];
    ssp_err_t i2c_status = SSP_SUCCESS;

    /* Open SCI2 IIC BMX055 devices */
    i2c_status = g_sf_i2c_bmx055_acc.p_api->open(g_sf_i2c_bmx055_acc.p_ctrl, g_sf_i2c_bmx055_acc.p_cfg);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    i2c_status = g_sf_i2c_bmx055_gyr.p_api->open(g_sf_i2c_bmx055_gyr.p_ctrl, g_sf_i2c_bmx055_gyr.p_cfg);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    i2c_status = g_sf_i2c_bmx055_mag.p_api->open(g_sf_i2c_bmx055_mag.p_ctrl, g_sf_i2c_bmx055_mag.p_cfg);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    tx_thread_sleep (10);

    /*Reset the Accelerometer*/
    data[0] = 0x14;
    data[1] = 0x00;
    i2c_status = g_sf_i2c_bmx055_acc.p_api->write(g_sf_i2c_bmx055_acc.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    tx_thread_sleep (1);

    /*Accelerometer Set Range = +/- 2g*/
    data[0] = 0x0F;
    data[1] = 0x03;
    i2c_status = g_sf_i2c_bmx055_acc.p_api->write(g_sf_i2c_bmx055_acc.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Accelerometer Set bandwidth = 7.81 Hz*/
    data[0] = 0x10;
    data[1] = 0x00;
    i2c_status = g_sf_i2c_bmx055_acc.p_api->write(g_sf_i2c_bmx055_acc.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Accelerometer Set Normal mode, Sleep duration = 0.5ms*/
    data[0] = 0x11;
    data[1] = 0x00;
    i2c_status = g_sf_i2c_bmx055_acc.p_api->write(g_sf_i2c_bmx055_acc.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Accelerometer Set Slope Threshold*/
    data[0] = 0x28;
    data[1] = tracker_settings.ACC_threshold;
    i2c_status = g_sf_i2c_bmx055_acc.p_api->write(g_sf_i2c_bmx055_acc.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Accelerometer Set Slope Duration*/
    data[0] = 0x27;
    data[1] = 0x03;
    i2c_status = g_sf_i2c_bmx055_acc.p_api->write(g_sf_i2c_bmx055_acc.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Accelerometer slope detection interrupt assign*/
    data[0] = 0x19;
    data[1] = 0x04;
    i2c_status = g_sf_i2c_bmx055_acc.p_api->write(g_sf_i2c_bmx055_acc.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Accelerometer Enabling axis interrupts*/
    data[0] = 0x16;
    data[1] = 0x07;
    i2c_status = g_sf_i2c_bmx055_acc.p_api->write(g_sf_i2c_bmx055_acc.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /* Gyroscope Deep Suspend Mode*/
    data[0] = 0x11;
    data[1] = 0x32;
    i2c_status = g_sf_i2c_bmx055_gyr.p_api->write(g_sf_i2c_bmx055_gyr.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {return false;}

    /*Magnetometer Suspend Mode*/
    data[0] = 0x4B;
    data[1] = 0x00;
    i2c_status = g_sf_i2c_bmx055_mag.p_api->write(g_sf_i2c_bmx055_mag.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {return false;}


# if 0
    /*Gyroscope Full scale = +/- 125 degree*/
    data[0] = 0x0F;
    data[1] = 0x04;
    i2c_status = g_sf_i2c_bmx055_gyr.p_api->write(g_sf_i2c_bmx055_gyr.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Gyroscope Bandwidth ODR = 100 Hz*/
    data[0] = 0x10;
    data[1] = 0x07;
    i2c_status = g_sf_i2c_bmx055_gyr.p_api->write(g_sf_i2c_bmx055_gyr.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Gyroscope Normal mode, Sleep duration = 2ms*/
    data[0] = 0x11;
    data[1] = 0x00;
    i2c_status = g_sf_i2c_bmx055_gyr.p_api->write(g_sf_i2c_bmx055_gyr.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Reset the Magnetometer*/
    data[0] = 0x4B;
    data[1] = 0x83;
    i2c_status = g_sf_i2c_bmx055_mag.p_api->write(g_sf_i2c_bmx055_mag.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    tx_thread_sleep (1);

    /*Magnetometer Normal Mode, ODR = 10 Hz*/
    data[0] = 0x4C;
    data[1] = 0x00;
    i2c_status = g_sf_i2c_bmx055_mag.p_api->write(g_sf_i2c_bmx055_mag.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Magnetometer X, Y, Z-Axis enabled*/
    data[0] = 0x4E;
    data[1] = 0x84;
    i2c_status = g_sf_i2c_bmx055_mag.p_api->write(g_sf_i2c_bmx055_mag.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Magnetometer No. of Repetitions for X-Y Axis = 9*/
    data[0] = 0x51;
    data[1] = 0x04;
    i2c_status = g_sf_i2c_bmx055_mag.p_api->write(g_sf_i2c_bmx055_mag.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Magnetometer No. of Repetitions for Z-Axis = 15*/
    data[0] = 0x52;
    data[1] = 0x0F;
    i2c_status = g_sf_i2c_bmx055_mag.p_api->write(g_sf_i2c_bmx055_mag.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

#endif

    tx_thread_sleep (10);

    return true;

    exit_with_error:
    g_sf_i2c_bmx055_acc.p_api->reset(g_sf_i2c_bmx055_acc.p_ctrl, TX_NO_WAIT);
    g_sf_i2c_bmx055_gyr.p_api->reset(g_sf_i2c_bmx055_gyr.p_ctrl, TX_NO_WAIT);
    g_sf_i2c_bmx055_mag.p_api->reset(g_sf_i2c_bmx055_mag.p_ctrl, TX_NO_WAIT);
    g_sf_i2c_bmx055_acc.p_api->close(g_sf_i2c_bmx055_acc.p_ctrl);
    g_sf_i2c_bmx055_gyr.p_api->close(g_sf_i2c_bmx055_gyr.p_ctrl);
    g_sf_i2c_bmx055_mag.p_api->close(g_sf_i2c_bmx055_mag.p_ctrl);
    return false;
}

_Bool bmx055_deinit(void)
{
    uint8_t data[32];
    ssp_err_t i2c_status = SSP_SUCCESS;

    /* Accelerometer Deep Suspend Mode*/
    data[0] = 0x11;
    data[1] = 0x32;
    i2c_status = g_sf_i2c_bmx055_acc.p_api->write(g_sf_i2c_bmx055_acc.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {return false;}

    /* Gyroscope Deep Suspend Mode*/
    data[0] = 0x11;
    data[1] = 0x32;
    i2c_status = g_sf_i2c_bmx055_gyr.p_api->write(g_sf_i2c_bmx055_gyr.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {return false;}

    /*Magnetometer Suspend Mode*/
    data[0] = 0x4B;
    data[1] = 0x00;
    i2c_status = g_sf_i2c_bmx055_mag.p_api->write(g_sf_i2c_bmx055_mag.p_ctrl, data, 2, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {return false;}

    /*Close the framework*/
    i2c_status = g_sf_i2c_bmx055_acc.p_api->close(g_sf_i2c_bmx055_acc.p_ctrl);
    if(i2c_status != SSP_SUCCESS) {return false;}
    i2c_status = g_sf_i2c_bmx055_gyr.p_api->close(g_sf_i2c_bmx055_gyr.p_ctrl);
    if(i2c_status != SSP_SUCCESS) {return false;}
    i2c_status = g_sf_i2c_bmx055_mag.p_api->close(g_sf_i2c_bmx055_mag.p_ctrl);
    if(i2c_status != SSP_SUCCESS) {return false;}

    return true;
}

ssp_err_t bmx055_measure(void)
{
    uint8_t data[32];
    ssp_err_t i2c_status = SSP_SUCCESS;

    /*Read 6 bytes of data from address 0x02*/
    data[0] = 0x02;
    i2c_status = g_sf_i2c_bmx055_acc.p_api->write(g_sf_i2c_bmx055_acc.p_ctrl, data, 1, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    i2c_status = g_sf_i2c_bmx055_acc.p_api->read(g_sf_i2c_bmx055_acc.p_ctrl, data, 6, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Convert accelerometer data*/
    sensors_data.bmx055_accx = ((data[1] & 0xFF) * 256 + (data[0] & 0xF0)) / 16;
    if (sensors_data.bmx055_accx > 2047)
    {
        sensors_data.bmx055_accx -= 4096;
    }
    sensors_data.bmx055_accy = ((data[3] & 0xFF) * 256 + (data[2] & 0xF0)) / 16;
    if (sensors_data.bmx055_accy > 2047)
    {
        sensors_data.bmx055_accy -= 4096;
    }
    sensors_data.bmx055_accz = ((data[5] & 0xFF) * 256 + (data[4] & 0xF0)) / 16;
    if (sensors_data.bmx055_accz > 2047)
    {
        sensors_data.bmx055_accz -= 4096;
    }

    /*Read 6 bytes of data from address 0x02*/
    data[0] = 0x02;
    i2c_status = g_sf_i2c_bmx055_gyr.p_api->write(g_sf_i2c_bmx055_gyr.p_ctrl, data, 1, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    i2c_status = g_sf_i2c_bmx055_gyr.p_api->read(g_sf_i2c_bmx055_gyr.p_ctrl, data, 6, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Convert gyroscope data*/
    sensors_data.bmx055_gyrx = ((data[1] & 0xFF) * 256 + (data[0] & 0xFF));
    if (sensors_data.bmx055_gyrx > 32767)
    {
        sensors_data.bmx055_gyrx -= 65536;
    }
    sensors_data.bmx055_gyry = ((data[3] & 0xFF) * 256 + (data[2] & 0xFF));
    if (sensors_data.bmx055_gyry > 32767)
    {
        sensors_data.bmx055_gyry -= 65536;
    }
    sensors_data.bmx055_gyrz = ((data[5] & 0xFF) * 256 + (data[4] & 0xFF));
    if (sensors_data.bmx055_gyrz > 32767)
    {
        sensors_data.bmx055_gyrz -= 65536;
    }

    /*Read 6 bytes of data from address 0x42*/
    data[0] = 0x42;
    i2c_status = g_sf_i2c_bmx055_mag.p_api->write(g_sf_i2c_bmx055_mag.p_ctrl, data, 1, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}
    i2c_status = g_sf_i2c_bmx055_mag.p_api->read(g_sf_i2c_bmx055_mag.p_ctrl, data, 6, false, IIC_TIMEOUT);
    if(i2c_status != SSP_SUCCESS)
    {goto exit_with_error;}

    /*Convert magnetometer data*/
    sensors_data.bmx055_magx = ((data[1] & 0xFF) * 256 + (data[0] & 0xF8)) / 8;
    if (sensors_data.bmx055_magx > 4095)
    {
        sensors_data.bmx055_magx -= 8192;
    }
    sensors_data.bmx055_magy = ((data[3] & 0xFF) * 256 + (data[2] & 0xF8)) / 8;
    if (sensors_data.bmx055_magy > 4095)
    {
        sensors_data.bmx055_magy -= 8192;
    }
    sensors_data.bmx055_magz = ((data[5] & 0xFF) * 256 + (data[4] & 0xFE)) / 2;
    if (sensors_data.bmx055_magz > 16383)
    {
        sensors_data.bmx055_magz -= 32768;
    }

    /*Calculate  X Y position*/
    sensors_data.bmx055_xy_angle = angle_conversion(sensors_data.bmx055_magx, sensors_data.bmx055_magy);

    /*No errors occurred*/
    return i2c_status;

    exit_with_error:
    /*reset IIC interface*/
    g_sf_i2c_bmx055_acc.p_api->reset(g_sf_i2c_bmx055_acc.p_ctrl, TX_NO_WAIT);
    g_sf_i2c_bmx055_gyr.p_api->reset(g_sf_i2c_bmx055_gyr.p_ctrl, TX_NO_WAIT);
    g_sf_i2c_bmx055_mag.p_api->reset(g_sf_i2c_bmx055_mag.p_ctrl, TX_NO_WAIT);
    g_sf_i2c_bmx055_acc.p_api->close(g_sf_i2c_bmx055_acc.p_ctrl);
    g_sf_i2c_bmx055_gyr.p_api->close(g_sf_i2c_bmx055_gyr.p_ctrl);
    g_sf_i2c_bmx055_mag.p_api->close(g_sf_i2c_bmx055_mag.p_ctrl);
    return i2c_status;
}

uint16_t angle_conversion(int32_t x, int32_t y)
{
  uint16_t result = 0;
  double radians = 0;

  if ((x == 0) && (y == 0))
  {
      radians = 0.0;
  }

  else if ((x > 0) && (y == 0))
  {
      radians =  0.0;
  }

  else if ((x > 0) && (y > 0))
  {
      radians = atan((double)y / (double)x);
  }

  else if ((x == 0) && (y > 0))
  {
      radians = M_PI_2;
  }

  else if ((x < 0) && (y > 0))
  {
      radians = M_PI + atan((double)y / (double)x);
  }

  else if ((x < 0) && (y == 0))
  {
      radians = M_PI;
  }

  else if ((x < 0) && (y < 0))
  {
      radians = M_PI + atan((double)y / (double)x);
  }

  else if ((x == 0) && (y < 0))
  {
      radians = 3.0 * M_PI_2;
  }

  else if ((x > 0) && (y < 0))
  {
      radians = 2.0 * M_PI + atan((double)y / (double)x);
  }

  result = (uint16_t)(radians * 180 /M_PI);

  return result;

}
