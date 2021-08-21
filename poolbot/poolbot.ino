#include <DS3231.h>
#include <Wire.h>
#include "LCD03.h"
#include "poolbot.h"

LCD03 lcd;
DS3231 clock;

t_mode mode = MODE_UNKNOWN;
t_port in_port = PORT_UNKNOWN;
t_port out_port = PORT_UNKNOWN;
t_speed speed = SPEED_OFF;

void stop_pump() {
	digitalWrite(PIN_PUMP_STOP, HIGH);
}

void unstop_pump() {
	digitalWrite(PIN_PUMP_STOP, LOW);
}

bool start_cleaner() {
	// TODO: check/assert requirements for cleaner
	/* 	* OUT=POOL (usually also IN=POOL)
		* Pump unstopped
		* Flow detected
	*/
	digitalWrite(PIN_CLEANER_PUMP, HIGH);
	return true;
}

void stop_cleaner() {
	digitalWrite(PIN_CLEANER_PUMP, LOW);
}

void setup() {
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

	stop_cleaner();
	stop_pump();

	// Set to 24h time
	clock.setClockMode(false);

	lcd.begin(20, 4);
	//lcd.backlight();
	update_display();
}

void loop() {
	// lcd.print("Hello, world!");
	// delay(1000);
	// Serial.println("loop begin");
	// handle_input();

	update_display();
	// lcd.print("Hello, world!");
	// lcd.display();
	// delay(100);
	// Serial.println("loop complete");
}

char * get_mode(t_mode md) {
	if (md == MODE_SPA)   return "SPA  \0";
	if (md == MODE_SPILL) return "SPILL\0";
	if (md == MODE_POOL)  return "POOL \0";
	if (md == MODE_CLEAN) return "CLEAN\0";
	return "?????\0";
}

char * get_speed(t_speed spd) {
	if (spd == SPEED_OFF) return "OFF\0";
	if (spd == SPEED_MIN) return "MIN\0";
	if (spd == SPEED_LOW) return "LOW\0";
	if (spd == SPEED_HI)  return "HI \0";
	if (spd == SPEED_MAX) return "MAX\0";
	return "???\0";
}

void update_display() {
	lcd.home();
	lcd.cursor();

	// show current time
	lcd.setCursor(12, 0);
	bool am_pm_enabled;
	bool pm_time;

	lcd.print(clock.getHour(am_pm_enabled, pm_time));
	lcd.print(":");
	lcd.print(clock.getMinute());
	lcd.print(":");
	lcd.print(clock.getSecond());

	lcd.setCursor(0, 1);
	lcd.print(">");
	lcd.print(get_mode(mode));
	lcd.print(" ");
	lcd.print(get_speed(speed));
}
