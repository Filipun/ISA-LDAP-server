/**
 * @file ldapParser.cpp
 * @author Filip Polomski, xpolom00
 * @brief LDAP parser file with implementation of LDAP parser class
 * @version 1.0
 * @date 2023-11-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "../include/ldapParser.h"
#include "../include/sender.h"
#include "../include/DBreader.h"

/**
 * @brief Construct a new ldap Parser::ldap Parser object
 * 
 * @param newsocket 
 */
ldapParser::ldapParser(int newsocket)
{
    // this->byteVector;
    this->indexOfVector = 0;
    this->maxIndexOfVector = 0;
    this->newsocket = newsocket;
}

/**
 * @brief Method for parsing message
 * 
 * @param file 
 */
void ldapParser::msgParse(std::string file)
{
    //  Load ldap message to vector of bytes
    while (1)
    {
        char byte; 
        int readresult;

        // Set monitoring socket
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(newsocket, &read_fds);

        // Wait for message
        int ready = select(newsocket + 1, &read_fds, NULL, NULL, NULL);

        if (ready == -1)
        {
            fprintf(stderr, "Error while select!\n");
            exit(1);
        }
        // Loading message with controll print
        else
        {
            int i = 0;
            int j = 0;
            while ((readresult = read(this->newsocket, &byte, 1)) > 0)
            {
                this->byteVector.push_back((unsigned char)byte);
                if (i == 0 || i % 16 == 0)
                {
                    printf("\n%04d:   ", j * 10);
                    j++;
                    i = 0;	
                }
                printf("%02X ", (unsigned char)byte);
                i++;
                if (i % 8 == 0 && i != 0)
                {
                    printf("  ");
                }
            }
            
            // Calling method for parsing and for unbind request ending
            this->maxIndexOfVector = this->byteVector.size() - 1;
            if (this->parseLDAPMessage(file))
            {
                break;
            }   

            // Clean vector
            this->byteVector.clear();
            this->indexOfVector = 0;
        }
    }
}

/**
 * @brief Method for incrementing index of vector and checking if it is not out of range.
 * 
 */
void ldapParser::indexIncrement()
{
    if (this->indexOfVector > this->maxIndexOfVector)
    {
        fprintf(stderr, "Index of vector out of range!");
        exit(1);
    }
    else if (this->indexOfVector == this->maxIndexOfVector)
    {
        printf("End of vector reached!\n");
    }
    else
    {
        this->indexOfVector += 1;
    }
}

/**
 * @brief Method for getting length of tag.
 * 
 * @return uint64_t Length of tag.
 */
uint64_t ldapParser::getLength()
{
    uint64_t length = 0;
    // Check if length is in long form
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
    // Length in short form
    else 
    {
        length = this->byteVector[this->indexOfVector] & 0x7f;
        this->indexIncrement();
    }

    return length;
}

/**
 * @brief Method for parsing LDAP message and calling database reader for filtering and then sender for sending responses.
 * 
 * @param file input file
 * @return bool True if message is unbind request, false otherwise. 
 */
bool ldapParser::parseLDAPMessage(std::string file)
{
    // Check SEQUENCE tag
    if (this->byteVector[this->indexOfVector] != SEQUENCE)
    {
        printf("%02x, %d", this->byteVector[this->indexOfVector], this->indexOfVector);
        fprintf(stderr, "Wrong BER message!\n Expected SEQUENCE tag!\n");
        exit(1);
    }
    this->indexIncrement();

    // Check length of SEQUENCE
    this->checkLengthOfMessage();
    this->indexIncrement();

    int MessageID = this->parseInteger();
    

    Sender sender(this->newsocket);

    SearchRequest request;

    DBreader dbReader(request, file);

    std::vector<Line> AllValidLines;

    // Parsing messages and sending responses
    switch (this->byteVector[this->indexOfVector])
    {
        case BIND_REQUEST:
            printf("BIND_REQUEST\n");
            this->indexIncrement();
            this->parseBindRequest();
            sender.BindResponse(MessageID, 0, "");
            return false;
        case SEARCH_REQUEST:
            printf("SEARCH_REQUEST\n");
            this->indexIncrement();
            request = this->parseSearchRequest();
            printf("mame po parse\n");
            dbReader.request = request;
            dbReader.messageID = MessageID;
            AllValidLines = dbReader.Run();
            // Send search result entrys
            if (request.sizeLimit != 0)
            {
                if ((int)AllValidLines.size() > request.sizeLimit)
                {
                    AllValidLines.resize(request.sizeLimit);
                }
            }
            for (auto line : AllValidLines)
            {
                sender.SearchResultEntry(request, line, MessageID);
            }
            sender.SearchResultDone(MessageID, "", 0);
            return false;
        case UNBIND_REQUEST:
            printf("UNBIND_REQUEST\n");
            return true;
        default:
            fprintf(stderr, "Wrong BER message!\nExpected BIND_REQUEST, SEARCH_REQUEST or UNBIND_REQUEST tag!\n");
            exit(1);
    }

}

