/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ConnectionProvider
#define STAF_ConnectionProvider

#include "STAF.h"
#include "STAFString.h"
#include "STAFDynamicLibrary.h"

#ifdef __cplusplus
extern "C" {
#endif

// Each Connection Provider must define a structure with a unique name
// (e.g. STAFTCPConnectionImpl, STAFLocalConnectionImpl) that extends this
// base structure
struct STAFConnectionImpl
{
};

// Each Connection Provider must define a structure with a unique name
// (e.g. STAFTCPConnectionProviderImpl, STAFLocalConnectionProviderImpl)
// that extends this base structure
struct STAFConnectionProviderImpl
{
};

typedef struct STAFConnectionImpl *STAFConnection_t;
typedef struct STAFConnectionProviderImpl *STAFConnectionProvider_t;
typedef struct STAFConnectionProviderFunctionTable
               STAFConnectionProviderFunctionTable;

typedef enum
{
    kSTAFConnectionProviderDuplex   = 0,
    kSTAFConnectionProviderInbound  = 1,
    kSTAFConnectionProviderOutbound = 2
} STAFConnectionProviderMode_t;

typedef enum
{
    kSTAFConnectionProviderStopped    = 0,
    kSTAFConnectionProviderStarting   = 1,
    kSTAFConnectionProviderActive     = 2,
    kSTAFConnectionProviderStopping   = 3
} STAFConnectionProviderState_t;


typedef enum
{
    // The "port" property is a string which can be appended to an endpoint
    // string to enable a protocol configured on one "port" to connect to a
    // compatible protocol connection provider configured for a different
    // "port".  Note, while this is used to represent the actual port on a
    // remote endpoint when dealing with a TCP/IP connection provider, this
    // "port" may represent anything relevant to the connection provider.  This
    // property is meant to be opaque.  The only valid operation on this
    // property is an equality comparision with the "port" of a compatible
    // connection provider.

    kSTAFConnectionProviderPortProperty = 0,

    // The "isSecure" property indicates whether this connection provider
    // implements a secure protocol.  This property will be set to either
    // "0" if the protocol is not secure, or "1" if the protocol is secure.
    // Any value other than "0" or "1" should be treated as though it were
    // unsecure.

    kSTAFConnectionProviderIsSecureProperty = 1
} STAFConnectionProviderProperty_t;

/******************************************************************************/
/* STAFConnectionProviderNewConnection - This is called for every new         */
/*                                       connection accepted by the provider  */
/*                                                                            */
/* Accepts: (In)  Connection Provider                                         */
/*          (In)  Connection                                                  */
/*          (In)  Pointer to connection provider function table               */
/*          (In)  Pointer to data set at construction time                    */
/*                                                                            */
/* Returns: kSTAFOk                                                           */
/*                                                                            */
/* Notes  : Currently, the return code of this function is not used, but, to  */
/*          be safe it should always return kSTAFOk                           */
/******************************************************************************/
typedef STAFRC_t (* STAFConnectionProviderNewConnectionFunc_t)(
    STAFConnectionProvider_t provider,
    STAFConnection_t connection,
    const STAFConnectionProviderFunctionTable *funcTable,
    void *data);


typedef struct STAFConnectionProviderConstructInfoLevel1
{
    STAFConnectionProviderMode_t mode;

    unsigned int numOptions;
    STAFStringConst_t *optionNames;
    STAFStringConst_t *optionValues;
} STAFConnectionProviderConstructInfoLevel1;

typedef struct STAFConnectionProviderStartInfoLevel1
{
    STAFConnectionProviderNewConnectionFunc_t newConnectionFunc;
    void *data;
} STAFConnectionProviderConnectStartLevel1;

typedef struct STAFConnectionProviderConnectInfoLevel1
{
    STAFStringConst_t endpoint;
} STAFConnectionProviderConnectInfoLevel1;


/******************************************************************************/
/* Note: In all the functions below, if a non-zero pointer to an error buffer */
/*       is returned, it is the responsibility of the caller to call          */
/*       STAFStringDestruct on the error buffer.                              */
/******************************************************************************/

/******************************************************************************/
/* STAFConnectionProviderConstruct - Creates a connection provider            */
/*                                                                            */
/* Accepts: (Out) Pointer to connection provider                              */
/*          (In)  Pointer to constructor info                                 */
/*          (In)  Constructor info level                                      */
/*          (Out) Pointer to error buffer                                     */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/******************************************************************************/
STAFRC_t STAFConnectionProviderConstruct(STAFConnectionProvider_t *provider,
                                         void *constructInfo,
                                         unsigned int constructInfoLevel,
                                         STAFString_t *errorBuffer);


/******************************************************************************/
/* STAFConnectionProviderStart - Starts accepting connections                 */
/*                                                                            */
/* Accepts: (In)  Connection provider                                         */
/*          (In)  Pointer to start info                                       */
/*          (In)  Start info level                                            */
/*          (Out) Pointer to error buffer                                     */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          kSTAFInvalidOperation, if the provider is outbound only           */
/*          other on error                                                    */
/******************************************************************************/
STAFRC_t STAFConnectionProviderStart(STAFConnectionProvider_t provider,
                                     void *startInfo,
                                     unsigned int startInfoLevel,
                                     STAFString_t *errorBuffer);


/******************************************************************************/
/* STAFConnectionProviderStop - Stop accepting connections                    */
/*                                                                            */
/* Accepts: (In)  Connection provider                                         */
/*          (In)  Pointer to stop info                                        */
/*          (In)  Stop info level                                             */
/*          (Out) Pointer to error buffer                                     */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/******************************************************************************/
STAFRC_t STAFConnectionProviderStop(STAFConnectionProvider_t provider,
                                    void *stopInfo,
                                    unsigned int stopInfoLevel,
                                    STAFString_t *errorBuffer);


/******************************************************************************/
/* STAFConnectionProviderDestruct - Destroys a connection provider            */
/*                                                                            */
/* Accepts: (I/O) Pointer to connection provider                              */
/*          (In)  Pointer to destruct info                                    */
/*          (In)  Destruct info level                                         */
/*          (Out) Pointer to error buffer                                     */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/******************************************************************************/
STAFRC_t STAFConnectionProviderDestruct(STAFConnectionProvider_t *provider,
                                        void *destructInfo,
                                        unsigned int destructInfoLevel,
                                        STAFString_t *errorBuffer);


/******************************************************************************/
/* STAFConnectionProviderConnect - Connect to a "remote" endpoint             */
/*                                                                            */
/* Accepts: (In)  Connection Provider                                         */
/*          (Out) Pointer to connection                                       */
/*          (In)  Pointer to connect info                                     */
/*          (In)  Connect info level                                          */
/*          (Out) Pointer to error buffer                                     */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          kSTAFInvalidOperation, if the provider is inbound only            */
/*          other on error                                                    */
/******************************************************************************/
STAFRC_t STAFConnectionProviderConnect(STAFConnectionProvider_t provider,
                                       STAFConnection_t *connection,
                                       void *connectInfo,
                                       unsigned int connectInfoLevel,
                                       STAFString_t *errorBuffer);


/******************************************************************************/
/* STAFConnectionGetMyNetworkIDs - Get the logical and physical network       */
/*                                 identifiers for this connection provider   */
/*                                                                            */
/* Accepts: (In)  Connection                                                  */
/*          (Out) Pointer to logical network identifier                       */
/*          (Out) Pointer to physical network identifier                      */
/*          (Out) Pointer to error buffer                                     */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/*                                                                            */
/* Notes  : If the logical network identifier can not be determined or is     */
/*          otherwise unavailable, the physical network identifier will be    */
/*          returned for both the logical and physical network identifiers    */
/******************************************************************************/
STAFRC_t STAFConnectionProviderGetMyNetworkIDs(STAFConnectionProvider_t provider,
                                               STAFStringConst_t *logicalID,
                                               STAFStringConst_t *physicalID,
                                               STAFString_t *errorBuffer);


/******************************************************************************/
/* STAFConnectionGetOptions - Get the options (name[=value]) for this         */
/*                              connection provider                           */
/*                                                                            */
/* Accepts: (In)  Connection                                                  */
/*          (Out) Pointer to a STAFObjectPtr containing a list of the options */
/*                for this connection provider.  Each option is the list has  */
/*                format:  <Option Name>=<Option Value>                       */
/*                For example: [ "PORT=6500", "PROTOCOL=ipv4 "]               */
/*          (Out) Pointer to error buffer                                     */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/******************************************************************************/
STAFRC_t STAFConnectionProviderGetOptions(
    STAFConnectionProvider_t provider,
    STAFObject_t *options,
    STAFString_t *errorBuffer);


/******************************************************************************/
/* STAFConnectionProviderGetProperty - Get a specified property for this      */
/*                                     connection provider                    */
/*                                                                            */
/* Accepts: (In)  Connection                                                  */
/*          (In)  The property to retrieve                                    */
/*          (Out) Pointer to a string containing the property's value         */
/*          (Out) Pointer to error buffer                                     */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/******************************************************************************/
STAFRC_t STAFConnectionProviderGetProperty(
    STAFConnectionProvider_t provider,
    STAFConnectionProviderProperty_t property, STAFStringConst_t *value,
    STAFString_t *errorBuffer);


/******************************************************************************/
/* STAFConnectionRead - Reads arbitrary data from a connection                */
/*                                                                            */
/* Accepts: (In)  Connection                                                  */
/*          (I/O) Pointer to buffer in which to write data                    */
/*          (In)  Length of data (in bytes) to read                           */
/*          (Out) Pointer to error buffer                                     */
/*          (In)  Flag indicating whether to do read/write timeouts           */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/*                                                                            */
/* Notes  : The buffer must be at least as long as the readLength passed in   */
/******************************************************************************/
STAFRC_t STAFConnectionRead(STAFConnection_t connection,
                            void *buffer,
                            unsigned int readLength,
                            STAFString_t *errorBuffer,
                            bool doTimeout);


/******************************************************************************/
/* STAFConnectionReadUInt - Reads an unsigned integer from a connection       */
/*                                                                            */
/* Accepts: (In)  Connection                                                  */
/*          (Out) Pointer to an unsigned integer                              */
/*          (Out) Pointer to error buffer                                     */
/*          (In)  Flag indicating whether to do read/write timeouts           */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/*                                                                            */
/* Notes  : This function is platform/endian agnostic.  In other words it     */
/*          will convert for endianness between different platforms.          */
/******************************************************************************/
STAFRC_t STAFConnectionReadUInt(STAFConnection_t connection,
                                unsigned int *uint,
                                STAFString_t *errorBuffer,
                                bool doTimeout);


/******************************************************************************/
/* STAFConnectionReadSTAFString - Reads a STAFString from a connection        */
/*                                                                            */
/* Accepts: (In)  Connection                                                  */
/*          (Out) Pointer to a STAFString_t                                   */
/*          (Out) Pointer to error buffer                                     */
/*          (In)  Flag indicating whether to do read/write timeouts           */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/*                                                                            */
/* Notes  : 1) This function is platform/endian agnostic.  In other words it  */
/*             will convert a STAFString between different platforms.         */
/*          2) On successful return, the caller is responsible for calling    */
/*             STAFStringDestruct on the returned string.                     */
/******************************************************************************/
STAFRC_t STAFConnectionReadSTAFString(STAFConnection_t connection,
                                      STAFString_t *stafString,
                                      STAFString_t *errorBuffer,
                                      bool doTimeout);


/******************************************************************************/
/* STAFConnectionWrite - Writes arbitrary data to a connection                */
/*                                                                            */
/* Accepts: (In)  Connection                                                  */
/*          (I/O) Pointer to buffer from which to read data                   */
/*          (In)  Length of data (in bytes) to write                          */
/*          (Out) Pointer to error buffer                                     */
/*          (In)  Flag indicating whether to do read/write timeouts           */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/*                                                                            */
/* Notes  : The buffer must be at least as long as the writeLength passed in  */
/******************************************************************************/
STAFRC_t STAFConnectionWrite(STAFConnection_t connection,
                             void *buffer,
                             unsigned int writeLength,
                             STAFString_t *errorBuffer,
                             bool doTimeout);


/******************************************************************************/
/* STAFConnectionWriteUInt - Writes an unsigned integer to a connection       */
/*                                                                            */
/* Accepts: (In)  Connection                                                  */
/*          (In)  An unsigned integer                                         */
/*          (Out) Pointer to error buffer                                     */
/*          (In)  Flag indicating whether to do read/write timeouts           */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/*                                                                            */
/* Notes  : This function is platform/endian agnostic.  In other words it     */
/*          will convert for endianness between different platforms.          */
/******************************************************************************/
STAFRC_t STAFConnectionWriteUInt(STAFConnection_t connection,
                                 unsigned int uint,
                                 STAFString_t *errorBuffer,
                                 bool doTimeout);


/******************************************************************************/
/* STAFConnectionWriteSTAFString - Writes a STAFString to a connection        */
/*                                                                            */
/* Accepts: (In)  Connection                                                  */
/*          (In)  A STAFString_t                                              */
/*          (Out) Pointer to error buffer                                     */
/*          (In)  Flag indicating whether to do read/write timeouts           */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/*                                                                            */
/* Notes  : This function is platform/endian agnostic.  In other words it     */
/*          will convert a STAFString between different platforms.            */
/******************************************************************************/
STAFRC_t STAFConnectionWriteSTAFString(STAFConnection_t connection,
                                       STAFStringConst_t stafString,
                                       STAFString_t *errorBuffer,
                                       bool doTimeout);


/******************************************************************************/
/* STAFConnectionGetPeerNetworkIDs - Get the logical and physical network     */
/*                                   identifiers for the connected peer       */
/*                                                                            */
/* Accepts: (In)  Connection                                                  */
/*          (Out) Pointer to logical network identifier                       */
/*          (Out) Pointer to physical network identifier                      */
/*          (Out) Pointer to error buffer                                     */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/*                                                                            */
/* Notes  : If the logical network identifier can not be determined or is     */
/*          otherwise unavailable, the physical network identifier will be    */
/*          returned for both the logical and physical network identifiers    */
/******************************************************************************/
STAFRC_t STAFConnectionGetPeerNetworkIDs(STAFConnection_t connection,
                                         STAFStringConst_t *logicalID,
                                         STAFStringConst_t *physicalID,
                                         STAFString_t *errorBuffer);


/******************************************************************************/
/* STAFConnectionDestruct - Destroys/frees a connection                       */
/*                                                                            */
/* Accepts: (I/O) Pointer to a connection                                     */
/*          (Out) Pointer to error buffer                                     */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/******************************************************************************/
STAFRC_t STAFConnectionDestruct(STAFConnection_t *connection,
                                STAFString_t *errorBuffer);


// Function typedefs

typedef STAFRC_t (*STAFConnectionProviderConstructFunc_t)(
    STAFConnectionProvider_t *provider, void *constructInfo,
    unsigned int constructInfoLevel, STAFString_t *errorBuffer);

typedef STAFRC_t (*STAFConnectionProviderStartFunc_t)(
    STAFConnectionProvider_t provider, void *startInfo,
    unsigned int startInfoLevel, STAFString_t *errorBuffer);

typedef STAFRC_t (*STAFConnectionProviderStopFunc_t)(
    STAFConnectionProvider_t provider, void *stopInfo,
    unsigned int stopInfoLevel, STAFString_t *errorBuffer);

typedef STAFRC_t (*STAFConnectionProviderDestructFunc_t)(
    STAFConnectionProvider_t *provider, void *destructInfo,
    unsigned int destructInfoLevel, STAFString_t *errorBuffer);

typedef STAFRC_t (*STAFConnectionProviderConnectFunc_t)(
    STAFConnectionProvider_t provider, STAFConnection_t *connection,
    void *connectInfo, unsigned int connectInfoLevel, STAFString_t *errorBuffer);

typedef STAFRC_t (*STAFConnectionProviderGetMyNetworkIDsFunc_t)(
    STAFConnectionProvider_t provider, STAFStringConst_t *logicalID,
    STAFStringConst_t *physicalID, STAFString_t *errorBuffer);

typedef STAFRC_t (*STAFConnectionProviderGetOptionsFunc_t)(
    STAFConnectionProvider_t provider,
    STAFObject_t *options,
    STAFString_t *errorBuffer);

typedef STAFRC_t (*STAFConnectionProviderGetPropertyFunc_t)(
    STAFConnectionProvider_t provider, STAFConnectionProviderProperty_t property,
    STAFStringConst_t *value, STAFString_t *errorBuffer);

typedef STAFRC_t (*STAFConnectionReadFunc_t)(
    STAFConnection_t connection, void *buffer, unsigned int readLength,
    STAFString_t *errorBuffer, bool doTimeout);

typedef STAFRC_t (*STAFConnectionReadUIntFunc_t)(
    STAFConnection_t connection, unsigned int *uint, STAFString_t *errorBuffer,
    bool doTimeout);

typedef STAFRC_t (*STAFConnectionReadSTAFStringFunc_t)(
    STAFConnection_t connection, STAFString_t *stafString,
    STAFString_t *errorBuffer, bool doTimeout);

typedef STAFRC_t (*STAFConnectionWriteFunc_t)(
    STAFConnection_t connection, void *buffer, unsigned int writeLength,
    STAFString_t *errorBuffer, bool doTimeout);

typedef STAFRC_t (*STAFConnectionWriteUIntFunc_t)(
    STAFConnection_t connection, unsigned int uint, STAFString_t *errorBuffer,
    bool doTimeout);

typedef STAFRC_t (*STAFConnectionWriteSTAFStringFunc_t)(
    STAFConnection_t connection, STAFStringConst_t stafString,
    STAFString_t *errorBuffer, bool doTimeout);

typedef STAFRC_t (*STAFConnectionGetPeerNetworkIDsFunc_t)(
    STAFConnection_t connection, STAFStringConst_t *logicalID,
    STAFStringConst_t *physicalID, STAFString_t *errorBuffer);

typedef STAFRC_t (*STAFConnectionDestructFunc_t)(
    STAFConnection_t *connection, STAFString_t *errorBuffer);

// Function table definition

typedef struct STAFConnectionProviderFunctionTable
{
    STAFConnectionProviderConstructFunc_t       provConstruct;
    STAFConnectionProviderStartFunc_t           provStart;
    STAFConnectionProviderStopFunc_t            provStop;
    STAFConnectionProviderDestructFunc_t        provDestruct;
    STAFConnectionProviderConnectFunc_t         provConnect;
    STAFConnectionProviderGetMyNetworkIDsFunc_t provGetMyNetworkIDs;
    STAFConnectionProviderGetOptionsFunc_t      provGetOptions;
    STAFConnectionProviderGetPropertyFunc_t     provGetProperty;
    STAFConnectionReadFunc_t                    connRead;
    STAFConnectionReadUIntFunc_t                connReadUInt;
    STAFConnectionReadSTAFStringFunc_t          connReadSTAFString;
    STAFConnectionWriteFunc_t                   connWrite;
    STAFConnectionWriteUIntFunc_t               connWriteUInt;
    STAFConnectionWriteSTAFStringFunc_t         connWriteSTAFString;
    STAFConnectionGetPeerNetworkIDsFunc_t       connGetPeerNetworkIDs;
    STAFConnectionDestructFunc_t                connDestruct;
} STAFConnectionProviderFunctionTable;


// Utility APIs

/******************************************************************************/
/* STAFConnectionProviderLoad - This will fill in a connection provider       */
/*                              function table given a dynamic library        */
/*                              linked to the provider                        */
/*                                                                            */
/* Accepts: (In)  Connection provider dynamic library                         */
/*          (Out) Pointer to a connetion provider function table              */
/*          (Out) Pointer to error buffer                                     */
/*                                                                            */
/* Returns: kSTAFOk, on success                                               */
/*          other on error                                                    */
/******************************************************************************/
STAFRC_t STAFConnectionProviderLoad(STAFDynamicLibrary_t library,
                                    STAFConnectionProviderFunctionTable *funcs,
                                    STAFString_t *errorBuffer);


#ifdef __cplusplus
}

