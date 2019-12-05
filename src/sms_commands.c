/*
 * sms_commands.c
 *
 *  Created on: 12 Dec 2018
 *      Author: GDR
 */


#include "sms_commands.h"
#include "common.h"
#include "StringCommandParser.h"
#include "stdlib.h"
#include "stdio.h"

/*Tracker settings storage*/
extern tracker_settings_t tracker_settings;
extern modem_data_storage_t modem_data;
extern char g_sms_buff[100];

/*AT Commands Parser Storage*/
extern TSCPHandler   SCPHandler;

/* Normal Mode */
void NormalModeConfig (const char *pString)
{
    unsigned int interval = 0;
    uint8_t repetition = 0;
    char *symbol = NULL;
    char local_buff[50];
    uint8_t previuos_mode;

    /* Skip text 'normal'*/
    pString += 6;
    if(pString)
    {
        interval = (uint32_t)atoi(pString);
        symbol = strstr(pString, "t");
        if(symbol)
        {
            symbol++;
            repetition = (uint8_t)atoi(symbol);
        }

        /*Copy received settings*/
        previuos_mode = tracker_settings.mode;
        tracker_settings.mode = NORMAL_MODE;
        tracker_settings.interval = 0;
        tracker_settings.repeat = 0;

        /*Save settings, only if mode has changed (decrease flash wearing)*/
        if(tracker_settings.mode != previuos_mode)
        {
            TrackerSettingsSave();
        }

        /*These settings are not saved to flash memory*/
        tracker_settings.interval = interval;
        tracker_settings.repeat = repetition;

        /*Send SMS confirmation*/
        memset(g_sms_buff, 0x00, sizeof(g_sms_buff));
        memset(local_buff, 0x00, sizeof(local_buff));
        strcat(g_sms_buff, "Normal mode OK! ");
        strcat(g_sms_buff, "Interval ");
        memset(local_buff, 0x00, sizeof(local_buff));
        sprintf(local_buff, "%u", interval);
        strcat(g_sms_buff, local_buff);
        strcat(g_sms_buff, " seconds. ");
        strcat(g_sms_buff, "Repeat ");
        memset(local_buff, 0x00, sizeof(local_buff));
        sprintf(local_buff, "%d", repetition);
        strcat(g_sms_buff, local_buff);
        strcat(g_sms_buff, " times.");

        /*Termination symbol*/
        strcat(g_sms_buff, "\032");

        sprintf(local_buff, "AT+CMGS=%s\r", tracker_settings.phone_number);
        SCP_SendDoubleCommandWaitAnswer(local_buff, g_sms_buff, ">", "OK", 2000, 1);

        /*Wait 5000ms for modem to finish send SMS*/
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));
    }

}


/* Real Time Clock Mode */
void RTCModeConfig (const char *pString)
{
    unsigned int interval = 0;
    char local_buff[50];

    /* Skip text 'rtc'*/
    pString += 3;
    if(pString)
    {
        interval = (uint32_t)atoi(pString);
        tracker_settings.mode = POWER_SAVING_RTC;
        tracker_settings.RTC_interval = interval;

        TrackerSettingsSave();

        /*Send SMS confirmation*/
        memset(g_sms_buff, 0x00, sizeof(g_sms_buff));
        memset(local_buff, 0x00, sizeof(local_buff));
        strcat(g_sms_buff, "RTC power saving mode OK! ");
        strcat(g_sms_buff, "Interval ");
        memset(local_buff, 0x00, sizeof(local_buff));
        sprintf(local_buff, "%u", interval);
        strcat(g_sms_buff, local_buff);
        strcat(g_sms_buff, " seconds. ");

        /*Termination symbol*/
        strcat(g_sms_buff, "\032");

        sprintf(local_buff, "AT+CMGS=%s\r", tracker_settings.phone_number);
        SCP_SendDoubleCommandWaitAnswer(local_buff, g_sms_buff, ">", "OK", 2000, 1);

        /*Wait 5000ms for modem to finish send SMS*/
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));
    }
}

/* Accelerometer Mode */
void ACCModeConfig (const char *pString)
{
    uint8_t threshold = 0;
    char local_buff[50];

    /* Skip text 'acc'*/
    pString += 3;
    if(pString)
    {
        threshold = (uint8_t)atoi(pString);
        tracker_settings.mode = POWER_SAVING_ACC;
        tracker_settings.ACC_threshold = threshold;

        TrackerSettingsSave();

        /*Send SMS confirmation*/
        memset(g_sms_buff, 0x00, sizeof(g_sms_buff));
        memset(local_buff, 0x00, sizeof(local_buff));
        strcat(g_sms_buff, "Movement detection mode OK! ");
        strcat(g_sms_buff, "Threshold ");
        memset(local_buff, 0x00, sizeof(local_buff));
        sprintf(local_buff, "%u", threshold);
        strcat(g_sms_buff, local_buff);
        strcat(g_sms_buff, " units. ");

        /*Termination symbol*/
        strcat(g_sms_buff, "\032");

        sprintf(local_buff, "AT+CMGS=%s\r", tracker_settings.phone_number);
        SCP_SendDoubleCommandWaitAnswer(local_buff, g_sms_buff, ">", "OK", 2000, 1);

        /*Wait 5000ms for modem to finish send SMS*/
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));
    }
}