/*
* @brief Method for parsing octet string.
* 
* @return std::string Octet string.
*/
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

    // Loading octet string
    for (u_int64_t i = 0; i < lengthOfOctetString; i++)
    {
        octetString += this->byteVector[this->indexOfVector];
        this->indexIncrement();
    }

    return octetString;
}

/**
 * @brief Method for parsing integer.
 * 
 * @return int Integer.
 */
int ldapParser::parseInteger()
{
    // Check integer tag
    if (this->byteVector[this->indexOfVector] != INTEGER)
    {
        fprintf(stderr, "Wrong BER message!\nExpected INTEGER tag!\n");
        exit(1);
    }
    this->indexIncrement();

    uint64_t lengthOfInteger = this->getLength();

    int integer = 0;

    // Loading integer
    for (u_int64_t i = 0; i < lengthOfInteger; i++)
    {
        integer = (integer << 8) | byteVector[this->indexOfVector];
        this->indexIncrement();
    }

    return integer;
}

/**
 * @brief Method for parsing scope.
 * 
 * @return Scope Scope.
 */
Scope ldapParser::parseScope()
{
    if (this->byteVector[this->indexOfVector] != ENUM)
    {
        fprintf(stderr, "Wrong BER message!\nExpected ENUM tag!\n");
        exit(1);
    }
    this->indexIncrement();

    // Enum length should be one 
    this->indexIncrement();

    Scope scope;
    switch (this->byteVector[this->indexOfVector])
    {
        case BASE_OBJECT:
            scope = BASE_OBJECT;
            break;
        case SINGLE_LEVEL:
            scope = SINGLE_LEVEL;
            break;
        case WHOLE_SUBTREE:
            scope = WHOLE_SUBTREE;
            break;
        default:
            fprintf(stderr, "Wrong BER message!\nExpected BASE_OBJECT, SINGLE_LEVEL or WHOLE_SUBTREE tag!\n");
            exit(1);
    }
    return scope;
}

/**
 * @brief Method for parsing boolean.
 * 
 * @return bool Boolean.
 */
bool ldapParser::parseBoolean()
{
    if (this->byteVector[this->indexOfVector] != BOOL)
    {
        fprintf(stderr, "Wrong BER message!\nExpected BOOLEAN tag!\n");
        exit(1);
    }

    // Boolean length should be one, skip
    this->indexIncrement();

    // Boolean value
    this->indexIncrement();

    bool boolean;

    if (this->byteVector[this->indexOfVector] == 0x00)
    {
        boolean = false;
    }
    else if (this->byteVector[this->indexOfVector] == 0xff)
    {
        boolean = true;
    }
    else
    {
        fprintf(stderr, "Wrong BER message!\nExpected 0x00 or 0xFF!\n");
        exit(1);
    }

    return boolean;
}

/**
 * @brief Loads filters from byte vector recursively.
 * 
 * @return Filters struct with loaded filters.
 */