#include <deque>
#include "STAFRefPtr.h"

class STAFConnection;
typedef STAFRefPtr<STAFConnection> STAFConnectionPtr;
class STAFConnectionProvider;
typedef STAFRefPtr<STAFConnectionProvider> STAFConnectionProviderPtr;

class STAFConnectionProvider
{
public:

    typedef std::deque<STAFString> OptionList;
    typedef STAFRC_t (* NewConnectionFunc)(
                         const STAFConnectionProvider *provider,
                         STAFConnectionPtr &connection);

    static STAFConnectionProviderPtr createRefPtr(
        const STAFString &name,
        const STAFString &connLib,
        void *constructInfo,
        unsigned int constructInfoLevel);

    static STAFConnectionProvider *create(
        const STAFString &name,
        const STAFString &connLib,
        void *constructInfo,
        unsigned int constructInfoLevel);

    void start(NewConnectionFunc newConnFunc);
    void stop();

    void getMyNetworkIDs(STAFString &logicalIdentifier,
                         STAFString &physicalIdentifier) const;
    void getOptions(STAFObjectPtr &options) const;
    STAFString getProperty(STAFConnectionProviderProperty_t property) const;
    const STAFString &getName() const;
    const STAFString &getLibrary() const;

    STAFConnectionPtr connect(const STAFString &endpoint) const;

