/*
 * @Author: HoGC
 * @Date: 2022-04-17 21:57:12
 * @Last Modified time: 2022-04-17 21:57:12
 */
#ifndef _APP_SPEECH_H_
#define _APP_SPEECH_H_

#define SPEECH_I2S_PORT                    1

#define SPEECH_I2S_BCK_PIN                 41
#define SPEECH_I2S_WS_PIN                  42
#define SPEECH_I2S_IN_PIN                  2
#define SPEECH_I2S_OUT_PIN                 -1

void app_speech_init(void);

#endif //_APP_SPEECH_H_