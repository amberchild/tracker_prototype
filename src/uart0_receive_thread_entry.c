#include "uart0_receive_thread.h"
#include "common.h"
#include "config_s3a7.h"
#include "StringCommandParser.h"
#include "tracker_task.h"

extern modem_data_storage_t modem_data;

/* UART0 Receive Thread entry function */
void uart0_receive_thread_entry(void)
{
    ssp_err_t ReturnVal = SSP_SUCCESS;
    uint8_t data;

    while (1)
    {
        /*Read UART only if modem is powered*/
        if(modem_data.gsm_pwr_status)
        {
            /*Wait for a byte*/
            ReturnVal = uart_read_byte(&data);

            /*Give data for AT parser*/
            if(ReturnVal == SSP_SUCCESS)
            {
                SCP_ByteReceived(data);
            }
        }
        else
        {
            tx_thread_sleep(1);
        }

    }
}