    ~STAFConnectionProvider();

private:

    // Don't allow copy construction or assignment
    STAFConnectionProvider(const STAFConnectionProvider &);
    STAFConnectionProvider &operator=(const STAFConnectionProvider &);

    STAFConnectionProvider(const STAFString &name,
                           const STAFString &library,
                           STAFDynamicLibrary_t connLib,
                           STAFConnectionProvider_t provider,
                           const STAFConnectionProviderFunctionTable funcTable);

    static STAFRC_t handleNewConnection(
                        STAFConnectionProvider_t provider,
                        STAFConnection_t conn,
                        const STAFConnectionProviderFunctionTable *funcTable,
                        void *data);

    STAFString fName;
    STAFString fLibrary;
    STAFDynamicLibrary_t fConnLib;
    STAFConnectionProvider_t fProvider;
    STAFConnectionProviderFunctionTable fFuncTable;
    NewConnectionFunc fNewConnFunc;
};


class STAFConnection
{
public:

    void read(void *buffer, unsigned int size, bool doTimeout = false);
    unsigned int readUInt(bool doTimeout = false);
    void readUInt(unsigned int &uint, bool doTimeout = false);
    STAFString readString(bool doTimeout = false);
    void readString(STAFString &theString, bool doTimeout = false);

    void write(void *buffer, unsigned int size, bool doTimeout = false);
    void writeUInt(unsigned int uint, bool doTimeout = false);
    void writeString(const STAFString &theString, bool doTimeout = false);

