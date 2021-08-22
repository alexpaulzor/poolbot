#include <DS3231.h>
#include <Wire.h>
//#include "LCD03.h"
#include "poolbot.h"
#include <LiquidCrystal_I2C.h>
// Set the LCD address to 0x27 for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);
//LCD03 lcd;
DS3231 clock;

t_mode mode = MODE_UNKNOWN;
t_speed speed = SPEED_OFF;

unsigned long valves_moving_until = 0;
unsigned long schedule_until = 0;

void set_schedule_item(t_schedule_item &item, unsigned short start_time_m, t_mode md, t_speed spd, byte duration_m) {
	item.start_time_m = start_time_m;
	item.duration_5m = duration_m / 5;
}

void reset_to_defaults() {
	// Rewrite non-volatile memory with default schedule
	t_schedule_item spill_purge;
	t_schedule_item clean_morning;
	t_schedule_item pool_filter;
	t_schedule_item spill_evening;
	t_schedule_item clean_evening;
	set_schedule_item(spill_purge,    9*60, MODE_SPA,   SPEED_LOW, 15);
	set_schedule_item(clean_morning, 10*60, MODE_CLEAN, SPEED_HI,  60);
	set_schedule_item(pool_filter,   11*60, MODE_POOL,  SPEED_HI,  4*60);
	set_schedule_item(spill_evening, 18*60, MODE_SPILL, SPEED_MIN, 120);
	set_schedule_item(clean_evening, 21*60, MODE_CLEAN, SPEED_HI,  60);
}


void complete_schedule_item() {
	if (schedule_until == 0 || schedule_until > millis())
		return;
	// TODO: find next-starting (or already in-progress) item from NVM
	set_speed(SPEED_OFF);
	schedule_until = 0;
	lcd.clear();
}

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

