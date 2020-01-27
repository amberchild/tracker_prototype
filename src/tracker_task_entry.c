#include "tracker_task.h"
#include "uart0_receive_thread.h"
#include "bme280.h"
#include "bmx055.h"
#include "math.h"
#include "util.h"
#include "common.h"
#include "user_rtc.h"
#include "config_s3a7.h"
#include "StringCommandParser.h"
#include "stdlib.h"
#include "stdio.h"
#include "sms_commands.h"

#define MAX_LONGITUDE               180
#define MAX_LATITUDE                90
#define CMD_BUFF_LENGTH             1024
#define SESSIONID_LENGTH            24
#define TELIT_BATT_ADC_COEFF        0.281       /*"Telit ADC1" battery voltage divider ratio*/
#define SHUT_DOWN_VOLTAGE           3333        /*System shut down voltage mV for battery saving/protection*/
#define GNSS_NSAT                   5           /*GNSS satellites number threshold */
#define GNSS_SAT_WAIT               300         /*GNSS signal wait time in seconds*/
#define SMS_CMD                     9           /*SMS commands used*/
#define GNSS_OFF_LITMIT             1800        /*Longest time interval in seconds when GNSS module will not be shut down*/
#define INIT_FAIL_WAKE_UP           900         /*Wake up interval after initialisation failure*/
#define KEEP_AWAKE_TIME             750         /*Keep awake time */
#define KEEP_AWAKE_TIME_EXT         3600        /*Keep awake time extended*/
#define KEEP_ONLINE_TIME            900         /*Keep online time*/
#define SLEEP_DURATION              3600        /*Sleep duration after online time expired*/
#define GPRS_CONNECT_RETRY          3           /*GPRS connect attempts*/

/*SMS with google maps link template*/
const char google_link[] ="http://maps.google.com/maps?q=%s,%s";

/*Telit Portal Authentication Templates*/
const char fcmd_HTTPPOST[] ="POST /api HTTP/1.0\r\nHost: api-de.devicewise.com\r\nContent-Type: application/json\r\nContent-Length:";
const char fcmd_dW_auth[]  = "{\"auth\":{\"command\":\"api.authenticate\",\"params\":{\"appToken\":\"%s\",\"appId\":\"%s\",\"thingKey\":\"%s\"}}}\r\n";
const char fcmd_dw_post_auth[]  = "{\"auth\":{\"sessionId\":\"%s\"},";

/*Telit Portal Post Templates*/
const char fcmd_dw_post_p1[]  = "\"1\":{\"command\":\"property.publish\",\"params\":{\"thingKey\":\"%s\",\"key\":\"temperature\",\"value\":%.2f}},";
const char fcmd_dw_post_p2[]  = "\"2\":{\"command\":\"property.publish\",\"params\":{\"thingKey\":\"%s\",\"key\":\"pressure\",\"value\":%.2f}},";
const char fcmd_dw_post_p3[]  = "\"3\":{\"command\":\"property.publish\",\"params\":{\"thingKey\":\"%s\",\"key\":\"humidity\",\"value\":%.2f}},";
const char fcmd_dw_post_p4[]  = "\"4\":{\"command\":\"property.publish\",\"params\":{\"thingKey\":\"%s\",\"key\":\"mag_xy\",\"value\":%d}},";
const char fcmd_dw_post_p5[]  = "\"5\":{\"command\":\"property.publish\",\"params\":{\"thingKey\":\"%s\",\"key\":\"batt_voltage\",\"value\":%d}},";
const char fcmd_dw_post_p6[]  = "\"6\":{\"command\":\"property.publish\",\"params\":{\"thingKey\":\"%s\",\"key\":\"signal_level\",\"value\":%d}},";
const char fcmd_dw_post_p7[]  = "\"7\":{\"command\":\"location.publish\",\"params\":{\"thingKey\":\"%s\",\"lat\":%s,\"lng\":%s,\"altitude\":%s,\"heading\":%s,\"speed\":%s,\"fixType\":\"manual\",\"fixAcc\":%s}}}}\r\n";

/*App ID and tokens*/
const char telit_appID[] = "5b542754447cfb36414b9b26";
const char telit_appToken[] = "SFBhqftn43LdjcrF";
char telit_sessionId[25];

/*SMS commands structure*/
typedef struct
 {
  char *Command;
  void (*OnExecute)( const char *pString );
 } tSMSproc;

/* LED type structure */
bsp_leds_t leds;

/*Modem and GNSS Power and Reset control pins*/
ioport_port_pin_t cell_reset = CELL_RESET_CTRL_PIN;
ioport_port_pin_t gnss_reset = GNSS_RESET_CTRL_PIN;
ioport_port_pin_t cell_pwr = CELL_POWER_CTRL_PIN;
ioport_port_pin_t gnss_pwr = GNSS_POWER_CTRL_PIN;
ioport_port_pin_t charger_ctrl = CHARGER_CTRL_PIN;
ioport_port_pin_t sensor_pwr = SENS_POWER_CTRL_PIN;

char g_sms_buff[100];
char incoming_number[20];
extern TSCPHandler   SCPHandler;
_Bool alarm_flag;
uint8_t sms_received = 0;
char post_buff[CMD_BUFF_LENGTH];
char post_length[16];
extern uint8_t led_mode;

/*Modem data structure */
modem_data_storage_t modem_data;

/*Sensors data structure */
sensors_data_storage_t sensors_data;

/*Sensors coefficients storage*/
sensors_coefficients_t sensors_coeff;

/*Tracker settings storage*/
tracker_settings_t tracker_settings;

/*SMS commands structure initialisation*/
static const tSMSproc SMScomands[ SMS_CMD ] =
{
 {"normal", NormalModeConfig }, /*Command for entering normal mode. Always on, may send certain amount of SMS in intervals. Etc.: normal60t10 */
 {"rtc", RTCModeConfig },       /*Command for entering RTC mode. RTC timer is used to wake up from Standby mode. Sends SMS in intervals. Etc.: rtc300*/
 {"acc", ACCModeConfig },       /*Command for entering ACC mode. Accelerometer interrupt is used (P304 IRQ9). Sends SMS every time waken. Etc.: acc10*/
 {"memnum", MemNumConfig },     /*Command for memorising the SMS recipients number. Send SMS with memnum to memorise yours number */
 {"cloud", CloudModeConfig},    /*Command for entering upload-to-cloud mode. Always on, sends certain amount of data packets in intervals: cloud60t60*/
 {"APN", APNConfig},
 {"AppID", AppIDConfig},
 {"AppToken", AppTokenConfig},
 {"help", HelpSMS},
};

/*Tick function for AT engine provided here*/
void g_modem_timer_callback(timer_callback_args_t * p_args)
{
    UNUSED(p_args);
    /*Execute AT parser tick function 10ms intervals*/
    SCP_Tick(10);
}

/*RTC Alarm interrupt*/
void rtc_alarm_callback(rtc_callback_args_t * p_args)
{
    UNUSED(p_args);

    /*Alarm event for intervals generation in normal mode*/
    alarm_flag = true;
}

/*Button interrupt IRQ0*/
void button_callback(external_irq_callback_args_t *p_args)
{
    UNUSED(p_args);
}

/*Accelerometer interrupt IRQ9*/
void accelerometer_callback(external_irq_callback_args_t *p_args)
{
    UNUSED(p_args);

    /*Movement update*/
    sensors_data.last_movement = user_function_get_rtc_time();
}

/*Charger Cable interrupt IRQ13*/
void detect_charger_callback(external_irq_callback_args_t *p_args)
{
    UNUSED(p_args);
}

/*Specific string copy function. Copies only digits and dots*/
static void strxcpy(char *dest, char *src)
{
    int limit;

    limit = 10;

    while(((*src >= '0') && (*src <= '9')) || (*src == '.') )
    {
        while(*src == ' ')
        {
            src++;
            limit--;

            if(limit <= 0)
            {
                break;
            }
        }

        *dest = *src;
        dest++;
        src++;
        limit--;

        if(limit <= 0)
        {
            break;
        }
    }
}

/*UART send function for AT commands parser*/
uint32_t uart_send_buff(uint8_t *data_out, uint32_t size)
{
    return g_sf_modem_uart0.p_api->write(g_sf_modem_uart0.p_ctrl, data_out, size, TX_WAIT_FOREVER);
}

/*UART receive function for AT commands parser*/
uint32_t uart_read_byte(uint8_t *pData)
{
    return g_sf_modem_uart0.p_api->read(g_sf_modem_uart0.p_ctrl, pData, 1, TX_WAIT_FOREVER);
}

/*Convert latitude and longitude from NMEA to decimal */
static double nmea2dec(char *nmea, char type, char *dir)
{
    unsigned int idx, dot = 0;
    double dec = 0;
    for (idx=0; idx<strlen(nmea);idx++){
        if (nmea[idx] == '.')
        {
            dot = idx;
            break;
        }
    }

    if ((dot < 3) || (dot > 5))
    {
        return 0;
    }

    int dd;
    double mm;
    char cdd[5], cmm[10];
    memset(cdd, 0, sizeof(cdd));
    memset(cmm, 0, sizeof(cmm));
    if(type == 1)
    {
        strncpy(cdd, nmea, dot-2);
        strncpy(cmm, nmea+dot-2, 7);
    }
    if(type == 2)
    {
        strncpy(cdd, nmea, dot-2);
        strncpy(cmm, nmea+dot-2, 7);
    }

    dd = atoi(cdd);
    mm = atof(cmm);

    dec = dd + (mm/60);

    if (type == 1 && dec > MAX_LATITUDE)
        return 0;
    else if (type == 2 && dec > MAX_LONGITUDE)
        return 0;

    if (*dir == 'N' || *dir == 'E')
      return dec;
    else
      return -1 * dec;
}

