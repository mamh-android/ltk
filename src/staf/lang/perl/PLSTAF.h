/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_PLSTAF
#define STAF_PLSTAF

XS(XS_STAF_Register); /* prototype to pass -Wmissing-prototypes */
XS(XS_STAF_UnRegister); /* prototype to pass -Wmissing-prototypes */
XS(XS_STAF_Submit); /* prototype to pass -Wmissing-prototypes */
XS(XS_STAF_AddPrivacyDelimiters); /* prototype to pass -Wmissing-prototypes */
XS(XS_STAF_EscapePrivacyDelimiters); /* prototype to pass -Wmissing-prototypes */
XS(XS_STAF_MaskPrivateData); /* prototype to pass -Wmissing-prototypes */
XS(XS_STAF_RemovePrivacyDelimiters); /* prototype to pass -Wmissing-prototypes */

extern "C"
XS(boot_PLSTAF); /* prototype to pass -Wmissing-prototypes */

#endif
