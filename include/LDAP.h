#ifndef LDAP_H
#define LDAP_H

//#include "ldapParser.h"
#include "sender.h"

class LDAP {
    private:
        int newsocket;
    public:
        LDAP(int newsocket);
        void LDAPrun();
};

#endif