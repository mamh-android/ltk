/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAFOSTypes.h"
#include <tcl.h>
#include "STAF.h"
#include "STAFString.h"

static STAFString registerString(STAFString("STAF::Register") + kUTF8_NULL);
static STAFString submitString(STAFString("STAF::Submit") + kUTF8_NULL);
static STAFString unregisterString(STAFString("STAF::UnRegister") + kUTF8_NULL);
static STAFString handleString(STAFString("STAF::Handle") + kUTF8_NULL);
static STAFString rcString(STAFString("STAF::RC") + kUTF8_NULL);
static STAFString resultString(STAFString("STAF::Result") + kUTF8_NULL);
static STAFString addPrivacyDelimitersString(
    STAFString("STAF::AddPrivacyDelimiters") + kUTF8_NULL);
static STAFString escapePrivacyDelimitersString(
    STAFString("STAF::EscapePrivacyDelimiters") + kUTF8_NULL);
static STAFString maskPrivateDataString(
    STAFString("STAF::MaskPrivateData") + kUTF8_NULL);
static STAFString removePrivacyDelimitersString(
    STAFString("STAF::RemovePrivacyDelimiters") + kUTF8_NULL);

static inline char *getBuffer(const STAFString &string)
{ return const_cast<char *>(string.buffer()); }

