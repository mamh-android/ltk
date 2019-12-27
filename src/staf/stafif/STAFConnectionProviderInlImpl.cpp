/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ConnectionProviderInlImpl
#define STAF_ConnectionProviderInlImpl

#include "STAF.h"
#include "STAFConnectionProvider.h"

#define CHECK_FOR_CONN_IO_EXCEPTIONS(where) \
if (rc != kSTAFOk)\
{\
    STAFString errorString = STAFString(where) + ": " +\
                             STAFString(errorBuffer, STAFString::kShallow);\
    STAFConnectionIOException ioError(errorString.toCurrentCodePage()->buffer(),\
                                      rc);\
    THROW_STAF_EXCEPTION(ioError);\
}


#define CHECK_FOR_CONN_EXCEPTIONS(where) \
if (rc != kSTAFOk)\
{\
    STAFString errorString = STAFString(where) + ": " +\
                             STAFString(errorBuffer, STAFString::kShallow);\
    STAFConnectionException connError(errorString.toCurrentCodePage()->buffer(),\
                                      rc);\
    THROW_STAF_EXCEPTION(connError);\
}


#define CHECK_FOR_PROV_EXCEPTIONS(where) \
if (rc != kSTAFOk)\
{\
    STAFString errorString = STAFString(where) + ": " +\
                             STAFString(errorBuffer, STAFString::kShallow);\
    STAFConnectionProviderException provError(\
        errorString.toCurrentCodePage()->buffer(), rc);\
    THROW_STAF_EXCEPTION(provError);\
}


#define CHECK_FOR_EXCEPTIONS(where) \
if (rc != kSTAFOk)\
{\
    STAFString errorString = STAFString(where) + ": " +\
                             STAFString(errorBuffer, STAFString::kShallow);\
    STAFException error(errorString.toCurrentCodePage()->buffer(), rc);\
    THROW_STAF_EXCEPTION(error);\
}


STAF_INLINE STAFRC_t STAFConnectionProvider::handleNewConnection(
    STAFConnectionProvider_t provider,
    STAFConnection_t conn,
    const STAFConnectionProviderFunctionTable *funcTable,
    void *data)
{
    STAFConnectionProvider *theProvider =
        reinterpret_cast<STAFConnectionProvider *>(data);

    STAFConnectionPtr theConn =
        STAFConnectionPtr(new STAFConnection(conn, &theProvider->fFuncTable),
                          STAFConnectionPtr::INIT);

    return theProvider->fNewConnFunc(theProvider, theConn);
}


STAF_INLINE STAFConnectionProviderPtr STAFConnectionProvider::createRefPtr(
    const STAFString &name,
    const STAFString &connLib,
    void *constructInfo,
    unsigned int constructInfoLevel)
{
    STAFConnectionProvider *theConnProv = create(name, connLib, constructInfo,
                                                 constructInfoLevel);

    return STAFConnectionProviderPtr(theConnProv,
                                     STAFConnectionProviderPtr::INIT);
}


STAF_INLINE STAFConnectionProvider *STAFConnectionProvider::create(
    const STAFString &name,
    const STAFString &connLib,
    void *constructInfo,
    unsigned int constructInfoLevel)
{
    STAFString_t errorBuffer = 0;
    STAFDynamicLibrary_t library = 0;
    STAFRC_t rc = STAFDynamicLibraryOpen(&library,
                                         connLib.toCurrentCodePage()->buffer(),
                                         &errorBuffer);

    CHECK_FOR_EXCEPTIONS("STAFDynamicLibrary");

    STAFConnectionProviderFunctionTable funcTable = { 0 };

    rc = STAFConnectionProviderLoad(library, &funcTable, &errorBuffer);

    if (rc != kSTAFOk) STAFDynamicLibraryClose(&library, 0);

    CHECK_FOR_PROV_EXCEPTIONS("STAFConnectionProviderLoad");

    STAFConnectionProvider_t provider = 0;

    rc = funcTable.provConstruct(&provider, constructInfo, constructInfoLevel,
                                 &errorBuffer);

    if (rc != kSTAFOk) STAFDynamicLibraryClose(&library, 0);

    CHECK_FOR_PROV_EXCEPTIONS("STAFConnectionProviderConstruct");

    return new STAFConnectionProvider(name, connLib, library,
                                      provider, funcTable);
}


