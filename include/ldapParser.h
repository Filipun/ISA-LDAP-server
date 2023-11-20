/**
 * @file ldapParser.h
 * @author Filip Polomski, xpolom00
 * @brief LDAP parser header file
 * @version 1.0
 * @date 2023-11-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef LDAP_PARSER_H
#define LDAP_PARSER_H
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <err.h>
#include <ctype.h>
#include <iostream>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <cstdio>
#include <vector>
#include <fcntl.h>



enum Tags {
    ENUM =                  0x0a,
    BOOL =                  0x01,
    INTEGER =               0x02,
    OCTET_STRING =          0x04,
    SEQUENCE =              0x30,
    BIND_REQUEST =          0x60,
    BIND_RESPONSE =         0x61,
    UNBIND_REQUEST =        0x42,
    SEARCH_REQUEST =        0x63,
    SEAECH_RESULT_ENTRY =   0x64,
    SEARCH_RESULT_DONE =    0x65,
    SIMPLE_AUTH =           0x80,
    SUB_INITIAL =           0x80,
    SUB_ANY =               0x81,
    SUB_FINAL =             0x82,
};

typedef enum {
    AND =                   0xa0,
    OR =                    0xa1,
    NOT =                   0xa2,
    EQUALITY_MATCH =        0xa3,
    SUBSTRINGS =            0xa4,
    FILTER_UNDEFINED =      0xff,
} Filter;

enum Scope{
    BASE_OBJECT =           0x00,
    SINGLE_LEVEL =          0x01,
    WHOLE_SUBTREE =         0x02
};

struct Substrings {
    std::string initial;
    std::vector<std::string> any;
    std::string final;
};

struct SubstringFilter {
    std::string type;  // AttributeDescription
    Substrings substrings;
};

struct AttributeValueAssertion {
    std::string attributeDesc;
    std::string assertionValue;
};

struct Filters{
    Filter type;
    std::vector<Filters> subFilters;
    SubstringFilter substringFilter;
    AttributeValueAssertion value;
};

typedef struct {
    std::string ldapdn; 
    Scope scope;
    int sizeLimit;
    int timeLimit;
    bool typesOnly;
    Filters filters;
    std::vector<std::string> attributes; // AttributeDescriptionList
} SearchRequest;


class ldapParser {
    private:
        int indexOfVector;
        int maxIndexOfVector;
        int newsocket;
        std::vector<uint8_t> byteVector;
        std::string parseOctetString();
        void indexIncrement();
        uint64_t getLength();
        bool parseLDAPMessage(std::string file);
        int parseInteger();
        Scope parseScope();
        Filters loadFilters();  
        bool parseBoolean();
        void checkLengthOfMessage();
        void parseBindRequest();
        SearchRequest parseSearchRequest();
        std::vector<std::string> parseAttributeDescriptionList();
    public:
        ldapParser(int newsocket);
        void msgParse(std::string file);
};

#endif