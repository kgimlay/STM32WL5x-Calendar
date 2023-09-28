/*
 *
 */

#include <rtc_calendar_control.h>
#include <stdbool.h>


/*
 * Macro function to check if the RTC has been initialized in HAL
 * and within this module.
 */
#define IS_RTC_INIT(rtc_handle) (rtc_handle != NULL && rtc_handle->Instance != NULL)


/*
 * Static operational variable to point to HAL RTC handle for module operation
 * across function calls.
 */
RTC_HandleTypeDef* _rtc_handle;


/* rtcCalendarControl_init
 *
 * Initializes the module and stores a pointer to the HAL RTC handle.
 *
 * Note: will not reinitialize if already initialized.
 */
RtcUtilsStatus rtcCalendarControl_init(RTC_HandleTypeDef* const hrtc)
{
	// if an initialized RTC handle has been passed
	if (!IS_RTC_INIT(_rtc_handle))
	{
		_rtc_handle = hrtc;		// store handle pointer
		return RTC_CALENDAR_CONTROL_OKAY;
	}

	// an invalid handle or uninitialized handle passed
	else
	{
		return RTC_CALENDAR_CONTROL_NOT_INIT;
	}
}


/* rtcCalendarControl_setDateTime
 *
 * Set the date and time within the RTC.
 *
 * Note: does not check if parameters are within correct range.
 */
RtcUtilsStatus rtcCalendarControl_setDateTime(const uint8_t year, const uint8_t month,
		const uint8_t day, const uint8_t hour, const uint8_t minute,
		const uint8_t second)
{
	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;

	// if module has been initialized
	if (IS_RTC_INIT(_rtc_handle))
	{
		// convert from decimal to BCD
		date.Year = ((year/10) << 4) | (year % 10);
		date.Month = ((month/10) << 4) | (month % 10);
		date.Date = ((day/10) << 4) | (day % 10);
		date.WeekDay = 0;
		time.Hours = ((hour/10) << 4) | (hour % 10);
		time.Minutes = ((minute/10) << 4) | (minute % 10);
		time.Seconds = ((second/10) << 4) | (second % 10);

		// Set the date.
		if (HAL_RTC_SetDate(_rtc_handle, &date, RTC_FORMAT_BCD) != HAL_OK) {
			// HAL timeout
			return RTC_CALENDAR_CONTROL_TIMEOUT;
		}

		// Set the time.
		if (HAL_RTC_SetTime(_rtc_handle, &time, RTC_FORMAT_BCD) != HAL_OK) {
			// HAL timeout
			return RTC_CALENDAR_CONTROL_TIMEOUT;
		}

		return RTC_CALENDAR_CONTROL_OKAY;
	}

	// the module has not been initialized
	else
	{
		return RTC_CALENDAR_CONTROL_NOT_INIT;
	}
}


/* rtcCalendarControl_getDateTime
 *
 * Gets the date and time within the RTC.
 */
RtcUtilsStatus rtcCalendarControl_getDateTime(uint8_t* const year, uint8_t* const month,
		uint8_t* const day, uint8_t* const hour, uint8_t* const minute,
		uint8_t* const second)
{
	RTC_TimeTypeDef time = {0};
	RTC_DateTypeDef date = {0};

	// if the module has been initialized
	if (IS_RTC_INIT(_rtc_handle))
	{
		// Get the time and the date.
		HAL_RTC_GetTime(_rtc_handle, &time, RTC_FORMAT_BCD);
		HAL_RTC_GetDate(_rtc_handle, &date, RTC_FORMAT_BCD);

		// Return through parameters
		*year = ((date.Year & 0xF0) >> 4) * 10 + (date.Year & 0x0F);
		*month = ((date.Month & 0xF0) >> 4) * 10 + (date.Month & 0x0F);
		*day = ((date.Date & 0xF0) >> 4) * 10 + (date.Date & 0x0F);
		*hour = ((time.Hours & 0xF0) >> 4) * 10 + (time.Hours & 0x0F);
		*minute = ((time.Minutes & 0xF0) >> 4) * 10 + (time.Minutes & 0x0F);
		*second = ((time.Seconds & 0xF0) >> 4) * 10 + (time.Seconds & 0x0F);

		return RTC_CALENDAR_CONTROL_OKAY;
	}

	// the module has not been initialized
	else
	{
		return RTC_CALENDAR_CONTROL_NOT_INIT;
	}
}


