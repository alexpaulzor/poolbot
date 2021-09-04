#include "poolbot_safe.h"

bool start_cleaner() {
	// TODO: check/assert requirements for cleaner
	/* 	* OUT=POOL (usually also IN=POOL)
		* Pump unstopped
		* Flow detected
	*/
	if (valves_moving() || (mode != MODE_POOL && mode != MODE_CLEAN)) {
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
	if (valves_moving() || mode != MODE_SPA) {
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

	lcd.clear();
	unsigned long now = millis();
	while (has_flow() && poll_buttons() == 0) {
		lcd.setCursor(0, 0);
		lcd.print("Stopping...(" + String((millis() - now) / 1000l) + "s)");
		Serial.println("Stopping...(" + String((millis() - now) / 1000l) + "s)");
		delay(IFACE_MS);
	}
	lcd.clear();
}

void unstop_pump() {
	Serial.println("unstop_pump");
	if (speed != SPEED_OFF) {
		stopped = false;
		digitalWrite(PIN_PUMP_STOP, HIGH);
		lcd.clear();
		unsigned long now = millis();
		while (!has_flow() && poll_buttons() == 0) {
			lcd.setCursor(0, 0);
			lcd.print("Starting...(" + String((millis() - now) / 1000l) + "s)");
			Serial.println("Starting...(" + String((millis() - now) / 1000l) + "s)");
			delay(IFACE_MS);
		}
		lcd.clear();
	} else {
		Serial.println("(speed=OFF)");
	}
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

	if (old_mode != md) {
		last_mode_change = millis();
	}
	unsigned long safe_time = millis() + (1000l * 60l * DEFAULT_DURATION_M * 4);

	if (md == MODE_SPA) {
		set_speed(SPEED_MAX);
		if (schedule_until > safe_time)
			schedule_until = safe_time;
	}
	if (md == MODE_SPILL)
		set_speed(SPEED_MIN);
	//if (md == MODE_POOL)
	//	set_speed(SPEED_LOW);
	if (md == MODE_CLEAN) {
		set_speed(SPEED_HI);
		if (schedule_until > safe_time)
			schedule_until = safe_time;
	}

	if (needs_valve_transition(old_mode, md)) {
		stop_pumps();

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
		valves_moving_until = millis() + MAX_VALVE_MOVE_TIME_MS;
		
	} else {
		valves_moving_until = max(valves_moving_until, millis());
	}
	last_mode_change = millis();
	complete_mode_transition();

	lcd.clear();
}

void complete_mode_transition() {
	if (valves_moving_until > millis()) {
		if (valves_moving() || (valves_moving_until - millis()) > (MAX_VALVE_MOVE_TIME_MS - MIN_VALVE_MOVE_TIME_MS)) {
			Serial.println("Waiting " + String((valves_moving_until - millis()) / 1000l) + "s to valve transition");
			return;
		}
	}
	if (valves_moving_until != 0) {
		valves_moving_until = 0;
		set_speed(speed);
	}
	if (mode == MODE_CLEAN && !cleaner_on && has_flow()) {
		start_cleaner();
	}
	if (mode == MODE_SPA && !heat_on && has_flow()) {
		start_heater();
	}
}

void set_speed(t_speed spd) {
	/* 
	NB: Pump refuses to obey SPEED_HI -> SPEED_MAX transition.
	    All other transitions appear to work ok.
	*/
	if (spd == SPEED_MAX && speed == SPEED_HI)
		set_speed(SPEED_LOW);
	Serial.println("set_speed " + String(spd) + " (from " + String(speed) + ")");
	speed = spd;
	
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
	if (valves_moving_until != 0 || spd == SPEED_OFF) {
		stop_pumps();
	} else {
		unstop_pump();
	}
}

int read_valve_current() {
	int current = 0;
	int new_raw_current;
	int new_current;
	for (int i = 0; i < 5; i++) {
		new_raw_current = analogRead(PIN_VALVE_CURRENT);
		new_current = map(new_raw_current, CURRENT_ZERO, CURRENT_MAX, 0, CURRENT_MAX_MA);
		current = max(current, abs(new_current));
		if (current > VALVE_CURRENT_MOVING_MA) {
			Serial.println("Read current " + String(new_raw_current) + " -> " + String(new_current));
			return current;
		}
		delay(1);
	}
	Serial.println("Read current " + String(new_raw_current) + " -> " + String(new_current));
	return current;
}

bool valves_moving() {
	if (valves_moving_until == 0)
		return false;
	return read_valve_current() > VALVE_CURRENT_MOVING_MA;
}

bool has_flow() {
	bool flow = digitalRead(PIN_FLOW_SWITCH) == LOW;
	Serial.println("flow=" + String(flow));
	return flow;
}
