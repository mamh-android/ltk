/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include <cstring>
#include "STAFCommandParser.h"

int main(void)
{
    STAFCommandParser parser;

    parser.addOption("START", 1, STAFCommandParser::kValueNotAllowed);
    parser.addOption("COMMAND", 1, STAFCommandParser::kValueRequired);
    parser.addOption("VAR", 0, STAFCommandParser::kValueRequired);
    parser.addOption("ENV", 0, STAFCommandParser::kValueRequired);
    parser.addOption("MAYBE", 1, STAFCommandParser::kValueAllowed);
    parser.addOption("NEEDER", 1, STAFCommandParser::kValueNotAllowed);
    parser.addOption("NEEDEE", 1, STAFCommandParser::kValueNotAllowed);

    parser.addOptionGroup("START", 1, 1);
    parser.addOptionGroup("COMMAND", 1, 1);
    parser.addOptionGroup("VAR ENV", 0, 1);
    parser.addOptionNeed("NEEDER", "NEEDEE");

    const char *strings[] = {
        "COMMAND hello",
        "START hello",
        "START hello COMMAND data",
        "START COMMAND VAR data",
        "START COMMAND \"VAR\"",
        "START COMMAND :3:VAR",
        "START COMMAND \\VAR",
        "START COMMAND hello",
        "START COMMAND :11:hello world",
        "START COMMAND \"hello world\"",
        "START COMMAND \"\\\"hello\\\\world\\\"\"",
        "START COMMAND hello MAYBE",
        "START COMMAND hello MAYBE yes",
        "START COMMAND hello COMMAND stuff",
        "START COMMAND hello VAR data1 VAR data2",
        "START COMMAND hello VAR data1 ENV data2",
        "START COMMAND hello NEEDER",
        "START COMMAND hello NEEDEE",
        "START COMMAND hello NEEDER NEEDEE",
        "start command hello"
    };

    for (int i = 0; i < (sizeof(strings) / sizeof(char *)); ++i)
    {
        STAFCommandParseResultPtr result = parser.parse(strings[i]);

        if (result->rc != 0)
        {
            cout << "Error parsing: " << strings[i] << endl;
            cout << "RC: " << result->rc << ", ErrorBuffer: "
                 << result->errorBuffer << endl;
        }
        else
        {
            cout << "Successfully parsed: " << strings[i] << endl;

            for (int j = 1; j <= result->numInstances(); ++j)
            {
                cout << "Instance #" << j << ": "
                     << result->instanceName(j) << "="
                     << result->instanceValue(j) << endl;
            }
        }

        cout << endl;
    }

    return 0;
}
