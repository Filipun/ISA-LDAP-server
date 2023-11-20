/**
 * @file sender.cpp
 * @author Filip Polomski, xpolom00
 * @brief Sender file with implementation of Sender class
 * @version 1.0
 * @date 2023-11-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "../include/sender.h"

/**
 * @brief Construct a new Sender:: Sender object
 * 
 * @param newsocket 
 */
Sender::Sender(int newsocket)
{
    this->newsocket = newsocket;
}

/**
 * @brief Method for sending BindResponse
 * 
 * @param messageID 
 * @param resultCode 
 * @param errorMessage 
 */
void Sender::BindResponse(int messageID, int resultCode, std::string errorMessage)
{
    std::vector<uint8_t> byteVectorToSend;

    if (errorMessage != "")
    {
        std::reverse(errorMessage.begin(), errorMessage.end());
        // insert errorMessage
        for (auto c : errorMessage)
        {
            byteVectorToSend.insert(byteVectorToSend.begin(), c);
        }
    }
    
    byteVectorToSend.insert(byteVectorToSend.begin(), errorMessage.size());
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x04);

    // insert matchedDN
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x00);
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x04);

    // insert resultCode
    byteVectorToSend.insert(byteVectorToSend.begin(), resultCode);
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x01);
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x0a);

    // insert SearchResultDone
    byteVectorToSend.insert(byteVectorToSend.begin(), byteVectorToSend.size());
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x61);

    // insert messageID
    byteVectorToSend.insert(byteVectorToSend.begin(), messageID);
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x01);
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x02);

    // insert Sequence
    byteVectorToSend.insert(byteVectorToSend.begin(), byteVectorToSend.size());
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x30);

    // Send message
    int length = 0;
    length = byteVectorToSend.size();
    int i = 0;
    i = write(this->newsocket, byteVectorToSend.data(), length);
    if (i == -1)
    {
        fprintf(stderr, "Error while sending message!");
        exit(1);
    }
    printf("***BIND_RESPONSE SENT***\n");
}

/**
 * @brief Method for sending SearchResultEntry
 * 
 * @param request 
 * @param line 
 * @param messageID 
 */
void Sender::SearchResultEntry(SearchRequest request, Line line, int messageID)
{
    std::vector<std::string> attributes = request.attributes;
    std::vector<uint8_t> byteVectorToSend;
    std::vector<uint8_t> encodedOctetString;
    std::vector<u_int8_t> lengthVec;

    std::string objectName = "uid=" + line.uid + ",dc=fit,dc=vutbr,dc=cz";
    std::reverse(objectName.begin(), objectName.end());

    // Send all rows
    if (attributes.size() == 0 || attributes[0] == "*")
    {
        encodedOctetString = octedStringToByteVector(ROW_TYPE_MAIL, line.mail);
        byteVectorToSend.insert(byteVectorToSend.begin(), encodedOctetString.begin(), encodedOctetString.end());

        encodedOctetString = octedStringToByteVector(ROW_TYPE_UID, line.uid);
        byteVectorToSend.insert(byteVectorToSend.begin(), encodedOctetString.begin(), encodedOctetString.end());

        encodedOctetString = octedStringToByteVector(ROW_TYPE_CN, line.cn);
        byteVectorToSend.insert(byteVectorToSend.begin(), encodedOctetString.begin(), encodedOctetString.end());

        byteVectorToSend.insert(byteVectorToSend.begin(), byteVectorToSend.size());
        byteVectorToSend.insert(byteVectorToSend.begin(), 0x30);
    }
    // Send no rows
    else if (attributes[0] == "1.1")
    {
        byteVectorToSend.insert(byteVectorToSend.begin(), 0x00);
        byteVectorToSend.insert(byteVectorToSend.begin(), 0x30);
    }
    // Send only selected rows
    else
    {
        if (std::find(attributes.begin(), attributes.end(), "mail") != attributes.end())
        {
            encodedOctetString = octedStringToByteVector(ROW_TYPE_MAIL, line.mail);
            byteVectorToSend.insert(byteVectorToSend.begin(), encodedOctetString.begin(), encodedOctetString.end());
        }
        if (std::find(attributes.begin(), attributes.end(), "uid") != attributes.end())
        {
            encodedOctetString = octedStringToByteVector(ROW_TYPE_UID, line.uid);
            byteVectorToSend.insert(byteVectorToSend.begin(), encodedOctetString.begin(), encodedOctetString.end());
        }
        if (std::find(attributes.begin(), attributes.end(), "cn") != attributes.end())
        {
            encodedOctetString = octedStringToByteVector(ROW_TYPE_CN, line.cn);
            byteVectorToSend.insert(byteVectorToSend.begin(), encodedOctetString.begin(), encodedOctetString.end());
        }

        lengthVec = getLenghtVector(byteVectorToSend.size());
        byteVectorToSend.insert(byteVectorToSend.begin(), lengthVec.begin(), lengthVec.end());
        byteVectorToSend.insert(byteVectorToSend.begin(), 0x30);
    }

    // insert LDAPDN
    for (char c : objectName)
    {
        byteVectorToSend.insert(byteVectorToSend.begin(), c);
    }
    byteVectorToSend.insert(byteVectorToSend.begin(), objectName.size());
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x04);

    // insert SearchResultEntry
    lengthVec = getLenghtVector(byteVectorToSend.size());
    byteVectorToSend.insert(byteVectorToSend.begin(), lengthVec.begin(), lengthVec.end());
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x64);

    // insert messageID
    byteVectorToSend.insert(byteVectorToSend.begin(), messageID);
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x01);
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x02);

    // insert Sequence
    lengthVec = getLenghtVector(byteVectorToSend.size());
    byteVectorToSend.insert(byteVectorToSend.begin(), lengthVec.begin(), lengthVec.end());
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x30);

    // Send message
    int length = 0;
    length = byteVectorToSend.size();
    int i = 0;
    i = write(this->newsocket, byteVectorToSend.data(), length);
    if (i == -1)
    {
        fprintf(stderr, "Error while sending message!");
        exit(1);
    }

    printf("***SEARCH_RESULT_ENTRY SENT***\n");
}