Filters ldapParser::loadFilters()
{
    int lengthOfFilter = 0;
    int endOfFilter = 0;
    int lengthOfSubstringSequence = 0;
    int endOfSubstringSequence = 0;
    int indexBeforeSubfilter = 0; 
    int lengthOfSubfilter = 0; 
    Filters newFilter = Filters();
    switch (this->byteVector[this->indexOfVector])
    {
        // AND filter
        case 0xa0:
            newFilter.type = AND;

            // Finding end of filter
            this->indexIncrement();
            lengthOfFilter = byteVector[this->indexOfVector];
            endOfFilter = this->indexOfVector + lengthOfFilter;

            // Loading subfilters
            while (this->indexOfVector < endOfFilter) 
            { 
                this->indexIncrement();    // Shift to next filter
                indexBeforeSubfilter = this->indexOfVector;
                lengthOfSubfilter = this->byteVector[this->indexOfVector + 1];
                newFilter.subFilters.push_back(loadFilters());
                // Insures that index will not be out of range and sets it to next filter
                if ((indexBeforeSubfilter + lengthOfSubfilter) < this->maxIndexOfVector)
                {
                    this->indexOfVector = indexBeforeSubfilter + lengthOfSubfilter + 1;
                }
                else
                {
                    break;
                }
            }
            break;
        // OR filter
        case 0xa1:
            newFilter.type = OR;

            // Finding end of filter
            this->indexIncrement();
            lengthOfFilter = byteVector[this->indexOfVector];
            endOfFilter = this->indexOfVector + lengthOfFilter;
            
            // Loading subfilters
            while (this->indexOfVector < endOfFilter) 
            { 
                this->indexIncrement();    // Shift to next filter
                indexBeforeSubfilter = this->indexOfVector;
                lengthOfSubfilter = this->byteVector[this->indexOfVector + 1];
                newFilter.subFilters.push_back(loadFilters());
               
                // Insures that index will not be out of range and sets it to next filter
                if ((indexBeforeSubfilter + lengthOfSubfilter) < this->maxIndexOfVector)
                {
                    this->indexOfVector = indexBeforeSubfilter + lengthOfSubfilter + 1;
                }
                else
                {
                    break;
                }
            }
            break;
        // NOT filter
        case 0xa2:
            newFilter.type = NOT;

            // Skip length of filter, NOT can have only one subfilter
            this->indexIncrement();

            this->indexIncrement();
            newFilter.subFilters.push_back(loadFilters());
            break;
        // EQUALITY_MATCH filter
        case 0xa3:
            newFilter.type = EQUALITY_MATCH;
            // Skip length of filter
            this->indexIncrement(); 
            this->indexIncrement();
            newFilter.value.attributeDesc = parseOctetString();
            newFilter.value.assertionValue = parseOctetString();
            break;
        // SUBSTRINGS filter
        case 0xa4:
            newFilter.type = SUBSTRINGS;
            // Skip length of filter
            this->indexIncrement(); 
            this->indexIncrement();
            newFilter.substringFilter.type = parseOctetString();
            if (this->byteVector[this->indexOfVector] != SEQUENCE)
            {
                fprintf(stderr, "Wrong BER message!\nExpected SEQUENCE tag!\n");
                exit(1);
            }

            // Finding end of substring sequence
            this->indexIncrement();
            lengthOfSubstringSequence = this->byteVector[this->indexOfVector];
            endOfSubstringSequence = this->indexOfVector + lengthOfSubstringSequence;
            this->indexIncrement();

            newFilter.substringFilter.substrings.initial = std::string();
            newFilter.substringFilter.substrings.final = std::string();
            newFilter.substringFilter.substrings.any = std::vector<std::string>();

            // Loading substrings
            while (this->indexOfVector < endOfSubstringSequence)
            {
                switch (this->byteVector[this->indexOfVector])
                {
                case SUB_INITIAL:
                    this->byteVector[this->indexOfVector] = OCTET_STRING; 
                    newFilter.substringFilter.substrings.initial = parseOctetString();
                    break;
                case SUB_ANY:
                    this->byteVector[this->indexOfVector] = OCTET_STRING; 
                    newFilter.substringFilter.substrings.any.push_back(parseOctetString());
                    break;
                case SUB_FINAL:
                    this->byteVector[this->indexOfVector] = OCTET_STRING; 
                    newFilter.substringFilter.substrings.final = parseOctetString();
                    break;
                default:
                    fprintf(stderr, "Wrong BER message!\nExpected SUB_INITIAL, SUB_ANY or SUB_FINAL tag!\n");
                    exit(1);
                }
            }
            break;
        default:
            fprintf(stderr, "Use only AND, OR, NOT, EQUALITY_MATCH or SUBSTRINGS operators!\n");
            exit(1);
    }
    this->indexIncrement();

    return newFilter;
}

