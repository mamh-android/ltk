/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_Timestamp
#define STAF_Timestamp

#include <time.h>
#include "STAFError.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct STAFRelativeTimeImpl *STAFRelativeTime_t;

/****************************************************************************/
/* STAFThreadSafeLocalTimeString - This is a thread safe wrapper around     */
/*                                 strftime()                               */
/*                                                                          */
/* Accepts: (In)  Pointer to buffer in which to place time string           */
/*          (In)  Size of buffer                                            */
/*          (In)  Format string (identical to that accepted by strftime())  */
/*          (In)  The time to convert to a string (obtained from time())    */
/*          (Out) Pointer to OS return code                                 */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/****************************************************************************/
STAFRC_t STAFThreadSafeLocalTimeString(char *buffer, unsigned int bufSize,
                                       const char *format, time_t theTime,
                                       unsigned int *osRC);

/****************************************************************************/
/* STAFThreadSafeLocalTime - Provides a thread safe wrapper around          */
/*                           localtime()                                    */
/*                                                                          */
/* Accepts: (Out) Pointer to a tm structure (from <time.h>                  */
/*          (In)  The time to convert to a string (obtained from time())    */
/*          (Out) Pointer to OS return code                                 */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/****************************************************************************/
STAFRC_t STAFThreadSafeLocalTime(struct tm *theTM, time_t theTime,
                                 unsigned int *osRC);


/****************************************************************************/
/* STAFTimestampGetRelativeTime - Gets the current relative time            */
/*                                                                          */
/* Accepts: (Out) Pointer to a relative time                                */
/*          (Out) Pointer to OS return code                                 */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/****************************************************************************/
STAFRC_t STAFTimestampGetRelativeTime(STAFRelativeTime_t *relTime,
                                      unsigned int *osRC);


/****************************************************************************/
/* STAFTimestampGetRelativeTimeDifference - Gets the difference (in         */
/*                                          milliseconds) between two       */
/*                                          relative times                  */
/*                                                                          */
/* Accepts: (In)  Relative time 1                                           */
/*          (In)  Relative time 2                                           */
/*          (Out) (Relative time 1 - Relative time 2) in milliseconds       */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/*                                                                          */
/* Notes  :  1) The difference is undefined if Relative Time 1 is less than */
/*              Relative Time 2                                             */
/*           2) The difference is undefined if it would be greater than the */
/*              size of an unsigned int                                     */
/****************************************************************************/
STAFRC_t STAFTimestampGetRelativeTimeDifference(const STAFRelativeTime_t lhs,
                                                const STAFRelativeTime_t rhs,
                                                unsigned int *diffInMillis);


/****************************************************************************/
/* STAFTimestampFreeRelativeTime - Frees a relative time                    */
/*                                                                          */
/* Accepts: (I/O) Pointer to a relative time                                */
/*                                                                          */
/* Returns:  kSTAFOk, on success                                            */
/*           other on error                                                 */
/****************************************************************************/
STAFRC_t STAFTimestampFreeRelativeTime(STAFRelativeTime_t *relTime);


#ifdef __cplusplus
}

#include "STAFString.h"
#include "STAFException.h"

// STAFTimestamp - Represents a specific point in time.  Provides methods
//                 to convert the timestamp to a string and to seconds
//                 past midnight, as well as operator-().

class STAFTimestamp
{
public:

    // Accepted input formats are
    //
    //     <date><sep><time>
    //     <date><sep>
    //     <date>
    //     <sep><time>
    //     <time>
    //     <sep>
    //     <empty string>
    //
    // Note: Empty <date> defaults to current date
    //       Empty <time> defaults to 00:00:00
    //
    // Accepted date formats are
    //
    //     YYYYMMDD
    //     MM/DD/YYYY
    //     MM/DD/YY      (YY < 90 is treated as 2000 + YY)
    //
    // Accepted time formats are (note: time is in 24-hour format)
    //
    //     HH:MM:SS
    //     HH:MM
    //     HH
    //
    // Note: You may specify multiple characters which could potentially
    //       separate the date from the time (e.g. '.', '-', '@', etc.)

