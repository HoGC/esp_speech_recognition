#ifndef _baidu_chunked_uploaded_h_
#define _baidu_chunked_uploaded_h_

#include <stdint.h>
#include <string>

// class WiFiClient;

// typedef struct
// {
//     std::string text;
//     std::string corpus_no;
//     std::string err_msg;
//     int err_no;
//     std::string result;
//     std::string sn;
// } Intent;

class baiduChunkedUploader
{
private:
    int mSocket;
    bool mConnected = false;

public:
    baiduChunkedUploader();
    ~baiduChunkedUploader();
    bool connected();
    void startChunk(int size_in_bytes);
    void sendChunkData(const uint8_t *data, int size_in_bytes);
    void finishChunk();
    int getResults(char *buf, int buf_len);
};

#endif