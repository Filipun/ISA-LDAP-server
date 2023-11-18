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
    public: 
        SearchRequest request; 
        DBreader(SearchRequest request, std::string file);
        std::vector<Line> Run();
};

#endif