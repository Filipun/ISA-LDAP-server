#include "../include/ldapParser.h"
#include "../include/sender.h"

ldapParser::ldapParser(int newsocket)
{
    // this->byteVector;
    this->indexOfVector = 0;
    this->maxIndexOfVector = 0;
    this->newsocket = newsocket;
}

void ldapParser::msgParse()
{
    //  Load ldap message to vector of bytes
    while (1)
    {
        char byte; 
        int readresult;
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
                printf("%02X ", (unsigned char)byte); // TODO check print
                i++;
                if (i % 8 == 0 && i != 0)
                {
                    printf("  ");
                }
            }
            
            this->maxIndexOfVector = this->byteVector.size() - 1;
            this->parseLDAPMessage();   

            // Clean vector
            this->byteVector.clear();
            this->indexOfVector = 0;
        }
    }
}

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

uint64_t ldapParser::getLength()
{
    uint64_t length = 0;
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
    else // Length in short form
    {
        length = this->byteVector[this->indexOfVector] & 0x7f;
        this->indexIncrement();
    }
    return length;
}

void ldapParser::parseLDAPMessage()
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

    printf("MessageID: %d\n", MessageID); // TODO check print
    
    // Check tag of LDAP message
    int ldapVersion = 0;
    std::string name = "";
    Sender sender(this->newsocket);

    SearchRequest request;

    // Messages 
    switch (this->byteVector[this->indexOfVector])
    {
        case BIND_REQUEST:
            printf("BIND_REQUEST\n");
            this->indexIncrement();
            this->parseBindRequest();
            sender.BindResponse(MessageID, 0, "", "");
            break;
        case SEARCH_REQUEST:
            printf("SEARCH_REQUEST\n");
            this->indexIncrement();
            request = this->parseSearchRequest();
            printf("mame po parse\n");
            break;
        case UNBIND_REQUEST:
            printf("UNBIND_REQUEST\n");
            break;
        default:
            fprintf(stderr, "Wrong BER message!\nExpected BIND_REQUEST, SEARCH_REQUEST or UNBIND_REQUEST tag!\n");
            exit(1);
    }

}

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

    for (u_int64_t i = 0; i < lengthOfOctetString; i++)
    {
        octetString += this->byteVector[this->indexOfVector];  // TODO bude to fungovat a prepisovat z hexa na pismenka?
        this->indexIncrement();
    }

    return octetString;
}

int ldapParser::parseInteger()
{
    // Check integer tag
    if (this->byteVector[this->indexOfVector] != INTEGER)
    {
        fprintf(stderr, "Wrong BER message!\nExpected INTEGER tag!\n");
        exit(1);
    }
    this->indexIncrement();

    //this->checkLengthOfTags();

    uint64_t lengthOfInteger = this->getLength();

    int integer = 0;

    for (u_int64_t i = 0; i < lengthOfInteger; i++)
    {
        integer = (integer << 8) | byteVector[this->indexOfVector];
        this->indexIncrement();
    }

    return integer;
}

