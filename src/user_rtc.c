/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

#include "hal_data.h"
#include "user_rtc.h"
#include "common.h"
#include <time.h>

static uint32_t weekday_get(rtc_time_t * p_time);
static uint32_t yearday_get(rtc_time_t * p_time);

/* Constant data used for calculating the weekday and day of year */
static const uint8_t month_table[12]    = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};
static const uint8_t ly_month_table[12] = {6, 2, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};
static const uint8_t days_in_month[12]  = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static uint32_t weekday_get(rtc_time_t * p_time);
static uint32_t yearday_get(rtc_time_t * p_time);

#define ASCII_OFFSET                (48)

/*
 * User Application function that will modify the RTC alarm time to current time + wanted amount of seconds
 * This RTC alarm is used to wake the device from Software Standby mode
 * Therefore, the device will wake wanted amount of seconds after entering Software Standby mode
 */
void user_function_set_rtc_alarm(time_t seconds_plus)
{
    ssp_err_t ssp_err;

    rtc_time_t          rtc_time;
    rtc_alarm_time_t    alarm_time;

    time_t time; /*Time in seconds*/
    struct tm *calendar; /*Pointer to read converter time*/

    /* Read the current RTC time */
    ssp_err = g_rtc0.p_api->calendarTimeGet(g_rtc0.p_ctrl, &rtc_time);
    if (SSP_SUCCESS != ssp_err)
    {
        __BKPT(1);
    }

    /*Convert calendar time to seconds*/
    time = mktime(&rtc_time);

    /*Add seconds*/
    time = seconds_plus + time;

    /*Convert back to calendar time for alarm setup*/
    calendar = localtime(&time);
    alarm_time.time.tm_sec = calendar->tm_sec;
    alarm_time.time.tm_min = calendar->tm_min;
    alarm_time.time.tm_hour = calendar->tm_hour;
    alarm_time.time.tm_mday = calendar->tm_mday;
    alarm_time.time.tm_mon = calendar->tm_mon;
    alarm_time.time.tm_year = calendar->tm_year;
    alarm_time.time.tm_wday = calendar->tm_wday;
    alarm_time.time.tm_yday = calendar->tm_yday;
    alarm_time.time.tm_isdst = calendar->tm_isdst;

    /*Set match for everything*/
    alarm_time.sec_match = true;
    alarm_time.min_match = true;
    alarm_time.hour_match = true;
    alarm_time.mday_match = true;
    alarm_time.mon_match = true;
    alarm_time.year_match = true;
    alarm_time.dayofweek_match = true;

    /*Alarm interrupt enable*/
    ssp_err = g_rtc0.p_api->irqEnable(g_rtc0.p_ctrl, RTC_EVENT_ALARM_IRQ);
    if (SSP_SUCCESS != ssp_err)
    {
        __BKPT(1);
    }

    /*Set the alarm*/
    ssp_err = g_rtc0.p_api->calendarAlarmSet(g_rtc0.p_ctrl, &alarm_time, true);
    if (SSP_SUCCESS != ssp_err)
    {
        __BKPT(1);
    }
}


/*
 * User Application function that will set a default time and date of the RTC
 * The default time is determined from the build time & date using the
 * predefined MACRO __TIME__ & __DATE__
 */