static inline void ModemReset(void)
{
    /*Turn on Reset MOSFET*/
    g_ioport.p_api->pinWrite(cell_reset, IOPORT_LEVEL_HIGH);

    /*Keep it on for a 200ms as required*/
    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND/5));


    /*Turn off Reset MOSFET*/
    g_ioport.p_api->pinWrite(cell_reset, IOPORT_LEVEL_LOW);

    /*wait for a 1000ms as required*/
    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND));
}

static inline void ModemOn(void)
{
    /*Turn on power for GSM Modem*/
    g_ioport.p_api->pinWrite(cell_pwr, IOPORT_LEVEL_HIGH);
}

static inline void ModemOff(void)
{
    /*Turn off power for GSM Modem*/
    g_ioport.p_api->pinWrite(cell_pwr, IOPORT_LEVEL_LOW);
}

static inline void GNSS_On(void)
{
    /*Turn on power for GNSS module*/
    g_ioport.p_api->pinWrite(gnss_pwr, IOPORT_LEVEL_HIGH);
}

static inline void GNSS_Off(void)
{
    /*Turn off power for GNSS module*/
    g_ioport.p_api->pinWrite(gnss_pwr, IOPORT_LEVEL_LOW);
}

static inline void Charger_On(void)
{
    /*Turn on Li-ion Battery Charger*/
    g_ioport.p_api->pinWrite(charger_ctrl, IOPORT_LEVEL_LOW);
}

static inline void Charger_Off(void)
{
    /*Turn off Li-ion Battery Charger*/
    g_ioport.p_api->pinWrite(charger_ctrl, IOPORT_LEVEL_HIGH);
}

static inline void sensors_power_on(void)
{
    /*Turn on power for the sensors*/
    g_ioport.p_api->pinWrite(sensor_pwr, IOPORT_LEVEL_HIGH);
}

static inline void sensors_power_off(void)
{
    /*Turn on power for the sensors*/
    g_ioport.p_api->pinWrite(sensor_pwr, IOPORT_LEVEL_LOW);
}

static void GSM_init( void )
{
    /*Release reset pins*/
    g_ioport.p_api->pinWrite(cell_reset, IOPORT_LEVEL_LOW);
    g_ioport.p_api->pinWrite(gnss_reset, IOPORT_LEVEL_LOW);

    /*Reset the Modem*/
    ModemReset();

    /*Open and start hardware timer for AT parser ticks*/
    g_modem_timer.p_api->open(g_modem_timer.p_ctrl,g_modem_timer.p_cfg);
    g_modem_timer.p_api->start(g_modem_timer.p_ctrl);
}

static void GSM_deinit( void )
{
    /*Stop hardware timer for AT parser ticks*/
    g_modem_timer.p_api->stop(g_modem_timer.p_ctrl);
    g_modem_timer.p_api->close(g_modem_timer.p_ctrl);

    ModemOff();
    modem_data.gsm_pwr_status = false;

    /*Shut Down GNSS module only if intervals are longer than GNSS_OFF_LITMIT in RTC mode*/
    if(tracker_settings.mode == POWER_SAVING_RTC)
    {
        if(tracker_settings.RTC_interval > GNSS_OFF_LITMIT)
        {
            GNSS_Off();
            modem_data.gnss_pwr_status = false;
        }
    }
}

/*Low power module callback*/
void lpm_callback(sf_power_profiles_v2_callback_args_t *p_args)
{
    /*Prepare to go to Low Power Mode*/
    if(p_args->event == SF_POWER_PROFILES_V2_EVENT_PRE_LOW_POWER)
    {
        g_ioport.p_api->pinWrite(leds.p_leds[0], IOPORT_LEVEL_HIGH);
        GSM_deinit();
        /*Do not suspend BMX055 in movement detection mode*/
        if(tracker_settings.mode == POWER_SAVING_RTC)
        {
            bmx055_deinit();
            //sensors_power_off();
        }
        bme280_deinit();
    }

    /*Returning from Low Power Mode*/
    if(p_args->event == SF_POWER_PROFILES_V2_EVENT_POST_LOW_POWER)
    {
        /*Restart*/
        NVIC_SystemReset();
    }
}

/*Incoming Call Function*/
static void IncomingCall(const char *pString)
{
    UNUSED(pString);
    char *result = NULL;

    /* Wait 50ms */
    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND/20));

      /**/
      result = strstr((char*)SCPHandler.RxBuffer, "+CLIP: ");
      /*Response found, lets look for operator string, begins with (") */
      if(result)
      {
          result = strchr(result, '"');

          /*Copy operator to the RAM and return*/
          if(result)
          {
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

      /*Hang up*/
      result = SCP_SendCommandWaitAnswer("ATH\r", "OK", 2000, 1);

      if(result)
      {
          /*Save the number with whole structure to Data flash*/
          TrackerSettingsSave();
      }
}

/*Incoming SMS Function*/
static void IncomingSMS(const char *pString)
{
    UNUSED(pString);

    /*Increase SMS counter*/
    sms_received++;
}

/*Returns network registration status*/
static int32_t NetworkRegistrationCheck(void)
{
    char *result = NULL;
    int32_t ntwrk_stat = 0;

    /*Request network status info*/
    result = SCP_SendCommandWaitAnswer("AT+CREG?\r", "OK", 2000, 1);

    if(result)
    {
        result = NULL;
        result = strstr((char*)SCPHandler.RxBuffer, "+CREG: ");
        if(result)
        {
            result += 9;
            ntwrk_stat = atoi(result);
        }
    }
    else
    {
        return 0;
    }

    return ntwrk_stat;
}

/*Returns GPRS status*/
static int32_t GPRS_StatusCheck(void)
{
    char *result = NULL;
    int32_t gprs_stat = 0;

    /*Request network status info*/
    result = SCP_SendCommandWaitAnswer("AT#SGACT?\r", "OK", 2000, 1);

    if(result)
    {
        result = NULL;
        result = strstr((char*)SCPHandler.RxBuffer, "#SGACT: ");
        if(result)
        {
            result += 10;
            gprs_stat = atoi(result);
        }
    }
    else
    {
        return 0;
    }

    return gprs_stat;
}

/*Connect GPRS, returns true and IP address if succeeded, must have 15 bytes allocated*/
static _Bool GPRS_Connect(char *ip_address)
{
    char *result = NULL;
    char *temp = NULL;

    temp = ip_address;

    /*Request network status info*/
    result = SCP_SendCommandWaitAnswer("AT#SGACT=1,1\r", "OK", 2000, 1);

    if(result)
    {
        result = NULL;
        result = strstr((char*)SCPHandler.RxBuffer, "#SGACT: ");
        result += 8;
        if(result)
        {
            memset(ip_address, 0x00, 15);

            /*Maximum 15 chars for IP address*/
            for(uint8_t i = 0; i < 15; i++)
            {
                /*Not a number or dot in IP address shall be treated as error*/
                if(!(*result > 47 && *result < 58) && !(*result == '.') && (!(*result == '\r')))
                {
                    memset(ip_address, 0x00, 15);
                    break;
                }

                *temp = *result;
                result++;
                temp++;

                /*The end of the string*/
                if(*result == '\r')
                {
                    break;
                }
            }

            return true;
        }
    }

    return false;
}

/*Waits for network available, suspends actual task*/
static _Bool WaitForNetwork(void)
{
    int32_t test = 0;

    /*Wait 5 minutes for network*/
    for(int i = 300; i > 0; i--)
    {
        /*Get current network state*/
        test = NetworkRegistrationCheck();

        /*registered, home network  or  registered, roaming is acceptable*/
        if((test == 1) || (test == 5))
        {
            return true;
        }

        else
        {
            /* Wait 1000ms */
            tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND));
        }
    }

    return false;
}

/*Returns ADC1 battery voltage*/
static int32_t MeasureBattVoltage(void)
{
    int32_t voltage_mv = 0;
    char *result = NULL;

    /*Request ADC1*/
    result = SCP_SendCommandWaitAnswer("AT#ADC=1,2\r", "OK", 5000, 1);

    if(result)
    {
        result = NULL;
        result = strstr((char*)SCPHandler.RxBuffer, "#ADC:");
        if(result)
        {
            result += 6;
            voltage_mv = atoi(result);
            voltage_mv = (int32_t)(voltage_mv/TELIT_BATT_ADC_COEFF);
        }
    }
    else
    {
        return 0;
    }

    return voltage_mv;
}

/*Returns received signal quality */
static int32_t SignalQuality(void)
{
    int32_t signal_level = 0;
    char *result = NULL;

    /*Request RSSI*/
    result = SCP_SendCommandWaitAnswer("AT+CSQ\r", "OK", 2000, 1);

    if(result)
    {
        result = NULL;
        result = strstr((char*)SCPHandler.RxBuffer, "+CSQ:");
        if(result)
        {
            result += 6;
            signal_level = atoi(result);
        }
    }
    else
    {
        return 0;
    }

    return signal_level;
}

