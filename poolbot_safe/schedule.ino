#include "poolbot_safe.h"

void complete_schedule_item() {
	// Due to rollover nonsense, we have to check for overflow
	long time_since = millis() - schedule_until;
	long time_until = schedule_until - millis();
	if (abs(time_since + time_until) > 2) {
		Serial.println("Resetting schedule_until from " + String(schedule_until) + " since=" + String(time_since) + " until=" + String(time_until));
		schedule_until = min(schedule_until, millis());

	}
	if (schedule_until > millis())
		return;
	Serial.println("schedule_until=" + String(schedule_until) + " reached");
	
	set_speed(SPEED_OFF);
	schedule_until = millis() + (1000l * 60l * DEFAULT_DURATION_M);
	Serial.println("No schedule. Sleeping 1h.");
	Serial.println("schedule_until=" + String(schedule_until));
	
	lcd.clear();
}