/**
 * @brief Method for checking length of message.
 * 
 */
void ldapParser::checkLengthOfMessage()
{
    uint64_t lengthOfTag = this->getLength();
    this->indexOfVector -= 1; // function getLength() increment index of vector

    if (lengthOfTag != static_cast<u_int64_t>(this->maxIndexOfVector - this->indexOfVector))
    {
        fprintf(stderr, "Wrong BER message!\nLength of Message is not right!");
        exit(1);
    }
}

/**
 * @brief Method for parsing attribute description list.
 * 
 * @return std::vector<std::string> Vector of attribute descriptions.
 */
std::vector<std::string> ldapParser::parseAttributeDescriptionList()
{
    std::vector<std::string> attributes;
    if (this->byteVector[this->indexOfVector] != SEQUENCE)
    {
        fprintf(stderr, "Wrong BER message!\nExpected SEQUENCE tag!\n");
        exit(1);
    }

    this->indexIncrement();
    // Check if attribute description list is empty
    if (this->byteVector[this->indexOfVector] == 0x00)
    {
        attributes[0] = "";
        return attributes;
    }

    // Finding end of attribute description list
    int lengthOfSequence = this->byteVector[this->indexOfVector];
    int endOfSequence = this->indexOfVector + lengthOfSequence;
    this->indexIncrement();

    // Loading attribute descriptions
    while (this->indexOfVector < endOfSequence)
    {
        attributes.push_back(parseOctetString());
    }

    return attributes;
}

/**
 * @brief Method parse bind request
 * 
 */
void ldapParser::parseBindRequest()
{
    // Check length of bind request
    this->checkLengthOfMessage();
        
    int ldapVersion = 0;
    std::string ldapdn = "";
    this->indexIncrement();

    ldapVersion = this->parseInteger();
    printf("LDAP version: %d\n", ldapVersion); // TODO check print

    ldapdn = this->parseOctetString();
    printf("LDAPDN: %s\n", ldapdn.c_str()); // TODO check print

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
        }
    }
}

/**
 * @brief Method for parsing search request.
 * 
 * @return SearchRequest Search request.
 */
SearchRequest ldapParser::parseSearchRequest()
{
    SearchRequest request;

    // Check length of search request
    this->checkLengthOfMessage();
    this->indexIncrement();

    request.ldapdn = this->parseOctetString(); 
    printf("LDAPDN: %s\n", request.ldapdn.c_str()); // TODO check print

    request.scope = this->parseScope();
    printf("Scope: %d\n", request.scope); // TODO check print
    this->indexIncrement();

    // Skip aliases
    this->indexIncrement();
    this->indexIncrement();
    this->indexIncrement();

    request.sizeLimit = this->parseInteger();
    printf("Size limit: %d\n", request.sizeLimit);

    request.timeLimit = this->parseInteger();
    printf("Time limit: %d\n", request.timeLimit);

    request.typesOnly = this->parseBoolean();
    printf("Types only: %d\n", request.typesOnly); 
    this->indexIncrement();

    request.filters = this->loadFilters();
    
    //this->indexOfVector -= 1;
    // Load attributes
    if (this->byteVector[this->indexOfVector] == SEQUENCE)
    {
        if (this->byteVector[this->indexOfVector + 1] == 0x00)
        {
            request.attributes = std::vector<std::string>();
        }
        else
        {
            request.attributes = this->parseAttributeDescriptionList();
        }
    }
    return request;
}