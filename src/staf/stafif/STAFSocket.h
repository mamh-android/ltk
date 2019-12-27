/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_Socket
#define STAF_Socket
  
#include "STAF.h"
 
#ifdef __cplusplus
extern "C"
{
#endif
 
/*********************************************************************/
/* The following socket calls work correctly across platforms, thus, */
/* STAF provides no equivalent.                                      */
/*                                                                   */
/* accept(), bind(), connect(), listen(), recv(), select(), send(),  */
/* htons(), htonl(), inet_addr()                                     */
/*                                                                   */
/* All socket descriptors should be declared to be of type           */
/* STAFSocket_t to work correctly across platforms.                  */
/*                                                                   */
/* The third paramater to accept(), the address length, should be    */
/* considered to have type STAFSocketLen_t *.                        */
/*                                                                   */
/* There are three constants that are defined as results of the      */
/* socket calls to aid in portability of the socket code.            */
/*                                                                   */
/*   SOCEINPROGRESS - The call is in progress                        */
/*   SOCEWOULDBLOCK - The call would block                           */
/*   SOCEINTR       - The call was interrupted                       */
/*********************************************************************/

typedef enum STAFSocketBlockingMode_e
{
    kSTAFSocketBlocking = 0,
    kSTAFSocketNonBlocking = 1
} STAFSocketBlockingMode_t;

/*********************************************************************/
/* STAFSocketIsValidSocket - Determines if a socket descriptor,      */
/*                           received from socket() or accept() is   */
/*                           valid                                   */
/*                                                                   */
/* Accepts: (In)  The socket to check                                */
/*                                                                   */
/* Returns:  0, if the socket is valid                               */
/*           1, if the socket is not valid                           */
/*********************************************************************/
unsigned int STAFSocketIsValidSocket(STAFSocket_t theSocket);


/*********************************************************************/
/* STAFSocketInit - Initialize socket functionality                  */
/*                           received from socket() or accept() is   */
/*                           valid                                   */
/*                                                                   */
/* Accepts: (Out) Pointer to error buffer                            */
/*                                                                   */
/* Returns: kSTAFOk, if socket functionality was initialized         */
/*                   successfully                                    */
/*          other, if socket functionaity could not be initialized   */
/*                                                                   */
/* Notes: 1) STAFSocketInit() must be called before any other socket */
/*           APIs are used in an application                         */
/*********************************************************************/
STAFRC_t STAFSocketInit(STAFString_t *errorBuffer);


/*********************************************************************/
/* STAFSocketClose - Closes a socket                                 */
/*                                                                   */
/* Accepts: (In)  The socket to close                                */
/*                                                                   */
/* Returns: kSTAFOk, if the socket was closed successfully           */
/*          other, if the socket could not be closed                 */
/*********************************************************************/
STAFRC_t STAFSocketClose(STAFSocket_t theSocket);


/*********************************************************************/
/* STAFSocketGetLastError - Returns the last error that occurred     */
/*                          during a socket operation                */
/*                                                                   */
/* Accepts: Nothing                                                  */
/*                                                                   */
/* Returns: The last socket error                                    */
/*                                                                   */
/* Notes: 1) The value returned by this function is not valid once   */
/*           any other operating system function has been invoked    */
/*********************************************************************/
int STAFSocketGetLastError();


/*********************************************************************/
/* STAFSocketGetMyHostInfo - Retrieves the hostname and printable    */
/*                           IP address of the local system          */
/*                                                                   */
/* Accepts: (Out) Pointer to hostname                                */
/*          (Out) Pointer to IP address                              */
/*          (Out) Pointer to error buffer                            */
/*                                                                   */
/* Returns: kSTAFOk, if hostname obtained successfully               */
/*          other, if an error is encountered                        */
/*********************************************************************/
STAFRC_t STAFSocketGetMyHostInfo(STAFString_t *hostname,
                                 STAFString_t *ipaddr,
                                 STAFString_t *errorBuffer);


/*********************************************************************/
/* STAFSocketGetInAddrByName - Gets the in_addr of a hostname        */
/*                                                                   */
/* Accepts: (In)  hostname                                           */
/*          (Out) Pointer to resultant in_addr                       */
/*          (Out) Pointer to error buffer                            */
/*                                                                   */
/* Returns: kSTAFOk, if successful                                   */
/*          other, if unsuccessful                                   */
/*********************************************************************/
STAFRC_t STAFSocketGetInAddrByName(STAFStringConst_t hostname, in_addr *addr,
                                   STAFString_t *errorBuffer);

/*********************************************************************/
/* STAFSocketGetNameByInAddr - Gets name of a system from an in_addr */
/*                             structure                             */
/*                                                                   */
/* Accepts: (In)  in_addr                                            */
/*          (Out) Pointer to resultant STAFString_t                  */
/*          (Out) Pointer to error buffer                            */
/*                                                                   */
/* Returns: kSTAFOk, if successful                                   */
/*          other, if unsuccessful                                   */
/*********************************************************************/
STAFRC_t STAFSocketGetNameByInAddr(in_addr *addr, STAFString_t *hostname,
                                   STAFString_t *errorBuffer);

/*********************************************************************/
/* STAFIPv6SocketGetNameByInAddr - Gets name of a system from an     */
/*                             sockaddr structure                    */
/*                                                                   */
/* Accepts: (In)  addr                                               */
/*          (In)  addrlen                                            */
/*          (Out) Pointer to resultant STAFString_t                  */
/*          (Out) Pointer to error buffer                            */
/*                                                                   */
/* Returns: kSTAFOk, if successful                                   */
/*          other, if unsuccessful                                   */
/*********************************************************************/
STAFRC_t STAFIPv6SocketGetNameByInAddr(sockaddr *addr, int addrlen,
                                       STAFString_t *hostname,
                                       STAFString_t *errorBuffer);

/*********************************************************************/
/* STAFSocketAddressFromInAddr - Gets a printable IP                 */
/*                                           address string from an  */
/*                                           in_addr structure       */
/*                                                                   */
/* Accepts: (In)  in_addr                                            */
/*          (Out) Pointer to resultant STAFString_t                  */
/*          (Out) Pointer to error buffer                            */
/*                                                                   */
/* Returns: kSTAFOk, if successful                                   */
/*          other, if unsuccessful                                   */
/*********************************************************************/
STAFRC_t STAFSocketGetPrintableAddressFromInAddr(in_addr *addr,
                                                 STAFString_t *ipaddr,
                                                 STAFString_t *errorBuffer);

/*********************************************************************/
/* STAFIPv6SocketAddressFromInAddr - Gets a printable IP             */
/*                               address string from an              */
/*                               sockaddr structure                  */
/*                                                                   */
/* Accepts: (In)  addr                                               */
/*          (In)  addrlen                                            */
/*          (Out) Pointer to resultant STAFString_t                  */
/*          (Out) Pointer to error buffer                            */
/*                                                                   */
/* Returns: kSTAFOk, if successful                                   */
/*          other, if unsuccessful                                   */
/*********************************************************************/
STAFRC_t STAFIPv6SocketGetPrintableAddressFromInAddr(sockaddr *addr, 
                                                     int addrlen,
                                                     STAFString_t *ipaddr,
                                                     STAFString_t *errorBuffer);
 
/*********************************************************************/
/* STAFSocketSetBlockingMode - Sets the blocking mode of the socket  */
/*                                                                   */
/* Accepts: (In)  The socket                                         */
/*          (In)  Blocking mode                                      */
/*          (Out) Pointer to error buffer                            */
/*                                                                   */
/* Returns: kSTAFOk, if successful                                   */
/*          other, if unsuccessful                                   */
/*********************************************************************/
STAFRC_t STAFSocketSetBlockingMode(STAFSocket_t socket,
                                   STAFSocketBlockingMode_t blockingMode,
                                   STAFString_t *errorBuffer);

 
#ifdef __cplusplus
}
#endif

#endif
