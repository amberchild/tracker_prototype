/*
 * common.c
 *
 *  Created on: 12 Dec 2018
 *      Author: GDR
 */

#include "common.h"
#include "util.h"

/* data flash array 40100000h - 40103FFFh */
/* code flash array 00000000h - 000FFFFFh */
#define DataFlashArray              (0x40100000L) /* (FLASH_DF_BLOCK_0) */
#define DATA_FLASH_BLOCK_SIZE       (1024)        /* (FLASH_DATA_BLOCK_SZ) */

flash_event_t  flash_event;

/*Tracker settings storage*/
extern tracker_settings_t tracker_settings;

void flash_CallBack(flash_callback_args_t * p_args)
{
    flash_event = p_args->event;
}

static int wait_flash_event( flash_event_t event_type )
{
    int i=0;
    flash_event = FLASH_EVENT_ERR_FAILURE;
    while ((flash_event != event_type)&&(++i < 1000000));
    return (i);
}

int OpenDriver( void )
{
    ssp_err_t state = SSP_SUCCESS;
    state = flash_driver.p_api->open( flash_driver.p_ctrl, flash_driver.p_cfg );
    return (state == SSP_SUCCESS);
}

void CloseDriver( void )
{
    flash_driver.p_api->close( flash_driver.p_ctrl );
}

ssp_err_t ReadSettings( void *buff, unsigned int size )
{
    ssp_err_t state;

    state = flash_driver.p_api->read( flash_driver.p_ctrl, (uint8_t*)buff, DataFlashArray, size );

    return state;
}

ssp_err_t WriteSettings( void *buff, unsigned int size )
{
    ssp_err_t state;
    unsigned int bloks = (size / DATA_FLASH_BLOCK_SIZE) + ((size % DATA_FLASH_BLOCK_SIZE)>0);

    if ((state = flash_driver.p_api->erase( flash_driver.p_ctrl, DataFlashArray, bloks )) == SSP_SUCCESS)
    {
        wait_flash_event( FLASH_EVENT_ERASE_COMPLETE );
        state = flash_driver.p_api->write( flash_driver.p_ctrl, (uint32_t)buff, DataFlashArray, size );
        wait_flash_event( FLASH_EVENT_WRITE_COMPLETE );
    }

    return state;
}

void TrackerSettingsSave(void)
{
    ssp_err_t err;
    uint32_t crc_result;

    if(OpenDriver())
    {
        /*Hardware CRC Engine open*/
        err =  g_crc.p_api->open(g_crc.p_ctrl, g_crc.p_cfg);
        if(err) { APP_ERR_TRAP(err); }

        /*Calculate new CRC*/
        err =  g_crc.p_api->calculate(g_crc.p_ctrl, &tracker_settings, sizeof(tracker_settings)-4, CRC_SEED, &crc_result);
        if(err) { APP_ERR_TRAP(err); }if(err) { APP_ERR_TRAP(err); }
        tracker_settings.CRC_data = crc_result;

        /*Write to Data flash */
        err = WriteSettings( &tracker_settings, sizeof(tracker_settings));
        if(err) { APP_ERR_TRAP(err); }

        /*Hardware CRC Engine close*/
        (void)g_crc.p_api->close(g_crc.p_ctrl);

        /*Flash driver close*/
        CloseDriver();
    }
}
