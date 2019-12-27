/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_iostream
#define STAF_iostream

#ifdef STAF_Config_NoSTDIOStreamSupport

  #include <iostream.h>
  #include <iomanip.h>

#else

  #include <iostream>
  #include <iomanip>

  using std::cin;
  using std::cout;
  using std::cerr;
  using std::endl;
  using std::flush;
  using std::dec;
  using std::hex;
  using std::ostream;
  using std::istream;
  using std::setw;
  using std::setfill;

#endif


#endif
