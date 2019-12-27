/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFFileSystem.h"

int main(void)
{
#ifdef STAF_OS_NAME_WIN32
    STAFString testPaths[] =
    {
        "c:\\windows",
        "c:\\staf\\bin\\STAF.cfg",
        "STAF.cfg",
        "c:",
        "",
        "windows/system32",
        "staf/bin/STAF.cfg",
        "\\windows\\",
        "a.b.c.d",
        "c:\\a.b\\c.d",
        "c:\\windows\\.a\\.b",
        "c:\\\\",
        "c:\\\\staf\\\\data\\\\log",
        "d:/stuff",
        "d:/stuff\\it/good.bad",
        "/stuff\\it",
        "d:\\c:\\",
        ":/stuff"
    };
#else
    STAFString testPaths[] =
    {
        "/usr/bin",
        "/usr/local/staf/STAF.cfg",
        "STAF.cfg",
        "/",
        "",
        "usr/bin",
        "usr/local/staf/STAF.cfg",
        "/usr/",
        "a.b.c.d",
        "/a.b/c.d",
        "/usr/.a/.b",
        "//",
        "//usr///bin//sh"
    };
#endif

    for (int i = 0; i < (sizeof(testPaths) / sizeof(STAFString)); ++i)
    {
        STAFFSPath path(testPaths[i]);

        cout << endl << endl << "Path: " << path.asString() << endl
             << "Root  : " << path.root() << endl
             << "# Dirs: " << path.numDirs() << endl;

        for (unsigned int j = 0; j < path.numDirs(); ++j)
            cout << "Dir #" << j << ": " << path.dir(j) << endl;

        cout << "Name  : " << path.name() << endl
             << "Ext   : " << path.extension() << endl;

        // Set the root so that the path gets recalculated

        path.setRoot(path.root());

        cout << "Path2 : " << path.asString() << endl;

        if (testPaths[i] != path.asString())
            cout << "Warning: Paths do not match" << endl;
    }

#ifdef STAF_OS_NAME_WIN32
    const unsigned int numTests = 16;

    STAFString testDirs[numTests][3] =
    {
        // These tests should result in a cyclic copy error

        {
            "C:/temp/recursetest",
            "C:/temp/recursetest/dir1",
            "Cyclic"
        },
        {
            "C:/temp/recursetest/",
            "C:/temp/recursetest/dir1/",
            "Cyclic"
        },
        {
            "C:/temp/recursetest",
            "C:\\\\temp\\recursetest\\dir1\\",
            "Cyclic"
        },
        {
            "C:\\temp\\recursetest",
            "C:/temp/recursetest/dir1",
            "Cyclic"
        },
        {
            "C:\\temp\\recursetest",
            "C:/temp/recursetest/dir1/dir2",
            "Cyclic"
        },
        {
            "C:\\temp\\recursetest",
            "C:/temp/Recursetest/dir1",
            "Cyclic"
        },
        {
            "C:/temp/recursetest",
            "C:/temp/recursetest/dir1.dir",
            "Cyclic"
        },   
        
        // These tests should not result in a cyclic copy error

        {
            "C:\\temp\\recursetest",
            "\\temp\\recursetest\\dir11",
            "Not cyclic"
        },
        {
            "C:/temp/recursetest",
            "C:/temp/recursetest2/dir1",
            "Not cyclic"
        },
        {
            "C:/recursetest",
            "C:/recursetest2",
            "Not cyclic"
        },
        {

            "C:/temp/recursetest",
            "C:/new",
            "Not cyclic"
        },
        {

            "C:/temp/recursetest",
            "C:/new/recursetest/dir1",
            "Not cyclic"
        },
        {
            "C:/temp/recursetest/dir1.dir",
            "C:/temp/Recursetest/dir1.dir2",
            "Not cyclic"
        },   

        // These tests should result in a same directory error

        {

            "C:/temp/recursetest",
            "C:/temp/recursetest",
            "Same directory"
        },
        {

            "C:/temp/recursetest",
            "C:/temp\\Recursetest",
            "Same directory"
        },
        {
            "C:/temp/recursetest/dir1.dir",
            "C:/temp/Recursetest/dir1.dir",
            "Same directory"
        }   
    };
#else
    // Unix tests
    const unsigned int numTests = 16;
    STAFString testDirs[numTests][3] =
    {
        {
            "/tmp/recursetest",
            "/tmp/recursetest/dir1",
            "Cyclic"
        },
        {
            "/tmp/recursetest/",
            "/tmp/recursetest/dir1/",
            "Cyclic"
        },
        {
            "/tmp/recursetest",
            "//tmp/recursetest//dir1//",
            "Cyclic"
        },
        {
            "/tmp/recursetest",
            "/tmp/recursetest/dir1/dir2",
            "Cyclic"
        },
        {
            "/tmp/recursetest",
            "/tmp/recursetest/dir1.dir",
            "Cyclic"
        },   

        // These tests should not result in a cyclic copy error

        {
            "/tmp/recursetest",
            "/tmp/recursetest2",
            "Not cyclic"
        },
        {
            "/tmp/recursetest",
            "/tmp/recursetest2/dir1",
            "Not cyclic"
        },
        {
            "/tmp/recursetest",
            "/tmp/Recursetest/dir1",
            "Not cyclic"
        },
        {
            "/tmp/recursetest",
            "/tmp/Recursetest/Dir1/dir2",
            "Not cyclic"
        },
        {
            "/tmp/recursetest",
            "/new",
            "Not cyclic"
        },
        {
            "/tmp/recursetest",
            "/tmp/recursetest2/dir1",
            "Not cyclic"
        },
        {
            "/tmp/recursetest",
            "/tmp/Recursetest",
            "Not cyclic"
        },
        {
            "/tmp/recursetest/dir1.dir",
            "/tmp/Recursetest/dir1.dir2",
            "Not cyclic"
        },   

        // These tests should result in a same directory error

        {
            "/tmp/recursetest",
            "/tmp/recursetest",
            "Same directory"
        },
        {
            "/tmp/recursetest",
            "/tmp//recursetest/",
            "Same directory"
        },
        {
            "/tmp/recursetest/dir1.dir",
            "/tmp/recursetest/dir1.dir",
            "Same directory"
        }   
    };
#endif

    cout << endl << endl << "Part2:" << endl << endl;
    
    unsigned int testPass = 0;
    unsigned int testFail = 0;

    for (unsigned int m = 0; m < numTests; ++m)
    {
        STAFString fromDir = testDirs[m][0];
        STAFString toDir   = testDirs[m][1];
        cout << endl << "Test #" << m + 1 << ":" << endl;
        cout << "- fromDir: " << fromDir << endl;
        cout << "- toDir  : " << toDir << endl;

        // Check if the from directory starts with (includes) the to
        // directory which means it's a cyclic copy.

        unsigned int compareResult = STAFFileSystem::comparePaths(
            toDir, fromDir);

        if (compareResult == kSTAFFSDoesIncludePath)
        {
            if (testDirs[m][2] == "Cyclic")
            {
                testPass++;
                cout << "Detected a cyclic copy - PASS" << endl;
            }
            else
            {
                testFail++;
                cout << "Detected a cyclic copy - FAIL" << endl;
            }
        }
        else if (compareResult == kSTAFFSSamePath)
        {
            if (testDirs[m][2] == "Same directory")
            {
                testPass++;
                cout << "Same directory specified for from and to - PASS" << endl;
            }
            else
            {
                testFail++;
                cout << "Same directory specified for from and to - FAIL" << endl;
            }
        }
        else
        {
            if (testDirs[m][2] == "Not cyclic")
            {
                testPass++;
                cout << "Not a cyclic copy - PASS" << endl;
            }
            else
            {
                testFail++;
                cout << "Not a cyclic copy - FAIL" << endl;
            }
        }
    }

    cout << endl << "Total Passes: " << testPass << endl;
    cout << "Total Fails: " << testFail << endl;

    return 0;
}
