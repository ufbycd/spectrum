/*
 * audio.h
 *
 *  Created on: 2016年8月3日
 *      Author: chen
 */

#ifndef AUDIO_H_
#define AUDIO_H_


#include "main.h"

void Audio_Init(void);
Util_ResourceHandle_t Audio_GetSpectrumResHandle(void);
void Audio_SetCalibrationOn(void);

#endif /* AUDIO_H_ */
