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
	cleaner_on = true;
	digitalWrite(PIN_CLEANER_PUMP, LOW);
	return true;
}

void stop_cleaner() {
	Serial.println("stop_cleaner");
	cleaner_on = false;
	digitalWrite(PIN_CLEANER_PUMP, HIGH);
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
	heat_on = true;
	digitalWrite(PIN_HEAT, HIGH);
	return true;
}

void stop_heater() {
	Serial.println("stop_heater");
	heat_on = false;
	digitalWrite(PIN_HEAT, LOW);
}

void stop_pumps() {
	stop_cleaner();
	stop_heater();
	Serial.println("stop_pumps");
	stopped = true;
	digitalWrite(PIN_PUMP_STOP, LOW);
	last_mode_change = millis();
}

void unstop_pump() {
	Serial.println("unstop_pump");
	if (speed != SPEED_OFF) {
		stopped = false;
		digitalWrite(PIN_PUMP_STOP, HIGH);
	} else {
		Serial.println("(speed=OFF)");
	}
	last_mode_change = millis();
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
	Serial.println("set_mode " + String(md) + " (from " + String(mode) + ")");
	t_mode old_mode = mode;
	mode = md;
	if (needs_valve_transition(old_mode, md)) {
		stop_pumps();
		// TODO: check flow switch
		update_display();
		delay(3000);
		valves_moving_until = millis() + MAX_VALVE_MOVE_TIME_MS;

		if (md == MODE_SPA) {
			Serial.println("IN=SPA");
			//digitalWrite(PIN_VALVE_IN_SPA, HIGH);
			digitalWrite(PIN_VALVE_IN_SPA, LOW);
		} else {
			Serial.println("IN=POOL");
			//digitalWrite(PIN_VALVE_IN_SPA, LOW);
			digitalWrite(PIN_VALVE_IN_SPA, HIGH);
		}
		if (md == MODE_SPA || md == MODE_SPILL) {
			Serial.println("OUT=SPA");
			//digitalWrite(PIN_VALVE_OUT_SPA, HIGH);
			digitalWrite(PIN_VALVE_OUT_SPA, LOW);
		} else {
			Serial.println("OUT=POOL");
			//digitalWrite(PIN_VALVE_OUT_SPA, LOW);
			digitalWrite(PIN_VALVE_OUT_SPA, HIGH);

		}
		last_mode_change = millis();
	}

	unsigned long safe_time = millis() + (1000l * 60l * DEFAULT_DURATION_M * 4);

	if (md == MODE_SPA) {
		set_speed(SPEED_MAX);
		if (schedule_until > safe_time)
			schedule_until = safe_time;
		valves_moving_until = max(valves_moving_until, millis());
	}
	if (md == MODE_SPILL)
		set_speed(SPEED_MIN);
	//if (md == MODE_POOL)
	//	set_speed(SPEED_LOW);
	if (md == MODE_CLEAN) {
		set_speed(SPEED_HI);
		if (schedule_until > safe_time)
			schedule_until = safe_time;
		valves_moving_until = max(valves_moving_until, millis());
	}
}

void complete_mode_transition() {
	if (valves_moving_until == 0)
		return;
		
	if (valves_moving_until > millis()) {
		valve_current = abs(map(
			analogRead(PIN_VALVE_CURRENT), 
			0, CURRENT_MAX, -CURRENT_MAX_MA, CURRENT_MAX_MA));
		if (valve_current > 1000)
			last_valve_current = millis();
		if (/*millis() - last_valve_current < MIN_VALVE_MOVE_TIME_MS ||*/ (valves_moving_until - millis()) > (MAX_VALVE_MOVE_TIME_MS - MIN_VALVE_MOVE_TIME_MS)) {
			Serial.println("Have valve current: " + String(valve_current) + "mA " + String((valves_moving_until - millis())/1000l) + "s / " + String((MAX_VALVE_MOVE_TIME_MS - MIN_VALVE_MOVE_TIME_MS)/1000l) + "s");
			return;
		}
	}

	valves_moving_until = 0;
	set_speed(speed);
	lcd.clear();
	update_display();
	if (mode == MODE_CLEAN) {
		// TODO: check flow switch?
		delay(10000);
		start_cleaner();
	}
	if (mode == MODE_SPA) {
		// TODO: check flow switch?
		delay(10000);
		start_heater();
	}
	lcd.clear();
}

void set_speed(t_speed spd) {
	Serial.println("set_speed " + String(spd) + " (from " + String(speed) + ")");
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
