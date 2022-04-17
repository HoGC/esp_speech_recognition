#include "baiduChunkedUploader.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_event.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#define WEB_SERVER "vop.baidu.com"
#define WEB_PORT "80"
#define TAG "baiduChunkedUploader"
#define REQUEST  "POST http://vop.baidu.com/server_api?cuid=1234543&token=you_token HTTP/1.1\r\n\
Accept-Encoding: identity\r\n\
Host: vop.baidu.com\r\n\
User-Agent: Python-urllib/3.7\r\n\
Content-Type: audio/pcm; rate=16000\r\n\
Transfer-Encoding: chunked\r\n\r\n"


/**
 * 构造函数
 */
baiduChunkedUploader::baiduChunkedUploader()
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;

    //获取IP信息
    int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);
    if(err != 0 || res == NULL) {
        ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
        return;
    }

    //创建套接字
    mSocket = socket(res->ai_family, res->ai_socktype, 0);
    if(mSocket < 0) {
        ESP_LOGE(TAG, "Failed to allocate socket.");
        freeaddrinfo(res);
        return;
    }

    //创建连接
    if(connect(mSocket, res->ai_addr, res->ai_addrlen) != 0) {
        ESP_LOGE(TAG, "Socket connect failed errno=%d", errno);
        close(mSocket);
        freeaddrinfo(res);
        return;
    }
    freeaddrinfo(res);

    //发送首部字段
    if (write(mSocket, REQUEST, strlen(REQUEST)) < 0) {
        ESP_LOGE(TAG, "HEAD send failed");
        close(mSocket);
        return;
    }else
    
    mConnected = true;
}


/**
 * 检查是否连接
 */
bool baiduChunkedUploader::connected()
{
    return mConnected;
}


/**
 * 发送块长度
 */
void baiduChunkedUploader::startChunk(int size_in_bytes)
{
    char send_data[16];
    sprintf(send_data,"%X\r\n",size_in_bytes);
    write(mSocket, send_data, strlen(send_data));
}


/**
 * 发送数据块
 */
void baiduChunkedUploader::sendChunkData(const uint8_t *data, int size_in_bytes)
{
    write(mSocket, data, size_in_bytes);
}


/**
 * 结束一个数据块
 */
void baiduChunkedUploader::finishChunk()
{
    write(mSocket, "\r\n", strlen("\r\n"));
}


static int response_parse(char *data, int len, char *buf, int buf_len){

    int crlf_pos;
    int response_code = 0;

    char *crlf_ptr = strstr(data, "\r\n");
    if (crlf_ptr == NULL) {
        printf("\\r\\n not found\n");
        return -1;
    }

    crlf_pos = crlf_ptr - data;
    data[crlf_pos] = '\0';


    /* Parse HTTP response */
    if ( sscanf(data, "HTTP/%*d.%*d %d %*[^\r\n]", &response_code) != 1 ) {
        /* Cannot match string, error */
        printf("Not a correct HTTP answer : %s\n", data);
        return -1;
    }

    if ( (response_code < 200) || (response_code >= 400) ) {
        /* Did not return a 2xx code; TODO fetch headers/(&data?) anyway and implement a mean of writing/reading headers */
        printf("Response code %d", response_code);

        if (response_code == 416) {
            printf("Requested Range Not Satisfiable\n");
            return -1;
        }
    }

    memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2) + 1); /* Be sure to move NULL-terminating char as well */
    len -= (crlf_pos + 2);

    while ( true ) {
        char *colon_ptr, *key_ptr, *value_ptr;
        int key_len, value_len;

        crlf_ptr = strstr(data, "\r\n");

        crlf_pos = crlf_ptr - data;
        if (crlf_pos == 0) { /* End of headers */
            memmove(data, &data[2], len - 2 + 1); /* Be sure to move NULL-terminating char as well */
            len -= 2;
            len = len>buf_len?buf_len:len;
            memcpy(buf, data, len);
            buf[len-1] = '\0';
            break;
        }

        crlf_pos = crlf_ptr - data;
        data[crlf_pos] = '\0';
        memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2) + 1); /* Be sure to move NULL-terminating char as well */
        len -= (crlf_pos + 2);
    }

    return response_code;
}

/**
 * 结束传输，获取结果
 */
int baiduChunkedUploader::getResults(char *buf, int buf_len)
{
    //发送结束标志
    write(mSocket, "0\r\n\r\n", strlen("0\r\n\r\n"));
   
    char recv_buf[64];
    char mid_buf[1024];
    bzero(mid_buf, sizeof(mid_buf));
    int len = read(mSocket, mid_buf, sizeof(mid_buf)-1);
    // ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.\r\ndata:\r\n%s", len, errno, mid_buf);
    return response_parse(mid_buf, len, buf, buf_len);
}

baiduChunkedUploader::~baiduChunkedUploader()
{
    if(mSocket != -1){
        close(mSocket);
    }
    mSocket = -1;
}