/*
 * @Author: HoGC
 * @Date: 2022-04-17 21:56:50
 * @Last Modified time: 2022-04-17 21:56:50
 */
#include "app_speech.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "rbuffer.hpp"

#include "NeuralNetwork.h"
#include "AudioProcessor.h"

#include "baiduChunkedUploader.h"

extern "C" {
    #include "esp_mn_iface.h"
    #include "esp_mn_models.h"
    #include "driver/i2s.h"
    #include "model_path.h"

    #include "app_wifi.h"
}

static const char *TAG = "app_speech";

#define WINDOW_SIZE 320
#define STEP_SIZE 160
#define POOLING_SIZE 6
#define AUDIO_LENGTH 16000

enum {
    SPEECH_STAUS_WAIT_WAKE = 0,
    SPEECH_STAUS_RECOGNITIONING,
    SPEECH_STAUS_RERECOGNITION,
};

static int m_speech_status = SPEECH_STAUS_WAIT_WAKE;

static Rbuffer *m_rbuffer = NULL;

static NeuralNetwork *m_nn = NULL;
static AudioProcessor *m_audio_processor = NULL;

static TaskHandle_t m_nnTaskHandle = NULL;

static model_iface_data_t *m_model_data_mn = NULL;
static const esp_mn_iface_t *m_multinet = &MULTINET_MODEL;

static void i2s_init(void){
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        // .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = SPEECH_I2S_BCK_PIN,
        .ws_io_num = SPEECH_I2S_WS_PIN,
        .data_out_num = SPEECH_I2S_OUT_PIN,
        .data_in_num = SPEECH_I2S_IN_PIN  
    };
    i2s_driver_install((i2s_port_t)SPEECH_I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin((i2s_port_t)SPEECH_I2S_PORT, &pin_config);
    i2s_zero_dma_buffer((i2s_port_t)SPEECH_I2S_PORT);
}

static void recsrcTask(void *arg)
{
    i2s_init();

    int samp_len = 1024;
    int32_t *buffer = (int32_t *)malloc(samp_len*sizeof(int32_t));
    int16_t *samp = (int16_t *)buffer;

    size_t read_len = 0;

    while(1) {
        i2s_read((i2s_port_t)SPEECH_I2S_PORT, buffer, samp_len*sizeof(int32_t), &read_len, portMAX_DELAY);
        for (size_t i = 0; i < samp_len; i++) {
            samp[i] = buffer[i] >> 13;
        }
        
        m_rbuffer->push(samp, samp_len*sizeof(int16_t), true);
        xTaskNotify(m_nnTaskHandle, 1, eSetBits);
    }

    vTaskDelete(NULL);
}

void nnTask(void *arg)
{
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    uint16_t mn_chunks = 0;
    int16_t *buffer = (int16_t *)malloc(AUDIO_LENGTH*sizeof(int16_t));

    int chunk_num = m_multinet->get_samp_chunknum(m_model_data_mn);
    ESP_LOGI(TAG, "chunk_num = %d\n", chunk_num);

    baiduChunkedUploader *speech_recogniser = NULL;

    while(1) {
        if(m_speech_status == SPEECH_STAUS_WAIT_WAKE){
            uint32_t index = 0;
            m_rbuffer->get_end_backward_index((uint32_t)(AUDIO_LENGTH*sizeof(int16_t)), &index);
            m_rbuffer->get_buffer(index, buffer, AUDIO_LENGTH*sizeof(int16_t));        
            float *input_buffer = m_nn->getInputBuffer();
            m_audio_processor->get_spectrogram(buffer, input_buffer);
            float output = m_nn->predict();
            if (output > 0.95)
            {
                printf("P(%.2f): Here I am, brain the size of a planet...\n", output);
                m_speech_status = SPEECH_STAUS_RECOGNITIONING;
                m_rbuffer->reset();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }else if(m_speech_status == SPEECH_STAUS_RECOGNITIONING){
            uint32_t size = m_rbuffer->used_size();
            if(size < 512*sizeof(int16_t)){
                uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                continue;
            }
            m_rbuffer->pop(buffer, 512*sizeof(int16_t));
            mn_chunks++;
            int command_id = m_multinet->detect(m_model_data_mn, buffer);
            if (mn_chunks == 32*2 || command_id > -1) {
                mn_chunks = 0;
                m_speech_status = SPEECH_STAUS_WAIT_WAKE;
                if (command_id > -1) {
                    ESP_LOGI(TAG, "speech command id： %d", command_id);
                } else {
                    ESP_LOGI(TAG, "can not recognize any speech commands");
                    ESP_LOGI(TAG, "start using Baidu speech recognition");
                    if(app_wifi_get_connect_status()){
                        m_rbuffer->suspend();
                        speech_recogniser = new baiduChunkedUploader();
                        // vTaskDelay(10 / portTICK_PERIOD_MS);
                        if (speech_recogniser && speech_recogniser->connected()){
                            for (size_t index = 0; index < 16000*2; )
                            {
                                m_rbuffer->get_buffer(index*sizeof(int16_t), buffer, 500*sizeof(int16_t));  
                                speech_recogniser->startChunk(500*sizeof(int16_t));
                                speech_recogniser->sendChunkData((const uint8_t *)buffer, 500*sizeof(int16_t));
                                speech_recogniser->finishChunk();
                                index += 500;
                            }
                            char http_results[512] = {0};
                            int resp_code = speech_recogniser->getResults(http_results, 512);
                            ESP_LOGI(TAG, "resp_code: %d   %s", resp_code, http_results);
                        }
                        delete speech_recogniser;
                        speech_recogniser = NULL;
                        m_rbuffer->resume();
                    }else{
                        ESP_LOGE(TAG, "wifi is not connect can not using Baidu speech recognition");
                    }
                }
            }
        }else if(m_speech_status == SPEECH_STAUS_RERECOGNITION){
            m_rbuffer->reset();
            m_speech_status = SPEECH_STAUS_RECOGNITIONING;
        }
    }

    free(buffer);
    vTaskDelete(NULL);
}


static void speech_task(void *arg)
{
    // Create our neural network
    m_nn = new NeuralNetwork();
    printf("Created Neral Net\n");
    // create our audio processor
    m_audio_processor = new AudioProcessor(AUDIO_LENGTH, WINDOW_SIZE, STEP_SIZE, POOLING_SIZE);
    printf("Created audio processor\n");

    m_rbuffer = new Rbuffer((AUDIO_LENGTH*3)*sizeof(int16_t));

#ifdef CONFIG_MODEL_IN_SPIFFS
    srmodel_spiffs_init();
#endif
    m_model_data_mn = m_multinet->create((model_coeff_getter_t *)&MULTINET_COEFF, 4000);

    xTaskCreatePinnedToCore(&recsrcTask, "rec", 4*1024, NULL, 4, NULL, 1);

    xTaskCreatePinnedToCore(&nnTask, "nn", 4*1024+2048, NULL, 2, &m_nnTaskHandle, 1);

    vTaskDelete(NULL);
}

void app_speech_init(void){

    xTaskCreatePinnedToCore(&speech_task, "speech", 4*1024, NULL, 5, NULL, 1);
}
