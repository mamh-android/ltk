/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_LtkTest
#define STAF_LtkTest

#ifndef NODEBUG
extern ofstream out;
#define DEBUGOUTFILE(A) do { \
		out << __PRETTY_FUNCTION__ << ":(" << __LINE__ << ")" << A << std::endl; \
} while (0)
#define DEBUGOUT(A) do { \
		std::cout << __PRETTY_FUNCTION__ << ":(" << __LINE__ << ")" << A << std::endl; \
} while (0)
#else
#define DEBUGOUT(A) do { } while (0)
#define DEBUGOUTFILE(A) do { } while (0)
#endif
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum STAFDeviceError_e {
	// add service-specific return codes here
	KSTAFLtkTestNotEntryOwner = 4001,
	kSTAFLtkTestHasPendingRequests = 4002,
	kSTAFLtkTestPutPendingRequests = 4003,
	kSTAFLtkTestNoEntriesAvailable = 4004,
	kSTAFLtkTestNotTaskMatch = 4005,
	kSTAFLtkTestRunNotKeyAvailable = 4006,
	kSTAFLtkTestNotPoolMatch = 4007,
} STAFDeviceError_t;


#ifdef __cplusplus
}
#endif

#endif
