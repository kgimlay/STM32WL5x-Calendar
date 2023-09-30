/*
 * Author:  Kevin Imlay
 * Date:  September, 2023
 *
 * Purpose:
 *		RTC Calendar Control provides functions to the calendar module for setting
 *	and getting RTC features.  This includes the RTC's date and time, and the RTC's
 *	alarms.
 */


#ifndef RTC_UTILS_H
#define RTC_UTILS_H


#include "stm32wlxx_hal.h"
#include <stdbool.h>

/*
 * Status returns for RTC.
 */
typedef enum {
  RTC_CALENDAR_CONTROL_OKAY = 0,
  RTC_CALENDAR_CONTROL_NOT_INIT,
  RTC_CALENDAR_CONTROL_TIMEOUT,
  RTC_CALENDAR_CONTROL_ERROR
} RtcUtilsStatus;

/* rtcCalendarControl_init
 *
 * Function:
 *	Initializes the module.
 *
 * Parameters:
 *	hrtc - a RTC_HandleTypeDef pointer to a HAL RTC handle
 *
 * Return:
 *	RtcUtilsStatus
 *		RTC_CALENDAR_CONTROL_OKAY - if successful
 *		RTC_CALENDAR_CONTROL_NOT_INIT - if a NULL pointer or an uninitialized
 *				handle was passed.
 */
RtcUtilsStatus rtcCalendarControl_init(RTC_HandleTypeDef* const hrtc);

/* rtcCalendarControl_setDateTime
 *
 * Function:
 *	Set the date and time of the RTC.
 *
 * Parameters:
 *	year - two digit 21st century year 		(0 - 99)
 *	month - two digit month 				(1 - 12)
 *	day - two digit day of month 			(1 - 28/29/30/31)
 *	hour - two digit hour in 24 hour format (0 - 23)
 *	minute - two digit minute 				(0 - 59)
 *	second - two digit second 				(0 - 59)
 *
 * Return:
 *	RtcUtilsStatus
 *		RTC_CALENDAR_CONTROL_NOT_INIT - if module has not been initialized
 *		RTC_CALENDAR_CONTROL_TIMEOUT - if the RTC is unresponsive
 *		RTC_CALENDAR_CONTROL_OKAY - otherwise
 *
 * Note:
 *	There is no error checking for inputs outside of valid range and behavior
 *	undefined if so.
 */
RtcUtilsStatus rtcCalendarControl_setDateTime(const uint8_t year, const uint8_t month,
		const uint8_t day, const uint8_t hour, const uint8_t minute,
		const uint8_t second);

/* rtcCalendarControl_getDateTime
 *
 * Function:
 *	Get the date and time of the RTC.
 *
 * Parameters:
 *	Pointers to store copy of date and time.  Will set values to:
 *	year - two digit 21st century year 		(0 - 99)
 *	month - two digit month 				(1 - 12)
 *	day - two digit day of month 			(1 - 28/29/30/31)
 *	hour - two digit hour in 24 hour format (0 - 23)
 *	minute - two digit minute 				(0 - 59)
 *	second - two digit second 				(0 - 59)
 *
 * Return:
 *	RtcUtilsStatus
 *		RTC_CALENDAR_CONTROL_NOT_INIT - if module has not been initialized
 *		RTC_CALENDAR_CONTROL_OKAY - otherwise
 */
RtcUtilsStatus rtcCalendarControl_getDateTime(uint8_t* const year, uint8_t* const month,
		uint8_t* const day, uint8_t* const hour, uint8_t* const minute,
		uint8_t* const second);

/* rtcCalendarControl_setAlarm_A
 *
 * Function:
 *	Set the time and day of the month for Alarm A to fire.
 *
 * Parameters:
 *	day - two digit day of month 			(1 - 28/29/30/31)
 *	hour - two digit hour in 24 hour format (0 - 23)
 *	minute - two digit minute 				(0 - 59)
 *	second - two digit second 				(0 - 59)
 *
 * Return:
 *	RtcUtilsStatus
 *		RTC_CALENDAR_CONTROL_NOT_INIT - if module has not been initialized
 *		RTC_CALENDAR_CONTROL_OKAY - otherwise
 */
RtcUtilsStatus rtcCalendarControl_setAlarm_A(const uint8_t day, const uint8_t hour,
		const uint8_t minute, const uint8_t second);

/* rtcCalendarControl_getAlarm_A
 *
 * Function:
 *	Get the current alarm values from Alarm A.
 *
 * Parameters:
 *	Pointers to store copy of date and time.  Will set values to:
 *	day - two digit day of month 			(1 - 28/29/30/31)
 *	hour - two digit hour in 24 hour format (0 - 23)
 *	minute - two digit minute 				(0 - 59)
 *	second - two digit second 				(0 - 59)
 *
 * Return:
 *	RtcUtilsStatus
 *		RTC_CALENDAR_CONTROL_NOT_INIT - if module has not been initialized
 *		RTC_CALENDAR_CONTROL_OKAY - otherwise
 *
 * Note:
 *	Getting the alarm A date/time does not distinguish if the alarm is enabled
 *	or disabled.
 */
RtcUtilsStatus rtcCalendarControl_getAlarm_A(uint8_t* const year, uint8_t* const month,
		uint8_t* const day, uint8_t* const hour, uint8_t* const minute,
		uint8_t* const second);

/* rtcCalendarControl_diableAlarm_A
 *
 * Function:
 *	Disables Alarm A.
 *
 * Return:
 *	RtcUtilsStatus
 *		RTC_CALENDAR_CONTROL_NOT_INIT - if module has not been initialized
 *		RTC_CALENDAR_CONTROL_OKAY - otherwise
 */
RtcUtilsStatus rtcCalendarControl_diableAlarm_A(void);


#endif