/*Returns pointer to operator string*/
static char* GetOperator(void)
{
    char *result = NULL;
    static char operator[17];

    /*Request operator*/
    result = SCP_SendCommandWaitAnswer("AT+COPS?\r", "OK", 30000, 1);

    /*We have response, lets look for the info in the receiver buffer*/
    if(result)
    {
        result = NULL;
        result = strstr((char*)SCPHandler.RxBuffer, "+COPS:");

        /*Response found, lets look for operator string, begins with (") */
        if(result)
        {
            result = strchr(result, '"');

            /*Copy operator to the RAM and return*/
            if(result)
            {
                /*Clean static buffer*/
                memset(operator, 0x00, 17);

                /*Maximum 16 chars for operator initials allowed*/
                for(uint8_t i = 0; i < 16; i++)
                {
                    operator[i] = *result;
                    result++;

                    /*Last operator char?*/
                    if(*result == '"')
                    {
                        i++;
                        operator[i] = *result;
                        return operator;
                    }
                }

                return operator;
            }
        }
    }

    return NULL;
}
/*Provider change procedure*/
static _Bool ChangeProvider(void)
{
    char *result = NULL;
    char provider[17];

    /* Set the behavior of +COPS command */
   // result = SCP_SendCommandWaitAnswer("AT#COPSMODE=1\r", "OK", 30000, 1);

    /* Send command to check what networks are available */
    result = SCP_SendCommandWaitAnswer("AT+COPS=?\r", "OK", 30000, 1);

    /*We have response, lets look for the info in the receiver buffer*/
    if(result)
    {
        result = NULL;
        result = strstr((char*)SCPHandler.RxBuffer, "+COPS:");
        if(result)
        {
            for(;;)
            {
                search:
                result = strchr(result, '(');
                if(result)
                {
                    result +=1;
                    if(*result == '1')
                    {
                        /*Provider found, copy the name*/
                        result = strchr(result, '"');

                        if(result)
                        {
                            /*Clean buffer*/
                            memset(provider, 0x00, 17);

                            /*Maximum 16 chars for operator initials allowed*/
                            for(uint8_t i = 0; i < 16; i++)
                            {
                                provider[i] = *result;
                                result++;

                                /*Last operator char?*/
                                if(*result == '"')
                                {
                                    i++;
                                    provider[i] = *result;

                                    /*Form command*/
                                    memset(post_buff, 0, sizeof(post_buff));
                                    sprintf(post_buff, "AT+COPS=1,0,%s\r",provider);
                                    result = SCP_SendCommandWaitAnswer(post_buff, "OK", 10000, 1);
                                    if(result)
                                    {
                                        return true;
                                    }
                                    else
                                    {
                                        return false;
                                    }
                                }
                            }

                        }

                        else
                        {
                            return false;
                        }
                    }
                    else
                    {
                        goto search;
                    }
                }
                else
                {
                    break;
                }

            }
        }

    }

    return false;
}

/*Returns pointer to IMEI string of 15 numbers*/
static char* GetIMEI(void)
{
    char *result = NULL;
    static char imei[16];
    _Bool isDigit = false;
    uint32_t j = 0, i=0;

    /*Request IMEI*/
    result = SCP_SendCommandWaitAnswer("AT+CGSN\r", "OK", 100, 1);

    /*We have response, lets look for the info in the receiver buffer*/
    if(result)
    {
        result = NULL;

        /*Lets look for a ASCII number...*/
        while((j < strlen((char*)SCPHandler.RxBuffer)) && (!isDigit))
        {
          if((SCPHandler.RxBuffer[j] > 47) && (SCPHandler.RxBuffer[j] < 58))
          {
              isDigit = true;
              result = (char*)&SCPHandler.RxBuffer[j];
              break;
          }

          j++;
        }

        /*First number of IMEI found, copy the number to the RAM and return */
        if(result)
        {
            memset(imei, 0x00, 16);

            /*Maximum 15 chars for IMEI is allowed*/
            for(i = 0; i < 15; i++)
            {
                /*Not a number in IMEI shall be treated as error*/
                if(!(*result > 47 && *result < 58))
                {
                    return NULL;
                }

                imei[i] = *result;

                result++;
            }

            return imei;
        }
    }

    return NULL;
}

/*Request model identification*/
static char* GetID(void)
{
    char *result = NULL;
    static char device_id[21];

    /*Request model identification*/
    result = SCP_SendCommandWaitAnswer("AT#CGMM\r", "OK", 100, 1);

    /*We have response, lets look for the info in the receiver buffer*/
    if(result)
    {
        result = NULL;
        result = strstr((char*)SCPHandler.RxBuffer, "#CGMM: ");

        if(result)
        {
            result += 7;

            /*Copy operator to the RAM and return*/
            memset(device_id, 0x00, 21);

            /*Maximum 20 chars for model identification allowed*/
            for(uint8_t i = 0; i < 20; i++)
            {
                device_id[i] = *result;
                result++;

                /*Device_id end*/
                if(*result == '\r')
                {
                    return device_id;
                }
            }
        }

    }

    return NULL;
}

/*Returns pointer to firmware version*/
static char* GetVersion(void)
{
    char *result = NULL;
    static char version[16];
    uint32_t i=0;

    /*Request IMEI*/
    result = SCP_SendCommandWaitAnswer("AT+CGMR\r", "OK", 100, 1);

    /*We have response, lets look for the info in the receiver buffer*/
    if(result)
    {
        result = NULL;

        /*Lets look for a (\n) char as the begining of firmware version string*/
        result = strchr((char*)SCPHandler.RxBuffer, '\n');

        /*Copy string to RAM */
        if(result)
        {
            result++;
            memset(version, 0x00, 16);

            /*Maximum 15 chars limit*/
            for(i = 0; i < 15; i++)
            {

                version[i] = *result;
                result++;

                if(*result == '\r')
                {
                    break;
                }
            }

            return version;
        }
    }

    return NULL;
}

/*Writes firmware version to given pointer, must have 50 bytes allocated*/
static _Bool GetGPSVersion(char *version)
{
    char *result = NULL;
    char *temp = NULL;

    temp = version;

    /*Request GPS module firmware version*/
    result = SCP_SendCommandWaitAnswer("AT$GPSSW\r", "OK", 30000, 1);

    if(result)
    {
        result = NULL;
        result = strstr((char*)SCPHandler.RxBuffer, "$GPSSW: ");
        result += 8;
        if(result)
        {
            memset(version, 0x00, 50);

            /*Maximum 49 chars for firmware version*/
            for(uint8_t i = 0; i < 49; i++)
            {
                *temp = *result;
                result++;
                temp++;

                /*The end of the string*/
                if(*result == '\r')
                {
                    break;
                }
            }

            return true;
        }
    }

    return false;
}

