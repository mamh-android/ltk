/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2003                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAF_iostream.h"
#include <stdlib.h>


bool submitRequest(const STAFHandlePtr handlePtr,
                   const STAFString &machine,
                   const STAFString &service,
                   const STAFString &request,
                   bool showResult,
                   const unsigned int expectedRC = 0)
{
    STAFResultPtr result = handlePtr->submit(machine, service, request);

    if (result->rc != expectedRC)
    {
        cout << endl << "STAF " << machine << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: "
                 << STAFObject::unmarshall(result->result)->asFormattedString() << endl;

        return false;
    }
    else
    {
        cout << endl << "STAF " << machine << " " << service << " "
             << request << " - RC=" << result->rc << endl;

        if (showResult)
        {
            cout << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
        }

        return true;
    }
}

int main(int argc, char **argv)
{
    if (argc != 4 && argc != 5 && argc != 6)
    {
        cout << "Usage: " << endl;
        cout << " " << argv[0] << " <TargetMachine> <UserId> <Password> [<Auth> [<ToMachine>]]" << endl;
        cout << "Examples: " << endl;
        cout << " " << argv[0] << " staf6c.austin.ibm.com user@us.ibm.com Password IBM lucas" << endl;
        cout << " " << argv[0] << " local  user@us.ibm.com Password IBM" << endl;
        return 1;
    }
    
    STAFString machine(argv[1]);  // Can be local
    STAFString userId(argv[2]);
    STAFString pw(argv[3]);
    
    cout << endl;

    STAFHandlePtr handlePtr;
    unsigned int rc = 0;
    STAFString handleName = "STAF/Test/Authentication";
    
    rc = STAFHandle::create(handleName, handlePtr);

    if (rc != 0)
    {
        cout << "Error registering with STAF, RC: " << rc << endl;
        return rc;
    }
    
    cout << "Using handle number: " << handlePtr->getHandle() << endl;

    // Assign the orgMachine name by resolving {STAF/Config/Machine}

    STAFString orgMachine;
    
    STAFString local("local");
    STAFString service("VAR");
    STAFString request("RESOLVE STRING {STAF/Config/Machine}");

    STAFResultPtr result = handlePtr->submit(local, service, request);

    if (result->rc != 0)
    {
        cout << endl << "STAF " << local << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        return result->rc;
    }
    else
    {
        orgMachine = result->result;
        cout << "OrgMachine   = " << orgMachine << endl;
    }

    STAFString authenticator;

    if (argc > 4)
    {
        authenticator = argv[4];
        cout << "Authenticator= " << authenticator << endl;
    }
    else
    {
        // Get the local system's default authenticator by resolving {STAF/Config/DefaultAuthenticator}
        
        STAFString local("local");
        STAFString service("VAR");
        STAFString request("RESOLVE STRING {STAF/Config/DefaultAuthenticator}");

        STAFResultPtr result = handlePtr->submit(local, service, request);

        if (result->rc != 0)
        {
            cout << endl << "STAF " << local << " " << service << " "
                 << request << endl
                 << "Error submitting request, RC: " << result->rc << endl;

            return result->rc;
        }
        else
        {
            authenticator = result->result;
            cout << "Authenticator= " << authenticator << endl;
        }
    }
    
    cout << "UserId       = " << userId << endl;
    cout << "Password     = " << pw << endl;
    cout << "TargetMachine= " << machine << endl;

    /* XXX: Delete
    // Get version of Org Machine.  If >= 3.0, list its authenticators

    service = "MISC";
    request = "VERSION";

    result = handlePtr->submit(orgMachine, service, request);

    if (result->rc != 0)
    {
        cout << endl << "STAF " << orgMachine << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: " << result->result << endl;
    }
    else
    {
        cout << "OrgMachineVersion= " << result->result << endl;

        if (result->result >= "3.0.0")
        {
            // List its authenticators

            service = "SERVICE";
            request = "LIST AUTHENTICATORS";

            result = handlePtr->submit(orgMachine, service, request);

            if (result->rc != 0)
            {
                cout << endl << "STAF " << orgMachine << " " << service << " "
                     << request << endl
                     << "Error submitting request, RC: " << result->rc << endl;

                if (result->result.length() != 0)
                    cout << "Additional info: " << result->result << endl;
            }
            else
            {
                cout << "OrgMachineAuthenticators:" << endl << endl
                     << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
            }
        }
    }
    
    // List all trusts for Org Machine.

    service = "TRUST";
    request = "LIST";

    result = handlePtr->submit(orgMachine, service, request);

    if (result->rc != 0)
    {
        cout << endl << "STAF " << orgMachine << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: " << result->result << endl;
    }
    else
    {
        cout << "OrgMachineTrusts:" << endl << endl
             << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
    }
    
    // Get version of Target Machine.  If >= 3.0, list its authenticators

    service = "MISC";
    request = "VERSION";

    result = handlePtr->submit(machine, service, request);

    if (result->rc != 0)
    {
        cout << endl << "STAF " << machine << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: " << result->result << endl;
    }
    else
    {
        cout << "TargetMachineVersion= " << result->result << endl;

        if (result->result >= "3.0.0")
        {
            // List its authenticators

            service = "SERVICE";
            request = "LIST AUTHENTICATORS";

            result = handlePtr->submit(machine, service, request);

            if (result->rc != 0)
            {
                cout << endl << "STAF " << machine << " " << service << " "
                     << request << endl
                     << "Error submitting request, RC: " << result->rc << endl;

                if (result->result.length() != 0)
                    cout << "Additional info: " << result->result << endl;
            }
            else
            {
                cout << "TargetMachineAuthenticators:" << endl << endl
                     << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
            }
        }
    }
    
    // List all trusts for Target Machine.

    service = "TRUST";
    request = "LIST";

    result = handlePtr->submit(machine, service, request);

    if (result->rc != 0)
    {
        cout << endl << "STAF " << machine << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: " << result->result << endl;
    }
    else
    {
        cout << "TargetMachineTrusts:" << endl << endl
             << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
    }
    */

    STAFString toMachine;
    if (argc == 6)
      toMachine = argv[5];
    else
      toMachine = orgMachine;

    cout << "ToMachine    = " << toMachine << endl;

    /*
    if (toMachine != orgMachine)
    {
        // Get version of To Machine.  If >= 3.0, list its authenticators

        service = "MISC";
        request = "VERSION";

        result = handlePtr->submit(toMachine, service, request);

        if (result->rc != 0)
        {
            cout << endl << "STAF " << toMachine << " " << service << " "
                 << request << endl
                 << "Error submitting request, RC: " << result->rc << endl;

            if (result->result.length() != 0)
                cout << "Additional info: " << result->result << endl;
        }
        else
        {
            cout << "ToMachineVersion = " << result->result << endl;

            if (result->result >= "3.0.0")
            {
                // List its authenticators

                service = "SERVICE";
                request = "LIST AUTHENTICATORS";

                result = handlePtr->submit(toMachine, service, request);

                if (result->rc != 0)
                {
                    cout << endl << "STAF " << toMachine << " " << service << " "
                         << request << endl
                         << "Error submitting request, RC: " << result->rc << endl;

                    if (result->result.length() != 0)
                        cout << "Additional info: " << result->result << endl;
                }
                else
                {
                    cout << "ToMachineAuthenticators:" << endl << endl
                         << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
                }
            }
        }

        // List all trusts for To Machine.

        service = "TRUST";
        request = "LIST";

        result = handlePtr->submit(toMachine, service, request);

        if (result->rc != 0)
        {
            cout << endl << "STAF " << toMachine << " " << service << " "
                 << request << endl
                 << "Error submitting request, RC: " << result->rc << endl;

            if (result->result.length() != 0)
                cout << "Additional info: " << result->result << endl;
        }
        else
        {
            cout << "ToMachineTrusts:" << endl << endl
                 << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
        }
    }
    */

    // Part 1: Authenticate the handle with a valid userid/password with
    //         trust level 5.  Verify can access all internal services.

    cout << endl << "Part 1: Authenticate the handle with a valid userid/password with"
         << endl << "        trust level 5.  Verify can access all internal services"
         << endl;

    service = "HANDLE";
    request = "AUTHENTICATE USER " + STAFHandle::wrapData(userId) +
        " CREDENTIALS " + STAFHandle::wrapData(
            STAFHandle::addPrivacyDelimiters(pw));

    if (authenticator.length() > 0)
      request += " AUTHENTICATOR " + STAFHandle::wrapData(authenticator);

    STAFString authUser = authenticator + "://" + userId;

    if (!submitRequest(handlePtr, local, service, request, false)) return 1;

    service = "HANDLE";
    request = STAFString("QUERY HANDLE ") + STAFString(handlePtr->getHandle());
    result = handlePtr->submit(local, service, request);

    if (result->rc != 0)
    {
        cout << endl << "STAF " << local << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: " << result->result << endl;

        return 1;
    }
    else
    {
        cout << endl << "STAF " << local << " " << service << " "
             << request << " - RC=" << result->rc << endl;
        
        STAFString handleUser = result->resultObj->get("user")->asString();

        if (handleUser != authUser)
        {
            cout << "ERROR: User is " << handleUser << ", Expected: " << authUser << endl;
            cout << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
            return 1;
        }
    }

    service = "VAR";
    request = "RESOLVE STRING {STAF/Handle/User}";
    result = handlePtr->submit(local, service, request);

    if (result->rc != 0)
    {
        cout << endl << "STAF " << local << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: " << result->result << endl;

        return 1;
    }
    else
    {
        cout << endl << "STAF " << local << " " << service << " "
             << request << " - RC=" << result->rc << endl;
        
        if (result->result != authUser)
        {
            cout << "ERROR: User is " << result
                 << ", Expected: " << authUser << endl;
            cout << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
            return 1;
        }
    }

    service = "MISC";
    request = "WHOAMI";
    result = handlePtr->submit(local, service, request);

    if (result->rc != 0)
    {
        cout << endl << "STAF " << local << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: " << result->result << endl;

        return 1;
    }
    else
    {
        cout << endl << "STAF " << local << " " << service << " "
             << request << " - RC=" << result->rc << endl;
        
        STAFString whoamiUser = result->resultObj->get("user")->asString();

        if (whoamiUser != authUser)
        {
            cout << "ERROR: User is " << whoamiUser << ", Expected: " << authUser << endl;
            cout << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
            return 1;
        }

        STAFString whoamiTrust = result->resultObj->get("trustLevel")->asString();

        if (whoamiTrust != STAFString("5"))
        {
            cout << "ERROR: Trust level is " << whoamiTrust << ", Expected: 5" << endl;
            cout << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
            return 1;
        }
    }

    service = "MISC";
    request = "WHOAMI";
    result = handlePtr->submit(machine, service, request);

    if (result->rc != 0)
    {
        cout << endl << "STAF " << machine << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: " << result->result << endl;

        return 1;
    }
    else
    {
        cout << endl << "STAF " << machine << " " << service << " "
             << request << " - RC=" << result->rc << endl;
        
        STAFString whoamiUser = result->resultObj->get("user")->asString();

        if (whoamiUser != authUser)
        {
            cout << "ERROR: User is " << whoamiUser << ", Expected: " << authUser << endl;
            cout << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
            return 1;
        }

        STAFString whoamiTrust = result->resultObj->get("trustLevel")->asString();

        if (whoamiTrust != STAFString("5"))
        {
            cout << "ERROR: Trust level is " << whoamiTrust << ", Expected: 5" << endl;
            cout << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
            return 1;
        }
    }

    service = "PROCESS";
    request = "START SHELL COMMAND :13:java -version WAIT";
    if (!submitRequest(handlePtr, machine, service, request, false)) return 1;

    service = "FS";
    request = "COPY FILE {STAF/Config/ConfigFile} TOFILE C:/test.xxx";
    if (!submitRequest(handlePtr, machine, service, request, false)) return 1;

    request = "COPY DIRECTORY {STAF/Config/STAFRoot}/docs TODIRECTORY C:/temp/docs";
    if (!submitRequest(handlePtr, machine, service, request, false)) return 1;
    
    request = "COPY FILE {STAF/Config/ConfigFile} TOFILE C:/test.xxx TOMACHINE " + toMachine;
    if (!submitRequest(handlePtr, machine, service, request, false)) return 1;
    
    request = "COPY DIRECTORY {STAF/Config/STAFRoot}/docs TODIRECTORY C:/temp/docs TOMACHINE " + toMachine;
    if (!submitRequest(handlePtr, machine, service, request, false)) return 1;
    
    request = "COPY FILE {STAF/Config/ConfigFile} TOFILE C:/test.xxx TOMACHINE " + machine;
    if (!submitRequest(handlePtr, local, service, request, false)) return 1;
    
    request = "COPY DIRECTORY {STAF/Config/STAFRoot}/docs TODIRECTORY C:/temp/docs TOMACHINE " + machine;
    if (!submitRequest(handlePtr, local, service, request, false)) return 1;

    service = "LOG";
    request = "SET NORESOLVEMESSAGE";   // Requires trust level 5
    if (!submitRequest(handlePtr, machine, service, request, true)) return 1;

    // Part 2: Try to authenticate the handle with a userid and wrong password
    
    cout << endl << "Part2: Authenticate the handle with a userid and wrong password" << endl;

    cout << endl << "Un-authenticate the handle" << endl;

    service = "HANDLE";
    request = "UNAUTHENTICATE";
    if (!submitRequest(handlePtr, local, service, request, false)) return 1;

    STAFString badUserId = "User2";
    STAFString badPw = "PasswordXYZ";
    
    service = "HANDLE";
    request = "AUTHENTICATE USER " + STAFHandle::wrapData(badUserId) +
        " CREDENTIALS " + STAFHandle::wrapData(badPw);

    if (authenticator.length() > 0)
      request += " AUTHENTICATOR " + STAFHandle::wrapData(authenticator);

    if (!submitRequest(handlePtr, local, service, request, true, 53)) return 1;
    
    service = "HANDLE";
    request = STAFString("QUERY HANDLE ") + STAFString(handlePtr->getHandle());
    result = handlePtr->submit(local, service, request);

    authUser = STAFString("none://anonymous");

    if (result->rc != 0)
    {
        cout << endl << "STAF " << local << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: " << result->result << endl;

        return 1;
    }
    else
    {
        cout << endl << "STAF " << local << " " << service << " "
             << request << " - RC=" << result->rc << endl;
        
        STAFString handleUser = result->resultObj->get("user")->asString();

        if (handleUser != authUser)
        {
            cout << "ERROR: User is " << handleUser << ", Expected: " << authUser << endl;
            cout << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
            return 1;
        }
    }

    service = "VAR";
    request = "RESOLVE STRING {STAF/Handle/User}";
    result = handlePtr->submit(local, service, request);

    if (result->rc != 0)
    {
        cout << endl << "STAF " << local << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: " << result->result << endl;

        return 1;
    }
    else
    {
        cout << endl << "STAF " << local << " " << service << " "
             << request << " - RC=" << result->rc << endl;
        
        if (result->result != authUser)
        {
            cout << "ERROR: User is " << result
                 << ", Expected: " << authUser << endl;
            cout << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
            return 1;
        }
    }

    service = "MISC";
    request = "WHOAMI";
    result = handlePtr->submit(machine, service, request);
    unsigned int machineTrustLevel = 0;

    if (result->rc != 0)
    {
        cout << endl << "STAF " << machine << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: " << result->result << endl;

        return 1;
    }
    else
    {
        cout << endl << "STAF " << machine << " " << service << " "
             << request << " - RC=" << result->rc << endl;
        
        STAFString whoamiUser = result->resultObj->get("user")->asString();

        if (whoamiUser != authUser)
        {
            cout << "ERROR: User is " << whoamiUser << ", Expected: " << authUser << endl;
            cout << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
            return 1;
        }

        STAFString whoamiTrust = result->resultObj->get("trustLevel")->asString();

        machineTrustLevel = (unsigned int)whoamiTrust.asUInt();
    }

    service = "MISC";
    request = "WHOAMI";
    result = handlePtr->submit(toMachine, service, request);
    unsigned int toMachineTrustLevel = 0;

    if (result->rc != 0)
    {
        cout << endl << "STAF " << toMachine << " " << service << " "
             << request << endl
             << "Error submitting request, RC: " << result->rc << endl;

        if (result->result.length() != 0)
            cout << "Additional info: " << result->result << endl;

        return 1;
    }
    else
    {
        cout << endl << "STAF " << toMachine << " " << service << " "
             << request << " - RC=" << result->rc << endl;
        
        STAFString whoamiUser = result->resultObj->get("user")->asString();

        if (whoamiUser != authUser)
        {
            cout << "ERROR: User is " << whoamiUser << ", Expected: " << authUser << endl;
            cout << STAFObject::unmarshall(result->result)->asFormattedString() << endl;
            return 1;
        }

        STAFString whoamiTrust = result->resultObj->get("trustLevel")->asString();

        toMachineTrustLevel = (unsigned int)whoamiTrust.asUInt();
    }

    service = "PROCESS";
    request = "START SHELL COMMAND :13:java -version WAIT";

    unsigned int expectedRC = 0;

    if (machineTrustLevel < 5)
        expectedRC = 25;
    
    if (!submitRequest(handlePtr, machine, service, request, true, expectedRC)) return 1;

    service = "FS";
    request = "COPY FILE {STAF/Config/ConfigFile} TOFILE C:/test.xxx";

    if (machineTrustLevel < 4)
        expectedRC = 25;

    unsigned int expectedRC2 = 0;

    if ((machineTrustLevel < 4) || (toMachineTrustLevel < 4))
        expectedRC2 = 25;

    if (!submitRequest(handlePtr, machine, service, request, true, expectedRC)) return 1;
    
    request = "COPY DIRECTORY {STAF/Config/STAFRoot}/docs TODIRECTORY C:/temp/docs";
    if (!submitRequest(handlePtr, machine, service, request, true, expectedRC)) return 1;
    
    request = "COPY FILE {STAF/Config/ConfigFile} TOFILE C:/test.xxx TOMACHINE " + toMachine;
    if (!submitRequest(handlePtr, machine, service, request, true, expectedRC2)) return 1;

    request = "COPY DIRECTORY {STAF/Config/STAFRoot}/docs TODIRECTORY C:/temp/docs TOMACHINE " + toMachine;
    if (!submitRequest(handlePtr, machine, service, request, true, expectedRC2)) return 1;
    
    request = "COPY FILE {STAF/Config/ConfigFile} TOFILE C:/test.xxx TOMACHINE " + machine;
    if (!submitRequest(handlePtr, local, service, request, true, expectedRC)) return 1;
    
    request = "COPY DIRECTORY {STAF/Config/STAFRoot}/docs TODIRECTORY C:/temp/docs TOMACHINE " + machine;
    if (!submitRequest(handlePtr, local, service, request, true, expectedRC)) return 1;

    // Part 3: Use a non-secure connection provider with the authenticated handle
    
    cout << endl << "Part3: Use a non-secure connection provider with the authenticated handle" << endl;

    cout << endl << "Un-authenticate the handle" << endl;

    service = "HANDLE";
    request = "UNAUTHENTICATE";
    if (!submitRequest(handlePtr, local, service, request, false)) return 1;

    service = "HANDLE";
    request = "AUTHENTICATE USER " + STAFHandle::wrapData(userId) +
        " CREDENTIALS " + STAFHandle::wrapData(
            STAFHandle::addPrivacyDelimiters(pw));

    if (authenticator.length() > 0)
      request += " AUTHENTICATOR " + STAFHandle::wrapData(authenticator);

    if (!submitRequest(handlePtr, local, service, request, false)) return 1;

    expectedRC = 25;
    expectedRC2 = 25;

    if ((machine == "local") || (machine == "local://local") || (machine == orgMachine))
    {
        expectedRC = 0;
    }

    if (((machine == "local") || (machine == "local://local") || (machine == orgMachine)) &&
        ((toMachine == "local") || (toMachine == "local://local") || (toMachine == orgMachine)))
    {
        expectedRC2 = 0;
    }
    
    STAFString tcpMachine = machine;
    STAFString toTcpMachine = toMachine;

    if ((machine == "local") || (machine == "local://local"))
    {
        // Do nothing
    }
    else
    {
        tcpMachine = STAFString("tcp://") + machine;
    }

    if ((toMachine == "local") || (toMachine == "local://local"))
    {
        // Do nothing
    }
    else
    {
        toTcpMachine = STAFString("tcp://") + toMachine;
    }

    service = "FS";
    request = "COPY FILE {STAF/Config/ConfigFile} TOFILE C:/test.xxx";
    if (!submitRequest(handlePtr, tcpMachine, service, request, true, expectedRC)) return 1;

    request = "COPY DIRECTORY {STAF/Config/STAFRoot}/docs TODIRECTORY C:/temp/docs";
    if (!submitRequest(handlePtr, tcpMachine, service, request, true, expectedRC)) return 1;
    
    request = "COPY FILE {STAF/Config/ConfigFile} TOFILE C:/test.xxx TOMACHINE " + toTcpMachine;
    if (!submitRequest(handlePtr, tcpMachine, service, request, true, expectedRC2)) return 1;
    
    request = "COPY DIRECTORY {STAF/Config/STAFRoot}/docs TODIRECTORY C:/temp/docs TOMACHINE " + toTcpMachine;
    if (!submitRequest(handlePtr, tcpMachine, service, request, true, expectedRC2)) return 1;
    
    request = "COPY FILE {STAF/Config/ConfigFile} TOFILE C:/test.xxx TOMACHINE " + tcpMachine;
    if (!submitRequest(handlePtr, local, service, request, true, expectedRC)) return 1;
    
    request = "COPY DIRECTORY {STAF/Config/STAFRoot}/docs TODIRECTORY C:/temp/docs TOMACHINE " + tcpMachine;
    if (!submitRequest(handlePtr, local, service, request, true, expectedRC)) return 1;

    // Part 4: Specify a port for a non-secure connection provider (so that
    //         interface cycling is done) with the authenticated handle
    
    cout << endl << "Part4: Specify a port for a non-secure connection provider with the authenticated handle" << endl;

    expectedRC = 25;
    expectedRC2 = 25;

    if ((machine == "local") || (machine == "local://local") || (machine == orgMachine))
    {
        expectedRC = 0;
    }

    if (((machine == "local") || (machine == "local://local") || (machine == orgMachine)) &&
        ((toMachine == "local") || (toMachine == "local://local") || (toMachine == orgMachine)))
    {
        expectedRC2 = 0;
    }

    tcpMachine = machine;
    toTcpMachine = toMachine;

    if ((machine == "local") || (machine == "local://local"))
    {
        // Do nothing
    }
    else
    {
        tcpMachine = machine + "@6500";
    }

    if ((toMachine == "local") || (toMachine == "local://local"))
    {
        // Do nothing
    }
    else
    {
        toTcpMachine = toMachine + "@6500";
    }

    service = "FS";
    request = "COPY FILE {STAF/Config/ConfigFile} TOFILE C:/test.xxx";
    if (!submitRequest(handlePtr, tcpMachine, service, request, true, expectedRC)) return 1;

    request = "COPY DIRECTORY {STAF/Config/STAFRoot}/docs TODIRECTORY C:/temp/docs";
    if (!submitRequest(handlePtr, tcpMachine, service, request, true, expectedRC)) return 1;
    
    request = "COPY FILE {STAF/Config/ConfigFile} TOFILE C:/test.xxx TOMACHINE " + toTcpMachine;
    if (!submitRequest(handlePtr, tcpMachine, service, request, true, expectedRC2)) return 1;
    
    request = "COPY DIRECTORY {STAF/Config/STAFRoot}/docs TODIRECTORY C:/temp/docs TOMACHINE " + toTcpMachine;
    if (!submitRequest(handlePtr, tcpMachine, service, request, true, expectedRC2)) return 1;
    
    request = "COPY FILE {STAF/Config/ConfigFile} TOFILE C:/test.xxx TOMACHINE " + tcpMachine;
    if (!submitRequest(handlePtr, local, service, request, true, expectedRC)) return 1;
    
    request = "COPY DIRECTORY {STAF/Config/STAFRoot}/docs TODIRECTORY C:/temp/docs TOMACHINE " + tcpMachine;
    if (!submitRequest(handlePtr, local, service, request, true, expectedRC)) return 1;

    cout << endl << "*** All tests completed successfully. ***" << endl;

    return 0;
}