void user_function_set_rtc_time(void)
{
    ssp_err_t ssp_err;
    rtc_time_t rtc_time;

    /*
     * Example of __TIME__ string: "14:15:16"
     *                             [01234567]
     * Calculations based upon MACRO definitions on Stack Overflow
     * https://stackoverflow.com/questions/11697820/how-to-use-date-and-time-predefined-macros-in-as-two-integers-then-stri
     */
    rtc_time.tm_hour = ((__TIME__[0] - ASCII_OFFSET) * 10 + __TIME__[1] - ASCII_OFFSET);
    rtc_time.tm_min =  ((__TIME__[3] - ASCII_OFFSET) * 10 + __TIME__[4] - ASCII_OFFSET);
    rtc_time.tm_sec =  ((__TIME__[6] - ASCII_OFFSET) * 10 + __TIME__[7] - ASCII_OFFSET);

    /*
     * Example of __DATE__ string: "Jul  2 2017"    If day < 10, day is padded as shown
     * Example of __DATE__ string: "Jul 20 2017"
     *                             [01234567890]
     * Calculations based upon MACRO definitions on Stack Overflow
     * https://stackoverflow.com/questions/11697820/how-to-use-date-and-time-predefined-macros-in-as-two-integers-then-stri
     */
    rtc_time.tm_year = (__DATE__[ 7] - ASCII_OFFSET) * 1000 +  \
                       (__DATE__[ 8] - ASCII_OFFSET) * 100 + \
                       (__DATE__[ 9] - ASCII_OFFSET) * 10 + \
                       (__DATE__[10] - ASCII_OFFSET);

    rtc_time.tm_mday = ((__DATE__[4] >= ASCII_OFFSET) ? (__DATE__[4] - ASCII_OFFSET) * 10 : 0) + \
                        (__DATE__[5] - ASCII_OFFSET);

    rtc_time.tm_mon =  ((__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')) ?  0 : \
                       ((__DATE__[0] == 'F'))                                             ?  1 : \
                       ((__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')) ?  2 : \
                       ((__DATE__[0] == 'A' && __DATE__[1] == 'p'))                       ?  3 : \
                       ((__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')) ?  4 : \
                       ((__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')) ?  5 : \
                       ((__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')) ?  6 : \
                       ((__DATE__[0] == 'A' && __DATE__[1] == 'u'))                       ?  7 : \
                       ((__DATE__[0] == 'S'))                                             ?  8 : \
                       ((__DATE__[0] == 'O'))                                             ?  9 : \
                       ((__DATE__[0] == 'N'))                                             ? 10 : \
                       ((__DATE__[0] == 'D'))                                             ? 11 : 255; /* 255 == error */

    rtc_time.tm_wday = (int)weekday_get( &rtc_time );    /* Calculate the weekday (0 - 6) */
    rtc_time.tm_yday = (int)yearday_get( &rtc_time );    /* Calculate the year-day */
    rtc_time.tm_year = rtc_time.tm_year - 1900;

    ssp_err = g_rtc0.p_api->configure(g_rtc0.p_ctrl, NULL);
    if (SSP_SUCCESS != ssp_err)
    {
        __BKPT(0);   /* If time set is unsuccessful, hit breakpoint */
    }

    ssp_err = g_rtc0.p_api->calendarTimeSet(g_rtc0.p_ctrl, &rtc_time, true);
    if (SSP_SUCCESS != ssp_err)
    {
        __BKPT(0);   /* If time set is unsuccessful, hit breakpoint */
    }
}

time_t user_function_get_rtc_time(void)
{
    ssp_err_t ssp_err;
    rtc_time_t          rtc_time;
    time_t time; /*Time in seconds*/

    /* Read the current RTC time */
    ssp_err = g_rtc0.p_api->calendarTimeGet(g_rtc0.p_ctrl, &rtc_time);
    if (SSP_SUCCESS != ssp_err)
    {
        __BKPT(1);
    }

    /*Convert calendar time to seconds*/
    time = mktime(&rtc_time);

    return time;
}

static uint32_t weekday_get(rtc_time_t * p_time)
{
    uint32_t day = (uint32_t)p_time->tm_mday;
    bool leap = false;


    if (0 == p_time->tm_year % 4)
    {
        leap = true;
        if (0 == p_time->tm_year % 100)
        {
            leap = false;
            if (0 != p_time->tm_year % 400)
            {
                leap = true;
            }
        }
    }

    uint32_t month_offset;
    if (true == leap)
    {
        month_offset = ly_month_table[ p_time->tm_mon ];
    }
    else
    {
        month_offset = month_table[ p_time->tm_mon ];
    }

    uint32_t year_offset    = (uint32_t)(p_time->tm_year % 100);
    uint32_t century_offset = (uint32_t)((3 - ((p_time->tm_year / 100) % 4)) * 2);
    uint32_t offset         = day + month_offset + year_offset + (year_offset / 4) + century_offset;
    uint32_t index          = offset % 7;

    return index;   /* Return 0 to 6, where:
                     * Sun = 0, Mon = 1, Tue = 2, Wed = 3, Thu = 4, Fri = 5, Saturday = 6
                     */
}

/*
 * Helper functions for creating the time and date
 *
 * yearday_get will return the day of the year
 * from the passed in data of year and day of month
 * */
static uint32_t yearday_get(rtc_time_t * p_time)
{
    uint8_t month_count;
    uint32_t year_day = 0;

    /* Add the number of elapsed months days together */
    /* For example, if the month is June, add Jan(31) + Feb(28) + Mar(31) + April(30) + May(31) */
    for( month_count=0; month_count<(p_time->tm_mon); month_count++ )
    {
        year_day += days_in_month[month_count];
    }

    /* For example, if the day is June 26
     * Add 26 to this total */
    year_day += (uint32_t) p_time->tm_mday;

    /* If we are in a leap year, and the month has passed Feb, add an extra day to the total */
    bool leap = false;

    if (0 == p_time->tm_year % 4)
    {
        leap = true;
        if (0 == p_time->tm_year % 100)
        {
            leap = false;
            if (0 != p_time->tm_year % 400)
            {
                leap = true;
            }
        }
    }

    if ((true == leap) && (p_time->tm_mon > 1))
    {
        year_day++;
    }

    return (year_day);
}