/*Read GPS properties*/
static _Bool GetGPSLocation( char *utc, char *date, char *lat, char *lon, char *alt, char *hdop, char *cog, char *speed, char *nsat)
{
    char *result = NULL;
    char *dir = NULL;
    double coordinate = 0;

    /*Request GPS data*/
    result = SCP_SendCommandWaitAnswer("AT$GPSACP\r", "OK", 2000, 1);

    /*We have response, lets look for the info in the receiver buffer*/
    if(result)
    {
        result = NULL;
        result = strstr((char*)SCPHandler.RxBuffer, "$GPSACP: ");
        result +=9;
        if(result)
        {
            strxcpy(utc, result);
            result = strchr(result, ',');
            result++;
            if(result)
            {
                dir = strchr(result, 'N');
                if(!dir)
                {
                    dir = strchr(result, 'S');
                }
                coordinate = nmea2dec(result, 1, dir);
                sprintf(lat, "%.6f", coordinate);
                result = strchr(result, ',');
                result++;
                if(result)
                {
                    dir = strchr(result, 'E');
                    if(!dir)
                    {
                        dir = strchr(result, 'W');
                    }
                    coordinate = nmea2dec(result, 2, dir);
                    sprintf(lon, "%.6f", coordinate);
                    result = strchr(result, ',');
                    result++;
                    if(result)
                    {
                        strxcpy(hdop, result);
                        result = strchr(result, ',');
                        result++;
                        if(result)
                        {
                            strxcpy(alt, result);
                            result = strchr(result, ',');
                            result++;
                            result = strchr(result, ',');
                            result++;
                            if(result)
                            {
                                strxcpy(cog, result);
                                result = strchr(result, ',');
                                result++;
                                if(result)
                                {
                                    strxcpy(speed, result);
                                    result = strchr(result, ',');
                                    result++;
                                    result = strchr(result, ',');
                                    result++;
                                    if(result)
                                    {
                                        strxcpy(date, result);
                                        result = strchr(result, ',');
                                        result++;
                                        if(result)
                                        {
                                            strxcpy(nsat, result);
                                        }

                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

static void read_gnss(void)
{
    /*Clean temporary and global GNSS data storages*/
    char temp_utc[16];
    char temp_dat[16];
    char temp_lat[16];
    char temp_lon[16];
    char temp_alt[16];
    char temp_pre[16];
    char temp_cou[16];
    char temp_spe[16];
    char temp_nsa[16];
    memset(temp_utc,0x00,sizeof(temp_utc));
    memset(temp_dat,0x00,sizeof(temp_dat));
    memset(temp_lat,0x00,sizeof(temp_lat));
    memset(temp_lon,0x00,sizeof(temp_lon));
    memset(temp_alt,0x00,sizeof(temp_alt));
    memset(temp_pre,0x00,sizeof(temp_pre));
    memset(temp_cou,0x00,sizeof(temp_cou));
    memset(temp_spe,0x00,sizeof(temp_spe));
    memset(temp_nsa,0x00,sizeof(temp_nsa));
    memset(modem_data.gps_utc,0x00,sizeof(modem_data.gps_utc));
    memset(modem_data.gps_date,0x00,sizeof(modem_data.gps_date));
    memset(modem_data.gps_latitude,0x00,sizeof(modem_data.gps_latitude));
    memset(modem_data.gps_longitude,0x00,sizeof(modem_data.gps_longitude));
    memset(modem_data.gps_altitude,0x00,sizeof(modem_data.gps_altitude));
    memset(modem_data.gps_precision,0x00,sizeof(modem_data.gps_precision));
    memset(modem_data.gps_course,0x00,sizeof(modem_data.gps_course));
    memset(modem_data.gps_speed,0x00,sizeof(modem_data.gps_speed));
    memset(modem_data.gps_satt_in_use,0x00,sizeof(modem_data.gps_satt_in_use));

    /*Read GNSS data*/
    modem_data.gnss_online = GetGPSLocation(temp_utc,temp_dat,temp_lat,temp_lon,temp_alt,temp_pre,temp_cou,temp_spe,temp_nsa);

    /*Save data to RAM*/
    strcpy(modem_data.gps_utc, temp_utc);
    strcpy(modem_data.gps_date, temp_dat);
    strcpy(modem_data.gps_latitude, temp_lat);
    strcpy(modem_data.gps_longitude, temp_lon);
    strcpy(modem_data.gps_altitude, temp_alt);
    strcpy(modem_data.gps_precision, temp_pre);
    strcpy(modem_data.gps_course, temp_cou);
    strcpy(modem_data.gps_speed, temp_spe);
    strcpy(modem_data.gps_satt_in_use, temp_nsa);
}

/*Request AGPS location*/
static _Bool RequestAGPSLocation(void)
{
    char *result = NULL;

    /*Request AGPS data*/
    result = SCP_SendCommandWaitAnswer("AT#AGPSSND\r", "OK", 120000, 1);

    /*We have response*/
    if(result)
    {
        return true;
    }


    return false;
}

static void GetAGPSLocation(const char *pString)
{

    uint8_t retry = 5;
    uint8_t i;
    char *result = NULL;

    /*#AGPSRING: Received, wait for the end of the line to arrive..*/
    for(i = 0; i < retry; i++)
    {
        /* Wait 20ms */
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND/50));

        result = strchr(pString, '\r');

        if(result)
        {
            result = strchr(pString, ',');
            result++;
            if(result)
            {
                memset(modem_data.gps_latitude, 0x00, sizeof(modem_data.gps_latitude));
                strxcpy(modem_data.gps_latitude, result);
                result = strchr(result, ',');
                result++;

                if(result)
                {
                    memset(modem_data.gps_longitude, 0x00, sizeof(modem_data.gps_longitude));
                    strxcpy(modem_data.gps_longitude, result);
                    result = strchr(result, ',');
                    result++;

                    if(result)
                    {
                        memset(modem_data.gps_altitude, 0x00, sizeof(modem_data.gps_altitude));
                        strxcpy(modem_data.gps_altitude, result);
                        result = strchr(result, ',');
                        result++;

                        if(result)
                        {
                            memset(modem_data.gps_precision, 0x00, sizeof(modem_data.gps_precision));
                            strxcpy(modem_data.gps_precision, result);

                            /*Delete message*/
                            result = strstr(pString, "#AGPSRING");
                            memset(result, ' ', strlen("#AGPSRING"));

                            /*Place dummy data to be able to upload to cloud*/
                            memset(modem_data.gps_altitude, 0x00, sizeof(modem_data.gps_altitude));
                            strxcpy(modem_data.gps_altitude, "0.0");
                            memset(modem_data.gps_course, 0x00, sizeof(modem_data.gps_course));
                            strxcpy(modem_data.gps_course, "0.0");
                            memset(modem_data.gps_speed, 0x00, sizeof(modem_data.gps_speed));
                            strxcpy(modem_data.gps_speed, "0.0");
                        }
                    }
                }
            }
            /*Data arrived...*/
            modem_data.data_request = false;
            led_mode = ONLINE_IDLE;
            break;
        }
    }
}

static void EphemerisUpdate(const char *pString)
{
    char *result = NULL;
    _Bool gps_update = false;
    _Bool glonass_update = false;
    uint8_t gprs_recon_counter = 0;
    char file[10];

    /* Wait 1000ms */
    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND));

    /*Check what URC request do we have?*/
    result = strstr(pString, "GPS SGEE File Update Requested");
    if(result) {gps_update = true;}
    result = NULL;
    result = strstr(pString, "GLONASS SGEE File Update Requested");
    if(result) {glonass_update = true;}

    /*GPS/GLONASS Ephemeris Request Received*/
    if(gps_update || glonass_update)
    {
        /*Check Network Status*/
        modem_data.network_status = NetworkRegistrationCheck();

        /*registered to home network or registered as roaming is acceptable*/
        if((modem_data.network_status == 1) || (modem_data.network_status == 5))
        {
            /*Store operator*/
            result = NULL;
            result = GetOperator();
            if(result)
            {
                memset(modem_data.operator, 0x00, 17);
                strcpy(modem_data.operator, result);
            }

            /*Check GPRS status*/
            gprs_check:
            modem_data.gprs_status = GPRS_StatusCheck();

            /*Try to connect if not connected*/
            if(!modem_data.gprs_status)
            {
                for(gprs_recon_counter = 5; gprs_recon_counter > 0; gprs_recon_counter--)
                {
                    modem_data.gprs_status = GPRS_Connect(modem_data.ip_address);

                    /*Count how many times tried to connect*/
                    if(modem_data.gprs_status)
                    {
                        goto gprs_connected;
                    }

                    /* Wait 1000ms */
                    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND));
                }

                /*Roaming and no GPRS. Retry few times and try to change network*/
                if(!modem_data.gprs_status && (modem_data.network_status == 5))
                {
                    /*change provider*/
                    if(ChangeProvider())
                    {
                        /*Check if we are connected to GSM network*/
                        WaitForNetwork();
                        goto gprs_check;
                    }
                }
            }
            /*We are connected to GPRS*/
            else
            {
                gprs_connected:

                if(gps_update)
                {
                    SCP_InitRx();
                    result = NULL;
                    result = SCP_SendCommandWaitAnswer("AT#HTTPCFG=0,\"agps.telit.com\",80,1,\"oXnk4wCb\",\"fPcdgS6w\",0,120,1\r", "OK", 2000, 1);
                    if(result) result = SCP_SendCommandWaitAnswer("AT#HTTPQRY=0,0,\"/gps/sifgps.f2p1enc.ee\"\r", "OK", 5000, 1);
                    if(result) result = SCP_WaitForAnswer("#HTTPRING:", 15000);
                    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND));
                    result += 35;
                    if(result)
                    {
                        memset(file, 0x00, sizeof(file));
                        memset(post_buff, 0, sizeof(post_buff));
                        for(uint8_t i = 0; i < sizeof(file); i++)
                        {
                            file[i] = *result;
                            result++;

                            /*Last operator char?*/
                            if(*result == '\r')
                            {
                                break;
                            }
                        }
                        sprintf(post_buff,"AT$HTTPGETIFIX=0,%s\r", file);
                        result = SCP_SendCommandWaitAnswer(post_buff, "OK", 30000, 1);
                    }
                }

                if(glonass_update)
                {
                    SCP_InitRx();
                    result = NULL;
                    result = SCP_SendCommandWaitAnswer("AT#HTTPCFG=0,\"agps.telit.com\",80,1,\"oXnk4wCb\",\"fPcdgS6w\",0,120,1\r", "OK", 2000, 1);
                    if(result) result = SCP_SendCommandWaitAnswer("AT#HTTPQRY=0,0,\"/glo/sifglo.f2p1enc.ee\"\r", "OK", 5000, 1);
                    if(result) result = SCP_WaitForAnswer("#HTTPRING:", 15000);
                    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND));
                    result += 35;
                    if(result)
                    {
                        memset(file, 0x00, sizeof(file));
                        memset(post_buff, 0, sizeof(post_buff));
                        for(uint8_t i = 0; i < sizeof(file); i++)
                        {
                            file[i] = *result;
                            result++;

                            /*Last operator char?*/
                            if(*result == '\r')
                            {
                                break;
                            }
                        }
                        sprintf(post_buff,"AT$HTTPGETIFIX=0,%s,1\r", file);
                        result = SCP_SendCommandWaitAnswer(post_buff, "OK", 30000, 1);
                    }
                }
            }
        }
    }
}

static void sms_parser(void)
{
    char *result = NULL;
    int i;

    if(sms_received != 0)
    {
        /*Read all messages*/
        result = SCP_SendCommandWaitAnswer("AT+CMGL=\"ALL\"\r", "OK", 5000, 1);
        if(result)
        {
            /*Search for declared SMS command and execute it*/
            for (i=0; i<SMS_CMD; i++)
             {
                result = NULL;
                result = strstr( (char*)SCPHandler.RxBuffer, SMScomands[i].Command );
                if ((result) && (SMScomands[i].OnExecute))
                {
                    SMScomands[i].OnExecute( result );
                }
             }
        }

        /*Must wait 5000ms*/
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));

        /*Delete all stored messages*/
        SCP_SendCommandWaitAnswer("AT+CMGD=1,4\r", "OK", 5000, 1);
        sms_received = 0;
    }
}


