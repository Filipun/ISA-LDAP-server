#ifndef LDAP_H
#define LDAP_H

//#include "ldapParser.h"
#include "sender.h"

class LDAP {
    private:
        int newsocket;
        std::string file;
    public:
        LDAP(int newsocket, std::string file);
        void LDAPrun();
};

#endif