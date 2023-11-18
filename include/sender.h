#ifndef SENDER_H
#define SENDER_H

#include "ldapParser.h"
#include "DBreader.h"
#include <algorithm>

enum RowType {
    ROW_TYPE_UID,
    ROW_TYPE_CN,
    ROW_TYPE_MAIL
};

class Sender {
    private:
        std::vector<uint8_t> byteVectorToSend;
        int newsocket;
        std::vector<uint8_t> octedStringToByteVector(RowType rowType, std::string value);
    public:
        Sender(int newsocket);
        void BindResponse(int messageID, uint8_t resultCode, std::string matchedDN, std::string errorMessage);
        void SearchResultEntry(SearchRequest request, Line line, int messageID);
        void SearchResultDone(int messageID, std::string errorMessage, int resultCode);
};

#endif