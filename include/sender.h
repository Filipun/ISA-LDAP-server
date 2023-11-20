/**
 * @file sender.h
 * @author Filip Polomski, xpolom00
 * @brief Sender header file
 * @version 1.0
 * @date 2023-11-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

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
        void BindResponse(int messageID, int resultCode, std::string errorMessage);
        void SearchResultEntry(SearchRequest request, Line line, int messageID);
        void SearchResultDone(int messageID, std::string errorMessage, int resultCode);
        std::vector<u_int8_t> getLenghtVector(int vectorSize);
};

#endif