void setup() {
	Serial.begin(9600);
	//pinMode(PIN_FLOW_SWITCH, INPUT);
	//pinMode(PIN_VALVE_CURRENT, INPUT);
	
	pinMode(PIN_PUMP_STOP, OUTPUT);
	pinMode(PIN_VALVE_IN_SPA, OUTPUT);
	pinMode(PIN_VALVE_OUT_SPA, OUTPUT);
	pinMode(PIN_CLEANER_PUMP, OUTPUT);
	pinMode(PIN_PUMP_SPEED_STEP_1, OUTPUT);
	pinMode(PIN_PUMP_SPEED_STEP_2, OUTPUT);
	pinMode(PIN_HEAT, OUTPUT);

	pinMode(PIN_BUTTON_MENU_UP, INPUT_PULLUP);
	pinMode(PIN_BUTTON_MENU_OK, INPUT_PULLUP);
	pinMode(PIN_BUTTON_MENU_DOWN, INPUT_PULLUP);

	pinMode(PIN_BUTTON_SPEED_OFF, INPUT_PULLUP);
	pinMode(PIN_BUTTON_SPEED_MIN, INPUT_PULLUP);
	pinMode(PIN_BUTTON_SPEED_LOW, INPUT_PULLUP);
	pinMode(PIN_BUTTON_SPEED_HI, INPUT_PULLUP);
	pinMode(PIN_BUTTON_SPEED_MAX, INPUT_PULLUP);

	pinMode(PIN_BUTTON_MODE_SPA, INPUT_PULLUP);
	pinMode(PIN_BUTTON_MODE_SPILL, INPUT_PULLUP);
	pinMode(PIN_BUTTON_MODE_POOL, INPUT_PULLUP);
	pinMode(PIN_BUTTON_MODE_CLEAN, INPUT_PULLUP);

	stop_pumps();

	// Set to 24h time
	clock.setClockMode(false);

	//lcd.begin(20, 4);
	lcd.init();
	lcd.noBacklight();
	lcd.clear();
	delay(IFACE_MS);
	set_speed(SPEED_OFF);
	set_mode(MODE_POOL);
	schedule_until = millis();
	update_display();
	lcd.backlight();
	Serial.println("setup() complete");
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

void menu_set_time() {
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Set Hour");
	
	bool am_pm_enabled;
	bool pm_time;

	byte hour = clock.getHour(am_pm_enabled, pm_time) % 24;
	byte minute = clock.getMinute() % 60;

	lcd.setCursor(0, 1);
	lcd.print(hour);
	lcd.setCursor(3, 1);
	lcd.print(":");
	lcd.print(minute);

	byte button_pin = wait_button_release();
	while (button_pin != PIN_BUTTON_MENU_OK) {
		if (button_pin == PIN_BUTTON_MENU_DOWN)
			hour = (hour - 1) % 24;
		if (button_pin == PIN_BUTTON_MENU_UP)
			hour = (hour + 1) % 24;
		lcd.setCursor(0, 1);
		lcd.print(hour);
		delay(IFACE_MS);
		button_pin = poll_buttons();
	}
	button_pin = wait_button_release();

	lcd.setCursor(0, 0);
	lcd.print("Set Minute");

	while (button_pin != PIN_BUTTON_MENU_OK) {
		if (button_pin == PIN_BUTTON_MENU_DOWN)
			minute = (minute - 1) % 60;
		if (button_pin == PIN_BUTTON_MENU_UP)
			minute = (minute + 1) % 60;

		lcd.setCursor(4, 1);
		lcd.print(minute);
		delay(IFACE_MS);
		button_pin = poll_buttons();
	}
	Serial.println("set clock = " + String(hour) + ":" + String(minute));
	clock.setHour(hour);
	clock.setMinute(minute);
}

void menu_root_show(byte row_index) {
	lcd.clear();

	if (row_index < 4) {
		lcd.setCursor(1, 0);
		lcd.print("Return");

		lcd.setCursor(1, 1);
		lcd.print("Set Time");

		lcd.setCursor(1, 2);
		lcd.print("Reset to Defaults");

		// TODO: render schedule rows 0 and 1
	}
	// TODO: render schedule rows 2+ in subsequent pages

	lcd.setCursor(0, row_index % 4);
	lcd.print(">");
}

void menu_root() {
	byte row_index = 0;
	byte num_rows = 4;
	Serial.println("root menu");
	menu_root_show(row_index);	

	// wait for no buttons to be pressed
	byte button_pin = wait_button_release();
	byte last_button_pin = button_pin;
	while (button_pin != PIN_BUTTON_MENU_OK) {
		if (button_pin == PIN_BUTTON_MENU_DOWN && row_index > 0)
			row_index--;
		if (button_pin == PIN_BUTTON_MENU_UP && row_index < num_rows - 1)
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
		if (digitalRead(INPUT_BUTTON_PINS[i]) == LOW)
			return INPUT_BUTTON_PINS[i];
	}
	return 0;
}

void handle_input() {
	byte pressed_button_pin = poll_buttons();
	switch (pressed_button_pin) {
		case PIN_BUTTON_MENU_DOWN:
			if (schedule_until > 0)
				schedule_until = max(millis(), schedule_until - (5l*60l*1000l));
			Serial.println("Decreased schedule_until to " + String(schedule_until));
			break;
		case PIN_BUTTON_MENU_OK:
			menu_root();
			lcd.clear();
			break;
		case PIN_BUTTON_MENU_UP:
			schedule_until = schedule_until + (5l*60l*1000l);
			Serial.println("Increased schedule_until to " + String(schedule_until));
			break;
		
		case PIN_BUTTON_MODE_SPA:
			set_mode(MODE_SPA);
			break;
		case PIN_BUTTON_MODE_SPILL:
			set_mode(MODE_SPILL);
			break;
		case PIN_BUTTON_MODE_POOL:
			set_mode(MODE_POOL);
			break;
		case PIN_BUTTON_MODE_CLEAN:
			set_mode(MODE_CLEAN);
			break;

		case PIN_BUTTON_SPEED_OFF:
			set_speed(SPEED_OFF);
			break;
		case PIN_BUTTON_SPEED_MIN:
			set_speed(SPEED_MIN);
			break;
		case PIN_BUTTON_SPEED_LOW:
			set_speed(SPEED_LOW);
			break;
		case PIN_BUTTON_SPEED_HI:
			set_speed(SPEED_HI);
			break;
		case PIN_BUTTON_SPEED_MAX:
			set_speed(SPEED_MAX);
			break;
		
		default:
			break;
	}
	
}

void loop() {
	// delay(1000);
	//Serial.println("loop begin");
	handle_input();
	complete_schedule_item();
	complete_mode_transition();
	update_display();
	delay(IFACE_MS);
	//lcd.print("Hello, world!");
	// lcd.display();
	// delay(100);
	//Serial.println("loop complete");
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
	//lcd.clear();
	//lcd.cursor();

	char buf[21];

	// show current time
	lcd.setCursor(12, 0);
	/*
	bool am_pm_enabled;
	bool pm_time;

	lcd.print(clock.getHour(am_pm_enabled, pm_time));
	lcd.print(":");
	lcd.print(clock.getMinute());
	lcd.print(":");
	lcd.print(clock.getSecond());
	*/
	unsigned int now = millis() / 1000l;
	sprintf(
		buf, "%02u:%02u:%02u\0", 
		now / 60 / 60,
		now / 60 % 60,
		now % 60);
	lcd.print(buf);

	lcd.setCursor(1, 1);
	//lcd.print(">");
	lcd.print(get_mode_str(mode));
	lcd.print(" ");
	lcd.print(get_speed_str(speed));

	if (schedule_until != 0) {
		lcd.print(" ");
		int time_left = (schedule_until - millis()) / 1000l;
		sprintf(
			buf, "%02d:%02d:%02d\0", 
			time_left / 60 / 60,
			time_left / 60 % 60,
			time_left % 60);
		lcd.print(buf);
	}
	// TODO: show next schedule item

	if (valves_moving_until != 0) {
		long valve_time_left = (valves_moving_until - millis()) / 1000l;
		lcd.setCursor(0, 3);
		lcd.print("Moving (" + String(valve_time_left) + "s)...");
	}
}
