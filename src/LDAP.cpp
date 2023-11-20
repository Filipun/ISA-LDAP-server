/**
 * @file LDAP.cpp
 * @author Filip Polomski, xpolom00
 * @brief LDAP file with implementation of LDAP class
 * @version 1.0
 * @date 2023-11-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "../include/LDAP.h"

/**
 * @brief Construct a new LDAP:: LDAP object
 * 
 * @param newsocket 
 * @param file 
 */
LDAP::LDAP(int newsocket, std::string file)
{
    this->newsocket = newsocket;
    this->file = file;
}

/**
 * @brief Method for running LDAP
 * 
 */
void LDAP::LDAPrun()
{
    ldapParser parser(this->newsocket);
    parser.msgParse(this->file);
}