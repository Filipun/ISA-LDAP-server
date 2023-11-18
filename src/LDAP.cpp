#include "../include/LDAP.h"

LDAP::LDAP(int newsocket, std::string file)
{
    this->newsocket = newsocket;
    this->file = file;
}

void LDAP::LDAPrun()
{
    ldapParser parser(this->newsocket);
    //Sender sender(this->newsocket);
    parser.msgParse(this->file);
}