static void send_sms(void)
{
    char * scp_result = NULL;
    char local_buff[50];

    /*Network Status*/
    modem_data.network_status = NetworkRegistrationCheck();

    /*registered to home network or registered as roaming is acceptable*/
    if((modem_data.network_status == 1) || (modem_data.network_status == 5))
    {
        /*Store operator*/
        scp_result = NULL;
        scp_result = GetOperator();
        if(scp_result)
        {
            memset(modem_data.operator, 0x00, 17);
            strcpy(modem_data.operator, scp_result);
        }

        if(atoi(modem_data.gps_satt_in_use) >= 5)
        {
            memset(g_sms_buff, 0x00, sizeof(g_sms_buff));
            memset(local_buff, 0x00, sizeof(local_buff));

            sprintf(g_sms_buff, google_link, modem_data.gps_latitude, modem_data.gps_longitude);

            /*Termination symbol*/
            strcat(g_sms_buff, "\032");

            sprintf(local_buff, "AT+CMGS=%s\r", tracker_settings.phone_number);
            SCP_SendDoubleCommandWaitAnswer(local_buff, g_sms_buff, ">", "OK", 2000, 1);

            /*Wait 5000ms for modem to finish send SMS*/
            tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));
        }

        memset(g_sms_buff, 0x00, sizeof(g_sms_buff));
        memset(local_buff, 0x00, sizeof(local_buff));

        if(atoi(modem_data.gps_satt_in_use) <= 4)
        {
            strcat(g_sms_buff, "No Satellites ");
        }

        else
        {
            strcat(g_sms_buff, modem_data.gps_latitude);
            strcat(g_sms_buff, ", ");
            strcat(g_sms_buff, modem_data.gps_longitude);
            strcat(g_sms_buff, ", ");
            strcat(g_sms_buff, modem_data.gps_altitude);
            strcat(g_sms_buff, "m ");
            strcat(g_sms_buff, modem_data.gps_speed);
            strcat(g_sms_buff, "km/h ");
        }

        memset(local_buff, 0x00, sizeof(local_buff));
        sprintf(local_buff, "%.2f", sensors_data.bme280_temperature);
        strcat(local_buff, "C, ");
        strcat(g_sms_buff, local_buff);

        memset(local_buff, 0x00, sizeof(local_buff));
        sprintf(local_buff, "%.2f", sensors_data.bme280_pressure);
        strcat(local_buff, "kPa, ");
        strcat(g_sms_buff, local_buff);

        memset(local_buff, 0x00, sizeof(local_buff));
        sprintf(local_buff, "%.2f", sensors_data.bme280_humidity);
        strcat(local_buff, "% ");
        strcat(g_sms_buff, local_buff);

        memset(local_buff, 0x00, sizeof(local_buff));
        sprintf(local_buff, "%d", (int)modem_data.batt_voltage_mv);
        strcat(local_buff, "mV");
        strcat(g_sms_buff, local_buff);

        /*Termination symbol*/
        strcat(g_sms_buff, "\032");

        sprintf(local_buff, "AT+CMGS=%s\r", tracker_settings.phone_number);
        SCP_SendDoubleCommandWaitAnswer(local_buff, g_sms_buff, ">", "OK", 2000, 1);

        /*Wait 5000ms for modem to finish send SMS*/
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));
    }
}

/*Open socket to address*/
static _Bool ModemOpenTcpSocket(char *pAddress, uint32_t port)
{
    char *result = NULL;
    memset(post_buff, 0, sizeof(post_buff));
    /* Form open socket command */
    sprintf(post_buff, "AT#SD=1,0,%d,\"%s\"\r", (int)port, pAddress);
    result = SCP_SendCommandWaitAnswer(post_buff, "CONNECT", 10000, 1);

    if(result)
    {
        return true;
    }

    return false;
}

/* Close socket to address */
static _Bool ModemCloseTcpSocket(void)
{
    char *result = NULL;
    /* Form close socket command */
    (void) SCP_SendCommandWaitAnswer("+++\r\n", "OK", 1000, 1);

    result = SCP_SendCommandWaitAnswer("AT#SH=1\r\n", "OK", 1000, 1);

    if(result)
    {
        return true;
    }

    return false;
}

/* Authentication */
static _Bool TelitPortalAuthenticate(void)
{
    char *result = NULL;
    memset(post_buff, 0, sizeof(post_buff));
    int i = 0;

    /*Reset rx buffer for data reception*/
    SCP_InitRx();

    /* Form data to be posted*/
    sprintf(post_buff, fcmd_dW_auth, tracker_settings.telit_appToken, tracker_settings.telit_appID, modem_data.imei);

    /* Get data length */
    sprintf(post_length, "%d\r\n\r\n", strlen(post_buff));

    /* Send http post header */
    SCP_SendData((char *)fcmd_HTTPPOST, strlen(fcmd_HTTPPOST));

    /* Send data length */
    SCP_SendData(post_length, strlen(post_length));

    /* Send post data */
    SCP_SendData(post_buff, strlen(post_buff));

    /* Wait for full answer */
    result = SCP_WaitForAnswer("}}}", 10000);
    if (result)
    {
        /* Getting session id */
        result = NULL;
        result = strstr((char*)SCPHandler.RxBuffer, "sessionId\":\"");
        if(result)
        {
            result += strlen("sessionId\":\"");
            while ((*result != '\"')&& (i < SESSIONID_LENGTH))
            {
                telit_sessionId[i++]=*(result++);
            }
            if (i <= SESSIONID_LENGTH)
            {
                /* We should wait for the server to shut down the connection */
                result = NULL;
                result = SCP_WaitForAnswer("NO CARRIER", 10000);
                if(result)
                {
                    ModemCloseTcpSocket();
                    return true;
                }
            }
        }
    }
    ModemCloseTcpSocket();
    return false;
}

/*Post*/
static _Bool TelitPortalPostData(void)
{
    char *result = NULL;
    uint32_t len;
    memset(post_buff, 0, sizeof(post_buff));

    double temperature, humidity, pressure;
    int mag_xy,signal_level,batt_voltage;

    /*Copy variables that might to change during data post*/
    temperature = sensors_data.bme280_temperature;
    pressure = sensors_data.bme280_pressure;
    humidity = sensors_data.bme280_humidity;
    mag_xy = sensors_data.bmx055_xy_angle;
    batt_voltage = modem_data.batt_voltage_mv;
    signal_level = modem_data.signal;

    /*Reset rx buffer for data reception*/
    SCP_InitRx();

    /*If we have no GPS data, we have to exclude them from post*/
    if(
            strlen(modem_data.gps_latitude) == 0
            || strlen(modem_data.gps_longitude) == 0
            || strlen(modem_data.gps_altitude) == 0
            || strlen(modem_data.gps_course) == 0
            || strlen(modem_data.gps_speed) == 0
            || strlen(modem_data.gps_precision) == 0
            )
    {
        len =  strlen(fcmd_dw_post_auth) - 2;
        len += strlen(fcmd_dw_post_p1) - 6;
        len += strlen(fcmd_dw_post_p2) - 6;
        len += strlen(fcmd_dw_post_p3) - 6;
        len += strlen(fcmd_dw_post_p4) - 4;
        len += strlen(fcmd_dw_post_p5) - 4;
        len += strlen(fcmd_dw_post_p6) - 4;

        /* Calculate parameters length */
        sprintf(
                (char *)post_buff,"%s%s%s%s%s%s%s%.2f%.2f%.2f%d%d%d",
                telit_sessionId,
                modem_data.imei,
                modem_data.imei,
                modem_data.imei,
                modem_data.imei,
                modem_data.imei,
                modem_data.imei,
                temperature,
                pressure,
                humidity,
                mag_xy,
                batt_voltage,
                signal_level
                );

        len += strlen((char *)post_buff);

        /* Form the buffer with post header */
        SCP_SendData((char *)fcmd_HTTPPOST, strlen(fcmd_HTTPPOST));
        sprintf((char *)post_buff,"%d\r\n\r\n", (int)len);
        SCP_SendData((char *)post_buff, strlen(post_buff));
        sprintf((char *)post_buff, fcmd_dw_post_auth, telit_sessionId);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff, fcmd_dw_post_p1, modem_data.imei, temperature);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff, fcmd_dw_post_p2, modem_data.imei, pressure);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff, fcmd_dw_post_p3, modem_data.imei, humidity);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff, fcmd_dw_post_p4, modem_data.imei, mag_xy);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff, fcmd_dw_post_p5, modem_data.imei, batt_voltage);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff, fcmd_dw_post_p6, modem_data.imei, signal_level);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        SCP_SendData("\r\n", strlen("\r\n"));

        result = SCP_WaitForAnswer("}}", 10000);
        if (result)
        {
            /* We should wait for the server to shut down the connection */
            result = NULL;
            result = SCP_WaitForAnswer("NO CARRIER", 10000);
            if(result)
            {
                ModemCloseTcpSocket();
                return true;
            }
        }
    }

    /*Include GNSS data*/
    else
    {
        len =  strlen(fcmd_dw_post_auth) - 2;
        len += strlen(fcmd_dw_post_p1) - 6;
        len += strlen(fcmd_dw_post_p2) - 6;
        len += strlen(fcmd_dw_post_p3) - 6;
        len += strlen(fcmd_dw_post_p4) - 4;
        len += strlen(fcmd_dw_post_p5) - 4;
        len += strlen(fcmd_dw_post_p6) - 4;
        len += strlen(fcmd_dw_post_p7) - 14;

        /* Calculate parameters length */
        sprintf(
                (char *)post_buff,"%s%s%s%s%s%s%s%.2f%.2f%.2f%d%d%d%s%s%s%s%s%s%s",
                telit_sessionId,
                modem_data.imei,
                modem_data.imei,
                modem_data.imei,
                modem_data.imei,
                modem_data.imei,
                modem_data.imei,
                temperature,
                pressure,
                humidity,
                mag_xy,
                batt_voltage,
                signal_level,
                modem_data.imei,
                modem_data.gps_latitude,
                modem_data.gps_longitude,
                modem_data.gps_altitude,
                modem_data.gps_course,
                modem_data.gps_speed,
                modem_data.gps_precision
                );

        len += strlen((char *)post_buff);

        /* Form the buffer with post header */
        SCP_SendData((char *)fcmd_HTTPPOST, strlen(fcmd_HTTPPOST));
        sprintf((char *)post_buff,"%d\r\n\r\n", (int)len);
        SCP_SendData((char *)post_buff, strlen(post_buff));
        sprintf((char *)post_buff, fcmd_dw_post_auth, telit_sessionId);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff, fcmd_dw_post_p1, modem_data.imei, temperature);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff, fcmd_dw_post_p2, modem_data.imei, pressure);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff, fcmd_dw_post_p3, modem_data.imei, humidity);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff, fcmd_dw_post_p4, modem_data.imei, mag_xy);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff, fcmd_dw_post_p5, modem_data.imei, batt_voltage);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff, fcmd_dw_post_p6, modem_data.imei, signal_level);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        sprintf((char *)post_buff,
                fcmd_dw_post_p7,
                modem_data.imei,
                modem_data.gps_latitude,
                modem_data.gps_longitude,
                modem_data.gps_altitude,
                modem_data.gps_course,
                modem_data.gps_speed,
                modem_data.gps_precision);
        SCP_SendData((char *)post_buff, strlen(post_buff));

        result = SCP_WaitForAnswer("}}", 10000);
        if (result)
        {
            /* We should wait for the server to shut down the connection */
            result = NULL;
            result = SCP_WaitForAnswer("NO CARRIER", 10000);
            if(result)
            {
                ModemCloseTcpSocket();
                return true;
            }
        }
    }

    /*Timeout. In case of error, no }} received*/
    ModemCloseTcpSocket();
    return false;
}

