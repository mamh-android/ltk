/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_TimestampInlImpl
#define STAF_TimestampInlImpl

#include "STAF.h"
#include "STAFTimestamp.h"
#include "STAFString.h"
#include "STAFException.h"

// Begin definitions for STAFTimestamp

STAF_INLINE STAFTimestamp::STAFTimestamp(time_t aTime) : fTime(aTime)
{ /* Do Nothing */ }


STAF_INLINE STAFTimestamp::STAFTimestamp(const STAFString &aString,
                                         const STAFString &seps)
{
    // Get current time
    unsigned int osRC = 0;
    struct tm theTime = { 0 };
    STAFRC_t rc = STAFThreadSafeLocalTime(&theTime,
                                          STAFTimestamp::now().getImpl(), &osRC);

    STAFTimestampException::checkRC(rc, "STAFThreadSafeLocalTime", osRC);

    // Set the time to 00:00:00 by default
    theTime.tm_hour = 0;
    theTime.tm_min = 0;
    theTime.tm_sec = 0;
    theTime.tm_isdst = -1;

    // Split the input into date and time
    //
    // If we can't find the separator then we assume the string is just a time
    // if we can find a colon in it, otherwise we assume it is just a date

    STAFString dateString;
    STAFString timeString;
    unsigned int sepLoc = aString.findFirstOf(seps);

    if (sepLoc != STAFString::kNPos)
    {
        dateString = aString.subString(0, sepLoc);
        timeString = aString.subString(sepLoc + aString.sizeOfChar(sepLoc));
    }
    else if (aString.find(kUTF8_COLON) != STAFString::kNPos)
    {
        timeString = aString;
    }
    else
    {
        dateString = aString;
    }

    // Calculate date stuff first

    unsigned int slashCount = dateString.count(kUTF8_SLASH);
    unsigned int firstSlash = dateString.find(kUTF8_SLASH);
    unsigned int secondSlash = dateString.findLastOf(kUTF8_SLASH);

    if ((dateString.length() > 0) && 
        ((slashCount == 0) && (dateString.length() == 8)) ||
        (((slashCount == 2) && (firstSlash == 2) && (secondSlash == 5) &&
          ((dateString.length() == 8) || (dateString.length() == 10)))))
    {
        STAFString yearString;
        STAFString monthString;
        STAFString dayString;

        if (slashCount == 0)
        {
            yearString = dateString.subString(0, 4);
            monthString = dateString.subString(4, 2);
            dayString = dateString.subString(6);
        }
        else
        {
            monthString = dateString.subString(0, 2);
            dayString = dateString.subString(3, 2);
            yearString = dateString.subString(6);
        }

        try
        {
            theTime.tm_year = yearString.asUInt();
            theTime.tm_mon  = monthString.asUInt() - 1;
            theTime.tm_mday = dayString.asUInt();
        }
        catch (STAFException)
        {
            STAFTimestampInvalidDateException error("STAFTimestamp(string)");
            THROW_STAF_EXCEPTION(error);
        }

        if (!isValidDate(theTime.tm_year, theTime.tm_mon + 1, theTime.tm_mday))
        {
            STAFTimestampInvalidDateException error("STAFTimestamp(string)");
            THROW_STAF_EXCEPTION(error);
        }

        if (theTime.tm_year < 90) theTime.tm_year += 100;
        else theTime.tm_year -= 1900;
    }
    else if (dateString.length() > 0)
    {
        STAFTimestampInvalidDateException error("STAFTimestamp(string)");
        THROW_STAF_EXCEPTION(error);
    }

    // Now do time stuff

    unsigned int colonCount = timeString.count(kUTF8_COLON);
    unsigned int firstColon = timeString.find(kUTF8_COLON);
    unsigned int secondColon = timeString.findLastOf(kUTF8_COLON);

    if ((timeString.length() > 0) &&
        (((colonCount == 0) && (timeString.length() == 2)) ||
          ((colonCount == 1) && (firstColon == 2) && timeString.length() == 5) ||
          ((colonCount == 2) && (firstColon == 2) && (secondColon == 5) &&
           (timeString.length() == 8))))
    {
        theTime.tm_hour = 0;
        theTime.tm_min  = 0;
        theTime.tm_sec  = 0;

        STAFString hourString = timeString.subString(0, 2);
        STAFString minString = timeString.subString(3, 2);
        STAFString secString = timeString.subString(6);

        if ((!hourString.isDigits()) ||
            ((minString.length() > 0) && !minString.isDigits()) ||
            ((secString.length() > 0) && !secString.isDigits()))
        {
            STAFTimestampInvalidTimeException error("STAFTimestamp(string)");
            THROW_STAF_EXCEPTION(error);
        }

        theTime.tm_hour = hourString.asUInt();

        if (colonCount > 0) theTime.tm_min = minString.asUInt();
        if (colonCount > 1) theTime.tm_sec = secString.asUInt();

        if (!isValidTime(theTime.tm_hour, theTime.tm_min, theTime.tm_sec))
        {
            STAFTimestampInvalidTimeException error("STAFTimestamp(string)");
            THROW_STAF_EXCEPTION(error);
        }
    }
    else if (timeString.length() > 0)
    {
        STAFTimestampInvalidTimeException error("STAFTimestamp(string)");
        THROW_STAF_EXCEPTION(error);
    }

    // Now try to get a time_t
    fTime = mktime(&theTime);

    if (fTime == -1)
    {
        STAFTimestampInvalidDateTimeException error("STAFTimestamp(string)");
        THROW_STAF_EXCEPTION(error);
    }
}


