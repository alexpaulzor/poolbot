#include "poolbot_safe.h"

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

	if (old_mode == md) {
		complete_mode_transition();
		return;
	}

	if (needs_valve_transition(old_mode, md)) {
		stop_pumps();
		lcd.clear();
		unsigned long now = millis();
		while (has_flow()) {
			lcd.setCursor(0, 0);
			lcd.println("Stopping...(" + String((millis() - now) / 1000l) + "s)");
			delay(IFACE_MS);
		}

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
		last_mode_change = millis();
	}

	lcd.clear();
}

void complete_mode_transition() {
	if (valves_moving_until == 0)
		return;
		
	if (valves_moving_until > millis()) {
		return;
	}

	valves_moving_until = 0;
	set_speed(speed);
	lcd.clear();
	unsigned long now = millis();
	lcd.setCursor(0, 0);
	if (mode == MODE_CLEAN) {
		lcd.print("Starting cleaner...");
		while (!has_flow()) {
			lcd.setCursor(0, 1);
			lcd.print("Starting pump (" + String((millis() - now) / 1000l) + "s)  ");
			delay(IFACE_MS);
		}
		start_cleaner();
	}
	if (mode == MODE_SPA) {
		lcd.print("Starting heater...");
		while (!has_flow()) {
			lcd.setCursor(0, 1);
			lcd.print("Starting pump (" + String((millis() - now) / 1000l) + "s)");
			delay(IFACE_MS);
		}
		start_heater();
	}
	lcd.clear();
}

void set_speed(t_speed spd) {
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
	for (int i = 0; i < 3; i++) {
		current = max(current, abs(analogRead(PIN_VALVE_CURRENT)));
		Serial.println("Read current " + String(current));
	}
	// TODO: map current to mA
	return current;
}

bool has_flow() {
	return digitalRead(PIN_FLOW_SWITCH) == LOW;
}