/*Upload to Telit IoT Portal*/
static void upload_data(void)
{
    char *scp_result = NULL;
    static uint32_t gprs_recon_counter = 0;
    static _Bool authenticated = false;
    _Bool result = false;

    modem_data.network_status = NetworkRegistrationCheck();

    /*registered to home network or registered as roaming is acceptable*/
    if((modem_data.network_status == 1) || (modem_data.network_status == 5))
    {
        /*Store operator*/
        scp_result = NULL;
        scp_result = GetOperator();
        if(scp_result)
        {
            memset(modem_data.operator, 0x00, 17);
            strcpy(modem_data.operator, scp_result);
        }

        /*Check GPRS status*/
        modem_data.gprs_status = GPRS_StatusCheck();

        /*Try to connect if not connected*/
        if(!modem_data.gprs_status)
        {
            modem_data.gprs_status = GPRS_Connect(modem_data.ip_address);

            /*Count how many times tried to connect*/
            if(!modem_data.gprs_status)
            {
                gprs_recon_counter++;
            }
            /*We are connected to GPRS now, reset the counter*/
            else
            {
                gprs_recon_counter = 0;
                goto gprs_connected;
            }

            /*Roaming and no GPRS. Retry few times and try to change network*/
            if(!modem_data.gprs_status && (modem_data.network_status == 5) && (gprs_recon_counter >= GPRS_CONNECT_RETRY))
            {
                /*reset counter*/
                gprs_recon_counter = 0;

                /*change provider*/
                if(ChangeProvider())
                {
                    /*Check if we are connected to GSM network*/
                    WaitForNetwork();
                }
            }
        }
        /*We are connected to GPRS*/
        else
        {
            gprs_connected:

            /*Try AGPS only if connected to GPRS and we have less than 4 satellites from GNSS module*/
            if(modem_data.agps_request && atoi(modem_data.gps_satt_in_use) < 4)
            {
                _Bool status = false;
                modem_data.agps_request = false;
                status = RequestAGPSLocation();
                if(status)
                {
                    /*Wait for AGPS data*/
                    modem_data.data_request = true;
                    for(uint8_t i = 180; i > 0; i--)
                    {
                        SCP_Process();
                        if(!modem_data.data_request)
                        {
                            break;
                        }
                        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND/10));
                    }

                }
            }

            /*Authenticate only once per session*/
            if(!authenticated)
            {
                Authentication:
                result = ModemOpenTcpSocket("api-de.devicewise.com", 80);
                if(result)
                {
                    authenticated = TelitPortalAuthenticate();
                }
                /*Post*/
                result = ModemOpenTcpSocket("api-de.devicewise.com", 80);
                if (result)
                {
                    authenticated = TelitPortalPostData();
                }
            }
            else
            {
                /*Post*/
                result = ModemOpenTcpSocket("api-de.devicewise.com", 80);
                if (result)
                {
                    authenticated = TelitPortalPostData();
                }
                /*If post fails try to authenticate*/
                if(!authenticated)
                {
                    goto Authentication;
                }
            }
        }
    }
}

static void battery_critical_shut_down(void)
{

    /*Shut down accelerometer*/
    bmx055_deinit();

    /*Shut down modules*/
    GNSS_Off();
    ModemOff();

    modem_data.gsm_pwr_status = false;
    modem_data.gnss_pwr_status = false;

    /*Enter Low Power Mode*/
     (void) g_sf_power_profiles_v2_common.p_api->lowPowerApply(g_sf_power_profiles_v2_common.p_ctrl, &g_sf_power_profiles_v2_low_power_0);

}

static void low_level_init(void)
{
    ssp_err_t err;
    uint32_t crc_result;

    /* Initialise the LED */
    err = R_BSP_LedsGet(&leds);
    if(err) { APP_ERR_TRAP(err); }

    /*Hardware CRC Engine open*/
    err =  g_crc.p_api->open(g_crc.p_ctrl, g_crc.p_cfg);
    if(err) { APP_ERR_TRAP(err); }

    /* Data flash memory initialisations*/
    if(OpenDriver())
    {
        /*Read whole structure from DATA memory to RAM*/
        err = ReadSettings( &tracker_settings, sizeof(tracker_settings) );
        if(!err)
        {
            /*Check its CRC, exclude last for bytes from calculation*/
            err =  g_crc.p_api->calculate(g_crc.p_ctrl, &tracker_settings, sizeof(tracker_settings)-4, CRC_SEED, &crc_result);
            if(err) { APP_ERR_TRAP(err); }

            /*Initialise all default settings if CRC does not match (most likely MCU memory is fresh)*/
            if(crc_result != tracker_settings.CRC_data)
            {
                tracker_settings.mode = DEFAULT_MODE;
                tracker_settings.interval = 0;
                tracker_settings.repeat = 0;
                tracker_settings.RTC_interval = 3600;
                tracker_settings.ACC_threshold = 5;
                memset(tracker_settings.phone_number, 0x00, sizeof(tracker_settings.phone_number));
                memset(tracker_settings.APN, 0x00, sizeof(tracker_settings.APN));
                memset(tracker_settings.telit_appID, 0x00, sizeof(tracker_settings.telit_appID));
                memset(tracker_settings.telit_appToken, 0x00, sizeof(tracker_settings.telit_appToken));
                strcpy(tracker_settings.APN, "omnitel");
                strcpy(tracker_settings.telit_appID, telit_appID);
                strcpy(tracker_settings.telit_appToken, telit_appToken);

                /*Calculate new CRC*/
                err =  g_crc.p_api->calculate(g_crc.p_ctrl, &tracker_settings, sizeof(tracker_settings)-4, CRC_SEED, &crc_result);
                if(err) { APP_ERR_TRAP(err); }if(err) { APP_ERR_TRAP(err); }
                tracker_settings.CRC_data = crc_result;

                /*Write to Data flash */
                err = WriteSettings( &tracker_settings, sizeof(tracker_settings));
                if(err) { APP_ERR_TRAP(err); }
            }
        }

        else /*Memory read error, initialise with defaults and may try to proceed*/
        {
            tracker_settings.mode = DEFAULT_MODE;
            tracker_settings.interval = 0;
            tracker_settings.repeat = 0;
            tracker_settings.RTC_interval = 3600;
            tracker_settings.ACC_threshold = 5;
            memset(tracker_settings.phone_number, 0x00, sizeof(tracker_settings.phone_number));
            tracker_settings.CRC_data = 0;
        }
    }
    else /*error*/
    {
        if(err) { APP_ERR_TRAP(err); }
    }

    /*Hardware CRC Engine close*/
    (void)g_crc.p_api->close(g_crc.p_ctrl);

    /*Flash driver close*/
    CloseDriver();

    /*RTC*/
    err = g_rtc0.p_api->open(g_rtc0.p_ctrl, g_rtc0.p_cfg);
    if(err) { APP_ERR_TRAP(err); }
    user_function_set_rtc_time();

    /*Count the last movement from startup*/
    sensors_data.last_movement = user_function_get_rtc_time();

    /*Save start up time moment*/
    modem_data.startup_time = sensors_data.last_movement;

    /*Button IRQ0 interrupt*/
    err =  g_external_irq0.p_api->open(g_external_irq0.p_ctrl, g_external_irq0.p_cfg);
    if(err) { APP_ERR_TRAP(err); }

    /*Accelerometer IRQ9 interrupt*/
    err =  g_external_irq9.p_api->open(g_external_irq9.p_ctrl, g_external_irq9.p_cfg);
    if(err) { APP_ERR_TRAP(err); }

    /*Charger Cable Detect (USB)  IRQ13 interrupt*/
    err =  g_external_irq13.p_api->open(g_external_irq13.p_ctrl, g_external_irq13.p_cfg);
    if(err) { APP_ERR_TRAP(err); }

    /*Select Run Profile*/
    err = g_sf_power_profiles_v2_common.p_api->runApply(g_sf_power_profiles_v2_common.p_ctrl, &g_sf_power_profiles_v2_run_0);
    if(err) { APP_ERR_TRAP(err); }
}

