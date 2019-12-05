/*
 * sms_commands.h
 *
 *  Created on: 12 Dec 2018
 *      Author: GDR
 */

#ifndef SMS_COMMANDS_H_
#define SMS_COMMANDS_H_

#include "hal_data.h"

void NormalModeConfig (const char *pString);
void RTCModeConfig (const char *pString);
void ACCModeConfig (const char *pString);
void MemNumConfig (const char *pString);
void CloudModeConfig (const char *pString);
void APNConfig (const char *pString);
void AppIDConfig (const char *pString);
void AppTokenConfig (const char *pString);
void HelpSMS (const char *pString);

#endif /* SMS_COMMANDS_H_ */
