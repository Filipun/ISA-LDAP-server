#ifndef SENDER_H
#define SENDER_H

#include "ldapParser.h"

class Sender {
    private:
        std::vector<uint8_t> byteVectorToSend;
        int newsocket;
    public:
        Sender(int newsocket);
        void BindResponse(int messageID, uint8_t resultCode, std::string matchedDN, std::string errorMessage);
};

#endif