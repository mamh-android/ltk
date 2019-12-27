/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include "STAF.h"
#include "STAFUtil.h"
#include "STAF_iostream.h"
#include "STAF_fstream.h"
#include "STAFFileSystem.h"

const unsigned int MAX_ATTEMPTS = 3;

// Need to specify the TCP/IP port to use for the registration machine in case
// a TCP/IP interface with port 6500 is not configured on the machine trying
// to register with STAF.  So, also specify the tcp interface (tcp://) so that
// won't attempt to use the ssl interface with port 6500.
const STAFString REGISTRATION_MACHINE("tcp://automate.austin.ibm.com@6500");

int main(void)
{
    // Register with STAF

    STAFHandlePtr handle;
    
    unsigned int rc = STAFHandle::create("STAFRegistrationProgram", handle);

    if (rc)
    {
        cout << "STAFReg: Error registering with STAF, RC: " << rc << endl;
        return 1;
    }

    // Get the values for some STAF Configuration variables needed to
    // register STAF by submitting a VAR RESOLVE request

    STAFResultPtr res = handle->submit(
        "local", "VAR", "RESOLVE STRING {STAF/Config/STAFRoot}"
        " STRING {STAF/Config/OS/Name}"
        " STRING {STAF/Config/OS/MajorVersion}"
        " STRING {STAF/Config/OS/MinorVersion}"
        " STRING {STAF/Config/OS/Revision}"
        " STRING {STAF/Version}"
        " STRING {STAF/Config/Sep/File}"
        " STRING {STAF/DataDir}");

    if (res->rc)
    {
        cout << "STAFReg: Error resolving STAF variable(s), RC: "
             << res->rc << ", Result: " << res->result << endl;
        return 1;
    }
 
    STAFObjectPtr mc = STAFObject::unmarshall(res->result);

    // Get the root object for the marshalling context (a list of maps)
    STAFObjectPtr varList = mc->getRootObject();
 
    STAFString stafRoot, osName, osMajorVersion, osMinorVersion, osRevision;
    STAFString stafVersion, fileSep, stafDataDir;

    // Iterate through the list of resolved variables and assign their values.
    // Return an error if a problem occurred resolving a variable.

    STAFObjectIteratorPtr varIter = varList->iterate();
    unsigned int i = 0;

    while (varIter->hasNext())
    {
        STAFObjectPtr varMap = varIter->next();

        if (varMap->get("rc")->asString() != "0")
        {
            cout << "STAFReg: Error resolving a STAF variable.  RC: "
                 << varMap->get("rc")->asString()
                 << ", Result: " << varMap->get("result")->asString() << endl;
            return 1;
        }

        i++;

        switch (i)
        {
            case 1:
            {
                stafRoot = varMap->get("result")->asString();
                break;
            }
            case 2:
            {
                osName = varMap->get("result")->asString();
                break;
            }
            case 3:
            {
                osMajorVersion = varMap->get("result")->asString();
                break;
            }
            case 4:
            {
                osMinorVersion = varMap->get("result")->asString();
                break;
            }
            case 5:
            {
                osRevision = varMap->get("result")->asString();
                break;
            }
            case 6:
            {
                stafVersion = varMap->get("result")->asString();
                break;
            }
            case 7:
            {
                fileSep = varMap->get("result")->asString();
                break;
            }
            case 8:
            {
                stafDataDir = varMap->get("result")->asString();
                break;
            }
        }
    }

    // Open registration information file

    STAFString inputFileName = stafRoot + fileSep + "STAFReg.inf";
    fstream input(inputFileName.toCurrentCodePage()->buffer(), ios::in);

    if (!input)
    {
        cout << "STAFReg: Error opening "
             << inputFileName.toCurrentCodePage()->buffer() << endl;
        return 1;
    }

    // Read in registration information

    char buffer[1024] = { 0 };
    STAFString data("version: ");

    data += stafVersion + kUTF8_SCOLON;
    data += "osname: " + osName + kUTF8_SCOLON;
    data += "osmajor: " + osMajorVersion + kUTF8_SCOLON;
    data += "osminor: " + osMinorVersion + kUTF8_SCOLON;
    data += "osrev: " + osRevision + kUTF8_SCOLON;

    do
    {
        input.getline(buffer, sizeof(buffer));

        if (input.eof()) continue;

        data += buffer;
        data += kUTF8_SCOLON;
    } while (!(!input) && !input.eof());


    // Submit registration data to the registration service machine.
    // Note, we try up to MAX_ATTEMPTS times, as some OSes (e.g., AIX) where
    // the first attempt gets a NoPathToMachine error (for unknown reasons).

    bool registered = false;

    STAFString request("REGISTER TYPE STAF DATA ");
    request += STAFHandle::wrapData(data);

    STAFResultPtr result = handle->submit(
        REGISTRATION_MACHINE, "REGISTER", request);

    for (unsigned int attemptCount = 1;
         (attemptCount < MAX_ATTEMPTS) && (result->rc == kSTAFNoPathToMachine);
         ++attemptCount)
    {
        result = handle->submit(
            REGISTRATION_MACHINE, "REGISTER", request);
    }

    if (result->rc == 0) registered = true;

    // Store STAFReg.cmp in directory {STAF/DataDir}/register

    STAFFSPath registerPath;
    registerPath.setRoot(stafDataDir);
    registerPath.addDir("register");
    STAFString registerDataDir = registerPath.asString();

    // Create the register data directory if it doesn't exist yet

    if (!registerPath.exists())
    {
        try
        {
            registerPath.createDirectory(kSTAFFSCreatePath);
        }
        catch (...)
        { 
            cout << "STAFReg: Error creating registration data directory: "
                 << registerDataDir.toCurrentCodePage()->buffer() << endl;
            return 1;
        }
    }

    // Write the registration data to the STAFReg.cmp file or if
    // registration failed, write an error message to STAFReg.cmp

    STAFString outputFileName = registerDataDir + fileSep + "STAFReg.cmp";

    if (registered)
    {
        fstream output(outputFileName.toCurrentCodePage()->buffer(),
                       ios::out);
        output << "Thanks for registering!" << endl
               << "The following data was sent to " << REGISTRATION_MACHINE
               << ":" << endl << endl << data << endl;
        cout << endl << "Successfully registered STAF, Thank you!" << endl;
    }
    else
    {
        fstream output(outputFileName.toCurrentCodePage()->buffer(),
                       ios::out);
        output << "Registration failed" << endl;
    }

    return 0;
}
