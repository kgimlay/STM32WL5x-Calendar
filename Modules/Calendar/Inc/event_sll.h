/*
 * Author: Kevin Imlay
 * Date: September, 2023
 *
 * Purpose:
 * 		The Event Statically-Linked List provides a neat data structure
 * 	for use by the calendar to store calendar events and check what
 * 	event aligns with the a given time (usually the current RTC time).
 * 	The event sll behaves similar to a standard linked list with the
 * 	notable difference that it is statically allocated at compile time,
 * 	giving it a fixed maximum number of events that it can store.
 * 		The sll provides the standar means to insert, remove, and peek
 * 	at events.  An additional function is provided to get the next alarm
 * 	to set within the RTC.
 */

#ifndef CALENDAR_INC_EVENT_SLL_H_
#define CALENDAR_INC_EVENT_SLL_H_


#include <stdint.h>
#include <stdbool.h>

/*
 * Size of the CalendarEvent queue.
 */
#define MAX_NUM_EVENTS 32

/*
 * Static linked-list index for end of list.
 */
#define EVENTS_SLL_NO_EVENT (-1)

/*
 * Structure to hold a date and time.
 */
typedef struct {
  uint8_t year;		// two digit 21st century year 		(0 - 99)
  uint8_t month;	// two digit month 					(1 - 12)
  uint8_t day;		// two digit day of month 			(1 - 28/29/30/31)
  uint8_t hour;		// two digit hour in 24 hour format (0 - 23)
  uint8_t minute;	// two digit minute 				(0 - 59)
  uint8_t second;	// two digit second 				(0 - 59)
} DateTime;

/*
 * Structure to hold the start and end DateTime of an event
 * along with callback function pointers to execute when an
 * event starts and ends.
 */
typedef struct CalendarEvent {
  DateTime start;
  DateTime end;
  void (*start_callback)(void);
  void (*end_callback)(void);
} CalendarEvent;

/*
 * Node within the Events SLL.
 */
struct EventSLL_Node {
	struct CalendarEvent event;
	int id;
	int next;
};

/*
 * Event Statically-Linked list.  Provides storage for the
 * linked list and overhead variables.
 */
typedef struct {
	struct EventSLL_Node events[MAX_NUM_EVENTS];		// array to hold statically-allocated nodes
	int usedHead;			// index to first node that is "in use"
	int freeHead;			// index to first node that is "not in use"
	int inProgress;			// index to the event currently in progress or CALENDAR_SLL_NO_EVENT
	unsigned int count;		// counter to keep track of how many events nodes are used;
} Event_SLL;


/* resetEventSLL
 *
 * Function:
 * 	Resets the overhead variables and clears the storage of an event sll.
 * 	Must be called before using sll.
 *
 * Parameters:
 * 	sll - pointer to an Event_SLL
 *
 * Return:
 * 	bool - false if a NULL pointer was passed, true otherwise.
 */
bool eventSLL_reset(Event_SLL* const sll);

/* eventSLL_insert
 *
 * Function:
 * 	Inserts an event into an event sll, maintaining monotonic ordering of
 * 	events.
 *
 * Parameters:
 * 	sll - pointer to an Event_SLL
 * 	event - CalendarEvent to insert
 *
 * Return:
 * 	bool - false if the sll is full, true otherwise (successfully inserted)
 *
 * Note:  it is recommended to run eventSLL_getNextAlarm() after inserting events.
 *  this can be done after a bulk of insert operations.
 *
 * Note:  monotonic ordering is preserved on start times of events only.
 */
bool eventSLL_insert(Event_SLL* const sll, const struct CalendarEvent event);

/* eventSLL_remove
 *
 * Function:
 * 	Removes an event from an event sll based on it's ID.
 *
 * Parameters:
 * 	sll - pointer to an Event_SLL
 * 	id - integer ID to remove
 *
 * Return:
 * 	bool - true if event was removed, false otherwise (no event to ID).
 *
 * Note:  it is recommended to run eventSLL_getNextAlarm() after removing events.
 *  this can be done after a bulk of remove operations.
 */
bool eventSLL_remove(Event_SLL* const sll, const unsigned int id);

/* eventSLL_peekIdx
 *
 * Function:
 * 	Gets an event for the given ID.
 *
 * Parameters:
 * 	sll - pointer to an Event_SLL
 * 	id - integer ID to remove
 *
 * Return:
 * 	bool - true if the event was valid and returned, false otherwise.
 * 	event - pointer to a CalendarEvent to store the result in
 */
bool eventSLL_peekIdx(Event_SLL* const sll, const unsigned int id, struct CalendarEvent* const event);

/* eventSLL_getNextAlarm
 *
 * Function:
 * 	Gets the next alarm to the DateTime passed in.  This will be an alarm to start
 * 	an event or to end an event.
 *
 * Parameters:
 * 	sll - pointer to an Event_SLL
 * 	dateTime - a DateTime to get the next alarm to
 *
 * Return:
 * 	bool - true if an alarm was found and returned, false otherwise
 * 	alarm - pointer to a DateTime to store the result in
 *
 * Note:  call eventSLL_getNextAlarm() after updating inserting or removing events
 * 	to prevent undefined behavior.
 */
bool eventSLL_getNextAlarm(Event_SLL* const sll, const DateTime dateTime, DateTime* const alarm);


#endif /* CALENDAR_INC_EVENT_SLL_H_ */
