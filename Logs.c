/**
 * @file Logs.c
 * @brief Event log and real-time data log.
 *
 * An event log circular buffer that includes an event code, timestamp, and an
 * optional floating point data item.  This is held in RAM.
 *
 * Real-time data log divides up a block of RAM into 1 to 8 channels, there are
 * data pointers for each channel to specify which signals should be recorded.
 * Triggering options include single-shot, circular buffer, pre and post trigger.
 * Skip will allow longer data records by skipping over a number of samples
 * between recording.
 *
 * LogBuf is placed in RAML6 which is a 4k block, allows up to 2048 floats.
 * DO NOT USE THE FULL SPACE.  '335 DSP HAS A KNOWN BUG THAT CAN LOCK IT UP IF
 * YOU READ OR WRITE TO THE VERY END OF A MEMORY BLOCK.
 * Starting address is 0xE000.
 *
 * This code is for TI 28335 DSP.  Adapt it for other platforms.
 */

#include "Setup.h"     // DSP2833x Headerfile Include File

#pragma SET_DATA_SECTION("Logs")	// start of "Logs" data section
float LogBuf[LOG_SIZE];				/**< Log, buffer of float data */
#pragma SET_DATA_SECTION()			// end of "Logs" data section

// Data log initial values set to capture data from any faults
int LogChan = 1;			/**< Log, number of channels to record */
int LogLength = LOG_SIZE;	/**< Log, length of each record */
float * LogBase[LOG_CHAN];	/**< Log, base address within LogBuf for each channel */
int LogCount = 0;			/**< Log, index into each channel, number of samples recorded */
int LogSingle = 0;			/**< Log, 0 for circular buffer, 1 for single-shot */
float * LogPtr[LOG_CHAN];	/**< Log, pointer to signal to be recorded per channel */
int LogSkip = 0;			/**< Log, number of samples to skip when recording */
int LogTrigger = 0;			/**< Log, 0=not recording, 1=recording, 2=reset, negative=record and increment till zero stops */
int OldTrigger = 0;			/**< Log, trigger value from prior pass */
int LogInit = 0;			/**< Log, set non-zero to initialize N channels in the log, resets to 0 when done */
int LogSkipCount = 0;		/**< Log, counter for skipping samples */
int LogAddr0;				/**< Log, integer addresses for the data */
int LogAddr1;				/**< Log, integer addresses for the data */
int LogAddr2;				/**< Log, integer addresses for the data */
int LogAddr3;				/**< Log, integer addresses for the data */
int LogAddr4;				/**< Log, integer addresses for the data */
int LogAddr5;				/**< Log, integer addresses for the data */
int LogAddr6;				/**< Log, integer addresses for the data */
int LogAddr7;				/**< Log, integer addresses for the data */
int LogAddr8;				/**< Log, integer addresses for the data */
int LogAuto;				/**< Log, automatic triggering options */

#pragma SET_DATA_SECTION("Events")	// start of "Events" data section
long EventTime1[EVENT_SIZE];		/**< Events, timestamp part 1 */
long EventTime2[EVENT_SIZE];		/**< Events, timestamp part 2 */
int EventCode[EVENT_SIZE];		/**< Events, code defined in Logs.h */
int EventData1[EVENT_SIZE];		/**< Events, optional integer argument */
float EventData2[EVENT_SIZE];	/**< Events, optional floating point  */
int EventIndex = 0;				/**< Events, index pointing to next slot */
int EventSize = EVENT_SIZE;		/**< Events, EVENT_SIZE in ram to be CANbus readable */
#pragma SET_DATA_SECTION()			// end of "Logs" data section

long FaultWord = 0L;

/** Assert a fault and log it
 *  fcode is the fault code defined in Logs.h
 *  data2 is a floating point optional argument
 *  that will get recorded in the EventLog */
void Fault(long fcode, float data2){

	// First thing, disable the PWM outputs
	PWM_disable();

	// Check if this is a new fault of this type
	if((FaultWord & (0x01<<fcode) )==0L){
		// This is a new so log it
		LogEvent(E_FAULT,fcode,data2);
	}

	// Always set a bit in the fault word
	FaultWord = FaultWord | (0x01<<fcode) ;

	// Check if just arrived in fault state
	if(mainState!=FAULT){
		// Just entered fault state
		mainState=FAULT;
		LogEvent(E_STATE,FAULT,0.0);
	}

	// If auto triggering == 1 turn off data logging
	// to save the data from this fault
	if(LogAuto==1){
		LogTrigger = 0;
	}

	// Reset the speed reference to zero
	WeRef = 0;


}

