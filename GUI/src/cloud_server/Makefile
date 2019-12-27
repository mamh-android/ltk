
####### Compiler, tools and options
DATE	      =$(shell date '+%Y-%m-%d')
ADD_VERSION   = -DADD_VERSION="\"$(DATE)\""
CC            = gcc -g
CXX           = g++ -g
DEFINES       = -DQT_NO_DEBUG -DQT_SQL_LIB -DQT_XML_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_CORE_LIB -DQT_SHARED $(ADD_VERSION)
CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT $(DEFINES) -m32 
CXXFLAGS      = -pipe -O2 -Wall -W -D_REENTRANT $(DEFINES) -m32 
INCPATH       = -I../../include/QtCore -I../../include/QtNetwork -I../../include/QtGui -I../../include/QtXml -I../../include/QtSql -I../../include -I. -I.
LINK          = $(CXX)
LFLAGS        = -Wl,-O1 -Wl,-rpath=. -m32 
LIBS          = $(SUBLIBS)  -L../../lib/linux/ -lQtSql -L../../lib/linux/ -lQtXml -lQtGui -L/usr/X11R6/lib -lQtNetwork -lQtCore -lpthread 
AR            = ar cqs
RANLIB        = 
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = main.cpp \
		mainwindow.cpp \
		dialog.cpp \
		ftp.cpp moc_mainwindow.cpp \
		moc_ftp.cpp \
		moc_dialog.cpp \
		qrc_template.cpp
OBJECTS       = main.o \
		mainwindow.o \
		dialog.o \
		ftp.o \
		moc_mainwindow.o \
		moc_ftp.o \
		moc_dialog.o \
		qrc_template.o
QMAKE_TARGET  = cs2
DESTDIR       = 
TARGET        = cs2

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET): ui_mainwindow.h ui_dialog.h $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)


dist: 
	@$(CHK_DIR_EXISTS) .tmp/cs21.0.0 || $(MKDIR) .tmp/cs21.0.0 
	$(COPY_FILE) --parents $(SOURCES) $(DIST) .tmp/cs21.0.0/ && $(COPY_FILE) --parents mainwindow.h ftp.h dialog.h .tmp/cs21.0.0/ && $(COPY_FILE) --parents main.cpp mainwindow.cpp dialog.cpp ftp.cpp .tmp/cs21.0.0/ && $(COPY_FILE) --parents mainwindow.ui dialog.ui .tmp/cs21.0.0/ && (cd `dirname .tmp/cs21.0.0` && $(TAR) cs21.0.0.tar cs21.0.0 && $(COMPRESS) cs21.0.0.tar) && $(MOVE) `dirname .tmp/cs21.0.0`/cs21.0.0.tar.gz . && $(DEL_FILE) -r .tmp/cs21.0.0


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


check: first

mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

compiler_moc_header_make_all: moc_mainwindow.cpp moc_ftp.cpp moc_dialog.cpp
compiler_moc_header_clean:
	-$(DEL_FILE) moc_mainwindow.cpp moc_ftp.cpp moc_dialog.cpp
moc_mainwindow.cpp: mainwindow.h
	../../compile_tools/moc $(DEFINES) $(INCPATH) mainwindow.h -o moc_mainwindow.cpp

moc_ftp.cpp: ftp.h
	../../compile_tools/moc $(DEFINES) $(INCPATH) ftp.h -o moc_ftp.cpp

moc_dialog.cpp: dialog.h
	../../compile_tools/moc $(DEFINES) $(INCPATH) dialog.h -o moc_dialog.cpp

compiler_rcc_make_all:
compiler_rcc_clean:
compiler_image_collection_make_all: qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) qmake_image_collection.cpp
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_uic_make_all: ui_mainwindow.h ui_dialog.h
compiler_uic_clean:
	-$(DEL_FILE) ui_mainwindow.h ui_dialog.h
ui_mainwindow.h: mainwindow.ui
	../../compile_tools/uic mainwindow.ui -o ui_mainwindow.h

ui_dialog.h: dialog.ui
	../../compile_tools/uic dialog.ui -o ui_dialog.h

compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: compiler_moc_header_clean compiler_uic_clean 

####### Compile

main.o: main.cpp dialog.h \
		mainwindow.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o main.cpp

mainwindow.o: mainwindow.cpp mainwindow.h \
		ui_mainwindow.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o mainwindow.o mainwindow.cpp

dialog.o: dialog.cpp dialog.h \
		ui_dialog.h \
		ftp.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o dialog.o dialog.cpp

ftp.o: ftp.cpp ftp.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ftp.o ftp.cpp

moc_mainwindow.o: moc_mainwindow.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_mainwindow.o moc_mainwindow.cpp

moc_ftp.o: moc_ftp.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_ftp.o moc_ftp.cpp

moc_dialog.o: moc_dialog.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_dialog.o moc_dialog.cpp

qrc_template.o: qrc_template.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o qrc_template.o qrc_template.cpp

####### Install

install:
	cp $(TARGET) ../../bin/linux/cloud_server/cloud_server

uninstall:   FORCE

FORCE:

