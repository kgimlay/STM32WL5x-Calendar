/*
 * Author:  Kevin Imlay
 * Date:  September, 2023
 *
 * Purpose:
 *		The Calendar module encapsulates the functionalities of scheduling
 *	events based on a calendar that runs in real time.  The calendar
 *	consists of two parts: the clock and the calendar.  The clock keeps track
 *	of real time using the RTC hardware peripheral as well as sets alarms on
 *	the RTC for managing event start and end times.  The calendar holds events
 *	start/end times and callback functions, and interacts with the clock to
 *	enter and exit events at their scheduled times.  Callback functions are
 *	provided for the an event's start and an event's end.
 *		The calendar is only capable of scheduling non-overlapping events.  If
 *	two or more events overlap, the event that has the sooner start time, or
 *	was added to the calendar first, will take precedence.  Once that event
 *	ends, the next event in the overlap that is still in progress will take
 *	precedence.  This means that events may not run for their full length or
 *	not run at all depending on how they overlap.
 *		The calendar stores events in statically-allocated memory at compilation
 *	time.  Increasing the maximum number of events at run time is not possible.
 */

#ifndef INC_CALENDAR_H_
#define INC_CALENDAR_H_


#include <stdint.h>
#include <stm32wlxx_hal.h>
#include <stdbool.h>
#include <event_sll.h>

/*
 * Return status codes for the calendar module.
 */
typedef enum {
	CALENDAR_OKAY = 0,
	CALENDAR_PARAMETER_ERROR,
	CALENDAR_NOT_INIT,
	CALENDAR_FULL,
	CALENDAR_PAUSED,
	CALENDAR_RUNNING
} CalendarStatus;

/* calendar_init
 *
 * Function:
 *	Initializes the calendar module.  Must be called before the module
 *	can operate.
 *
 * Parameters:
 *	hrtc - pointer to RTC_HandleTypeDef (HAL) handle of the RTC peripheral
 *			to be used.
 *
 * Return:
 *	CalendarStatus
 *		CALENDAR_OKAY - if a pointer to an initialized HAL RTC handle was
 *				passed
 *		CALENDAR_PARAMETER_ERROR - otherwise
 *
 * Note:
 * 	Will not reinitialize if the module is already initialized.
 */
CalendarStatus calendar_init(RTC_HandleTypeDef* hrtc);

/* calendar_resetCalendar
 *
 * Function:
 * 	Resets the event queue, clearing all events and stopping the calendar
 * 	if running.
 *
 * Return:
 * 	CalendarStatus
 * 		CALENDAR_NOT_INIT - if the calendar module has not been initialized
 * 		CALENDAR_OKAY - if successful
 */
CalendarStatus calendar_resetEvents(void);

/* calendar_startScheduler
 *
 * Function:
 *	Starts execution of calendar events.  Will not start if the module has not
 *	been initialized.
 *
 * Return:
 *	CalendarStatus
 *		CALENDAR_NOT_INIT - if the calendar module hasn't been initialized
 *		CALENDAR_RUNNING - if the calendar is already running (not an error)
 *		CALENDAR_OKAY - if the calendar was started
 *
 * Note:
 * 	Starting the calendar is still successful if there are no events in the queue
 * 	or if all events ended prior to the current RTC date and time.
 */
CalendarStatus calendar_startScheduler(void);

/* calendar_pauseScheduler
 *
 * Function:
 *	Pauses execution of calendar events.  Will not start if the module has not
 *	been initialized.
 *
 * Return:
 *	CalendarStatus
 *		CALENDAR_NOT_INIT - if the calendar module hasn't been initialized
 *		CALENDAR_PAUSED - if the calendar is already paused (not an error)
 *		CALENDAR_OKAY - if the calendar was paused
 *
 * Note:
 * 	Pausing the calendar is still successful if there are no events in the queue
 * 	or if the calendar is not within any events.  Pausing within an event will
 * 	delay the end event callback function execution until the calendar is unpaused
 * 	with calendar_start().  Events that would have started and completed while
 * 	paused are skipped entirely.
 */
CalendarStatus calendar_pauseScheduler(void);

