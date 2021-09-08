#include "poolbot3.h"

bool start_cleaner() {
	/* 	* OUT=POOL (usually also IN=POOL)
		* Pump unstopped
		* Flow detected
	*/
	if (stopped || valves_moving() || (mode != MODE_POOL && mode != MODE_CLEAN)) {
		stop_cleaner();
		return false;
	}
	if (!has_flow()) {
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
	/* 	* IN=SPA, OUT=SPA
		* Pump unstopped
		* Flow detected
	*/
	if (stopped || valves_moving() || mode != MODE_SPA || !has_flow()) {
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
	wait_button_release();
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
		wait_button_release();
		while (!has_flow() && poll_buttons() == 0) {
			lcd.setCursor(0, 0);
			lcd.print("Starting...(" + String((millis() - now) / 1000l) + "s)");
			Serial.println("Starting...(" + String((millis() - now) / 1000l) + "s)");
			delay(IFACE_MS);
		}
		lcd.clear();
	} else {
		Serial.println("(speed=OFF)");
		stop_pumps();
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

	unsigned long min_time = millis() + (1000l * 60l * DEFAULT_DURATION_M);
	unsigned long safe_time = millis() + (1000l * 60l * DEFAULT_DURATION_M * 4);

	schedule_until = max(min_time, min(safe_time, schedule_until));

	if (md == MODE_SPA) {
		set_speed(SPEED_MAX);
	}
	if (md == MODE_SPILL)
		set_speed(SPEED_MIN);
	if (md == MODE_POOL)
		set_speed(SPEED_HI);
	if (md == MODE_CLEAN) {
		set_speed(SPEED_HI);
	}

	if (needs_valve_transition(old_mode, md)) {
		transition_valves(md);
	} else {
		valves_moving_until = max(valves_moving_until, 1l);
	}
	last_mode_change = millis();
	complete_mode_transition();

	lcd.clear();
}

void transition_valves(t_mode md) {
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
	if (mode == MODE_CLEAN) {
		if (!cleaner_on && has_flow()) {
			start_cleaner();
		}
	} else {
		stop_cleaner();
	}
	if (mode == MODE_SPA) {
		if (!heat_on && has_flow()) {
			start_heater();
		}
	} else {
		stop_heater();
	}
}

void set_speed(t_speed new_speed) {
	t_speed old_speed = speed;
	Serial.println("set_speed " + String(new_speed) + " (from " + String(speed) + ")");
	speed = new_speed;
	
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
	}
	if (valves_moving_until != 0 || new_speed == SPEED_OFF) {
		stop_pumps();
	} else {
		unstop_pump();
	}
	/* 
	NB: Pump refuses to obey SPEED_HI -> SPEED_MAX transition.
	             (SPD1=LOW,SPD2=HIGH) -> (SPD1=HIGH,SPD2=HIGH)
	    All other transitions appear to work ok.
	    As a workaround, set speed to LOW (SPD1=HIGH,SPD2=LOW)
	    for 5s, then back to MAX.
	*/
	if (new_speed == SPEED_MAX && old_speed == SPEED_HI) {
		Serial.println("(HI->MAX workaround) SPD2=LOW");
		digitalWrite(PIN_PUMP_SPEED_STEP_2, LOW);
		wait_screen("Turbo charging\0", MIN_VALVE_MOVE_TIME_MS);
		Serial.println("(HI->MAX workaround) SPD2=HIGH");
		digitalWrite(PIN_PUMP_SPEED_STEP_2, HIGH);
	}
}

int read_valve_current() {
	int current = 0;
	int new_raw_current;
	int new_current;
	for (int i = 0; i < 10; i++) {
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