STAF_INLINE STAFTimestamp::STAFTimestamp(unsigned int year, unsigned int month,
                                         unsigned int day,  unsigned int hour,
                                         unsigned int minute,
                                         unsigned int second)
{
    if (!isValidDate(year, month, day))
    {
        STAFTimestampInvalidDateException error("STAFTimestamp(y,m,d,h,m,s)");
        THROW_STAF_EXCEPTION(error);
    }

    if (!isValidTime(hour, minute, second))
    {
        STAFTimestampInvalidTimeException error("STAFTimestamp(y,m,d,h,m,s)");
        THROW_STAF_EXCEPTION(error);
    }

    struct tm aTime = { 0 };

    if (year < 90) aTime.tm_year = year + 100;
    else if (year < 1900) aTime.tm_year = year;
    else aTime.tm_year = year - 1900;

    aTime.tm_mon  = month - 1;
    aTime.tm_mday = day;
    aTime.tm_hour = hour;
    aTime.tm_min  = minute;
    aTime.tm_sec  = second;
    aTime.tm_isdst = -1;

    // Now try to get a time_t
    fTime = mktime(&aTime);

    if (fTime == -1)
    {
        STAFTimestampInvalidDateTimeException error("STAFTimestamp(string)");
        THROW_STAF_EXCEPTION(error);
    }
}


STAF_INLINE STAFTimestamp STAFTimestamp::now()
{
    return STAFTimestamp();
}


STAF_INLINE bool STAFTimestamp::isValidTimestampString(
                 const STAFString &aString, const STAFString &dateTimeSeps)
{
    try
    {
        STAFTimestamp test(aString, dateTimeSeps);
    }
    catch (STAFTimestampInvalidDateTimeException)
    {
        return false;
    }

    return true;
}


STAF_INLINE bool STAFTimestamp::isValidTimestampData(unsigned int year,
    unsigned int month, unsigned int day,  unsigned int hour,
    unsigned int minute, unsigned int second)
{
    try
    {
        STAFTimestamp test(year, month, day, hour, minute, second);
    }
    catch (STAFTimestampInvalidDateTimeException)
    {
        return false;
    }

    return true;
}


STAF_INLINE STAFString STAFTimestamp::getElapsedTime(unsigned int seconds)
{
    if (seconds != 0)
    {
        return STAFString(getTimeFormat(seconds/60/60)) + ":" + 
            getTimeFormat(seconds/60 % 60) + ":" +
            getTimeFormat(seconds % 60);
    }
    else
    {
        return "00:00:00";
    }
}


STAF_INLINE STAFString STAFTimestamp::asString(const char *format) const
{
    char timeString[18] = { 0 };
    unsigned int osRC = 0;

    STAFRC_t rc = STAFThreadSafeLocalTimeString(timeString,
                  sizeof(timeString), format, fTime, &osRC);

    STAFTimestampException::checkRC(rc, "STAFThreadSafeLocalTimeString", osRC);

    return STAFString(timeString);
}


STAF_INLINE unsigned int STAFTimestamp::operator-(
                         const STAFTimestamp &rhs) const
{
    return (unsigned int)difftime(fTime, rhs.fTime);
}


STAF_INLINE unsigned int STAFTimestamp::asSecondsPastMidnight() const
{
    struct tm theTime = { 0 };
    unsigned int osRC = 0;
    STAFRC_t rc = STAFThreadSafeLocalTime(&theTime, fTime, &osRC);

    STAFTimestampException::checkRC(rc, "STAFThreadSafeLocalTime", osRC);

    return (theTime.tm_hour * 3600) + (theTime.tm_min * 60) + theTime.tm_sec;
}


STAF_INLINE unsigned int STAFTimestamp::getYear() const
{
    struct tm theTime = { 0 };
    unsigned int osRC = 0;
    STAFRC_t rc = STAFThreadSafeLocalTime(&theTime, fTime, &osRC);

    STAFTimestampException::checkRC(rc, "STAFThreadSafeLocalTime", osRC);

    return theTime.tm_year + 1900;
}


STAF_INLINE unsigned int STAFTimestamp::getMonth() const
{
    struct tm theTime = { 0 };
    unsigned int osRC = 0;
    STAFRC_t rc = STAFThreadSafeLocalTime(&theTime, fTime, &osRC);

    STAFTimestampException::checkRC(rc, "STAFThreadSafeLocalTime", osRC);

    return theTime.tm_mon + 1;
}


