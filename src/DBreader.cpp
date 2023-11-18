#include "../include/DBreader.h"

DBreader::DBreader(SearchRequest request, std::string file)
{
    this->request = request;
    this->file = file;
}

std::vector<Line> DBreader::Run()
{
    std::ifstream file(this->file);
    std::string line;

    Line splitLine;
    FilterReturnType validityOfLine = FILTER_RETURN_BOOL_FALSE;

    std::vector<Line> AllValidLines;

    while (std::getline(file, line))
    {
        splitLine = lineSplit(line.erase(line.size() - 1));
        // printf("%s\n", splitLine.cn.c_str()); //TODO remove
        // printf("%s\n", splitLine.uid.c_str()); //TODO remove
        // printf("%d\n", splitLine.uid.size()); //TODO remove
        // printf("%s\n", filters.value.assertionValue.c_str()); //TODO remove
        // printf("%d\n", filters.value.assertionValue.size()); //TODO remove
        // printf("%s\n", splitLine.mail.c_str()); //TODO remove

        validityOfLine = validLineCheck(splitLine, this->request.filters);
        // printf("validityOfLine: %d\n", validityOfLine);
        // printf("filter return type: %d\n", this->filters.type);

        if (validityOfLine == FILTER_RETURN_BOOL_TRUE)
        {
            printf("%s\n", splitLine.cn.c_str());
            printf("%s\n", splitLine.uid.c_str());
            printf("%s\n", splitLine.mail.c_str());
            AllValidLines.push_back(splitLine);
        }
        else if (validityOfLine == FILTER_RETURN_BOOL_FALSE)
        {
            continue;
        }
        else
        {
            fprintf(stderr, "Error: Unknown return type\n");
            exit(1); // TODO return msg with error code
        }
    }
    return AllValidLines;
}

Line DBreader::lineSplit(std::string& line)
{
    Line splitLine;

    std::vector<std::string> split;
    std::stringstream ss(line);
    std::string stringOfSplit;

    char delimiter = ';';

    while (std::getline(ss, stringOfSplit, delimiter))
    {
        split.push_back(stringOfSplit);
    }

    splitLine.cn = split[0];
    splitLine.uid = split[1];
    splitLine.mail = split[2];

    return splitLine;
}