/* calendar_setDateTime
 *
 * Function:
 *	Set the date and time of the RTC.
 *
 * Parameters:
 *	dateTime - the time and date to set the RTC to.
 *
 * Return:
 * 	CalendarStatus
 *		CALENDAR_NOT_INIT - if the calendar module hasn't been initialized
 *		CALENDAR_RUNNING - if the calendar is not paused
 *		CALENDAR_OKAY - if the calendar's date and time were set
 *
 * Note:
 * 	Only sets time and date if the module has been initialized and has been paused.
 */
CalendarStatus calendar_setDateTime(const DateTime dateTime);

/* calendar_getDateTime
 *
 * Function:
 *	Get the date and time of the RTC.
 *
 * Parameters:
 *	dateTime - pointer to a DateTime as a destination.
 *
 * Return:
 * 	CalendarStatus
 *		CALENDAR_NOT_INIT - if the calendar module hasn't been initialized
 *		CALENDAR_OKAY - if the calendar's date and time were read
 *
 * Note:
 * 	Only gets time and date if the module has been initialized.  Can get the time
 * 	and date regardless of if the calendar is running or paused.
 */
CalendarStatus calendar_getDateTime(DateTime* const dateTime);

/* calendar_addEvent
 *
 * Function:
 *	Add a calendar event to the calendar.
 *
 * Parameters:
 *	event - pointer to CalendarEvent to copy event details from.
 *
 * Return:
 *	CalendarStatus
 *		CALENDAR_NOT_INIT - if the calendar module hasn't been initialized
 *		CALENDAR_FULL - if the calendar's queue is full
 *		CALENDAR_RUNNING - if the calendar is not paused
 *		CALENDAR_OKAY - if the event was successfully added
 */
CalendarStatus calendar_addEvent(const struct CalendarEvent event);

/* calendar_peekEvent
 *
 * Function:
 *	Get the contents of a calendar event from the calendar.
 *
 * Parameters:
 *	id - the index of the calendar event to look at.
 *	event - todo
 *
 * Return:
 *	CALENDAR_NOT_INITIALIZED - if the module has not been initialized
 *	CALENDAR_PARAMETER_ERROR - if the index goes beyond the end of the occupied list of
 *		events, or if the index is greater than the max number of events allowed
 *	CALENDAR_OKAY - if successful
 *
 * Note:
 * 	Point for future development.
 */
CalendarStatus calendar_peekEvent(unsigned int id, CalendarEvent* const event);

/* calendar_removeEvent
 *
 * Function:
 *	Remove a calendar event from the calendar.
 *
 * Parameters:
 *	id - the index of the calendar event to remove at.
 *
 * Return:
 *	CALENDAR_NOT_INITIALIZED - if the module has not been initialized
 *	CALENDAR_PARAMETER_ERROR - if the index goes beyond the end of the occupied list of
 *		events, or if the index is greater than the max number of events allowed
 *	CALENDAR_RUNNING - if the calendar is not paused
 *	CALENDAR_OKAY - if successful
 */
CalendarStatus calendar_removeEvent(unsigned int id);

/* calendar_update
 *
 * Function:
 *	Performs an update of the current event if an event has begun or ended.  Does
 *	not update if the calendar is paused.
 *
 * Return:
 *	CALENDAR_NOT_INITIALIZED - if the module has not been initialized
 *	CALENDAR_PAUSED - if the calendar is currently paused
 *	CALENDAR_OKAY - otherwise (does not distinguish if any events began/ended.
 *
 * Note:
 * 	Updates to the calendar, and consequently the callback functions registered for
 * 	each event, only occur as often as this function is called.  A result of this is
 * 	that the execution of callback functions for starting or ending an event may be
 * 	delayed for some time after the event actually began/ended.  This is up to the
 * 	application to determine response time.
 */
CalendarStatus calendar_updateScheduler(void);

/* calendar_AlarmA_ISR
 *
 * Function:
 *	Sets a flag to signal to the calendar_update() function that an event has either
 *	began or ended.
 *
 * Note:
 * 	Call only within the RTC Alarm A ISR.  Otherwise the behavior is undefined.
 */
void calendar_AlarmA_ISR(void);


#endif /* INC_CALENDAR_H_ */
