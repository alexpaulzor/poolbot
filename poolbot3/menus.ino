#include "poolbot3.h"

void menu_set_time() {
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Set Hour");

	char buf[21];
	// TODO: remove
	//short hour = get_now_m() / 60;
	//short minute = get_now_m() % 60;

	bool am_pm_enabled;
	bool pm_time;
	int hour = clock.getHour(am_pm_enabled, pm_time);
	int minute = clock.getMinute();

	lcd.setCursor(0, 1);
	sprintf(buf, "%02d:%02d", hour, minute);
	lcd.print(buf);

	byte button_pin = wait_button_release();
	while (button_pin != PIN_BUTTON_MENU_OK) {
		if (button_pin == PIN_BUTTON_MENU_DOWN) {
			hour = hour - 1;
			if (hour < 0)
				hour += 24;
		}
		if (button_pin == PIN_BUTTON_MENU_UP)
			hour = (hour + 1) % 24;
		lcd.setCursor(0, 1);
		sprintf(buf, "%02d:%02d", hour, minute);
		lcd.print(buf);
		delay(IFACE_MS);
		button_pin = poll_buttons();
	}
	button_pin = wait_button_release();

	lcd.setCursor(0, 0);
	lcd.print("Set Minute");

	while (button_pin != PIN_BUTTON_MENU_OK) {
		if (button_pin == PIN_BUTTON_MENU_DOWN) {
			minute = minute - 1;
			if (minute < 0)
				minute += 60;
		}
		if (button_pin == PIN_BUTTON_MENU_UP)
			minute = (minute + 1) % 60;

		lcd.setCursor(0, 1);
		sprintf(buf, "%02d:%02d", hour, minute);
		lcd.print(buf);
		delay(IFACE_MS);
		button_pin = poll_buttons();
	}
	clock.setHour(hour);
	clock.setMinute(minute);

	Serial.println("set clock = " + String(hour) + ":" + String(minute));
	
	schedule_until = millis();
}

// void sprintf_duration(char *buf, t_schedule_item item) {
// 	if ((item.duration_5m * 5) % 60 == 0)
// 		sprintf(buf, "%2dh\0", item.duration_5m * 5 / 60);
// 	else if (item.duration_5m * 5 < 100)
// 		sprintf(buf, "%2dm\0", item.duration_5m * 5);
// 	else
// 		sprintf(buf, "%3d\0", item.duration_5m * 5);
// }

void menu_root_show(byte row_index) {
	lcd.clear();

	lcd.setCursor(1, 0);
	lcd.print("Return");

	lcd.setCursor(1, 1);
	lcd.print("Set Time");

	lcd.setCursor(1, 2);
	lcd.print("Diagnostics");

	lcd.setCursor(0, row_index % 4);
	lcd.print(">");
}

void menu_root() {
	byte row_index = 1;
	//byte num_rows = 4;
	Serial.println("root menu");
	menu_root_show(row_index);	

	// wait for no buttons to be pressed
	byte button_pin = wait_button_release();
	byte last_button_pin = button_pin;
	while (button_pin != PIN_BUTTON_MENU_OK) {
		if (button_pin == PIN_BUTTON_MENU_DOWN && row_index > 0)
			row_index--;
		if (button_pin == PIN_BUTTON_MENU_UP && row_index < 2)
			row_index++;
		menu_root_show(row_index);
		last_button_pin = button_pin;
		do {
			delay(IFACE_MS);
			button_pin = poll_buttons();
		} while (button_pin == last_button_pin);
	}
	// wait for no buttons to be pressed
	wait_button_release();
	if (row_index == 0) 
		// return menu option
		return;
	if (row_index == 1) {
		menu_set_time();
	}
	if (row_index == 2) {
		diagnostics();
	}
	
	lcd.clear();
}

byte wait_button_release() {
	byte button_pin = poll_buttons();
	while (button_pin != 0) {
		delay(IFACE_MS);
		button_pin = poll_buttons();
	}
	return button_pin;
}

byte poll_buttons() {
	for (byte i = 0; i < NUM_BUTTONS; i++) {
		if (digitalRead(INPUT_BUTTON_PINS[i]) == LOW) {
			return INPUT_BUTTON_PINS[i];
		}
	}
	return 0;
}

void handle_input() {
	byte pressed_button_pin = poll_buttons();
	if (pressed_button_pin != 0) {
		last_button_press = millis();
		lcd.backlight();
		Serial.println("Detected button pin " + String(pressed_button_pin));
	} else if (millis() - last_button_press > BACKLIGHT_TIMEOUT_MS) {
		lcd.noBacklight();
	}
	
	if (pressed_button_pin == PIN_BUTTON_MENU_UP) {
		schedule_until = schedule_until + (15l*60l*1000l);
		Serial.println("Increased schedule_until to " + String(schedule_until));
	} else if (pressed_button_pin == PIN_BUTTON_MENU_OK) {
		menu_root();
		lcd.clear();
	} else if (pressed_button_pin == PIN_BUTTON_MENU_DOWN) {
			// Due to rollover nonsense, we have to check for overflow
		long time_since = millis() - schedule_until;
		long time_until = schedule_until - millis();

		if (time_until > (15l*60l*1000l) && time_since == -time_until) {
			schedule_until = schedule_until - (15l*60l*1000l);
		} else {
			schedule_until = millis();
		}
		Serial.println("Decreased schedule_until to " + String(schedule_until));

	} else {
		handle_mode_speed_input();
	}
	if (pressed_button_pin != 0)
		Serial.println("Completed button pin " + String(pressed_button_pin));
	
	
}