FilterReturnType DBreader::validLineCheck(Line line, Filters filters)
{
    // if (line.cn.empty() || line.uid.empty() || line.mail.empty())
    // {
    //     // fprintf(stderr, "Error: Invalid line in DB file\n");
    //     // exit(1); // TODO return msg with error code
    // }
    int length = filters.subFilters.size();
    FilterReturnType wasAllValid = FILTER_RETURN_BOOL_FALSE;
    FilterReturnType wasAnyValid = FILTER_RETURN_BOOL_FALSE;

    std::string cn = "cn";
    std::string uid = "uid";
    std::string mail = "mail";

    // printf("Filter: %s\n", filters.value.attributeDesc.c_str());

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
                // else
                // {
                //     continue;
                // }
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
                if (filters.value.assertionValue == line.cn)
                {
                    return FILTER_RETURN_BOOL_TRUE;
                }
                return FILTER_RETURN_BOOL_FALSE;
            }
            else if (filters.value.attributeDesc == uid)
            {
                // printf("%s\n", line.uid.c_str()); //TODO remove
                // printf("%d\n", line.uid.size()); //TODO remove
                // printf("%s\n", filters.value.assertionValue.c_str()); //TODO remove
                // printf("%d\n", filters.value.assertionValue.size()); //TODO remove
                if (filters.value.assertionValue == line.uid)
                {
                    // printf("aaaaaaaaaaaaaaaaaaaaaa");
                    return FILTER_RETURN_BOOL_TRUE;
                }
                return FILTER_RETURN_BOOL_FALSE;
            }
            else if (filters.value.attributeDesc == mail)
            {
                if (filters.value.assertionValue == line.mail)
                {
                    return FILTER_RETURN_BOOL_TRUE;
                }
                return FILTER_RETURN_BOOL_FALSE;
            }
            else
            {
                printf("Error: Takovy sloupecek tam neni.\n"); // TODO SMAZ
            }
            break;
        case SUBSTRINGS:
            // printf("Substring initial %s\n", filters.substringFilter.substrings.initial.c_str());
            // printf("Substring any %s\n", filters.substringFilter.substrings.any[0].c_str());
            // printf("Substring final %s\n", filters.substringFilter.substrings.final.c_str());
            if (filters.substringFilter.type == cn)
            {
                if (filters.substringFilter.substrings.initial != "")
                {
                    if ((line.uid.find(filters.substringFilter.substrings.initial) != std::string::npos) && (line.uid.find(filters.substringFilter.substrings.initial) == 0))
                    {
                        wasAllValid = FILTER_RETURN_BOOL_TRUE;
                    }
                    else
                    {
                        return FILTER_RETURN_BOOL_FALSE;
                    }
                }
                if (filters.substringFilter.substrings.any.size() != 0)
                {
                    for (int i = 0; i < filters.substringFilter.substrings.any.size(); i++)
                    {
                        if (line.cn.find(filters.substringFilter.substrings.any[i]) != std::string::npos)
                        {
                            wasAllValid = FILTER_RETURN_BOOL_TRUE;
                        }
                        else
                        {
                            return FILTER_RETURN_BOOL_FALSE;
                        }
                    }
                }
                if (filters.substringFilter.substrings.final != "")
                {
                    if ((line.cn.find(filters.substringFilter.substrings.final) == (line.cn.size() - (filters.substringFilter.substrings.final.size() + 1))) != std::string::npos)
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
            else if (filters.substringFilter.type == uid)
            {
                if (filters.substringFilter.substrings.initial != "")
                {
                    if ((line.uid.find(filters.substringFilter.substrings.initial) != std::string::npos) && (line.uid.find(filters.substringFilter.substrings.initial) == 0))
                    {
                        wasAllValid = FILTER_RETURN_BOOL_TRUE;
                    }
                    else
                    {
                        return FILTER_RETURN_BOOL_FALSE;
                    }
                }
                if (filters.substringFilter.substrings.any.size() != 0)
                {
                    for (int i = 0; i < filters.substringFilter.substrings.any.size(); i++)
                    {
                        if (line.uid.find(filters.substringFilter.substrings.any[i]) != std::string::npos)
                        {
                            wasAllValid = FILTER_RETURN_BOOL_TRUE;
                        }
                        else
                        {
                            return FILTER_RETURN_BOOL_FALSE;
                        }
                    }
                }
                if (filters.substringFilter.substrings.final != "")
                {
                    if ((line.uid.find(filters.substringFilter.substrings.final) == (line.uid.size() - (filters.substringFilter.substrings.final.size() + 1))) != std::string::npos)
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
            else if (filters.substringFilter.type == mail)
            {
               if (filters.substringFilter.substrings.initial != "")
                {
                    if ((line.uid.find(filters.substringFilter.substrings.initial) != std::string::npos) && (line.uid.find(filters.substringFilter.substrings.initial) == 0))
                    {
                        wasAllValid = FILTER_RETURN_BOOL_TRUE;
                    }
                    else
                    {
                        return FILTER_RETURN_BOOL_FALSE;
                    }
                }
                if (filters.substringFilter.substrings.any.size() != 0)
                {
                    for (int i = 0; i < filters.substringFilter.substrings.any.size(); i++)
                    {
                        if (line.mail.find(filters.substringFilter.substrings.any[i]) != std::string::npos)
                        {
                            wasAllValid = FILTER_RETURN_BOOL_TRUE;
                        }
                        else
                        {
                            return FILTER_RETURN_BOOL_FALSE;
                        }
                    }
                }
                if (filters.substringFilter.substrings.final != "")
                {
                    if ((line.mail.find(filters.substringFilter.substrings.final) == (line.mail.size() - (filters.substringFilter.substrings.final.size() + 1))) != std::string::npos)
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
                printf("Error: Takovy sloupecek tam neni.\n"); // TODO SMAZ
            }
            break;
        default:
            return FILTER_RETURN_UNDEFINED;
    }


    return FILTER_RETURN_UNDEFINED;
}