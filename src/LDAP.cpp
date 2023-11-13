#include "../include/LDAP.h"

LDAP::LDAP(int newsocket)
{
    this->newsocket = newsocket;
}

void LDAP::LDAPrun()
{
    ldapParser parser(this->newsocket);
    //Sender sender(this->newsocket);
    parser.msgParse();
}