/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_Timing
#define STAF_Timing

// Warning: These timing routines are only valid on Unix systems.

#ifndef STAF_DO_TIMING

#define INIT_TIMES()
#define RECORD_TIME(theName)
#define OUTPUT_TIMES()

#else

#ifndef MAXTIMES
#define MAXTIMES 25
#endif

static struct
{
    struct timeval time;
    char *name;
} sTheTimes[MAXTIMES];

static unsigned int sTotalTimes = 0;

#define INIT_TIMES() sTotalTimes = 0;

#define RECORD_TIME(theName) \
{\
    gettimeofday(&sTheTimes[sTotalTimes].time, 0);\
    sTheTimes[sTotalTimes++].name = theName;\
}

#define OUTPUT_TIMES() \
cout << endl;\
for (int t_i = 0; t_i < sTotalTimes; ++t_i) \
{\
    cout << t_i << ": " << sTheTimes[t_i].time.tv_sec << "." << \
                           sTheTimes[t_i].time.tv_usec;\
    if (t_i != sTotalTimes - 1) cout << " delta: " \
                       << (sTheTimes[t_i + 1].time.tv_usec - \
                           sTheTimes[t_i].time.tv_usec);\
    else cout << " delta: 000";\
    cout << " name: " << sTheTimes[t_i].name << endl;\
}\
cout << "Total time: " << \
     (sTheTimes[sTotalTimes - 1].time.tv_usec - sTheTimes[0].time.tv_usec) \
     << endl;
#endif

#endif
