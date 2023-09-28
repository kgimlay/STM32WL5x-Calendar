/*
 * calendar.c
 *
 *  Created on: Jul 7, 2023
 *      Author: kevinimlay
 */


#include <rtc_calendar_control.h>
#include <calendar.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


/*
 * Private function prototypes.
 */
void _update(void);


/*
 * Static operational variables used across function calls within this
 * module.
 */
static bool _isInit = false;		// signals if the module has been initialized
static bool _isRunning = false;		// signals if the calendar is running
volatile static bool _alarmAFired = false;	// signals if Alarm A has fired, need to update calendar
static Event_SLL _eventQueue;		// queue of events to execute on the calendar


/* calendar_init
 *
 * Initialize the RTC Calendar Control and reset operational variables for
 * this module.
 *
 * Note: will not reinitialize/reset if already initialized.
 */
CalendarStatus calendar_init(RTC_HandleTypeDef* hrtc)
{
	// check for pointer to initialized RTC handle
	if (hrtc != NULL && hrtc->Instance != NULL)
	{
		// initialize only if not already initialized
		if (!_isInit)
		{
			// pass pointer to alarm control
			rtcCalendarControl_init(hrtc);

			// initialize the calendar
			eventSLL_reset(&_eventQueue);

			// set init flag
			_isInit = true;
		}

		return CALENDAR_OKAY;
	}

	// module already initialized
	else
	{
		return CALENDAR_PARAMETER_ERROR;
	}
}


/* calendar_resetEvents
 *
 * Reset the events linked list.
 */
CalendarStatus calendar_resetEvents(void)
{
	// if the module is initialized
	if (_isInit)
	{
		eventSLL_reset(&_eventQueue);

		return CALENDAR_OKAY;
	}

	// module has not been initialized
	else
	{
		return CALENDAR_NOT_INIT;
	}
}


/* calendar_start
 *
 * Starts execution of events.  Sets flag to signal if currently within an event
 * and sets RTC Alarm A for the next event transition (start or end of event).
 * Executes start of event callback if starting within an event.  Ignores past
 * events.
 */
CalendarStatus calendar_startScheduler(void)
{
	// if the module has been initialized
	if (_isInit)
	{
		// only start if the calendar has been paused
		if (!_isRunning)
		{
			_update();

			// set is running flag
			_isRunning = true;

			return CALENDAR_OKAY;
		}

		// report that the calendar is already running
		else
		{
			return CALENDAR_RUNNING;
		}
	}

	// module is not initialized
	else
	{
		return CALENDAR_NOT_INIT;
	}
}


/* calendar_pause
 *
 * Pauses execution of the calendar without disabling RTC Alarm A.  This is done
 * so that if pausing within an event, the end of event callback will execute while
 * restarting the calendar.
 */
CalendarStatus calendar_pauseScheduler(void)
{
	// if module has been initialized
	if (_isInit)
	{
		// only pause if module is running
		if (_isRunning)
		{
			_isRunning = false;

			return CALENDAR_OKAY;
		}

		// report that the calendar is already paused
		else
		{
			return CALENDAR_PAUSED;
		}
	}

	// module is not initialized
	else
	{
		return CALENDAR_NOT_INIT;
	}
}


/* calendar_setDateTime
 *
 * Set the date/time within the RTC.
 */
CalendarStatus calendar_setDateTime(const DateTime dateTime)
{
	// if the module has been initialized
	if (_isInit)
	{
		// if the calendar is paused
		if (!_isRunning)
		{
			// set the date and time in the RTC
			rtcCalendarControl_setDateTime(dateTime.year, dateTime.month, dateTime.day,
					dateTime.hour, dateTime.minute, dateTime.second);

			return CALENDAR_OKAY;
		}

		else
		{
			return CALENDAR_RUNNING;
		}
	}

	// if the module has not been initialized
	else
	{
		return CALENDAR_NOT_INIT;
	}
}


/* calendar_getDateTime
 *
 * Get the date/time within the RTC.
 */
CalendarStatus calendar_getDateTime(DateTime* const dateTime)
{
	// if the module is initialized
	if (_isInit)
	{
		// get the date and time in the RTC
		rtcCalendarControl_getDateTime(&(dateTime->year), &(dateTime->month),
				&(dateTime->day), &(dateTime->hour), &(dateTime->minute),
				&(dateTime->second));

		return CALENDAR_OKAY;
	}

	// the module has not been initialized
	else
	{
		return CALENDAR_NOT_INIT;
	}
}


/* calendar_addEvent
 *
 * Add an event to the calendar's event linked list.
 */
