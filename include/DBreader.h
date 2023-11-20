/**
 * @file DBreader.h
 * @author Filip Polomski
 * @brief DB reader header file
 * @version 1.0
 * @date 2023-11-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef DB_READER_H
#define DB_READER_H

#include "ldapParser.h"
#include <fstream>
#include <sstream>

struct Line{
    std::string uid;
    std::string cn;
    std::string mail;
};

enum FilterReturnType {
    FILTER_RETURN_BOOL_TRUE     = 1,
    FILTER_RETURN_BOOL_FALSE    = 0,
    FILTER_RETURN_UNDEFINED     = -1
};

class DBreader {
    private:
        std::string file;
        Line lineSplit(std::string& line);
        FilterReturnType validLineCheck(Line line, Filters filters);
        std::string stringToLower(std::string str);
    public: 
        int messageID;
        SearchRequest request; 
        DBreader(SearchRequest request, std::string file);
        std::vector<Line> Run();
};

#endif