Scope ldapParser::parseScope()
{
    if (this->byteVector[this->indexOfVector] != ENUM)
    {
        fprintf(stderr, "Wrong BER message!\nExpected ENUM tag!\n");
        exit(1);
    }
    this->indexIncrement();

    // Enum length should be one TODO check
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

bool ldapParser::parseBoolean()
{
    if (this->byteVector[this->indexOfVector] != BOOL)
    {
        fprintf(stderr, "Wrong BER message!\nExpected BOOLEAN tag!\n");
        exit(1);
    }
    printf("Boolean: %02x\n", this->byteVector[this->indexOfVector]); // TODO check print
    this->indexIncrement();

    printf("Boolean length: %02x\n", this->byteVector[this->indexOfVector]); // TODO check print
    // Bool length should be one TODO check
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
    int indexBeforeSubfilter = 0; // TODO drzi index pred subfilterem
    int lengthOfSubfilter = 0; // TODO drzi delku subfilteru
    Filters newFilter = Filters();
    switch (this->byteVector[this->indexOfVector])
    {
        case 0xa0:
            newFilter.type = AND;
            this->indexIncrement();
            lengthOfFilter = byteVector[this->indexOfVector];
            endOfFilter = this->indexOfVector + lengthOfFilter;
            // int indexOfSubFilter = 0; TODO to asi ani neni potreba
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
        case 0xa1:
            newFilter.type = OR;
            this->indexIncrement();
            lengthOfFilter = byteVector[this->indexOfVector];
            endOfFilter = this->indexOfVector + lengthOfFilter;
            
            // int indexOfSubFilter = 0; TODO to asi ani neni potreba
            while (this->indexOfVector < endOfFilter) 
            { 
                this->indexIncrement();
                // printf("Byte %02X\n", this->byteVector[this->indexOfVector]); // TODO check print
                // printf("Byte delka: %02X\n", this->byteVector[this->indexOfVector + 1]); // TODO check print
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
        case 0xa2:
            newFilter.type = NOT;
            this->indexIncrement();
            lengthOfFilter = byteVector[this->indexOfVector];
            endOfFilter = this->indexOfVector + lengthOfFilter;
            // int indexOfSubFilter = 0; TODO to asi ani neni potreba
            this->indexIncrement();
            // TODO mel by tam byt jen jeden 
            newFilter.subFilters.push_back(loadFilters());
            break;
        case 0xa3:
            newFilter.type = EQUALITY_MATCH;
            this->indexIncrement(); // Skip length of substring filter
            this->indexIncrement();
            newFilter.value.assertionValue = parseOctetString();
            newFilter.value.attributeDesc = parseOctetString();

            break;
        case 0xa4:
            newFilter.type = SUBSTRINGS;
            this->indexIncrement(); // Skip length of substring filter
            this->indexIncrement();
            newFilter.substringFilter.type = parseOctetString();
            if (this->byteVector[this->indexOfVector] != SEQUENCE)
            {
                fprintf(stderr, "Wrong BER message!\nExpected SEQUENCE tag!\n");
                exit(1);
            }
            this->indexIncrement();
            lengthOfSubstringSequence = this->byteVector[this->indexOfVector];
            endOfSubstringSequence = this->indexOfVector + lengthOfSubstringSequence;
            this->indexIncrement();

            newFilter.substringFilter.substrings.initial = std::string();
            newFilter.substringFilter.substrings.final = std::string();
            newFilter.substringFilter.substrings.any = std::vector<std::string>();
            // TODO kontrola jestli je vse na svem miste pouze 1x initial, 1x final a final muze byt jen na konci a initial na zacatku
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
            return newFilter;
    }
    this->indexIncrement();

    return newFilter;
}

void ldapParser::checkLengthOfMessage()
{
    uint64_t lengthOfTag = this->getLength();
    this->indexOfVector -= 1; // function getLength() increment index of vector
    printf("\nLength of tag: %ld\n", lengthOfTag);  // TODO check print
    printf("Length of vector: %d\n", this->maxIndexOfVector - this->indexOfVector);  // TODO check print
    if (lengthOfTag != static_cast<u_int64_t>(this->maxIndexOfVector - this->indexOfVector))
    {
        fprintf(stderr, "Wrong BER message!\nLength of Message is not right!");
        exit(1);
    }
}

std::vector<std::string> ldapParser::parseAttributeDescriptionList()
{
    std::vector<std::string> attributes;
    if (this->byteVector[this->indexOfVector] != SEQUENCE)
    {
        fprintf(stderr, "Wrong BER message!\nExpected SEQUENCE tag!\n");
        exit(1);
    }
    this->indexIncrement();

    int lengthOfSequence = this->byteVector[this->indexOfVector];
    int endOfSequence = this->indexOfVector + lengthOfSequence;
    this->indexIncrement();
    printf("Length of sequence: %02x\n", this->byteVector[this->indexOfVector]); // TODO check print

    while (this->indexOfVector < endOfSequence)
    {
        attributes.push_back(parseOctetString());
    }

    return attributes;
}

/**
 * @brief Function parse bind request
 * 
 */
void ldapParser::parseBindRequest()
{
    // Check length of bind request
    this->checkLengthOfMessage();
        
    int ldapVersion = 0;
    std::string ldapdn = "";
    this->indexIncrement();

    // Check LDAP version TODO
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
            printf("Password is empty and thats right!\n"); // TODO check print
        }
        // TODO kdyz neni prazdny
    }
}

/**
 * @brief Function parse search request
 * 
 */
SearchRequest ldapParser::parseSearchRequest()
{
    SearchRequest request;

    // Check length of search request
    this->checkLengthOfMessage();
    printf("Length of search request: %ld\n", this->getLength()); // TODO check print
    //this->indexIncrement();

    request.ldapdn = this->parseOctetString(); 
    printf("LDAPDN: %s\n", request.ldapdn.c_str()); // TODO check print
    //this->indexIncrement();

    request.scope = this->parseScope();
    printf("Scope: %d\n", request.scope); // TODO check print
    this->indexIncrement();

    // TODO skip aliases
    this->indexIncrement();
    this->indexIncrement();
    this->indexIncrement();

    request.sizeLimit = this->parseInteger();
    printf("Size limit: %d\n", request.sizeLimit); // TODO check print
    //this->indexIncrement();

    request.timeLimit = this->parseInteger();
    printf("Time limit: %d\n", request.timeLimit); // TODO check print
    //this->indexIncrement();

    request.typesOnly = this->parseBoolean();
    printf("Types only: %d\n", request.typesOnly); // TODO check print
    this->indexIncrement();

    // request.filter = this->parseFilters();
    // printf("Filter: %02X\n", request.filter); // TODO check print

    request.filters = this->loadFilters();
    
    if (this->byteVector[this->indexOfVector] == SEQUENCE)
    {
        request.attributes = this->parseAttributeDescriptionList();
    }
    
    // printf("Filters: %02X\n", request.filters.type); // TODO check print
    // printf("Filters: %02X\n", request.filters.subFilters[0].type); // TODO check print
    // printf("Filters: %02X\n", request.filters.subFilters[1].type); // TODO check print
    // printf("Filters: %02X\n", request.filters.subFilters[2].type); // TODO check print
    // printf("Filters: %s\n", request.filters.subFilters[2].substringFilter.type.c_str()); // TODO check print
    // printf("Filters: %s\n", request.filters.subFilters[2].substringFilter.substrings.any.c_str()); // TODO check print
    // printf("Filters: %s\n", request.filters.subFilters[2].substringFilter.substrings.final.c_str()); // TODO check print
    // printf("Filters: %s\n", request.filters.subFilters[2].substringFilter.substrings.initial.c_str()); // TODO check print

    return request;
}