#include "../include/sender.h"

Sender::Sender(int newsocket)
{
    this->newsocket = newsocket;
}

void Sender::BindResponse(int messageID, uint8_t resultCode, std::string matchedDN, std::string errorMessage)
{
    int length = 0;
    this->byteVectorToSend = {0x30, 0x0c,0x02, 0x01, 0x01, 0x61, 0x07, 0x02, 0x01, 0x01, 0x04, 0x00, 0x04, 0x00};
    length = this->byteVectorToSend.size();
    int i = 0;
    i = write(this->newsocket, this->byteVectorToSend.data(), length);
    if (i == -1)
    {
        fprintf(stderr, "Error while sending message!");
        exit(1);
    }
    printf("BindResponse sent\n");
}