/*Memorise Number*/
void MemNumConfig (const char *pString)
{
    char *result = NULL;
    char local_buff[50];

    UNUSED(pString);

    /*Parse phone number from received SMS*/
    result = strstr( (char*)SCPHandler.RxBuffer, "+CMGL: ");
    if (result)
    {
        result = NULL;
        result = strstr( (char*)SCPHandler.RxBuffer, ",");
        if (result)
        {
            result++;
            result = strstr( result, ",");
            if(result)
            {
                result++;

                if(*result == '"')
                {
                    /*Copy operator to the RAM and return*/
                    /*Prepare to save the callers number*/
                    memset(tracker_settings.phone_number, 0x00, sizeof(tracker_settings.phone_number));

                    /*Save callers number*/
                    for(uint8_t i = 0; i < sizeof(tracker_settings.phone_number); i++)
                    {
                        tracker_settings.phone_number[i] = *result;
                        result++;

                        /*Last operator char?*/
                        if(*result == '"')
                        {
                            i++;
                            tracker_settings.phone_number[i] = *result;
                            break;
                        }
                    }
                }
            }
        }
    }

    /*Saving into Data Flash*/
    if(result)
    {
        TrackerSettingsSave();

        /*Send SMS confirmation*/
        memset(g_sms_buff, 0x00, sizeof(g_sms_buff));
        memset(local_buff, 0x00, sizeof(local_buff));
        strcat(g_sms_buff, "Saved: ");
        strcat(g_sms_buff, tracker_settings.phone_number);

        /*Termination symbol*/
        strcat(g_sms_buff, "\032");

        sprintf(local_buff, "AT+CMGS=%s\r", tracker_settings.phone_number);
        SCP_SendDoubleCommandWaitAnswer(local_buff, g_sms_buff, ">", "OK", 2000, 1);

        /*Wait 5000ms for modem to finish send SMS*/
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));
    }

}

void CloudModeConfig (const char *pString)
{
    unsigned int interval = 0;
    uint8_t repetition = 0;
    char *symbol = NULL;
    char local_buff[50];

    /*Allow AGPS request only once per SMS command*/
    modem_data.agps_request = true;

    /* Skip text 'cloud'*/
    pString += 5;
    if(pString)
    {
        interval = (uint32_t)atoi(pString);
        symbol = strstr(pString, "t");
        if(symbol)
        {
            symbol++;
            repetition = (uint8_t)atoi(symbol);
        }

        /*Copy received settings*/
        tracker_settings.mode = CLOUD_MODE;
        tracker_settings.interval = interval;
        tracker_settings.repeat = repetition;

        /*Save settings*/
        TrackerSettingsSave();

        /*Send SMS confirmation*/
        memset(g_sms_buff, 0x00, sizeof(g_sms_buff));
        memset(local_buff, 0x00, sizeof(local_buff));
        strcat(g_sms_buff, "Cloud mode OK! ");
        strcat(g_sms_buff, "Interval ");
        memset(local_buff, 0x00, sizeof(local_buff));
        sprintf(local_buff, "%u", interval);
        strcat(g_sms_buff, local_buff);
        strcat(g_sms_buff, " seconds. ");
        strcat(g_sms_buff, "Repeat ");
        memset(local_buff, 0x00, sizeof(local_buff));
        sprintf(local_buff, "%d", repetition);
        strcat(g_sms_buff, local_buff);
        strcat(g_sms_buff, " times.");

        /*Termination symbol*/
        strcat(g_sms_buff, "\032");

        sprintf(local_buff, "AT+CMGS=%s\r", tracker_settings.phone_number);
        SCP_SendDoubleCommandWaitAnswer(local_buff, g_sms_buff, ">", "OK", 2000, 1);

        /*Wait 5000ms for modem to finish send SMS*/
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));
    }
}

void APNConfig (const char *pString)
{
    char *symbol = NULL;
    char local_buff[128];


    /* Search for text 'APN'*/
    symbol = strstr(pString, "APN");
    if(symbol)
    {
        symbol = NULL;
        symbol = strstr(pString, "\"");
        if(symbol)
        {
            /*Copy APN to the RAM and return*/
            /*Prepare to save the APN*/
            symbol++;
            memset(tracker_settings.APN, 0x00, sizeof(tracker_settings.APN));

            /*Save callers number*/
            for(uint8_t i = 0; i < sizeof(tracker_settings.APN); i++)
            {
                tracker_settings.APN[i] = *symbol;
                symbol++;

                /*Last operator char?*/
                if(*symbol == '"')
                {
                    break;
                }
            }
        }

        /*Save settings*/
        TrackerSettingsSave();

        /*Send SMS confirmation*/
        memset(g_sms_buff, 0x00, sizeof(g_sms_buff));
        memset(local_buff, 0x00, sizeof(local_buff));
        strcat(g_sms_buff, "APN Saved: ");
        strcat(g_sms_buff, tracker_settings.APN);

        /*Termination symbol*/
        strcat(g_sms_buff, "\032");

        sprintf(local_buff, "AT+CMGS=%s\r", tracker_settings.phone_number);
        SCP_SendDoubleCommandWaitAnswer(local_buff, g_sms_buff, ">", "OK", 2000, 1);

        /*Wait 5000ms for modem to finish send SMS*/
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));
    }
}

