/**
 * @file LDAP.h
 * @author Filip Polomski, xpolom00
 * @brief LDAP header file
 * @version 1.0
 * @date 2023-11-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef LDAP_H
#define LDAP_H

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