static void system_startup(void)
{
    _Bool bme280_init_status = false;
    _Bool bmx055_init_status = false;

    _Bool temp_result = false;
    _Bool init_OK = false;
    char * scp_result = NULL;
    int32_t voltage = 0;
    ioport_level_t p_pin_value;

    /*Initialise GL865 V3.1 Modem and sensors*/

    led_mode = LED_SYSTEM_INIT;

    /* Data flash memory initialisations*/
    if(OpenDriver())
    {
        /*Read whole structure from DATA memory to RAM*/
        ReadSettings( &tracker_settings, sizeof(tracker_settings) );
        /*Flash driver close*/
        CloseDriver();
    }

    /*Apply Default Settings*/
    tracker_settings.interval = 0;
    tracker_settings.repeat = 0;
    modem_data.agps_request = false;

    /*Initialise BME280 Sensors*/
    bme280_init_status = bme280_init();
    if(!bme280_init_status) { APP_ERR_TRAP(bme280_init_status); }

    /*Initialise Accelerometer. Gyroscope and Magnetometer are put to deep suspend modes*/
    bmx055_init_status = bmx055_init();
    if(!bmx055_init_status) { APP_ERR_TRAP(bmx055_init_status); }

    /*Restart uart_comms framework*/
    (void)g_sf_modem_uart0.p_api->close(g_sf_modem_uart0.p_ctrl);
    (void)g_sf_modem_uart0.p_api->open(g_sf_modem_uart0.p_ctrl, g_sf_modem_uart0.p_cfg);

    /*Apply power for GSM and GNSS modules*/
    ModemOn();
    GNSS_On();
    Charger_On();
    modem_data.gsm_pwr_status = true;
    modem_data.gnss_pwr_status = true;
    modem_data.charger_pwr_status = true;
    /*GSM and GNSS initialisation*/
    GSM_init();
    /*AT parser initialisation*/
    SCP_Init(uart_send_buff, uart_read_byte);
    /*AT CaLlbacks Register*/
    SCP_AddCallback("+CLIP:", IncomingCall);
    SCP_AddCallback("+CMTI:", IncomingSMS);
    SCP_AddCallback("#AGPSRING:", GetAGPSLocation);
    SCP_AddCallback("$SIFIXEV:", EphemerisUpdate);

    /*Modem AT command check*/
    for(uint8_t i = 10; i > 0; i--)
    {
        /*Wait 1000ms for modem to be ready*/
        tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND));
        /*Try AT command*/
        scp_result = SCP_SendCommandWaitAnswer("AT\r\n", "OK", 100, 1);
        if(scp_result) break;
    }
    if(!scp_result)
    goto init_failure;

    /*Echo commands turn off*/
    if (scp_result) scp_result = SCP_SendCommandWaitAnswer("ATE0\r\n", "OK", 2000, 1);
    /*Wait 5000ms more..*/
    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));
    /*Set provider to default*/
    if (scp_result) scp_result = SCP_SendCommandWaitAnswer("AT+COPS=0\r\n", "OK", 60000, 1);
    if(scp_result)
    {
        /*Get IMEI*/
        scp_result = NULL;
        scp_result = GetIMEI();
        if(scp_result)
        {
            memset(modem_data.imei, 0x00, 16);
            strcpy(modem_data.imei, scp_result);
        }

        /*Get module type*/
        scp_result = NULL;
        scp_result = GetID();
        if(scp_result)
        {
            memset(modem_data.device_name, 0x00, 21);
            strcpy(modem_data.device_name, scp_result);
        }

        /*Get firmware version*/
        scp_result = NULL;
        scp_result = GetVersion();
        if(scp_result)
        {
            memset(modem_data.fw_version, 0x00, 16);
            strcpy(modem_data.fw_version, scp_result);
        }
    }

    /*Message format = text mode*/
    if (scp_result) scp_result = SCP_SendCommandWaitAnswer("AT+CMGF=1\r", "OK", 2000, 1);
    /*Select how the new received message event is notified by the DCE to the DTE.*/
    if (scp_result) scp_result = SCP_SendCommandWaitAnswer("AT+CNMI=1,1,0,0,0\r", "OK", 2000, 1);
    /*Wait 1000ms for modem to be ready*/
    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND));
    /*Enable error report*/
    if (scp_result) scp_result = SCP_SendCommandWaitAnswer("AT+CMEE=2\r", "OK", 2000, 1);
    /*Enable the extended call type format reporting*/
    if (scp_result) scp_result = SCP_SendCommandWaitAnswer("AT+CRC=1\r", "OK", 2000, 1);
    /*Enable the caller number identification.*/
    if (scp_result) scp_result = SCP_SendCommandWaitAnswer("AT+CLIP=1\r", "OK", 2000, 1);
    /*CPU Clock Mode.*/
    if (scp_result) scp_result = SCP_SendCommandWaitAnswer("AT#CPUMODE=6\r", "OK", 2000, 1);
    /*Set APN*/
    memset(post_buff, 0, sizeof(post_buff));
    sprintf(post_buff, "AT+CGDCONT=1,\"IP\",\"%s\"\r", tracker_settings.APN);
    if (scp_result) scp_result = SCP_SendCommandWaitAnswer(post_buff, "OK", 500, 1);

    /*Battery critical shut down*/
    voltage = MeasureBattVoltage();
    if(voltage <= SHUT_DOWN_VOLTAGE)
    {
        battery_critical_shut_down();
    }

    /*GSM network and GNSS module status*/
    if (scp_result)
    {
        /*Wait for GSM network, 5 minutes at most*/
        temp_result = WaitForNetwork();

        /*We are registered to the network now*/
        if(temp_result)
        {
            temp_result = false;

            /*Try to get GNSS software version*/
            for(int i = 10; i > 0; i--)
            {
                /*Get GPS firmware version*/
                temp_result = GetGPSVersion(modem_data.gps_sw_version);
                tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND/2));
                if(temp_result) break;
            }

            /*Indicate system status*/
            if(temp_result)
            {
                init_OK = true;
                led_mode = ONLINE_IDLE;
            }
            else
            {
                init_OK = false;
                led_mode = CONSTANT_OFF;
            }
        }
        else
        {
            init_OK = false;
            led_mode = CONSTANT_OFF;
        }
    }

    /*Initialisation failure*/
    if(!init_OK)
    {
        init_failure:

        /*Prepare to sleep and wake up on next movement*/
        led_mode = CONSTANT_OFF;
        GSM_deinit();
        GNSS_Off();
        modem_data.gsm_pwr_status = false;
        modem_data.gnss_pwr_status = false;
        bme280_deinit();

        /*Read USB cable status*/
        g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_15, &p_pin_value);
        if(p_pin_value == IOPORT_LEVEL_LOW)
        {
            /*Alarm setup*/
            user_function_set_rtc_alarm((time_t)INIT_FAIL_WAKE_UP);
        }

        /*LED blink for initialisation failure*/
        led_mode = CONSTANT_OFF;

        /*Enter Low Power Mode*/
        (void) g_sf_power_profiles_v2_common.p_api->lowPowerApply(g_sf_power_profiles_v2_common.p_ctrl, &g_sf_power_profiles_v2_low_power_0);
    }
}