CalendarStatus calendar_addEvent(const struct CalendarEvent event)
{
	// add only if the calendar has been initialized
	if (_isInit)
	{
		// if the calendar is paused
		if (!_isRunning)
		{
			// attempt to add event and report success/failure
			if (eventSLL_insert(&_eventQueue, event))
			{
				return CALENDAR_OKAY;
			}
			else
			{
				return CALENDAR_FULL;
			}
		}

		else
		{
			return CALENDAR_RUNNING;
		}
	}

	// the calendar has not been initialized
	else
	{
		return CALENDAR_NOT_INIT;
	}
}


/* calendar_peekEvent
 *
 * Gets info on the event at the provided index within the linked list.
 * Index from iterating across linkages, not index of array.
 */
CalendarStatus calendar_peekEvent(unsigned int id, CalendarEvent* const event)
{
	// if the calendar module has been initialized
	if (_isInit)
	{
		if (eventSLL_peekIdx(&_eventQueue, id, event))
		{
			return CALENDAR_OKAY;
		}

		else
		{
			return CALENDAR_PARAMETER_ERROR;
		}
	}

	// the module is not initialized
	else
	{
		return CALENDAR_NOT_INIT;
	}
}


/* calendar_removeEvent
 *
 * Removes the event at the provided index within the linked list.
 * Index from iterating across linkages, not index of array.
 */
CalendarStatus calendar_removeEvent(unsigned int id)
{
	// if the calendar module has been initialized
	if (_isInit)
	{
		// if the calendar is paused
		if (!_isRunning)
		{
			if (eventSLL_remove(&_eventQueue, id))
			{
				return CALENDAR_OKAY;
			}

			else
			{
			return CALENDAR_PARAMETER_ERROR;
			}
		}

		else
		{
			return CALENDAR_RUNNING;
		}
	}

	else
	{
		return CALENDAR_NOT_INIT;
	}
}


/* calendar_update
 *
 * Update loop.  Updates the state of the calendar (changing events) if the
 * RTC Alarm A has fired to signal an event change.
 *
 * Dependency on _update().
 *
 * Note:
 * 	Will not run if the module has not been initialized and if the calendar
 * 	is not running.
 */
CalendarStatus calendar_updateScheduler(void)
{
	// if the calendar module has been initialized
	if (_isInit)
	{
		// only update if the calendar is running
		if (_isRunning)
		{
			// only update if an alarm has fired
			if (_alarmAFired) {
				// update the calendar's state
				_update();

				// reset alarm fired flag
				_alarmAFired = false;
			}

			return CALENDAR_OKAY;
		}

		// report that the calendar is paused
		else
		{
			return CALENDAR_PAUSED;
		}
	}

	// the module is not initialized
	else
	{
		return CALENDAR_NOT_INIT;
	}
}


/* calendar_AlarmA_ISR
 *
 * RTC Alarm A interrupt service routine.  To only be called within the
 * RTC Alarm A ISR (HAL_RTC_AlarmAEventCallback()).
 */
void calendar_AlarmA_ISR(void)
{
	// set flag that an alarm fired
	_alarmAFired = true;
}


/* _update
 *
 * Update loop for module.  If an alarm to signal an event start/end has fired,
 * then this loop will call the callback functions for ending and starting events
 * appropriately.
 *
 * Also handles reseting the alarm for events that occur in a following month/year.
 */
void _update(void)
{
	DateTime nextAlarm;
	DateTime now;
	int prevInProgress;

	// get calendar alarm for next alarm in event list relative to now
	rtcCalendarControl_getDateTime(&(now.year), &(now.month), &(now.day),
			&(now.hour), &(now.minute), &(now.second));

	// store the currently running event to test index to check if an
	// event change has occurred
	prevInProgress = _eventQueue.inProgress;

	// if there is an alarm to set upon updating the events queue
	if (eventSLL_getNextAlarm(&_eventQueue, now, &nextAlarm))
	{
		// set Alarm A
		rtcCalendarControl_setAlarm_A(nextAlarm.day, nextAlarm.hour,
				nextAlarm.minute, nextAlarm.second);
	}

	// if there is no alarm to set, disable the alarm
	else
	{
		rtcCalendarControl_diableAlarm_A();
	}

	// if exiting an event
	if (_eventQueue.inProgress != prevInProgress
			&& prevInProgress != EVENTS_SLL_NO_EVENT)
	{
		// call end event callback for exited event (if registered)
		if (_eventQueue.events[prevInProgress].event.end_callback != NULL)
			(*_eventQueue.events[prevInProgress].event.end_callback)();
	}

	// if entering an event
	if (_eventQueue.inProgress != EVENTS_SLL_NO_EVENT
			&& _eventQueue.inProgress != prevInProgress)
	{
		// call start event callback for entered event (if registered)
		if (_eventQueue.events[_eventQueue.inProgress].event.start_callback != NULL)
			(*_eventQueue.events[_eventQueue.inProgress].event.start_callback)();
	}
}
