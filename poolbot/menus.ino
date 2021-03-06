#include "poolbot.h"

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

void sprintf_duration(char *buf, t_schedule_item item) {
	if ((item.duration_5m * 5) % 60 == 0)
		sprintf(buf, "%2dh\0", item.duration_5m * 5 / 60);
	else if (item.duration_5m * 5 < 100)
		sprintf(buf, "%2dm\0", item.duration_5m * 5);
	else
		sprintf(buf, "%3d\0", item.duration_5m * 5);
}

void schedule_row_to_buf(char *buf, t_schedule_item item) {
	//int start_time_m = item.start_time_m;
	if (item.duration_5m == 0) {
		sprintf(buf, "OFF\0");
		return;
	}
	char duration_str[4];
	sprintf_duration(duration_str, item);
	sprintf(
		buf, 
		"%5s %3s %02d:%02d %3s\0",
		get_mode_str(nibble_to_mode(item.mode_speed)),
		get_speed_str(nibble_to_speed(item.mode_speed)),
		item.start_time_m / 60,
		item.start_time_m % 60,
		duration_str);
}

void menu_edit_schedule_show(byte row_index) {
	lcd.clear();
	byte page = row_index / 4;
	char buf[21];
	
	for (int i = 0; i < 4; i++) {
		lcd.setCursor(1, i);
		if (page == 0 && i == 0) {
			lcd.print("Return");
		} else if (page*4 + i < SCHED_SLOTS) {
			schedule_row_to_buf(buf, schedule[page*4 + i - 1]);
			lcd.print(buf);

		}
	}
	
	lcd.setCursor(0, row_index % 4);
	lcd.print(">");
}

void menu_edit_schedule() {
	byte row_index = 1;
	Serial.println("edit schedule menu");
	menu_edit_schedule_show(row_index);	

	// wait for no buttons to be pressed
	byte button_pin = wait_button_release();
	byte last_button_pin = button_pin;
	while (row_index != 0) {
		while (button_pin != PIN_BUTTON_MENU_OK) {
			if (button_pin == PIN_BUTTON_MENU_DOWN && row_index > 0)
				row_index--;
			if (button_pin == PIN_BUTTON_MENU_UP && row_index < SCHED_SLOTS)
				row_index++;
			menu_edit_schedule_show(row_index);
			last_button_pin = button_pin;
			do {
				delay(IFACE_MS);
				button_pin = poll_buttons();
			} while (button_pin == last_button_pin);
		}
		if (row_index > 0)
			menu_edit_schedule_item(row_index - 1);
		// wait for no buttons to be pressed
		button_pin = wait_button_release();
	}
	lcd.clear();
}

void menu_edit_schedule_item_show(byte sched_row, byte row_index) {
	lcd.clear();
	
	lcd.setCursor(1, 0);
	lcd.print("Set Duration");

	char duration_str[5];
	sprintf_duration(duration_str, schedule[sched_row]);
	lcd.setCursor(16, 0);
	lcd.print(duration_str);

	lcd.setCursor(1, 1);
	lcd.print("Set Start");
	sprintf(duration_str, "%02d:%02d\0", 
		schedule[sched_row].start_time_m / 60,
		schedule[sched_row].start_time_m % 60);
	lcd.setCursor(15, 1);
	lcd.print(duration_str);

	lcd.setCursor(0, 3);
	char buf[21];
	schedule_row_to_buf(buf, schedule[sched_row]);
	lcd.print(buf);

	lcd.setCursor(0, row_index % 4);
	lcd.print(">");
}

short menu_edit_schedule_item_handle_button(byte sched_row, byte button_pin) {
	t_mode md = nibble_to_mode(schedule[sched_row].mode_speed);
	t_speed spd = nibble_to_speed(schedule[sched_row].mode_speed);
	switch (button_pin) {
		case PIN_BUTTON_MENU_DOWN:
			return -1;
			break;
		case PIN_BUTTON_MENU_OK:
			return 0;
			break;
		case PIN_BUTTON_MENU_UP:
			return 1;
			break;
		
		case PIN_BUTTON_MODE_SPA:
			md = MODE_SPA;
			break;
		case PIN_BUTTON_MODE_SPILL:
			md = MODE_SPILL;
			break;
		case PIN_BUTTON_MODE_POOL:
			md = MODE_POOL;
			break;
		case PIN_BUTTON_MODE_CLEAN:
			md = MODE_CLEAN;
			break;

		case PIN_BUTTON_SPEED_OFF:
			spd = SPEED_OFF;
			break;
		case PIN_BUTTON_SPEED_MIN:
			spd = SPEED_MIN;
			break;
		case PIN_BUTTON_SPEED_LOW:
			spd = SPEED_LOW;
			break;
		case PIN_BUTTON_SPEED_HI:
			spd = SPEED_HI;
			break;
		case PIN_BUTTON_SPEED_MAX:
			spd = SPEED_MAX;
			break;
		
		default:
			break;
	}
	set_schedule_item(
		schedule[sched_row], 
		schedule[sched_row].start_time_m, 
		md, spd, schedule[sched_row].duration_5m*5);
	return 0;
}

