#include "STAF.h"
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"


MODULE = PLSTAF		PACKAGE = PLSTAF		
PROTOTYPES: ENABLE

void
basictest()
  CODE:
    PerlIO_stdoutf("ok");

char *
returntest()
  CODE:
    RETVAL= "ok";
  OUTPUT:
    RETVAL

void
argtest(input)
    int input
  CODE:
    PerlIO_stdoutf("ok %d\n", input);

void
listandargtest(input)
    int input
  PPCODE:
    XPUSHs(sv_2mortal(newSVpvn("ok", 3)));
    XPUSHs(sv_2mortal(newSVnv(input)));

STAFHandle_t *
newHandle ()
  CODE:
    RETVAL=(STAFHandle_t *)malloc(sizeof (STAFHandle_t));
  OUTPUT:
    RETVAL

void
ProcRegister (processName, handle)
    char *processName
    STAFHandle_t *handle
  CODE:
    {
      int i;
      if (kSTAFOk != (i = STAFRegister(processName, handle))) {
	Perl_croak(aTHX_ "Unexpected return code %d",i);
	XSRETURN_NO;
      }
      else
	XSRETURN_YES;
    }

void
ProcUnRegister (handle)
    STAFHandle_t *handle
  CODE:
    {
      int i;
      if (kSTAFOk != (i = STAFUnRegister(*handle))) {
	Perl_croak(aTHX_ "Unexpected return code %d",i);
	XSRETURN_NO;
      }
      else
	XSRETURN_YES;
    }

void
ProcSubmit (handle, syncoption, targetmachine, targetservice, requestSV)
    STAFHandle_t *handle
    char *targetmachine
    char *targetservice
    SV *requestSV
    STAFSyncOption_t syncoption
  PPCODE:
    {
      int i;
      char * stafres;
      unsigned int reslen;
      char *request = SvPV(requestSV, PL_na);
      if (DO_UTF8(requestSV))
      {
	if (kSTAFOk != (i = STAFSubmit2UTF8(*handle, syncoption, targetmachine,
	targetservice, request, strlen(request), &stafres, &reslen))) {
	  Perl_croak(aTHX_ "Unexpected return code %d",i);
	  if (stafres != 0) STAFFree (*handle, stafres);
	  XSRETURN_NO;
	}
	else
	{
	  if (reslen > 0) {
	    XPUSHs(sv_2mortal(newSVpvn(stafres, reslen)));
	    STAFFree (*handle, stafres);
	    }
	  else XSRETURN_YES;
	}
      }
      else
      {
	if (kSTAFOk != (i = STAFSubmit2(*handle, syncoption, targetmachine,
	targetservice, request, strlen(request), &stafres, &reslen))) {
	  Perl_croak(aTHX_ "Unexpected return code %d",i);
	  if (stafres != 0) STAFFree (*handle, stafres);
	  XSRETURN_NO;
	}
	else
	{
	  if (reslen > 0) {
	    XPUSHs(sv_2mortal(newSVpvn(stafres, reslen)));
	    STAFFree (*handle, stafres);
	    }
	  else XSRETURN_YES;
	}
      }
    }

void
delHandle(handle)
    STAFHandle_t *handle
  CODE:
    if (handle != NULL)
    {
      free(handle);
      XSRETURN_YES;
    }
    else
      XSRETURN_NO;

STAFSyncOption_t
SyncReq ()
  CODE:
    RETVAL = kSTAFReqSync;
  OUTPUT:
    RETVAL

STAFSyncOption_t
FireAndForgetReq ()
  CODE:
    RETVAL = kSTAFReqFireAndForget;
  OUTPUT:
    RETVAL

STAFSyncOption_t
QueueReq ()
  CODE:
    RETVAL = kSTAFReqQueue;
  OUTPUT:
    RETVAL

STAFSyncOption_t
RetainReq ()
  CODE:
    RETVAL = kSTAFReqRetain;
  OUTPUT:
    RETVAL

STAFSyncOption_t
QueueRetainReq ()
  CODE:
    RETVAL = kSTAFReqQueueRetain;
  OUTPUT:
    RETVAL
