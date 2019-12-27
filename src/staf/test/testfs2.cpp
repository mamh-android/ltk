/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFFileSystem.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " <file system name> ..." << endl;
        return 1;
    }
    
    for (int i = 1; i < argc; ++i)
    {
        try
        {
            STAFFSPath entryPath(argv[i]);

            cout << endl << argv[i] << ":";

            if (!entryPath.exists())
            {
                cout << " Does not exist" << endl;
            }
            else
            {
                STAFFSEntryPtr entry = entryPath.getEntry();

                STAFFSEntryType_t theType = entry->type();

                cout << endl << "Type     : ";

                switch (theType)
                {
                    case kSTAFFSFile:      cout << "File"; break;
                    case kSTAFFSDirectory: cout << "Directory"; break;
                    case kSTAFFSPipe:      cout << "Pipe"; break;
                    case kSTAFFSSocket:    cout << "Socket"; break;
                    case kSTAFFSSymLink:   cout << "Symbolic Link"; break;
                    case kSTAFFSCharDev:   cout << "Character device"; break;
                    case kSTAFFSBlkDev:    cout << "Block device"; break;
                    default:               cout << "Unknown";
                }

                cout << endl << "UpperSize: " << entry->size().first << endl
                     << "LowerSize: " << entry->size().second << endl;

                STAFFSPath toPath(entryPath);
                toPath.setExtension(STAFString("1"));

                // Test move with non-existing toName - should work

                cout << endl << "Test move of " << entryPath.asString() 
                     << " to " << toPath.asString() << endl;

                entryPath.getEntry()->move(toPath.asString());
                
                if (!entryPath.exists() && toPath.exists())
                {
                    cout << "Move of " << entryPath.asString() << " to " 
                         << toPath.asString() << " worked." << endl;
                }
                else
                {
                    cout << "Move of " << entryPath.asString() << " to "
                         << toPath.asString() << " failed." << endl;

                    return 1;
                }

                // Copy back

                cout << endl << "Test copy of " << toPath.asString()
                     << " to " << entryPath.asString() << endl;

                toPath.getEntry()->copy(entryPath.asString());
                
                if (entryPath.exists() && toPath.exists())
                {
                    cout << "Copy of " << toPath.asString() << " to " 
                         << entryPath.asString() << " worked." << endl;
                }
                else
                {
                    cout << "Copy of " << toPath.asString() << " to "
                         << entryPath.asString() << " failed." << endl;

                    return 1;
                }

                // Test move with existing toPath - should work
                
                cout << endl << "Test move of " << entryPath.asString()
                     << " to " << toPath.asString() << endl;

                entryPath.getEntry()->move(toPath.asString());
                
                if (!entryPath.exists() && toPath.exists())
                {
                    cout << "Move of " << entryPath.asString() << " to " 
                         << toPath.asString() << " worked." << endl;
                }
                else
                {
                    cout << "Move of " << entryPath.asString() << " to "
                         << toPath.asString() << " failed." << endl;

                    return 1;
                }

                // Copy back

                cout << endl << "Test copy of " << toPath.asString()
                     << " to " << entryPath.asString() << endl;

                toPath.getEntry()->copy(entryPath.asString());

                if (entryPath.exists() && toPath.exists())
                {
                    cout << "Copy of " << toPath.asString() << " to " 
                         << entryPath.asString() << " worked." << endl;
                }
                else
                {
                    cout << "Copy of " << toPath.asString() << " to "
                         << entryPath.asString() << " failed." << endl;

                    return 1;
                }

                // Delete toPath

                cout << endl << "Test remove of " << toPath.asString() << endl;

                toPath.getEntry()->remove();

                cout << "Remove of " << toPath.asString() 
                     << " was successful." << endl;

                if (!toPath.exists())
                {
                    cout << "Remove of " << toPath.asString() << " worked." 
                         << endl;
                }
                else
                {
                    cout << "Remove of " << toPath.asString() << " failed."
                         << endl;
                    return 1;
                }

                // Test rename with non-existing toPath - should work

                cout << endl << "Test rename of " << entryPath.asString()
                     << " to " << toPath.asString() << endl;

                entryPath.getEntry()->rename(toPath.asString());

                if (!entryPath.exists() && toPath.exists())
                {
                    cout << "Rename of " << entryPath.asString() << " to " 
                         << toPath.asString() << " worked." << endl;
                }
                else
                {
                    cout << "Rename of " << entryPath.asString() << " to "
                         << toPath.asString() << " failed." << endl;

                    return 1;
                }

                // Copy back

                cout << endl << "Test copy of " << toPath.asString()
                     << " to " << entryPath.asString() << endl;

                toPath.getEntry()->copy(entryPath.asString());

                if (entryPath.exists() && toPath.exists())
                {
                    cout << "Copy of " << toPath.asString() << " to " 
                         << entryPath.asString() << " worked." << endl;
                }
                else
                {
                    cout << "Copy of " << toPath.asString() << " to "
                         << entryPath.asString() << " failed." << endl;

                    return 1;
                }
                
                // Test rename with existing toPath - should fail
                
                cout << endl << "Try rename of " << entryPath.asString()
                     << " to " << toPath.asString() << endl;

                try
                {
                    entryPath.getEntry()->rename(toPath.asString());
                }
                catch (STAFException &e)
                {
                    e.write();

                    cout << "Rename of " << entryPath.asString() << " to "
                         << toPath.asString() << " failed as planned." << endl;
                    
                    // Delete toPath

                    cout << endl << "Test remove of " << toPath.asString() << endl;

                    toPath.getEntry()->remove();

                    cout << "Remove of " << toPath.asString() 
                         << " worked." << endl;
                }
            }
        }
        catch (STAFException &se)
        { se.write(); }
        catch (...)
        { cout << "Caught unknown exception" << endl; }
    }

    return 0;
}
