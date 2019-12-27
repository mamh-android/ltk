/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_exception
#define STAF_exception

#include <exception>

#ifdef STAF_CONFIG_NoSTDSetTerminate

  #define STAF_set_terminate(f) set_terminate(f)

#else

  #define STAF_set_terminate(f) std::set_terminate(f)

#endif

#endif