STAF_INLINE void STAFConnectionProvider::start(
    NewConnectionFunc newConnFunc)
{
    STAFConnectionProviderStartInfoLevel1 startInfo = { 0 };

    startInfo.newConnectionFunc = handleNewConnection;
    startInfo.data = this;
    fNewConnFunc = newConnFunc;

    STAFString_t errorBuffer = 0;
    STAFRC_t rc = fFuncTable.provStart(fProvider, &startInfo, 1, &errorBuffer);

    CHECK_FOR_PROV_EXCEPTIONS("STAFConnectionProviderStart");
}


STAF_INLINE void STAFConnectionProvider::stop()
{
    STAFString_t errorBuffer = 0;
    STAFRC_t rc = fFuncTable.provStop(fProvider, 0, 0, &errorBuffer);

    CHECK_FOR_PROV_EXCEPTIONS("STAFConnectionProviderStop");
}


STAF_INLINE void STAFConnectionProvider::getMyNetworkIDs(
    STAFString &logicalIdentifier, STAFString &physicalIdentifier) const
{
    STAFStringConst_t logicalImpl = 0;
    STAFStringConst_t physicalImpl = 0;
    STAFString_t errorBuffer = 0;
    STAFRC_t rc = fFuncTable.provGetMyNetworkIDs(fProvider, &logicalImpl,
                                                 &physicalImpl, &errorBuffer);

    CHECK_FOR_PROV_EXCEPTIONS("STAFConnectionProviderGetMyNetworkIDs");

    logicalIdentifier = logicalImpl;
    physicalIdentifier = physicalImpl;
}


STAF_INLINE void STAFConnectionProvider::getOptions(
    STAFObjectPtr &options) const
{
    STAFObject_t optionsImpl = 0;
    STAFString_t errorBuffer = 0;

    STAFRC_t rc = fFuncTable.provGetOptions(
        fProvider, &optionsImpl, &errorBuffer);

    CHECK_FOR_PROV_EXCEPTIONS("STAFConnectionProviderGetOptions");

    options = STAFObject::create(optionsImpl);
}


STAF_INLINE STAFString STAFConnectionProvider::getProperty(
    STAFConnectionProviderProperty_t property) const
{
    STAFStringConst_t valueImpl = 0;
    STAFString_t errorBuffer = 0;

    STAFRC_t rc = fFuncTable.provGetProperty(fProvider, property,
                                             &valueImpl, &errorBuffer);

    CHECK_FOR_PROV_EXCEPTIONS("STAFConnectionProviderGetProperty");

    return valueImpl;
}


STAF_INLINE const STAFString &STAFConnectionProvider::getName() const
{
    return fName;
}


STAF_INLINE const STAFString &STAFConnectionProvider::getLibrary() const
{
    return fLibrary;
}


STAF_INLINE STAFConnectionPtr STAFConnectionProvider::connect(
    const STAFString &endpoint) const
{
    STAFConnection_t conn = 0;
    STAFConnectionProviderConnectInfoLevel1 connectInfo = { 0 };

    connectInfo.endpoint = endpoint.getImpl();

    STAFString_t errorBuffer = 0;
    STAFRC_t rc = fFuncTable.provConnect(fProvider, &conn, &connectInfo,
                                         1, &errorBuffer);

    CHECK_FOR_PROV_EXCEPTIONS("STAFConnectionProviderConnect");

    return STAFConnectionPtr(new STAFConnection(conn, &fFuncTable),
                             STAFConnectionPtr::INIT);
}


STAF_INLINE STAFConnectionProvider::~STAFConnectionProvider()
{
    // The try/catch is here because when running STAF on OpenSolaris x86 when
    // built using Sun Studio CC, an exception is thrown while the stack is
    // unwinding which forces STAFProc to call abort() and core dump

    try
    {
        fFuncTable.provDestruct(&fProvider, 0, 0, 0);
        STAFDynamicLibraryClose(&fConnLib, 0);
    }
    catch (STAFException &se)
    {
        se.write("STAFConnectionProvider::~STAFConnectionProvider()", cerr);
    }
    catch (...)
    {
        cerr << "Unknown exception at STAFConnectionProvider::"
             << "~STAFConnectionProvider()" << endl;
    }
}


STAF_INLINE STAFConnectionProvider::STAFConnectionProvider(
    const STAFString &name,
    const STAFString &library,
    STAFDynamicLibrary_t connLib,
    STAFConnectionProvider_t provider,
    const STAFConnectionProviderFunctionTable funcTable)
    : fName(name), fLibrary(library), fConnLib(connLib),
      fProvider(provider), fFuncTable(funcTable)
{
    // Do nothing
}