void handle_mode_speed_input() {
	_handle_mode_speed_input(poll_buttons());
}

void _handle_mode_speed_input(byte pressed_button_pin) {
	if (pressed_button_pin == PIN_BUTTON_MODE_SPA) {
		set_mode(MODE_SPA);
	} else if (pressed_button_pin == PIN_BUTTON_MODE_SPILL) {
		set_mode(MODE_SPILL);
	} else if (pressed_button_pin == PIN_BUTTON_MODE_POOL) {
		set_mode(MODE_POOL);
	} else if (pressed_button_pin == PIN_BUTTON_MODE_CLEAN) {
		set_mode(MODE_CLEAN);
	} else if (pressed_button_pin == PIN_BUTTON_SPEED_OFF) {
		set_speed(SPEED_OFF);
	} else if (pressed_button_pin == PIN_BUTTON_SPEED_MIN) {
		set_speed(SPEED_MIN);
	} else if (pressed_button_pin == PIN_BUTTON_SPEED_LOW) {
		set_speed(SPEED_LOW);
	} else if (pressed_button_pin == PIN_BUTTON_SPEED_HI) {
		set_speed(SPEED_HI);
	} else if (pressed_button_pin == PIN_BUTTON_SPEED_MAX) {
		set_speed(SPEED_MAX);
	}
}

char * get_mode_str(t_mode md) {
	if (md == MODE_SPA)   return "SPA  \0";
	if (md == MODE_SPILL) return "SPILL\0";
	if (md == MODE_POOL)  return "POOL \0";
	if (md == MODE_CLEAN) return "CLEAN\0";
	return ".....\0";
}

char * get_speed_str(t_speed spd) {
	if (spd == SPEED_OFF) return "OFF\0";
	if (spd == SPEED_MIN) return "MIN\0";
	if (spd == SPEED_LOW) return "LOW\0";
	if (spd == SPEED_HI)  return "HI \0";
	if (spd == SPEED_MAX) return "MAX\0";
	return "...\0";
}

void update_display() {
	char buf[21];

	lcd.setCursor(0, 0);
	if (valves_moving_until > millis() || valves_moving()) {
		lcd.print("MOVING \0");
	} else if (stopped) {
		lcd.print("STOPPED\0");
	} else if (cleaner_on) {
		lcd.print("CLEANER\0");
	} else if (heat_on) {
		lcd.print("HEATER \0");
	} else  {
		lcd.print("       \0");
	}

	// show current time
	lcd.setCursor(11, 0);
	
	bool am_pm_enabled;
	bool pm_time;

	int hour = clock.getHour(am_pm_enabled, pm_time);
	int minute = clock.getMinute();
	int second = clock.getSecond();

	//Serial.println("Got now=" + String(hour) + ":" + String(minute) + ":" + String(second));

	sprintf(
		buf, "%02d:%02d:%02d\0", 
		hour,
		minute,
		second);
	lcd.print(buf);

	lcd.setCursor(0, 1);
	lcd.print(">");
	lcd.print(get_mode_str(mode));
	lcd.print(" ");
	lcd.print(get_speed_str(speed));

	lcd.print(" ");
	int time_left_m = schedule_until/1000l/60l - millis()/1000l/60l;
	int time_in_mode_s = (millis()/1000 - last_mode_change/1000);
	char time_in_mode_str[4];

	if (time_in_mode_s < 60l) 
		sprintf(time_in_mode_str, "%2ds\0", time_in_mode_s);
	else if (time_in_mode_s < 60l*60l)
		sprintf(time_in_mode_str, "%2dm\0", time_in_mode_s / 60l);
	else
		sprintf(time_in_mode_str, "%2dh\0", time_in_mode_s / 60l / 60l);

	sprintf(
		buf, "%02d:%02d %3s\0", 
		time_left_m / 60,
		time_left_m % 60,
		time_in_mode_str);
	lcd.print(buf);

	lcd.setCursor(0, 3);
	time_in_mode_s = valves_moving_until/1000l - millis()/1000l;
	if (valves_moving_until > millis()) {
		sprintf(buf, 
			"Moving (%ds/%6dmA)\0",
			time_in_mode_s, read_valve_current());
		lcd.print(buf);
	}
}

void wait_screen(char *msg, long timeout_ms) {
	lcd.clear();
	unsigned long now = millis();
	wait_button_release();
	while (poll_buttons() == 0 && millis() < now + timeout_ms) {
		lcd.setCursor(0, 0);
		lcd.print(String(msg) + " (" + String((now + timeout_ms - millis()) / 1000l) + "s)     ");
		Serial.println(String(msg) + " (" + String((now + timeout_ms - millis()) / 1000l) + "s)");
		delay(IFACE_MS);
	}
	lcd.clear();
}


void diagnostics() {
	char buf[21];
	lcd.clear();
	while (poll_buttons() != PIN_BUTTON_MENU_OK) {
		handle_mode_speed_input();
		update_display();
		int valve_current = read_valve_current();
		bool flow = has_flow();
		lcd.setCursor(0, 2);
		sprintf(buf, "vlv %4d flo %2d \0", valve_current, flow);
		lcd.print(buf);
		delay(IFACE_MS);
	}
}