extern "C"
{

#if defined(STAF_OS_NAME_HPUX) && !defined(__ia64)
    void _main();
#endif

/****************************************/
/*  Tcl Wrappers for libSTAF functions  */
/****************************************/

int RegisterCmd(ClientData clientData, Tcl_Interp *interp, int objc,
                Tcl_Obj *CONST objv[]);

int SubmitCmd(ClientData clientData, Tcl_Interp *interp, int objc,
              Tcl_Obj *CONST objv[]);

int UnRegisterCmd(ClientData clientData, Tcl_Interp *interp, int objc,
                  Tcl_Obj *CONST objv[]);

int AddPrivacyDelimitersCmd(ClientData clientData, Tcl_Interp *interp,
                            int objc, Tcl_Obj *CONST objv[]);

int EscapePrivacyDelimitersCmd(ClientData clientData, Tcl_Interp *interp,
                               int objc, Tcl_Obj *CONST objv[]);

int MaskPrivateDataCmd(ClientData clientData, Tcl_Interp *interp,
                       int objc, Tcl_Obj *CONST objv[]);

int RemovePrivacyDelimitersCmd(ClientData clientData, Tcl_Interp *interp,
                               int objc, Tcl_Obj *CONST objv[]);

/********************************************************************/
/* Initialization Procedure called when package is loaded in script */
/********************************************************************/

int Tclstaf_Init(Tcl_Interp *interp) 
{
    #if defined(STAF_OS_NAME_HPUX) && !defined(__ia64)
        _main();
    #endif

    /* Register STAFRegister() */
    Tcl_CreateObjCommand(interp, getBuffer(registerString), RegisterCmd,
                         (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    /* Register STAFSubmit() */
    Tcl_CreateObjCommand(interp, getBuffer(submitString), SubmitCmd,
                         (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    
    /* Register STAFUnRegister() */
    Tcl_CreateObjCommand(interp, getBuffer(unregisterString), UnRegisterCmd,
                         (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    /* Register STAFAddPrivacyDelimiters() */
    Tcl_CreateObjCommand(interp, getBuffer(addPrivacyDelimitersString),
                         AddPrivacyDelimitersCmd, (ClientData)NULL,
                         (Tcl_CmdDeleteProc *)NULL);

    /* Register STAFEscapePrivacyDelimiters() */
    Tcl_CreateObjCommand(interp, getBuffer(escapePrivacyDelimitersString),
                         EscapePrivacyDelimitersCmd, (ClientData)NULL,
                         (Tcl_CmdDeleteProc *)NULL);

    /* Register STAFMaskPrivateData() */
    Tcl_CreateObjCommand(interp, getBuffer(maskPrivateDataString),
                         MaskPrivateDataCmd, (ClientData)NULL,
                         (Tcl_CmdDeleteProc *)NULL);

    /* Register STAFRemovePrivacyDelimiters() */
    Tcl_CreateObjCommand(interp, getBuffer(removePrivacyDelimitersString),
                         RemovePrivacyDelimitersCmd, (ClientData)NULL,
                         (Tcl_CmdDeleteProc *)NULL);

    Tcl_PkgProvide(interp, "TCLSTAF", "1.0");

    return TCL_OK;
}


/******************************************/
/* Implementation of Wrappers             */
/******************************************/

// XXX: In serveral places below, I use Tcl_NewStringObj, but I don't know
//      how to get rid of the new strings, or, even if I need to


int RegisterCmd(ClientData clientData, Tcl_Interp *interp, int objc,
                Tcl_Obj *CONST objv[])
{  
    if (objc != 2)
    {
        STAFString usage("Usage: STAF::Register processName");
        usage += kUTF8_NULL;
        Tcl_WrongNumArgs(interp, 1, objv, getBuffer(usage));
           return TCL_ERROR;
    }
    
    STAFHandle_t handle = 0;
    unsigned int rc = STAFRegisterUTF8(Tcl_GetStringFromObj(objv[1], 0),
                                       &handle);  

    Tcl_ObjSetVar2(interp, Tcl_NewStringObj(getBuffer(rcString), -1), 0,
                   Tcl_NewIntObj(static_cast<int>(rc)), 0);

    if (rc == 0)
    {
        Tcl_ObjSetVar2(interp, Tcl_NewStringObj(getBuffer(handleString), -1), 0,
                       Tcl_NewIntObj(static_cast<int>(handle)), 0);
    }
    
    Tcl_SetIntObj(Tcl_GetObjResult(interp), static_cast<int>(rc));
          
    return TCL_OK;
}


int SubmitCmd (ClientData clientData, Tcl_Interp *interp, int objc,
               Tcl_Obj *CONST objv[])
{
    if (objc != 4)
    {
        STAFString usage("Usage: STAF::Register location service request");
        usage += kUTF8_NULL;
        Tcl_WrongNumArgs(interp, 1, objv, getBuffer(usage));
           return TCL_ERROR;
    }
    
    STAFHandle_t handle = 0;
    Tcl_Obj *handleObj =
            Tcl_ObjGetVar2(interp, Tcl_NewStringObj(getBuffer(handleString),
                           -1), 0, TCL_LEAVE_ERR_MSG);

    if (handleObj == 0) return TCL_ERROR;

    if (Tcl_GetIntFromObj(interp, handleObj,
                          reinterpret_cast<int *>(&handle)) != TCL_OK)
    {
        return TCL_ERROR;
    }

    int requestLength = 0;
    char *request = Tcl_GetStringFromObj(objv[3], &requestLength);
    unsigned int resultLength = 0;
    char *result = 0;
    unsigned int rc = STAFSubmitUTF8(handle, Tcl_GetStringFromObj(objv[1], 0),
                                     Tcl_GetStringFromObj(objv[2], 0),
                                     request, requestLength,
                                     &result, &resultLength);  

    Tcl_ObjSetVar2(interp, Tcl_NewStringObj(getBuffer(rcString), -1), 0,
                   Tcl_NewIntObj(static_cast<int>(rc)), 0);
    Tcl_ObjSetVar2(interp, Tcl_NewStringObj(getBuffer(resultString), -1), 0,
                   Tcl_NewStringObj(result, static_cast<int>(resultLength)), 0);
    

    STAFFree(handle, result);

    Tcl_SetIntObj(Tcl_GetObjResult(interp), static_cast<int>(rc));

    return TCL_OK;
}


int UnRegisterCmd(ClientData clientData, Tcl_Interp *interp, int objc,
                  Tcl_Obj *CONST objv[])
{
    if (objc != 1)
    {
        STAFString usage("Usage: STAF::UnRegister");
        usage += kUTF8_NULL;
        Tcl_WrongNumArgs(interp, 1, objv, getBuffer(usage));
        return TCL_ERROR;
    }
    
    STAFHandle_t handle = 0;
    Tcl_Obj *handleObj =
            Tcl_ObjGetVar2(interp, Tcl_NewStringObj(getBuffer(handleString),
                           -1), 0, TCL_LEAVE_ERR_MSG);

    if (handleObj == 0) return TCL_ERROR;


    if (Tcl_GetIntFromObj(interp, handleObj,
                          reinterpret_cast<int *>(&handle)) != TCL_OK)
    {
        return TCL_ERROR;
    }

    unsigned int rc = STAFUnRegister(handle);

    Tcl_ObjSetVar2(interp, Tcl_NewStringObj(getBuffer(rcString), -1), 0,
                   Tcl_NewIntObj(static_cast<int>(rc)), 0);

    Tcl_UnsetVar(interp, getBuffer(handleString), TCL_LEAVE_ERR_MSG);

    Tcl_SetIntObj(Tcl_GetObjResult(interp), static_cast<int>(rc));

    return TCL_OK;
}


int AddPrivacyDelimitersCmd(ClientData clientData, Tcl_Interp *interp,
                            int objc, Tcl_Obj *CONST objv[])
{
    if (objc != 2)
    {
        STAFString usage("Usage: STAF::AddPrivacyDelimiters data");
        usage += kUTF8_NULL;
        Tcl_WrongNumArgs(interp, 1, objv, getBuffer(usage));
        return TCL_ERROR;
    }
    
    /* Get data argument (type String) and convert to type STAFString_t */

    int dataLength = 0;
    char *dataPtr = Tcl_GetStringFromObj(objv[1], &dataLength);
    STAFString_t data = 0;
    STAFStringConstruct(&data, dataPtr, dataLength, 0);

    STAFString_t result = 0;

    STAFAddPrivacyDelimiters(data, &result);

    /* Convert result to a Tcl string object */

    unsigned int resultLength = 0;
    const char *resultBuffer = 0;
    
    STAFStringGetBuffer(result, &resultBuffer, &resultLength, 0);

    Tcl_SetStringObj(Tcl_GetObjResult(interp),
                     const_cast<char *>(resultBuffer),
                     static_cast<int>(resultLength));

    STAFStringDestruct(&data, 0);
    STAFStringDestruct(&result, 0);

    return TCL_OK;
}


int EscapePrivacyDelimitersCmd(ClientData clientData, Tcl_Interp *interp,
                               int objc, Tcl_Obj *CONST objv[])
{
    if (objc != 2)
    {
        STAFString usage("Usage: STAF::EscapePrivacyDelimiters data");
        usage += kUTF8_NULL;
        Tcl_WrongNumArgs(interp, 1, objv, getBuffer(usage));
        return TCL_ERROR;
    }
    
    /* Get data argument (type String) and convert to type STAFString_t */

    int dataLength = 0;
    char *dataPtr = Tcl_GetStringFromObj(objv[1], &dataLength);
    STAFString_t data = 0;
    STAFStringConstruct(&data, dataPtr, dataLength, 0);

    STAFString_t result = 0;

    STAFEscapePrivacyDelimiters(data, &result);

    /* Convert result to a Tcl string object */

    unsigned int resultLength = 0;
    const char *resultBuffer = 0;
    
    STAFStringGetBuffer(result, &resultBuffer, &resultLength, 0);

    Tcl_SetStringObj(Tcl_GetObjResult(interp),
                     const_cast<char *>(resultBuffer),
                     static_cast<int>(resultLength));

    STAFStringDestruct(&data, 0);
    STAFStringDestruct(&result, 0);

    return TCL_OK;
}



int MaskPrivateDataCmd(ClientData clientData, Tcl_Interp *interp,
                       int objc, Tcl_Obj *CONST objv[])
{
    if (objc != 2)
    {
        STAFString usage("Usage: STAF::MaskPrivateData data");
        usage += kUTF8_NULL;
        Tcl_WrongNumArgs(interp, 1, objv, getBuffer(usage));
        return TCL_ERROR;
    }
    
    /* Get data argument (type String) and convert to type STAFString_t */

    int dataLength = 0;
    char *dataPtr = Tcl_GetStringFromObj(objv[1], &dataLength);
    STAFString_t data = 0;
    STAFStringConstruct(&data, dataPtr, dataLength, 0);

    STAFString_t result = 0;

    STAFMaskPrivateData(data, &result);

    /* Convert result to a Tcl string object */

    unsigned int resultLength = 0;
    const char *resultBuffer = 0;
    
    STAFStringGetBuffer(result, &resultBuffer, &resultLength, 0);

    Tcl_SetStringObj(Tcl_GetObjResult(interp),
                     const_cast<char *>(resultBuffer),
                     static_cast<int>(resultLength));

    STAFStringDestruct(&data, 0);
    STAFStringDestruct(&result, 0);

    return TCL_OK;
}


int RemovePrivacyDelimitersCmd(ClientData clientData, Tcl_Interp *interp,
                               int objc, Tcl_Obj *CONST objv[])
{
    if (objc != 2 && objc != 3)
    {
        STAFString usage(
            "Usage: STAF::RemovePrivacyDelimiters data [numLevels]");
        usage += kUTF8_NULL;
        Tcl_WrongNumArgs(interp, 1, objv, getBuffer(usage));
        return TCL_ERROR;
    }
    
    /* Get data argument (type String) and convert to type STAFString_t */

    int dataLength = 0;
    char *dataPtr = Tcl_GetStringFromObj(objv[1], &dataLength);
    STAFString_t data = 0;
    STAFStringConstruct(&data, dataPtr, dataLength, 0);

    /* The numLevels argument is optional.  Defaults to 0 */

    int numLevels = 0;
    
    if (objc == 3)
    {
        Tcl_GetIntFromObj(interp, objv[2], &numLevels);
    }

    STAFString_t result = 0;

    STAFRemovePrivacyDelimiters(data, numLevels, &result);

    /* Convert result to a Tcl string object */

    unsigned int resultLength = 0;
    const char *resultBuffer = 0;
    
    STAFStringGetBuffer(result, &resultBuffer, &resultLength, 0);

    Tcl_SetStringObj(Tcl_GetObjResult(interp),
                     const_cast<char *>(resultBuffer),
                     static_cast<int>(resultLength));

    STAFStringDestruct(&data, 0);
    STAFStringDestruct(&result, 0);

    return TCL_OK;
}


static char sInfoBuffer[256] = { 0 };
static int sInfoBufferInited = 0;

char *STAFGetInformation()
{
   if (!sInfoBufferInited)
   {
       sprintf(sInfoBuffer, "%s STAF Tcl support library version 1.0",
               #ifdef NDEBUG
                   "Retail");
               #else
                   "Debug");
               #endif
       sInfoBufferInited = 1;
   }

   return sInfoBuffer;
}

} // end extern "C"