/** Reset all the faults, they may be re-asserted
 * right away if the fault condition still exists
 */
void ResetFaults(void){

	FaultWord = 0L;
	LogEvent(E_RESET,0,0);
	if(mainState==FAULT){
		LogEvent(E_STATE,READY,0);
		mainState = READY;
	}
	// Toggle on the fault reset line on Flag2
	// will be turned off when FaultResetCount is decremented
	// in the main ISR
	FLAG2ON;
	FaultResetCount = 20;
}

/** Initialize the event log */
void InitEvents(void){

	int i;

	for(i=0;i<EVENT_SIZE;i++){
		EventTime1[i] = 0L;
		EventTime2[i] = 0L;
		EventCode[i] = 0;
		EventData1[i] = 0;
		EventData2[i] = 0.0;
	}
	EventIndex = 0;
}

/** Add an event to the log with some optional data */
void LogEvent(int Code, int Data1, float Data2){

	long i1;
	long i2;
	TimeStamp(&i1,&i2);

	EventTime1[EventIndex] = i1;
	EventTime2[EventIndex] = i2;
	EventCode[EventIndex] = Code;
	EventData1[EventIndex] = Data1;
	EventData2[EventIndex] = Data2;
	EventIndex++;
	if(EventIndex==EVENT_SIZE) EventIndex = 0;

}

/** Setup default data logging */
void DefaultLog(int i){

	switch(i){
	case 1:
		LogAddr0 = (int)&IdRef;
		LogAddr1 = (int)&IqRef;
		LogAddr2 = (int)&RpmRef;
		LogAddr3 = (int)&Id;
		LogAddr4 = (int)&Iq;
		LogAddr5 = (int)&RpmOut;
		LogAddr6 = (int)&VdRef;
		LogAddr7 = (int)&VqRef;
		LogAddr8 = (int)&ThetaOut;
		LogChan = 9;
		LogSingle = 0;
		LogSkip = 20;
		LogAuto = 1;
		LogInit = 1;
		break;

	}

}

/** Initialize the realtime datalog */
#pragma CODE_SECTION(InitLog, "ramfuncs")
void InitLog(void){

	int i;

	LogTrigger = 0;
	LogLength = LOG_SIZE / LogChan;
	for(i=0;i<LogChan;i++){
		LogBase[i] = LogBuf + i*LogLength;
	}
	LogInit=0;
	LogCount=0;
	LogSkipCount=0;
	LogPtr[0]=(float *)(LogAddr0&0x0000FFFF);
	LogPtr[1]=(float *)(LogAddr1&0x0000FFFF);
	LogPtr[2]=(float *)(LogAddr2&0x0000FFFF);
	LogPtr[3]=(float *)(LogAddr3&0x0000FFFF);
	LogPtr[4]=(float *)(LogAddr4&0x0000FFFF);
	LogPtr[5]=(float *)(LogAddr5&0x0000FFFF);
	LogPtr[6]=(float *)(LogAddr6&0x0000FFFF);
	LogPtr[7]=(float *)(LogAddr7&0x0000FFFF);
	LogPtr[8]=(float *)(LogAddr8&0x0000FFFF);
}

	/** Update the log state, includes resets and triggering */
#pragma CODE_SECTION(UpdateLog, "ramfuncs")
void UpdateLog(void){

	int i;

	// Make eventlog entry if trigger has changed
	if(LogTrigger!=OldTrigger){
		LogEvent(E_DATALOG,LogTrigger,LogSkip);
	}
	OldTrigger=LogTrigger;

	// Initialize a new number of channels
	if(LogInit!=0) InitLog();

	// Record data when LogTrigger is non-zero
	if(LogTrigger != 0){
		if(LogSkipCount<LogSkip){
			LogSkipCount++;
		}else{
			LogSkipCount=0;
			// Record data
			for(i=0;i<LogChan;i++){
				LogBase[i][LogCount] = *(LogPtr[i]);
			}
			// Increment data count
			LogCount++;
			// Check for end of buffer
			if(LogCount==LogLength){
				if(LogSingle==1){			// if single-shot turn off trigger
					LogTrigger=0;
				}
				LogCount=0;					// next sample at start of buffer
			}
			// Setting LogTrigger to a negative number will take data until it
			// is incremented to zero
			if(LogTrigger<0){
				LogTrigger++;
			}
		}
	}

}

