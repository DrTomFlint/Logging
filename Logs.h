/**
 * @file Logs.h
 * @brief Event-Code and Fault-Code definitions.
 *
 *
 */

#ifndef LOGS_H_
#define LOGS_H_

// Definitions for event codes
#define E_START 1	/**< CAN Start Command */
#define E_STOP 2	/**< CAN Stop Command */
#define E_RESET 3	/**< CAN Reset Faults Command */
#define E_FORCE 4	/**< CAN Force Fault Command */
#define E_STATE 5	/**< State Changed  */
#define E_PARAM 6	/**< Set Parameter */
#define E_FAULT 7	/**< Fault */
#define E_DATALOG 8 	/**< Realtime Datalog triggering */
#define E_SETPOINT 9	/**< Speed setpoint changed */
#define E_FLASH 10		/**< Load, Save, Default Params */
#define E_CANBAD 11		/**< CANbus error occurred */

#if(0)
// Definitions for fault codes, use powers of 2 for bit bashing
#define F_STATE 		1L		/**< Invalid State */
#define F_OVERCURRENT 	2L		/**< Over Current */
#define F_OVERSPEED		4L		/**< Over Speed */
#define F_OVERTEMP		8L		/**< Over Temperature */
#define F_OVERVOLT		16L		/**< DC Over Voltage */
#define F_CHECKSUM		32L		/**< Flash Parameter Checksum Error */
#define F_WDOG			64L		/**< Watchdog Timeout */
#define F_GROUND		128L	/**< Ground Fault */
#define F_ENCODER		256L	/**< Encoder Fault */
#define F_RESOLVER		512L	/**< Resolver Fault */
#define F_UNDERVOLT		1024L	/**< DC Under Voltage */
#define F_UVLO			2048L	/**< Under voltage lockout from gate driver */
#define F_CANBUS		4096L	/**< CANbus fault, 1=timeout */
#define F_VOLTBALANCE	8192L	/**< DC bus voltage balance */
#define F_OVERRUN		16384L	/**< Main ISR overrun */
#define F_SPEED			32768L  /**< Speed Error */
#else
// Definitions for fault code, when too many codes to assign bitwise
#define F_STATE 		0		/**< Invalid State */
#define F_OVERCURRENT 	1		/**< Over Current */
#define F_OVERSPEED		2		/**< Over Speed */
#define F_OVERTEMP		3		/**< Over Temperature */
#define F_OVERVOLT		4		/**< DC Over Voltage */
#define F_CHECKSUM		5		/**< Flash Parameter Checksum Error */
#define F_WDOG			6		/**< Watchdog Timeout */
#define F_GROUND		7		/**< Ground Fault */
#define F_ENCODER		8		/**< Encoder Fault */
#define F_RESOLVER		9   	/**< Resolver Fault */
#define F_UNDERVOLT		10		/**< DC Under Voltage */
#define F_UVLO			11		/**< Under voltage lockout from gate driver */
#define F_CANBUS		12		/**< CANbus fault, 1=timeout */
#define F_VOLTBALANCE	13		/**< DC bus voltage balance */
#define F_OVERRUN		14		/**< Main ISR overrun */
#define F_SPEED			15  	/**< Speed Error */
#define F_STALL			16  	/**< Stall Protection */
#endif

#endif /* LOGS_H_ */
