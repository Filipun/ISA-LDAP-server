/**
 * @file DBreader.cpp
 * @author Filip Polomski, xpolom00 
 * @brief DB reader file with implementation of DBreader class
 * @version 1.0
 * @date 2023-11-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "../include/DBreader.h"

/**
 * @brief Construct a new DBreader:: DBreader object
 * 
 * @param request 
 * @param file 
 */
DBreader::DBreader(SearchRequest request, std::string file)
{
    this->request = request;
    this->file = file;
}

/**
 * @brief Method for running DBreader
 * 
 * @return std::vector<Line> Retuns all valid lines
 */
std::vector<Line> DBreader::Run()
{
    std::ifstream file(this->file);
    std::string line;

    Line splitLine;
    FilterReturnType validityOfLine = FILTER_RETURN_BOOL_FALSE;

    std::vector<Line> AllValidLines;

    while (std::getline(file, line))
    {
        // Split line
        splitLine = lineSplit(line.erase(line.size() - 1));

        // Check if line is valid
        validityOfLine = validLineCheck(splitLine, this->request.filters);

        if (validityOfLine == FILTER_RETURN_BOOL_TRUE)
        {
            AllValidLines.push_back(splitLine);
        }
        else if (validityOfLine == FILTER_RETURN_BOOL_FALSE)
        {
            continue;
        }
        else
        {
            fprintf(stderr, "Undefined atribut type, there are only these: cn, uid or mail!\n");
            exit(1);
        }
    }
    return AllValidLines;
}

/**
 * @brief Method for splitting line
 * 
 * @param line String to split
 * @return Line Returns struct with split line
 */
Line DBreader::lineSplit(std::string& line)
{
    Line splitLine;

    std::vector<std::string> split;
    std::stringstream ss(line);
    std::string stringOfSplit;

    char delimiter = ';';

    // Spliting string by delimiter
    while (std::getline(ss, stringOfSplit, delimiter))
    {
        split.push_back(stringOfSplit);
    }

    // Saving split line to struct
    splitLine.cn = split[0];
    splitLine.uid = split[1];
    splitLine.mail = split[2];

    return splitLine;
}

/**
 * @brief Method for checking if line is valid recursively
 * 
 * @param line Line to check
 * @param filters Filters to check
 * @return FilterReturnType Returns if line is valid or not.
 */
