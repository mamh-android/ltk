#!/usr/bin/python

debug = 0

import sys
import re
import os

if len(sys.argv) < 5:
    print "Usage: %s <source file> <dependency file> <obj suffix> " \
          "<include path>..." % sys.argv[0]
    print
    print "Note: The object file and dependency file are assumed to be live"
    print "      in the same directory"
    sys.exit(1)

sourceFile = sys.argv[1]
depFile = sys.argv[2]
objSuffix = sys.argv[3]
includePaths = [ os.getcwd(), sourceFile[:sourceFile.rfind("/")] ] + \
               sys.argv[4:]

if debug:
    print "Source file      : %s" % sourceFile
    print "Dependency file  : %s" % depFile
    print "Object suffix    : %s" % objSuffix
    print "Include paths    : %s" % includePaths

filter = re.compile('[ \t]*#[ \t]*include[ \t]+"(.*)"')
depList = [ ]
depFiles = [ ]
index = 0

# First open the file

try:
    sourceObj = open(sourceFile, 'r')
except IOError, ex:
    print "Error opening source file: %s" % sourceFile
    print "Error code: %s, Reason: %s" % (ex.errno, ex.strerror)
    sys.exit(1)

# Now read it and use it's includes to form the initial dependency list

source = sourceObj.readlines()

if debug:
    print "Processing source file itself"

for line in source:
    result = filter.match(line)
    if result:
        if(not result.group(1).count(":")):
            depList.append(result.group(1))
            if debug:
                print "Added dependency: %s" % result.group(1)


# Now go through all the dependencies (recursively) and setup the list
# of dependent files

while index < len(depList):
    thisFile = depList[index]
    if debug:
        print "Processing file: %s" % thisFile
    sourceObj = None

    for includePath in includePaths:
        try:
            sourceObj  = open(includePath + "/" + thisFile, 'r')
            depFiles.append(includePath + "/" + thisFile)
            if debug:
                print "Added dependency file: %s" % (includePath + "/" + thisFile)
        except IOError: pass
        else: break

    # Hack Hack: Perl headers include non-file includes to handle error cases.
    #            All of these cases use a colon in the error string.
    #            So, we work around these in the dependency generator, here.

    if not sourceObj:
        print "Warning: Could not open dependent file: %s" % thisFile
        index = index + 1
        continue

    source = sourceObj.readlines()

    for line in source:
        result = filter.match(line)
        if result and (result.group(1) not in depList):
            if(not result.group(1).count(":")):
                depList.append(result.group(1))
                if debug:
                    print "Added dependency: %s" % result.group(1)

    index = index + 1

# Now we need to output the dependencies

try:
    depFileObj = open(depFile, 'w')
except IOError:
    print "Error opening dependency file: %s" % depFile
    sys.exit(1)

objFile = depFile[:depFile.rfind(".")] + "." + objSuffix

outlines = [ depFile + " " + objFile + ": " + sourceFile ]

if len(depFiles) == 0: outlines[0] = outlines[0] + "\n"
else: outlines[0] = outlines[0] + "\\\n"

for depFile in depFiles[:-1]: outlines.append("\t " + depFile + " \\\n")

if len(depFiles) != 0: outlines.append("\t " + depFiles[-1] + "\n")

depFileObj.writelines(outlines)

sys.exit(0)
