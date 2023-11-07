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
        void checkLengthOfMessage();
    public:
        ldapParser(int newsocket);
        void LDAPparse ();
};

enum Tags {
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

struct SearchRequest {
    std::string baseObject; // LDAPDN
    uint8_t scope;
    uint8_t derefAliases;
    int sizeLimit;
    int timeLimit;
    bool typesOnly;
    int filter;
    std::vector<std::string> attributes; // AttributeDescriptionList
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