FilterReturnType DBreader::validLineCheck(Line line, Filters filters)
{
    int length = filters.subFilters.size();
    FilterReturnType wasAllValid = FILTER_RETURN_BOOL_FALSE;
    FilterReturnType wasAnyValid = FILTER_RETURN_BOOL_FALSE;

    std::string cn = "cn";
    std::string uid = "uid";
    std::string mail = "mail";

    switch (filters.type)
    {
        case AND:
            for (int i = 0; i < length; i++)
            {
                if (validLineCheck(line, filters.subFilters[i]))
                {
                    continue;
                }
                return FILTER_RETURN_BOOL_FALSE;
            }
            return FILTER_RETURN_BOOL_TRUE;
        case OR:
            for (int i = 0; i < length; i++)
            {
                if (validLineCheck(line, filters.subFilters[i]))
                {
                    wasAnyValid = FILTER_RETURN_BOOL_TRUE;
                }
            }
            if (wasAnyValid)
            {
                return FILTER_RETURN_BOOL_TRUE;
            }
            return FILTER_RETURN_BOOL_FALSE;
        case NOT:
            if (validLineCheck(line, filters.subFilters[0]))
            {
                return FILTER_RETURN_BOOL_FALSE;
            }
            return FILTER_RETURN_BOOL_TRUE;
        case EQUALITY_MATCH:

            if (filters.value.attributeDesc == cn)
            {
                if (stringToLower(filters.value.assertionValue) == stringToLower(line.cn))
                {
                    return FILTER_RETURN_BOOL_TRUE;
                }
                return FILTER_RETURN_BOOL_FALSE;
            }
            else if (filters.value.attributeDesc == uid)
            {
                if (stringToLower(filters.value.assertionValue) == stringToLower(line.uid))
                {
                    return FILTER_RETURN_BOOL_TRUE;
                }
                return FILTER_RETURN_BOOL_FALSE;
            }
            else if (filters.value.attributeDesc == mail)
            {
                if (stringToLower(filters.value.assertionValue) == stringToLower(line.mail))
                {
                    return FILTER_RETURN_BOOL_TRUE;
                }
                return FILTER_RETURN_BOOL_FALSE;
            }
            break;
        case SUBSTRINGS:
            // Substring in cn
            if (filters.substringFilter.type == cn)
            {
                // Initial substring
                line.cn = stringToLower(line.cn);
                if (filters.substringFilter.substrings.initial != "")
                {
                    filters.substringFilter.substrings.initial = stringToLower(filters.substringFilter.substrings.initial);
                    if ((line.uid.find(filters.substringFilter.substrings.initial) != std::string::npos) && (line.uid.find(filters.substringFilter.substrings.initial) == 0))
                    {
                        wasAllValid = FILTER_RETURN_BOOL_TRUE;
                    }
                    else
                    {
                        return FILTER_RETURN_BOOL_FALSE;
                    }
                }
                // Any substring
                if (filters.substringFilter.substrings.any.size() != 0)
                {
                    for (int i = 0; i < (int)filters.substringFilter.substrings.any.size(); i++)
                    {
                        if (line.cn.find(stringToLower(filters.substringFilter.substrings.any[i])) != std::string::npos)
                        {
                            wasAllValid = FILTER_RETURN_BOOL_TRUE;
                        }
                        else
                        {
                            return FILTER_RETURN_BOOL_FALSE;
                        }
                    }
                }
                // Final substring
                if (filters.substringFilter.substrings.final != "")
                {
                    filters.substringFilter.substrings.final = stringToLower(filters.substringFilter.substrings.final);
                    if ((line.cn.find(filters.substringFilter.substrings.final) == (line.cn.size() - (filters.substringFilter.substrings.final.size() + 1))) && (line.cn.find(filters.substringFilter.substrings.final) != std::string::npos)) //TODO koukni se na to jeste more
                    {
                        wasAllValid = FILTER_RETURN_BOOL_TRUE;
                    }
                    else
                    {
                        return FILTER_RETURN_BOOL_FALSE;
                    }
                }
                if (wasAllValid)
                {
                    return FILTER_RETURN_BOOL_TRUE;
                }
                return FILTER_RETURN_BOOL_FALSE;
            }
            // Substring in uid
            else if (filters.substringFilter.type == uid)
            {
                // Initial substring
                line.uid = stringToLower(line.uid);
                if (filters.substringFilter.substrings.initial != "")
                {
                    filters.substringFilter.substrings.initial = stringToLower(filters.substringFilter.substrings.initial);
                    if ((line.uid.find(filters.substringFilter.substrings.initial) != std::string::npos) && (line.uid.find(filters.substringFilter.substrings.initial) == 0))
                    {
                        wasAllValid = FILTER_RETURN_BOOL_TRUE;
                    }
                    else
                    {
                        return FILTER_RETURN_BOOL_FALSE;
                    }
                }
                // Any substring
                if (filters.substringFilter.substrings.any.size() != 0)
                {
                    for (int i = 0; i < (int)filters.substringFilter.substrings.any.size(); i++)
                    {
                        printf("%s\n", line.uid.c_str());
                        printf("%s\n", filters.substringFilter.substrings.any[i].c_str());
                        if (line.uid.find(stringToLower(filters.substringFilter.substrings.any[i])) != std::string::npos)
                        {
                            wasAllValid = FILTER_RETURN_BOOL_TRUE;
                        }
                        else
                        {
                            return FILTER_RETURN_BOOL_FALSE;
                        }
                    }
                }
                // Final substring
                if (filters.substringFilter.substrings.final != "")
                {
                    filters.substringFilter.substrings.final = stringToLower(filters.substringFilter.substrings.final);
                    if ((line.uid.find(filters.substringFilter.substrings.final) == (line.uid.size() - (filters.substringFilter.substrings.final.size() + 1))) && (line.uid.find(filters.substringFilter.substrings.final) != std::string::npos))
                    {
                        wasAllValid = FILTER_RETURN_BOOL_TRUE;
                    }
                    else
                    {
                        return FILTER_RETURN_BOOL_FALSE;
                    }
                }
                if (wasAllValid)
                {
                    return FILTER_RETURN_BOOL_TRUE;
                }
                return FILTER_RETURN_BOOL_FALSE;
            }
            // Substring in mail
            else if (filters.substringFilter.type == mail)
            {
                // Initial substring
                line.mail = stringToLower(line.mail);
                if (filters.substringFilter.substrings.initial != "")
                {
                    filters.substringFilter.substrings.initial = stringToLower(filters.substringFilter.substrings.initial);
                    if ((line.mail.find(filters.substringFilter.substrings.initial) != std::string::npos) && (line.mail.find(filters.substringFilter.substrings.initial) == 0))
                    {
                        wasAllValid = FILTER_RETURN_BOOL_TRUE;
                    }
                    else
                    {
                        return FILTER_RETURN_BOOL_FALSE;
                    }
                }
                // Any substring
                if (filters.substringFilter.substrings.any.size() != 0)
                {
                    for (int i = 0; i < (int)filters.substringFilter.substrings.any.size(); i++)
                    {
                        if (line.mail.find(stringToLower(filters.substringFilter.substrings.any[i])) != std::string::npos)
                        {
                            wasAllValid = FILTER_RETURN_BOOL_TRUE;
                        }
                        else
                        {
                            return FILTER_RETURN_BOOL_FALSE;
                        }
                    }
                }
                // Final substring
                if (filters.substringFilter.substrings.final != "")
                {
                    filters.substringFilter.substrings.final = stringToLower(filters.substringFilter.substrings.final);
                    if ((line.mail.find(filters.substringFilter.substrings.final) == (line.mail.size() - (filters.substringFilter.substrings.final.size() + 1))) && (line.mail.find(filters.substringFilter.substrings.final) != std::string::npos))
                    {
                        wasAllValid = FILTER_RETURN_BOOL_TRUE;
                    }
                    else
                    {
                        return FILTER_RETURN_BOOL_FALSE;
                    }
                }
                if (wasAllValid)
                {
                    return FILTER_RETURN_BOOL_TRUE;
                }
                return FILTER_RETURN_BOOL_FALSE;
            }
            else
            {
                fprintf(stderr, "Three value logic is not supported\nFilter by rows cn, uid or mail!\n");
                exit(1); 
            }
            break;
        default:
            return FILTER_RETURN_UNDEFINED;
    }
    return FILTER_RETURN_UNDEFINED;
}

/**
 * @brief Method for converting string to lower case
 * 
 * @param str String to convert
 * @return std::string Returns converted string.
 */
std::string DBreader::stringToLower(std::string str)
{
    std::string lowerStr;
    
    for (char c : str)
    {
        lowerStr += tolower(c);
    }

    return lowerStr;
}