/* Tracker Task entry function */
void tracker_task_entry(void)
{
    uint32_t i;
    ssp_err_t err;
    time_t current_time;
    ioport_level_t p_pin_value;

    /*Initialise hardware that will retain configuration in low power mode*/
    low_level_init();

    while (1)
    {
        /*Setup Telit modem and BOSH sensors always on wake up or startup*/
        system_startup();

        /*Initial alarm state is true*/
        alarm_flag = true;

        mode_select:
        /*Select operation mode*/
        switch (tracker_settings.mode)
        {
            /*Normal Mode*/
            case NORMAL_MODE:
            {
                while(tracker_settings.mode == NORMAL_MODE)
                {
                    /*Check if we still have to keep tracker awake*/
                    current_time = user_function_get_rtc_time();
                    if(((current_time - sensors_data.last_movement) > KEEP_AWAKE_TIME) && (tracker_settings.repeat == 0))
                    {
                        /*Enter Low Power Mode*/
                        (void) g_sf_power_profiles_v2_common.p_api->lowPowerApply(g_sf_power_profiles_v2_common.p_ctrl, &g_sf_power_profiles_v2_low_power_0);
                    }

                    /*Look for URC (Unsolicited Result Code)*/
                   SCP_Process();

                    /* Read/Parse messages */
                    sms_parser();

                    /*Break if operational mode has changed*/
                    if(tracker_settings.mode != NORMAL_MODE)
                    {
                        /*Accelerometer IRQ9 interrupt*/
                        if(tracker_settings.mode == POWER_SAVING_RTC)
                        {
                            /*IRQ9 interrupt turn off*/
                            err =  g_external_irq9.p_api->close(g_external_irq9.p_ctrl);
                            if(err) { APP_ERR_TRAP(err); }
                        }

                        /*Mode exit*/
                        goto mode_select;
                    }

                    /*Read GNSS information*/
                    read_gnss();

                    /*Read Sensors*/
                    bme280_measure();

                    /*Measure battery voltage*/
                    modem_data.batt_voltage_mv = MeasureBattVoltage();

                    /*Battery critical shut down*/
                    if(modem_data.batt_voltage_mv <= SHUT_DOWN_VOLTAGE)
                    {
                        /*Send messages*/
                        send_sms();
                        /*Shut down*/
                        battery_critical_shut_down();
                    }

                    /*Send SMS in intervals generated by RTC alarm interrupt*/
                    if(tracker_settings.repeat != 0)
                    {
                        if(alarm_flag)
                        {
                            tracker_settings.repeat--;

                            /*Send messages*/
                            send_sms();

                            /*Alarm setup*/
                            user_function_set_rtc_alarm((time_t)tracker_settings.interval);

                            alarm_flag = false;
                        }
                    }

                    /*Wait 1000 milliseconds*/
                    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND));
                }

                break;
            }

            /*Real time clock mode*/
            case POWER_SAVING_RTC:
            {
                /*Reset Normal Mode variables*/
                alarm_flag = true;
                tracker_settings.interval = 0;
                tracker_settings.repeat = 0;

                /*Read GNSS data, wait for satellites */
                for(i = 0; i < GNSS_SAT_WAIT; i++)
                {
                    /*Look for URC (Unsolicited Result Code)*/
                    SCP_Process();

                    /* Read/Parse messages */
                    sms_parser();

                    /*Break if operational mode has changed*/
                    if(tracker_settings.mode != POWER_SAVING_RTC)
                    {
                        /*Accelerometer IRQ9 interrupt*/
                        err =  g_external_irq9.p_api->open(g_external_irq9.p_ctrl, g_external_irq9.p_cfg);
                        if(err) { APP_ERR_TRAP(err); }

                        /*Mode exit*/
                        goto mode_select;
                    }

                    /*Read GNSS information*/
                    read_gnss();

                    /*Stop the search if certain accuracy is reached*/
                    if(atoi(modem_data.gps_satt_in_use) > GNSS_NSAT)
                    {
                        break;
                    }

                    /*Wait 1000 milliseconds*/
                    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND));
                }

                /*Read Temperature, Pressure, Humidity*/
                bme280_measure();

                /*Measure battery voltage*/
                modem_data.batt_voltage_mv = MeasureBattVoltage();

                /*Send messages*/
                send_sms();

                /*Wait 5000ms for modem to finish send SMS*/
                tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));

                /*Enter low power mode if operation mode have not changed*/
                if(tracker_settings.mode == POWER_SAVING_RTC)
                {
                    /*Alarm setup*/
                    user_function_set_rtc_alarm((time_t)tracker_settings.RTC_interval);

                    /*Enter Low Power Mode*/
                    (void) g_sf_power_profiles_v2_common.p_api->lowPowerApply(g_sf_power_profiles_v2_common.p_ctrl, &g_sf_power_profiles_v2_low_power_0); /*Enter low power mode*/
                }
                break;
            }

            /*Movement detection mode*/
            case POWER_SAVING_ACC:
            {
                /*Reset Normal Mode variables*/
                alarm_flag = true;
                tracker_settings.interval = 0;
                tracker_settings.repeat = 0;

                /*Read GNSS data, wait for satellites */
                for(i = 0; i < GNSS_SAT_WAIT; i++)
                {
                    /*Look for URC (Unsolicited Result Code)*/
                    SCP_Process();

                    /* Read/Parse messages */
                    sms_parser();

                    /*Break if operational mode has changed*/
                    if(tracker_settings.mode != POWER_SAVING_ACC)
                    {
                        /*Accelerometer IRQ9 interrupt*/
                        if(tracker_settings.mode == POWER_SAVING_RTC)
                        {
                            /*IRQ9 interrupt turn off*/
                            err =  g_external_irq9.p_api->close(g_external_irq9.p_ctrl);
                            if(err) { APP_ERR_TRAP(err); }
                        }

                        /*Mode exit*/
                        goto mode_select;
                    }

                    /*Read GNSS information*/
                    read_gnss();

                    /*Stop the search if certain accuracy is reached*/
                    if(atoi(modem_data.gps_satt_in_use) > GNSS_NSAT)
                    {
                        break;
                    }

                    /*Wait 1000 milliseconds*/
                    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND));
                }

                /*Read Temperature, Pressure, Humidity*/
                bme280_measure();

                /*Measure battery voltage*/
                modem_data.batt_voltage_mv = MeasureBattVoltage();

                /*Send messages*/
                send_sms();

                /*Wait 5000ms for modem to finish send SMS*/
                tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND*5));

                /*Enter low power mode if operation mode have not changed*/
                if(tracker_settings.mode == POWER_SAVING_ACC)
                {
                    /*Enter Low Power Mode*/
                    (void) g_sf_power_profiles_v2_common.p_api->lowPowerApply(g_sf_power_profiles_v2_common.p_ctrl, &g_sf_power_profiles_v2_low_power_0); /*Enter low power mode*/
                }
                break;
            }

            /*Cloud Mode*/
            case CLOUD_MODE:
            {
                while(tracker_settings.mode == CLOUD_MODE)
                {
                    /*Check if we still have to keep tracker awake*/
                    current_time = user_function_get_rtc_time();
                    if(((current_time - sensors_data.last_movement) > KEEP_AWAKE_TIME) && (tracker_settings.repeat == 0))
                    {
                        /*Read USB cable status*/
                        g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_15, &p_pin_value);

                        if(p_pin_value == IOPORT_LEVEL_LOW)
                        {
                            led_mode = CONSTANT_ON;

                            /*Request AGPS */
                            modem_data.agps_request = true;

                            /*Upload to cloud*/
                            upload_data();
                            led_mode = CONSTANT_OFF;

                            /*Enter Low Power Mode*/
                            (void) g_sf_power_profiles_v2_common.p_api->lowPowerApply(g_sf_power_profiles_v2_common.p_ctrl, &g_sf_power_profiles_v2_low_power_0);
                        }
                        /*Extend wake up time if the cable is attached*/
                        else
                        {
                            if(((current_time - sensors_data.last_movement) > KEEP_AWAKE_TIME_EXT) && (tracker_settings.repeat == 0))
                            {
                                led_mode = CONSTANT_ON;

                                /*Request AGPS */
                                modem_data.agps_request = true;

                                /*Upload to cloud*/
                                upload_data();
                                led_mode = CONSTANT_OFF;

                                /*Enter Low Power Mode*/
                                (void) g_sf_power_profiles_v2_common.p_api->lowPowerApply(g_sf_power_profiles_v2_common.p_ctrl, &g_sf_power_profiles_v2_low_power_0);
                            }
                        }
                    }

                    /*Do not allow to be online on batteries longer than certain time defined*/
                    g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_15, &p_pin_value);
                    if(p_pin_value == IOPORT_LEVEL_LOW)
                    {
                        /*Check if we still have to keep tracker awake*/
                        current_time = user_function_get_rtc_time();
                        if(((current_time - modem_data.startup_time) > KEEP_ONLINE_TIME) && (tracker_settings.repeat == 0))
                        {
                            /*Shut down accelerometer*/
                            bmx055_deinit();

                            /*Shut down modem*/
                            ModemOff();
                            modem_data.gsm_pwr_status = false;

                            /*Setup RTC timer for wake up*/
                            user_function_set_rtc_alarm((time_t)SLEEP_DURATION);

                            /*Enter Low Power Mode*/
                             (void) g_sf_power_profiles_v2_common.p_api->lowPowerApply(g_sf_power_profiles_v2_common.p_ctrl, &g_sf_power_profiles_v2_low_power_0);
                        }
                    }

                    /*Look for URC (Unsolicited Result Code)*/
                    SCP_Process();

                    /* Read/Parse messages */
                    sms_parser();

                    /*Break if operational mode has changed*/
                    if(tracker_settings.mode != CLOUD_MODE)
                    {
                        /*Accelerometer IRQ9 interrupt*/
                        if(tracker_settings.mode == POWER_SAVING_RTC)
                        {
                            /*IRQ9 interrupt turn off*/
                            err =  g_external_irq9.p_api->close(g_external_irq9.p_ctrl);
                            if(err) { APP_ERR_TRAP(err); }
                        }

                        /*Mode exit*/
                        goto mode_select;
                    }

                    /*Read GNSS information*/
                    read_gnss();

                    /*Read Sensors*/
                    bme280_measure();

                    /*Measure battery voltage*/
                    modem_data.batt_voltage_mv = MeasureBattVoltage();

                    /*We get 0 readings if transparency mode is still active, try to close it*/
                    if(modem_data.batt_voltage_mv == 0)
                    {
                        (void)ModemCloseTcpSocket();
                    }

                    /*Battery critical shut down*/
                    if((modem_data.batt_voltage_mv <= SHUT_DOWN_VOLTAGE) && (modem_data.batt_voltage_mv != 0))
                    {
                        led_mode = CONSTANT_OFF;
                        /*Shut down*/
                        battery_critical_shut_down();
                    }

                    /*Store signal quality*/
                    modem_data.signal = SignalQuality();

                    /*Upload data in intervals generated by RTC alarm interrupt*/
                    if(tracker_settings.repeat != 0)
                    {
                        if(alarm_flag)
                        {
                            tracker_settings.repeat--;

                            /*Upload to cloud*/
                            led_mode = CONSTANT_ON;
                            upload_data();
                            led_mode = ONLINE_IDLE;

                            /*Alarm setup*/
                            user_function_set_rtc_alarm((time_t)tracker_settings.interval);
                            alarm_flag = false;
                        }
                    }

                    /*Wait 1000 milliseconds*/
                    tx_thread_sleep ((ULONG)(TX_TIMER_TICKS_PER_SECOND));
                }

                break;
            }
        }
    }
}