void AppIDConfig (const char *pString)
{
    char *symbol = NULL;
    char local_buff[128];


    /* Search for text 'AppID'*/
    symbol = strstr(pString, "AppID");
    if(symbol)
    {
        symbol = NULL;
        symbol = strstr(pString, "\"");
        if(symbol)
        {
            /*Copy AppID to the RAM and return*/
            /*Prepare to save the AppID*/
            symbol++;
            memset(tracker_settings.telit_appID, 0x00, sizeof(tracker_settings.telit_appID));

            /*Save AppID*/
            for(uint8_t i = 0; i < sizeof(tracker_settings.telit_appID); i++)
            {
                tracker_settings.telit_appID[i] = *symbol;
                symbol++;

                /*Last operator char?*/
                if(*symbol == '"')
                {
                    break;
                }
            }
        }

        /*Save settings*/
        TrackerSettingsSave();

        /*Send SMS confirmation*/
        memset(g_sms_buff, 0x00, sizeof(g_sms_buff));
        memset(local_buff, 0x00, sizeof(local_buff));
        strcat(g_sms_buff, "AppID Saved: ");
        strcat(g_sms_buff, tracker_settings.telit_appID);

        /*Termination symbol*/
        strcat(g_sms_buff, "\032");

        sprintf(local_buff, "AT+CMGS=%s\r", tracker_settings.phone_number);
        SCP_SendDoubleCommandWaitAnswer(local_buff, g_sms_buff, ">", "OK", 2000, 1);

        /*Wait 5000ms for modem to finish send SMS*/
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));
    }
}

void AppTokenConfig (const char *pString)
{
    char *symbol = NULL;
    char local_buff[128];


    /* Search for text 'AppToken'*/
    symbol = strstr(pString, "AppToken");
    if(symbol)
    {
        symbol = NULL;
        symbol = strstr(pString, "\"");
        if(symbol)
        {
            /*Copy AppToken to the RAM and return*/
            /*Prepare to save AppToken*/
            symbol++;
            memset(tracker_settings.telit_appToken, 0x00, sizeof(tracker_settings.telit_appToken));

            /*Save AppToken*/
            for(uint8_t i = 0; i < sizeof(tracker_settings.telit_appToken); i++)
            {
                tracker_settings.telit_appToken[i] = *symbol;
                symbol++;

                /*Last operator char?*/
                if(*symbol == '"')
                {
                    break;
                }
            }
        }

        /*Save settings*/
        TrackerSettingsSave();

        /*Send SMS confirmation*/
        memset(g_sms_buff, 0x00, sizeof(g_sms_buff));
        memset(local_buff, 0x00, sizeof(local_buff));
        strcat(g_sms_buff, "AppToken Saved: ");
        strcat(g_sms_buff, tracker_settings.telit_appToken);

        /*Termination symbol*/
        strcat(g_sms_buff, "\032");

        sprintf(local_buff, "AT+CMGS=%s\r", tracker_settings.phone_number);
        SCP_SendDoubleCommandWaitAnswer(local_buff, g_sms_buff, ">", "OK", 2000, 1);

        /*Wait 5000ms for modem to finish send SMS*/
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));
    }
}

void HelpSMS (const char *pString)
{
    char *symbol = NULL;
    char local_buff[50];

    /* Search for text 'AppToken'*/
    symbol = strstr(pString, "help");
    if(symbol)
    {
        /*Send SMS confirmation*/
        memset(g_sms_buff, 0x00, sizeof(g_sms_buff));
        memset(local_buff, 0x00, sizeof(local_buff));
        strcat(g_sms_buff, "normal60t3 ");
        strcat(g_sms_buff, "rtc60 ");
        strcat(g_sms_buff, "acc10 ");
        strcat(g_sms_buff, "memnum ");
        strcat(g_sms_buff, "cloud60t3 ");
        strcat(g_sms_buff, "APN\"\" ");
        strcat(g_sms_buff, "AppID\"\" ");
        strcat(g_sms_buff, "AppToken\"\" ");
        strcat(g_sms_buff, "help");

        /*Termination symbol*/
        strcat(g_sms_buff, "\032");

        sprintf(local_buff, "AT+CMGS=%s\r", tracker_settings.phone_number);
        SCP_SendDoubleCommandWaitAnswer(local_buff, g_sms_buff, ">", "OK", 2000, 1);

        /*Wait 5000ms for modem to finish send SMS*/
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));
    }

}