STAF_INLINE void STAFConnection::read(void *buffer, unsigned int size,
                                      bool timeout)
{
    STAFString_t errorBuffer = 0;
    STAFRC_t rc =
        fFuncTable->connRead(fConn, buffer, size, &errorBuffer, timeout);

    CHECK_FOR_CONN_IO_EXCEPTIONS("STAFConnectionRead");
}


STAF_INLINE unsigned int STAFConnection::readUInt(bool timeout)
{
    unsigned int theUInt = 0;
    STAFString_t errorBuffer = 0;
    STAFRC_t rc =
        fFuncTable->connReadUInt(fConn, &theUInt, &errorBuffer, timeout);

    CHECK_FOR_CONN_IO_EXCEPTIONS("STAFConnectionReadUInt");

    return theUInt;
}


STAF_INLINE void STAFConnection::readUInt(unsigned int &uint, bool timeout)
{
    STAFString_t errorBuffer = 0;
    STAFRC_t rc =
        fFuncTable->connReadUInt(fConn, &uint, &errorBuffer, timeout);

    CHECK_FOR_CONN_IO_EXCEPTIONS("STAFConnectionReadUInt");
}


STAF_INLINE STAFString STAFConnection::readString(bool timeout)
{
    STAFString_t theString = 0;
    STAFString_t errorBuffer = 0;
    STAFRC_t rc =  fFuncTable->connReadSTAFString(fConn, &theString,
                                                  &errorBuffer, timeout);

    CHECK_FOR_CONN_IO_EXCEPTIONS("STAFConnectionReadSTAFString");

    return STAFString(theString, STAFString::kShallow);
}


STAF_INLINE void STAFConnection::readString(STAFString &theString, bool timeout)
{
    STAFString_t tempString = 0;
    STAFString_t errorBuffer = 0;
    STAFRC_t rc =  fFuncTable->connReadSTAFString(fConn, &tempString,
                                                  &errorBuffer, timeout);

    CHECK_FOR_CONN_IO_EXCEPTIONS("STAFConnectionReadSTAFString");

    theString.replaceImpl(tempString);
}


STAF_INLINE void STAFConnection::write(void *buffer, unsigned int size,
                                       bool timeout)
{
    STAFString_t errorBuffer = 0;
    STAFRC_t rc =
        fFuncTable->connWrite(fConn, buffer, size, &errorBuffer, timeout);

    CHECK_FOR_CONN_IO_EXCEPTIONS("STAFConnectionWrite");
}


STAF_INLINE void STAFConnection::writeUInt(unsigned int uint, bool timeout)
{
    STAFString_t errorBuffer = 0;
    STAFRC_t rc =
        fFuncTable->connWriteUInt(fConn, uint, &errorBuffer, timeout);

    CHECK_FOR_CONN_IO_EXCEPTIONS("STAFConnectionWriteUInt");
}


STAF_INLINE void STAFConnection::writeString(const STAFString &theString,
                                             bool timeout)
{
    STAFString_t errorBuffer = 0;
    STAFRC_t rc =  fFuncTable->connWriteSTAFString(fConn, theString.getImpl(),
                                                   &errorBuffer, timeout);

    CHECK_FOR_CONN_IO_EXCEPTIONS("STAFConnectionWriteSTAFString");
}


STAF_INLINE void STAFConnection::getPeerNetworkIDs(
    STAFString &logicalIdentifier,
    STAFString &physicalIdentifier)
{
    STAFStringConst_t logicalImpl = 0;
    STAFStringConst_t physicalImpl = 0;
    STAFString_t errorBuffer = 0;
    STAFRC_t rc =  fFuncTable->connGetPeerNetworkIDs(fConn, &logicalImpl,
                                                     &physicalImpl,
                                                     &errorBuffer);

    CHECK_FOR_CONN_EXCEPTIONS("STAFConnectionGetPeerNetworkIDs");

    logicalIdentifier = logicalImpl;
    physicalIdentifier = physicalImpl;
}


STAF_INLINE STAFConnection::~STAFConnection()
{
    fFuncTable->connDestruct(&fConn, 0);
}


STAF_INLINE STAFConnection::STAFConnection(
    STAFConnection_t conn,
    const STAFConnectionProviderFunctionTable *funcTable)
    : fConn(conn), fFuncTable(funcTable)
{
    // Do nothing
}

#endif
