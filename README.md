# esp_speech_recognition
ESP32 语音唤醒+离线识别+百度在线识别    
* 语音唤醒主要使用[diy-alexa](https://github.com/atomic14/diy-alexa)的训练模型    
* 离线识别使用[esp-sr](https://github.com/espressif/esp-sr), 同时esp-sr也提供语音唤醒，但是使用固定的唤醒词，所以使用[tflite-micro](https://github.com/tensorflow/tflite-micro)进行训练以实现任意唤醒词唤醒    
* 离线识别无法匹配，则使用百度的在线语音识别，暂时没有做语义的识别

```
P(0.99): Here I am, brain the size of a planet...       //使用马文唤醒
I (14006) app_speech: speech command id： 18            //匹配离线识别的id, 打开电灯

P(0.97): Here I am, brain the size of a planet...       //使用马文唤醒
I (18916) app_speech: can not recognize any speech commands
I (18916) app_speech: start using Baidu speech recognition
I (19586) app_speech: resp_code: 200   {"corpus_no":"7087581481571160454","err_msg":"success.","err_no":0,"result":["打开电视机。"],"sn":"958384688661650206158"}           //百度在线识别结果
```

## 编译说明
1. 开发环境参考[esp-idf](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html)
2. [tflite-micro](https://github.com/tensorflow/tflite-micro)需要cmake-3.16.0, idf默认安装的camke可能不能满足要求，需要手动升级, ubuntu参考[cmakk升级](https://blog.csdn.net/Boys_Wu/article/details/104940575)
3. 项目默认使用[ESP32-S3-EYE](https://github.com/espressif/esp-who/blob/master/docs/zh_CN/get-started/ESP32-S3-EYE_Getting_Started_Guide.md)开发板, 使用I2S麦克风
4. 需要在[百度AI平台](https://ai.baidu.com/tech/speech)添加一个语音识别应用并获取token, [文档](https://cloud.baidu.com/doc/ABCROBOT/s/Yjwvy41dk)
    ```C
    // 修改baidu_ai/baiduChunkedUploader.cc->you_token
    #define REQUEST  "POST http://vop.baidu.com/server_api?cuid=1234543&token=you_token HTTP/1.1\r\n\
    ```