STAF_INLINE unsigned int STAFTimestamp::getDay() const
{
    struct tm theTime = { 0 };
    unsigned int osRC = 0;
    STAFRC_t rc = STAFThreadSafeLocalTime(&theTime, fTime, &osRC);

    STAFTimestampException::checkRC(rc, "STAFThreadSafeLocalTime", osRC);

    return theTime.tm_mday;
}


STAF_INLINE unsigned int STAFTimestamp::getHour() const
{
    struct tm theTime = { 0 };
    unsigned int osRC = 0;
    STAFRC_t rc = STAFThreadSafeLocalTime(&theTime, fTime, &osRC);

    STAFTimestampException::checkRC(rc, "STAFThreadSafeLocalTime", osRC);

    return theTime.tm_hour;
}


STAF_INLINE unsigned int STAFTimestamp::getMinute() const
{
    struct tm theTime = { 0 };
    unsigned int osRC = 0;
    STAFRC_t rc = STAFThreadSafeLocalTime(&theTime, fTime, &osRC);

    STAFTimestampException::checkRC(rc, "STAFThreadSafeLocalTime", osRC);

    return theTime.tm_min;
}


STAF_INLINE unsigned int STAFTimestamp::getSecond() const
{
    struct tm theTime = { 0 };
    unsigned int osRC = 0;
    STAFRC_t rc = STAFThreadSafeLocalTime(&theTime, fTime, &osRC);

    STAFTimestampException::checkRC(rc, "STAFThreadSafeLocalTime", osRC);

    return theTime.tm_sec;
}


STAF_INLINE bool STAFTimestamp::operator<(const STAFTimestamp &rhs) const
{
    struct tm myTime = { 0 };
    struct tm rhsTime = { 0 };
    unsigned int osRC = 0;
    STAFRC_t rc = STAFThreadSafeLocalTime(&myTime, fTime, &osRC);

    STAFTimestampException::checkRC(rc, "STAFThreadSafeLocalTime", osRC);

    rc = STAFThreadSafeLocalTime(&rhsTime, rhs.fTime, &osRC);

    STAFTimestampException::checkRC(rc, "STAFThreadSafeLocalTime", osRC);

    if (myTime.tm_year < rhsTime.tm_year) return true;
    else if (rhsTime.tm_year < myTime.tm_year) return false;

    if (myTime.tm_mon < rhsTime.tm_mon) return true;
    else if (rhsTime.tm_mon < myTime.tm_mon) return false;

    if (myTime.tm_mday < rhsTime.tm_mday) return true;
    else if (rhsTime.tm_mday < myTime.tm_mday) return false;

    if (myTime.tm_hour < rhsTime.tm_hour) return true;
    else if (rhsTime.tm_hour < myTime.tm_hour) return false;

    if (myTime.tm_min < rhsTime.tm_min) return true;
    else if (rhsTime.tm_min < myTime.tm_min) return false;

    if (myTime.tm_sec < rhsTime.tm_sec) return true;

    return false;
}


STAF_INLINE time_t STAFTimestamp::getImpl() const
{ return fTime; }


STAF_INLINE bool STAFTimestamp::isValidDate(unsigned int year,
                                            unsigned int month,
                                            unsigned int day)
{
    static unsigned int daysInMonth[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30,
                                            31, 30, 31 };

    if (year > 9999) return false;
    if (month > 12) return false;
    if (day > daysInMonth[month - 1]) return false;

    if ((month == 2) && (day == 29))
    {
        if ((year % 400) == 0) return true;
        if ((year % 100) == 0) return false;
        if ((year % 4) == 0) return true;

        return false;
    }

    return true;
}


STAF_INLINE bool STAFTimestamp::isValidTime(unsigned int hour, unsigned int min,
                                            unsigned int sec)
{
    if (hour > 23) return false;
    if (min > 59) return false;
    if (sec > 59) return false;

    return true;
}


STAF_INLINE STAFString STAFTimestamp::getTimeFormat(unsigned int input)
{
    STAFString in = STAFString(input);

    if (in.length() == 1)
        return "0" + in;
    else
        return in;
}


// Begin definitions for STAFRelativeTime

STAF_INLINE STAFRelativeTime::STAFRelativeTime()
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFTimestampGetRelativeTime(&fRelTime, &osRC);

    STAFTimestampException::checkRC(rc, "STAFTimestampGetRelativeTime", osRC);
}


STAF_INLINE unsigned int STAFRelativeTime::operator-(const STAFRelativeTime &rhs)
{
    unsigned int diffInMillis = 0;
    STAFRC_t rc = STAFTimestampGetRelativeTimeDifference(fRelTime, rhs.fRelTime,
                                                         &diffInMillis);

    STAFTimestampException::checkRC(rc, "STAFTimestampGetRelativeTimeDifference",
                                    0);

    return diffInMillis;
}


STAF_INLINE STAFRelativeTime::~STAFRelativeTime()
{
    STAFTimestampFreeRelativeTime(&fRelTime);
}

#endif
