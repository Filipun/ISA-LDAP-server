#include "../include/sender.h"

Sender::Sender(int newsocket)
{
    this->newsocket = newsocket;
}

void Sender::BindResponse(int messageID, uint8_t resultCode, std::string matchedDN, std::string errorMessage)
{
    int length = 0;
    this->byteVectorToSend = {0x30, 0x0c,0x02, 0x01, 0x01, 0x61, 0x07, 0x0A, 0x01, 0x00, 0x04, 0x00, 0x04, 0x00};
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

void Sender::SearchResultEntry(SearchRequest request, Line line, int messageID)
{
    std::vector<std::string> attributes = request.attributes;
    std::vector<uint8_t> byteVectorToSend;
    std::vector<uint8_t> encodedOctetString;

    std::string objectName = "uid=" + line.uid + ",dc=fit,dc=vutbr,dc=cz";
    std::reverse(objectName.begin(), objectName.end());

    //printf("dojde to sem?\n");

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
    else if (attributes[0] == "1.1")
    {
        byteVectorToSend.insert(byteVectorToSend.begin(), 0x00);
        byteVectorToSend.insert(byteVectorToSend.begin(), 0x30);
    }
    else
    {
        // insert partial attribute list
        for (auto atribut : attributes)
        {
            if (atribut == "mail")
            {
                encodedOctetString = octedStringToByteVector(ROW_TYPE_MAIL, line.mail);
                byteVectorToSend.insert(byteVectorToSend.begin(), encodedOctetString.begin(), encodedOctetString.end());
            }
            if (atribut == "uid")
            {
                encodedOctetString = octedStringToByteVector(ROW_TYPE_UID, line.uid);
                byteVectorToSend.insert(byteVectorToSend.begin(), encodedOctetString.begin(), encodedOctetString.end());
            }
            if (atribut == "cn" )
            {
                encodedOctetString = octedStringToByteVector(ROW_TYPE_CN, line.cn);
                byteVectorToSend.insert(byteVectorToSend.begin(), encodedOctetString.begin(), encodedOctetString.end());
            }
        }
        byteVectorToSend.insert(byteVectorToSend.begin(), byteVectorToSend.size());
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
    byteVectorToSend.insert(byteVectorToSend.begin(), byteVectorToSend.size());
    byteVectorToSend.insert(byteVectorToSend.begin(), 0x64);

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

    for (auto c : byteVectorToSend)
    {
        // printf(" %c-", c);
        printf("%02x ", c);
    }


    printf("SearchResultEntry sent\n");
}

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

void Sender::SearchResultDone(int messageID, std::string errorMessage, int resultCode)
{
    std::vector<uint8_t> byteVectorToSend;
    std::vector<uint8_t> encodedOctetString;

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
    printf("SearchResultDone sent\n");
}