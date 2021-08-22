#include "poolbot.h"

bool start_cleaner() {
	// TODO: check/assert requirements for cleaner
	/* 	* OUT=POOL (usually also IN=POOL)
		* Pump unstopped
		* Flow detected
	*/
	if (valves_moving_until != 0 || (mode != MODE_POOL && mode != MODE_CLEAN)) {
		stop_cleaner();
		return false;
	}
	if (speed != SPEED_MAX && speed != SPEED_HI) {
		stop_cleaner();
		return false;
	}
	Serial.println("start_cleaner");
	digitalWrite(PIN_CLEANER_PUMP, HIGH);
	return true;
}

void stop_cleaner() {
	Serial.println("stop_cleaner");
	digitalWrite(PIN_CLEANER_PUMP, LOW);
}

bool start_heater() {
	// TODO: check/assert requirements for heater
	/* 	* IN=SPA, OUT=SPA
		* Pump unstopped
		* Flow detected
	*/
	if (valves_moving_until != 0 || mode != MODE_SPA) {
		stop_heater();
		return false;
	}
	if (speed != SPEED_MAX && speed != SPEED_HI) {
		stop_heater();
		return false;
	}
	Serial.println("start_heater");
	digitalWrite(PIN_HEAT, HIGH);
	return true;
}

void stop_heater() {
	Serial.println("stop_heater");
	digitalWrite(PIN_HEAT, LOW);
}

void stop_pumps() {
	stop_cleaner();
	stop_heater();
	Serial.println("stop_pumps");
	digitalWrite(PIN_PUMP_STOP, HIGH);
}

void unstop_pump() {
	Serial.println("unstop_pump");
	if (speed != SPEED_OFF)
		digitalWrite(PIN_PUMP_STOP, LOW);
	else
		Serial.println("(speed=OFF)");
}

bool needs_valve_transition(t_mode from_mode, t_mode to_mode) {
	/* 
	from/to: spa    spill  pool   clean
	----     ---    -----  ----   -----
	spa      no     yes    yes    yes
	spill    yes    no     yes    yes
	pool     yes    yes    no     no
	clean    yes    yes    no     no
	unknown  yes    yes    yes    yes
	*/
	if (from_mode == MODE_UNKNOWN)
		return true;
	// Same mode needs no transition
	if (from_mode == to_mode)
		return false;
	// Into or out of spa always needs transition
	// Into or out of spill always needs transition
	if (from_mode == MODE_SPA || to_mode == MODE_SPA)
		return true;
	if (from_mode == MODE_SPILL || to_mode == MODE_SPILL)
		return true;
	return false;
}

void set_mode(t_mode md) {
	if (needs_valve_transition(mode, md)) {
		stop_pumps();
		valves_moving_until = millis() + VALVE_MOVE_TIME_MS;

		if (md == MODE_SPA) {
			Serial.println("IN=SPA");
			digitalWrite(PIN_VALVE_IN_SPA, HIGH);
		} else {
			Serial.println("IN=POOL");
			digitalWrite(PIN_VALVE_IN_SPA, LOW);
		}
		if (md == MODE_SPA || md == MODE_SPILL) {
			Serial.println("OUT=SPA");
			digitalWrite(PIN_VALVE_OUT_SPA, HIGH);
		} else {
			Serial.println("OUT=POOL");
			digitalWrite(PIN_VALVE_OUT_SPA, LOW);
		}
	}

	if (md == MODE_SPA)
		set_speed(SPEED_MAX);
	if (md == MODE_SPILL)
		set_speed(SPEED_MIN);
	//if (md == MODE_POOL)
	//	set_speed(SPEED_LOW);
	if (md == MODE_CLEAN)
		set_speed(SPEED_HI);
	
	if (schedule_until == 0) {
		schedule_until = millis() + (1000l * 60l * DEFAULT_DURATION_M);
	}
	mode = md;
}

void complete_mode_transition() {
	if (valves_moving_until == 0 || valves_moving_until > millis())
		return;
	
	valves_moving_until = 0;
	set_speed(speed);

	if (mode == MODE_CLEAN)
		start_cleaner();
	if (mode == MODE_SPA)
		start_heater();
	lcd.clear();
}

void set_speed(t_speed spd) {
	speed = spd;
	if (valves_moving_until != 0 || spd == SPEED_OFF) {
		stop_pumps();
		if (mode == MODE_CLEAN && spd != SPEED_MAX && spd != SPEED_HI)
			set_mode(MODE_POOL);
		return;
	}
	
	if (speed == SPEED_LOW || speed == SPEED_MAX) {
		Serial.println("SPD1=HIGH");
		digitalWrite(PIN_PUMP_SPEED_STEP_1, HIGH);
	} else { 
		Serial.println("SPD1=LOW");
		digitalWrite(PIN_PUMP_SPEED_STEP_1, LOW);
	}
	if (speed == SPEED_HI || speed == SPEED_MAX) {
		Serial.println("SPD2=HIGH");
		digitalWrite(PIN_PUMP_SPEED_STEP_2, HIGH);
	} else {
		Serial.println("SPD2=LOW");
		digitalWrite(PIN_PUMP_SPEED_STEP_2, LOW);
		stop_cleaner();
		stop_heater();
		if (mode == MODE_CLEAN)
			set_mode(MODE_POOL);
	}
	unstop_pump();
}