/**
 * @brief Method for sending SearchResultDone
 * 
 * @param messageID 
 * @param errorMessage 
 * @param resultCode 
 */
void Sender::SearchResultDone(int messageID, std::string errorMessage, int resultCode)
{
    std::vector<uint8_t> byteVectorToSend;

    if (errorMessage != "")
    {
        std::reverse(errorMessage.begin(), errorMessage.end());
        // insert errorMessage
        for (auto c : errorMessage)
        {
            byteVectorToSend.insert(byteVectorToSend.begin(), c);
        }
    }
    
    byteVectorToSend.insert(byteVectorToSend.begin(), errorMessage.size());
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x04);

    // insert matchedDN
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x00);
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x04);

    // insert resultCode
    byteVectorToSend.insert(byteVectorToSend.begin(), resultCode);
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x01);
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x0a);

    // insert SearchResultDone
    byteVectorToSend.insert(byteVectorToSend.begin(), byteVectorToSend.size());
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x65);

    // insert messageID
    byteVectorToSend.insert(byteVectorToSend.begin(), messageID);
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x01);
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x02);

    // insert Sequence
    byteVectorToSend.insert(byteVectorToSend.begin(), byteVectorToSend.size());
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x30);

    // Send message
    int length = 0;
    length = byteVectorToSend.size();
    int i = 0;
    i = write(this->newsocket, byteVectorToSend.data(), length);
    if (i == -1)
    {
        fprintf(stderr, "Error while sending message!");
        exit(1);
    }
    printf("***SEARCH_RESULT_DONE SENT***\n");
}

/**
 * @brief Method for converting string to byte vector
 * 
 * @param rowType 
 * @param value 
 * @return std::vector<u_int8_t> 
 */
std::vector<u_int8_t> Sender::octedStringToByteVector(RowType rowType, std::string value)
{
    std::vector<u_int8_t> byteVector;

    std::vector<u_int8_t> uid = {0x75, 0x69, 0x64};
    std::vector<u_int8_t> cn = {0x63, 0x6e};
    std::vector<u_int8_t> mail = {0x6d, 0x61, 0x69, 0x6c};

    std::reverse(value.begin(), value.end());

    // insert value
    for (auto c : value)
    {
        byteVector.insert(byteVector.begin(), c);
    }
    byteVector.insert(byteVector.begin(), value.size());
    byteVector.insert(byteVector.begin(), 0x04);

    // insert SET
    byteVector.insert(byteVector.begin(), byteVector.size());
    byteVector.insert(byteVector.begin(), 0xa0);

    // `insert type
    switch (rowType)
    {
        case ROW_TYPE_UID:
            // insert uid
            byteVector.insert(byteVector.begin(), uid.begin(), uid.end());
            byteVector.insert(byteVector.begin(), uid.size());
            break;
        case ROW_TYPE_CN:
            // insert cn
            byteVector.insert(byteVector.begin(), cn.begin(), cn.end());
            byteVector.insert(byteVector.begin(), cn.size());
            break;
        case ROW_TYPE_MAIL:
            // insert mail
            byteVector.insert(byteVector.begin(), mail.begin(), mail.end());
            byteVector.insert(byteVector.begin(), mail.size());
            break;
        default:
            printf("Unkown row type!\n");
            break;
    }
    byteVector.insert(byteVector.begin(), 0x04);
    // insert sequence
    byteVector.insert(byteVector.begin(), byteVector.size());
    byteVector.insert(byteVector.begin(), 0x30);

    return byteVector;
}

/**
 * @brief Method for getting length vector
 * 
 * @param vectorSize 
 * @return std::vector<u_int8_t> 
 */
std::vector<u_int8_t> Sender::getLenghtVector(int vectorSize)
{
    std::vector<u_int8_t> length;

    // Length bigger than 127
    if (vectorSize >= 128)
    {
        std::vector<uint8_t> lengthBytes;

        // Get length of vector in bytes
        while (vectorSize > 0) {
            lengthBytes.insert(lengthBytes.begin(), static_cast<uint8_t>(vectorSize & 0xFF));
            vectorSize >>= 8;
        }

        // Number of bytes for length
        length.push_back(static_cast<uint8_t>(0x80 | lengthBytes.size())); 

        // Insert length of vector
        for (uint8_t byte : lengthBytes) 
        {
            length.push_back(byte); 
        }
    }
    // Length smaller than 127
    else
    {
        length.push_back(vectorSize);
    }

    return length;
}