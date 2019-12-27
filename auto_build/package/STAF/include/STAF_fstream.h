/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_fstream
#define STAF_fstream

#ifdef STAF_Config_NoSTDFStreamSupport

  #include <fstream.h>

#else

  #include <fstream>

  using std::fstream;
  using std::ofstream;
  using std::ifstream;
  using std::ios;

#endif


#ifdef STAF_NO_IOS_BINARY

  #define STAF_ios_binary 0x00000000

#else

  #define STAF_ios_binary ios::binary

#endif


#endif