    void getPeerNetworkIDs(STAFString &logicalIdentifier,
                           STAFString &physicalIdentifier);

    ~STAFConnection();

private:

    friend class STAFConnectionProvider;

    // Don't allow copy construction or assignment
    STAFConnection(const STAFConnection &);
    STAFConnection &operator=(const STAFConnection &);

    STAFConnection(STAFConnection_t conn,
                   const STAFConnectionProviderFunctionTable *funcTable);

    STAFConnection_t fConn;
    const STAFConnectionProviderFunctionTable *fFuncTable;
};


STAF_EXCEPTION_DEFINITION(STAFConnectionProviderException, STAFException);
STAF_EXCEPTION_DEFINITION(STAFConnectionProviderConnectException,
                          STAFConnectionProviderException);
STAF_EXCEPTION_DEFINITION(STAFConnectionProviderConnectTimeoutException,
                          STAFConnectionProviderConnectException);

STAF_EXCEPTION_DEFINITION(STAFConnectionException, STAFException);
STAF_EXCEPTION_DEFINITION(STAFConnectionIOException, STAFConnectionException);

// Now include inline definitions

#ifndef STAF_NATIVE_COMPILER
#include "STAFConnectionProviderInlImpl.cpp"
#endif

// End C++ language definitions

// End #ifdef __cplusplus
#endif

#endif
