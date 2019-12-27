from PySTAF import *
import sys
import time

counter = 10

try:
    handle = STAFHandle("STAFTestcase")
except STAFException, e:
    print "Error registering with STAF, RC: %d" % e.rc
    sys.exit(e.rc)

print "Using handle " + str(handle.handle);

if (len(sys.argv) > 1):
    counter = sys.argv[1]

for i in range(int(counter)):
    print "Loop #" + str(i)
    result = handle.submit("local", "monitor",
        "log message " + wrapData("Loop #" + str(i)))
    result = handle.submit("local", "log",
        "log machine logname STAFTestcase1.log level info message " +
        wrapData("Loop #" + str(i)))
    result = handle.submit("local", "queue",
        "get wait 1000")
    if (result.rc == 0 and result.result != None):
        mc = unmarshall(result.result)
        messageMap = mc.getRootObject()
        if (messageMap['message'] == "STAFTestcase4/Terminate"):
            sys.exit(0)

sys.exit(0)
