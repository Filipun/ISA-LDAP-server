#include "../include/ldapParser.h"


ldapParser::ldapParser()
{
    // this->byteVector;
    this->indexOfVector = 0;
    this->maxIndexOfVector = 0;
}

void ldapParser::LDAPparse(int newsocket)
{

    //  Load ldap message to vector 
    char byte; 
    while (read(newsocket, &byte, 1) > 0)
    {
        this->byteVector.push_back((unsigned char)byte);
    }

    this->maxIndexOfVector = this->byteVector.size() - 1;

    // check print in hexadecimal shape
    for (int i : this->byteVector)
    {
        printf("%02X ", i);
    }


    this->parseLDAPMessage();
    
}

void ldapParser::indexIncrement()
{
    if (this->indexOfVector > this->maxIndexOfVector)
    {
        fprintf(stderr, "Index of vector out of range!");
        exit(1);
    }
    this->indexOfVector += 1;
}

uint64_t ldapParser::getLength()
{
    uint64_t length = 0;
    if (((this->byteVector[this->indexOfVector] >> 7) & 1))
    {
        uint8_t LengthOfLongLength; 
        LengthOfLongLength = this->byteVector[this->indexOfVector] & 0x7f;
        for (int i = 1; i <= LengthOfLongLength; i++)
        {
            length = (length << 8) | byteVector[this->indexOfVector + i];
            this->indexIncrement();
        }
    }
    else // Length in short form
    {
        length = this->byteVector[this->indexOfVector] & 0x7f;
        this->indexIncrement();
    }
    return length;
}

void ldapParser::parseLDAPMessage()
{
    // Check SEQUENCE tag
    if (this->byteVector[this->indexOfVector] != SEQUENCE)
    {
        fprintf(stderr, "Wrong BER message!\n Expected SEQUENCE tag!\n");
        exit(1);
    }
    this->indexIncrement();

    // Check length of SEQUENCE
    this->checkLengthOfMessage();
    this->indexIncrement();

    int MessageID = this->parseInteger();

    printf("MessageID: %d\n", MessageID); // TODO check print
    
    // Check tag of LDAP message
    int ldapVersion = 0;
    std::string name = "";
    switch (this->byteVector[this->indexOfVector])
    {
        case BIND_REQUEST:
            printf("BIND_REQUEST\n");
            this->indexIncrement();
            this->checkLengthOfMessage();
            this->indexIncrement();
            ldapVersion = this->parseInteger();
            // Check LDAP version TODO
            printf("LDAP version: %d\n", ldapVersion); // TODO check print
            name = this->parseOctetString();
            printf("Name: %s\n", name.c_str()); // TODO check print
            // check name, should be empty
            if (this->byteVector[this->indexOfVector] == OCTET_STRING)
            {
                std::string password = this->parseOctetString();
                printf("Password: %s\n", password.c_str()); // TODO check print
            }
            // check password, should be empty
            if (this->byteVector[this->indexOfVector] == SIMPLE_AUTH)
            {
                this->indexIncrement();
                if (this->byteVector[this->indexOfVector + 1] == 0x00)
                {
                    printf("Password is empty and thats right!\n"); // TODO check print
                }
            }
            break;
        case SEARCH_REQUEST:
            printf("SEARCH_REQUEST\n");
            break;
        case UNBIND_REQUEST:
            printf("UNBIND_REQUEST\n");
            break;
        default:
            fprintf(stderr, "Wrong BER message!\nExpected BIND_REQUEST, SEARCH_REQUEST or UNBIND_REQUEST tag!\n");
            exit(1);
    }

}

std::string ldapParser::parseOctetString() 
{
    // Check octet string tag
    if (this->byteVector[this->indexOfVector] != OCTET_STRING)
    {
        fprintf(stderr, "Wrong BER message!\nExpected OCTET_STRING tag!\n");
        exit(1);
    }
    this->indexIncrement();

    uint64_t lengthOfOctetString = this->getLength();

    std::string octetString = "";

    for (int i = 0; i < lengthOfOctetString; i++)
    {
        octetString += this->byteVector[this->indexOfVector];  // TODO bude to fungovat a prepisovat z hexa na pismenka?
        this->indexIncrement();
    }

    return octetString;
}

int ldapParser::parseInteger()
{
    // Check integer tag
    if (this->byteVector[this->indexOfVector] != INTEGER)
    {
        fprintf(stderr, "Wrong BER message!\nExpected INTEGER tag!\n");
        exit(1);
    }
    this->indexIncrement();

    //this->checkLengthOfTags();

    uint64_t lengthOfInteger = this->getLength();

    int integer = 0;

    for (int i = 0; i < lengthOfInteger; i++)
    {
        integer = (integer << 8) | byteVector[this->indexOfVector];
        this->indexIncrement();
    }

    return integer;
}

void ldapParser::checkLengthOfMessage()
{
    uint64_t lengthOfTag = this->getLength();
    this->indexOfVector -= 1; // function getLength() increment index of vector
    printf("Length of tag: %ld\n", lengthOfTag);  // TODO check print
    printf("Length of vector: %d\n", this->maxIndexOfVector - this->indexOfVector);  // TODO check print
    if (lengthOfTag != (this->maxIndexOfVector - this->indexOfVector))
    {
        fprintf(stderr, "Wrong BER message!\nLength of Message is not right!");
        exit(1);
    }
}