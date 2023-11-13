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
    // Aplication 6x
    BIND_REQUEST =          0x60,
    BIND_RESPONSE =         0x61,
    UNBIND_REQUEST =        0x62,
    SEARCH_REQUEST =        0x63,
    SEAECH_RESULT_ENTRY =   0x64,
    SEARCH_RESULT_DONE =    0x65,
    SIMPLE_AUTH =           0x80
};

typedef enum {
    AND =                   0xa0,
    OR =                    0xa1,
    NOT =                   0xa2,
    EQUALITY_MATCH =        0xa3,
    SUBSTRINGS =            0xa4,
} Filter;

typedef enum {
    BASE_OBJECT =           0x00,
    SINGLE_LEVEL =          0x01,
    WHOLE_SUBTREE =         0x02
} Scope;

typedef struct {
    std::string ldapdn; // LDAPDN
    Scope scope;
    // uint8_t derefAliases; TODO asi skip xd
    int sizeLimit;
    int timeLimit;
    bool typesOnly;
    Filter filter;
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
        void parseLDAPMessage();
        int parseInteger();
        Scope parseScope(void);
        Filter parseFilters(void);
        bool parseBoolean();
        void checkLengthOfMessage();
        void parseBindRequest();
        SearchRequest parseSearchRequest();
    public:
        ldapParser(int newsocket);
        void msgParse();
};



struct ldapMessage {
    int messageID;
    uint64_t length;
    uint8_t tag;
};


struct BindRequest {
    uint8_t version;
    std::string name; // LDAPDN
    std::string password; // TODO poop asi idk
};

struct BindResponse {
    uint8_t resultCode;
    std::string matchedDN; // LDAPDN
    std::string errorMessage; // LDAPString
};

struct UnbindRequest { //TODO asi smazat
    // nothing
};


struct SearchResultEntry {
    std::string objectName; // LDAPDN
    std::vector<std::string> attributes; // PartialAttributeList
};

struct SearchResultDone {
    int resultCode;
    std::string matchedDN; // LDAPDN
    std::string errorMessage; // LDAPString
};



#endif