/* rtcCalendarControl_setAlarm_A
 *
 * Sets and enables RTC Alarm A with an interrupt enabled.
 *
 * Note: does not validate that parameters are within valid range.
 */
RtcUtilsStatus rtcCalendarControl_setAlarm_A(const uint8_t day, const uint8_t hour,
		const uint8_t minute, const uint8_t second)
{
  RTC_AlarmTypeDef alarm = {0};

  // if the module has been initialized
  if (IS_RTC_INIT(_rtc_handle))
  {
	  // settings for alarm
	  // convert decimal to BCD
	  alarm.AlarmDateWeekDay = ((day/10) << 4) | (day % 10);
	  alarm.AlarmTime.Hours = ((hour/10) << 4) | (hour % 10);
	  alarm.AlarmTime.Minutes = ((minute/10) << 4) | (minute % 10);
	  alarm.AlarmTime.Seconds = ((second/10) << 4) | (second % 10);
	  // other settings for setting alarm
	  alarm.AlarmTime.SubSeconds = 0x0;
	  alarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	  alarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	  alarm.AlarmMask = RTC_ALARMMASK_NONE;
	  alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	  alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	  alarm.Alarm = RTC_ALARM_A;

	  // set the alarm and enable an interrupt to fire
	  HAL_RTC_SetAlarm_IT(_rtc_handle, &alarm, RTC_FORMAT_BCD);

	  return RTC_CALENDAR_CONTROL_OKAY;
  }

  // the module has not been initialized
  else
  {
	  return RTC_CALENDAR_CONTROL_NOT_INIT;
  }
}


/* rtcCalendarControl_getAlarm_A
 *
 * Gets the day and time that RTC Alarm A is set to trigger.
 *
 * Note: Does not distinguish if the alarm is enabled or not.
 */
RtcUtilsStatus rtcCalendarControl_getAlarm_A(uint8_t* const year, uint8_t* const month,
		uint8_t* const day, uint8_t* const hour, uint8_t* const minute,
		uint8_t* const second)
{
	RTC_AlarmTypeDef alarm = {0};

	// if the module has been initialized
	if (IS_RTC_INIT(_rtc_handle))
	{
		// Get the alarm information.
		HAL_RTC_GetAlarm(_rtc_handle, &alarm, RTC_ALARM_A, RTC_FORMAT_BCD);

		// Return through parameters
		*year = 0;
		*month = 0;
		*day = alarm.AlarmDateWeekDay;
		*hour = alarm.AlarmTime.Hours;
		*minute = alarm.AlarmTime.Minutes;
		*second = alarm.AlarmTime.Seconds;

		return RTC_CALENDAR_CONTROL_OKAY;
	}

	// the module has not been initialized
	else
	{
		return RTC_CALENDAR_CONTROL_NOT_INIT;
	}
}


/* rtcCalendarControl_diableAlarm_A
 *
 * Disables alarm A from firing.
 */
RtcUtilsStatus rtcCalendarControl_diableAlarm_A(void)
{
	// if the module has been initlaized
	if (IS_RTC_INIT(_rtc_handle))
	{
		HAL_RTC_DeactivateAlarm(_rtc_handle, RTC_ALARM_A);

		return RTC_CALENDAR_CONTROL_OKAY;
	}

	// the module has not been initialized
	else
	{
		return RTC_CALENDAR_CONTROL_NOT_INIT;
	}
}