void menu_edit_schedule_item(byte sched_row) {
	Serial.println("menu_edit_schedule_item " + String(sched_row));
	byte row_index = 0;
	menu_edit_schedule_item_show(sched_row, row_index);
	byte button_pin = wait_button_release();
	while (button_pin != PIN_BUTTON_MENU_OK) {
		short dt = menu_edit_schedule_item_handle_button(sched_row, button_pin);
		int dur = max(schedule[sched_row].duration_5m + dt, 0);

		schedule[sched_row].duration_5m = dur;

		menu_edit_schedule_item_show(sched_row, row_index);
		do {
			delay(IFACE_MS);
			button_pin = poll_buttons();
		} while (button_pin == 0);
	}
	button_pin = wait_button_release();
	if (schedule[sched_row].duration_5m == 0) {
		set_schedule_item(
			schedule[sched_row], 
			0, 
			MODE_POOL, SPEED_OFF, 0);
	} else {
		row_index = 1;
		while (button_pin != PIN_BUTTON_MENU_OK) {
			short dt = menu_edit_schedule_item_handle_button(sched_row, button_pin);
			int start_time = schedule[sched_row].start_time_m + dt * 5;
			if (start_time > 24 * 60) start_time -= 24 * 60;
			if (start_time < 0) start_time += 24 * 60;
			schedule[sched_row].start_time_m = start_time;

			menu_edit_schedule_item_show(sched_row, row_index);
			do {
				delay(IFACE_MS);
				button_pin = poll_buttons();
			} while (button_pin == 0);
		}
	}
	save_schedule();
	load_schedule();
}

void menu_root_show(byte row_index) {
	lcd.clear();

	lcd.setCursor(1, 0);
	lcd.print("Return");

	lcd.setCursor(1, 1);
	lcd.print("Set Time");

	lcd.setCursor(1, 2);
	lcd.print("Edit Schedule");

	lcd.setCursor(1, 3);
	lcd.print("Reset to Defaults");

	lcd.setCursor(0, row_index);
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
		if (button_pin == PIN_BUTTON_MENU_UP && row_index < 3)
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
		menu_edit_schedule();
	}
	if (row_index == 3) {
		reset_to_defaults();
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

	} else if (pressed_button_pin == PIN_BUTTON_MODE_SPA) {
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
	if (pressed_button_pin != 0)
		Serial.println("Completed button pin " + String(pressed_button_pin));
	
	
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
	if (cleaner_on) {
		lcd.print("CLEANER\0");
	} else if (heat_on) {
		lcd.print("HEATER \0");
	} else if (stopped) {
		lcd.print("STOPPED\0");
	} else {
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
	int time_left = abs(schedule_until - millis()) / 1000l;
	int time_in_mode = (millis() - last_mode_change) / 1000l;
	char time_in_mode_str[4];

	if (time_in_mode < 60) 
		sprintf(time_in_mode_str, "%2ds\0", time_in_mode);
	else if (time_in_mode < 60*60)
		sprintf(time_in_mode_str, "%2dm\0", time_in_mode / 60);
	else
		sprintf(time_in_mode_str, "%2dh\0", time_in_mode / 60 / 60);

	sprintf(
		buf, "%02d:%02d %3s\0", 
		time_left / 60 / 60,
		time_left / 60 % 60,
		time_in_mode_str);
	lcd.print(buf);
	int next_after_m = get_now_m() + time_left / 60;
	int next_schedule_item_idx = get_next_schedule_item_idx(current_schedule_item_idx, next_after_m);
	if (next_schedule_item_idx >= 0) {
		schedule_row_to_buf(buf, schedule[next_schedule_item_idx]);
		lcd.setCursor(1, 2);
		lcd.print(buf);
	}

	lcd.setCursor(0, 3);
	time_left = abs(valves_moving_until - millis()) / 1000l;
	if (valves_moving_until > millis()) {
		sprintf(buf, 
			"Moving (%2ds/%5dmA)\0",
			time_left,
			valve_current);
		lcd.print(buf);
	} else {
		next_after_m = schedule[next_schedule_item_idx].start_time_m + schedule[next_schedule_item_idx].duration_5m * 5;
		next_schedule_item_idx = get_next_schedule_item_idx(next_schedule_item_idx, next_after_m);
		if (next_schedule_item_idx >= 0) {
			schedule_row_to_buf(buf, schedule[next_schedule_item_idx]);
			lcd.setCursor(1, 3);
			lcd.print(buf);
		}
	}
}
