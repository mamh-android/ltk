#ifndef JOB__DATA__H
#define JOB__DATA__H
//job element and task is a map
enum JobDataState {
	kUnknown =0,
	kRunning =1,
	kPending =2,
	kFinished =3
};
struct JobData {
	JobData()
	{ /*do nothing*/
	}

	JobData(const STAFString &taskName, const STAFString   &ID, const STAFString &board_id, const JobDataState &state) :
		taskName(taskName), ID(ID), board_id(board_id), state(state), accessSem(new STAFMutexSem(), STAFMutexSemPtr::INIT)
	{
		JobName=taskName + "_" + ID;
	}

	STAFString JobName;                     //jobname={BOARD_ID}-{TimeStamp}
	STAFString statusUpdateTime;            // status update Time
	STAFString statusAll;                   //record all the stauts info send from rcc_utilc
	STAFString status;                      //pass,fail,running
	STAFString output;                      //log output
	JobDataState state;                     //running,pending,finished
	STAFString ID;                          //auto generated identification of deviceData
	STAFString board_id;                    // Device board_id name
	STAFString ipaddr;                      // lanuched ipaddr
	STAFString taskName;                    // task name
	STAFString caseName;                    // launched case name
	STAFString startTime;                   // job startTime
	STAFMutexSemPtr accessSem;              // Semaphore to control access to JobData
};

typedef STAFRefPtr<JobData> JobDataPtr;
typedef std::vector<JobDataPtr> JobList;
//////////////////////////////////////////////////////////////////////
//end of the respool + job queue ////////////////////////////////////
//////////////////////////////////////////////////////////////////


#endif