    STAFTimestamp(const STAFString &aString,
                  const STAFString &dateTimeSeps = STAFString(kUTF8_HYPHEN));
    STAFTimestamp(unsigned int year, unsigned int month, unsigned int day,
                  unsigned int hour, unsigned int minute, unsigned int second);
    STAFTimestamp(time_t aTime = time(0));

    static STAFTimestamp now();
    static bool isValidTimestampString(const STAFString &aString,
                                       const STAFString &dateTimeSeps =
                                       STAFString(kUTF8_HYPHEN));
    static bool isValidTimestampData(unsigned int year, unsigned int month,
                                     unsigned int day, unsigned int hour,
                                     unsigned int minute, unsigned int second);

    // Pass in the time difference in seconds and get a string containing the
    // time difference in format "HH:MM:SS", or "HHH:MM:SS" if HH > 24, etc.
    static STAFString getElapsedTime(unsigned int seconds);

    STAFString asString(const char *format = "%Y%m%d-%H:%M:%S") const;
    STAFString asDateString() const { return asString("%Y%m%d"); }
    STAFString asTimeString() const { return asString("%H:%M:%S"); }

    unsigned int getYear() const;       // 4-digit year
    unsigned int getMonth() const;      // 1-12
    unsigned int getDay() const;        // 1-31
    unsigned int getHour() const;       // 0-23
    unsigned int getMinute() const;     // 0-59
    unsigned int getSecond() const;     // 0-59

    unsigned int asSecondsPastMidnight() const;

    time_t getImpl() const;

    // The difference is in seconds
    unsigned int operator-(const STAFTimestamp &rhs) const;

    bool operator<(const STAFTimestamp &rhs)  const;
    bool operator==(const STAFTimestamp &rhs) const { return fTime == rhs.fTime; }
    bool operator!=(const STAFTimestamp &rhs) const { return !(*this == rhs); }
    bool operator>(const STAFTimestamp &rhs)  const { return rhs < *this; }
    bool operator<=(const STAFTimestamp &rhs) const { return !(*this > rhs); }
    bool operator>=(const STAFTimestamp &rhs) const { return !(*this < rhs); }

private:

    // Need functions to validate year, month, day and hour, min, sec as
    // mktime doesn't really handle this.

    bool isValidDate(unsigned int year, unsigned int month, unsigned int day);
    bool isValidTime(unsigned int hour, unsigned int min, unsigned int sec);
  
    static STAFString getTimeFormat(unsigned int in);

    time_t fTime;
};


// STAFRelativeTime - Represents a relative point in time.  The primary purpose
//                    of this class is to get the difference between two
//                    of its instances.

class STAFRelativeTime
{
public:

    STAFRelativeTime();

    unsigned int operator-(const STAFRelativeTime &rhs);

    ~STAFRelativeTime();

private:

    // Disallow copy construction and assignment
    STAFRelativeTime(const STAFRelativeTime &);
    STAFRelativeTime &operator=(const STAFRelativeTime &);

    STAFRelativeTime_t fRelTime;
};


// Define exceptions

STAF_EXCEPTION_DEFINITION(STAFTimestampException, STAFException);
STAF_EXCEPTION_DEFINITION(STAFTimestampInvalidDateTimeException,
                          STAFTimestampException);
STAF_EXCEPTION_DEFINITION(STAFTimestampInvalidDateException,
                          STAFTimestampInvalidDateTimeException);
STAF_EXCEPTION_DEFINITION(STAFTimestampInvalidTimeException,
                          STAFTimestampInvalidDateTimeException);


// Now include inline definitions

#ifndef STAF_NATIVE_COMPILER
#include "STAFTimestampInlImpl.cpp"
#endif

// End C++ language definitions

// End #ifdef __cplusplus
#endif

#endif
