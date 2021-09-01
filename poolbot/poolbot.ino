#include "poolbot.h"
LiquidCrystal_I2C lcd(0x27, 20, 4);
DS3231 clock;

t_mode mode = MODE_UNKNOWN;
t_speed speed = SPEED_OFF;

t_schedule_item schedule[SCHED_SLOTS];
byte current_schedule_item_idx = SCHED_SLOTS + 1;

unsigned long valves_moving_until = 0;
unsigned long starting_until = 0;
unsigned long schedule_until = 0;
unsigned long last_mode_change = 0;
unsigned long last_button_press = 0;
int valve_current = 0;
unsigned long last_valve_current = 0;
bool heat_on = false;
bool cleaner_on = false;
bool stopped = true;
char state_msg[10] = "\0";

void setup() {
	Serial.begin(9600);
	//pinMode(PIN_FLOW_SWITCH, INPUT);
	pinMode(PIN_VALVE_CURRENT, INPUT);
	
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
	load_schedule();
	update_display();
	lcd.backlight();
	Serial.println("setup() complete");
}

void loop() {
	handle_input();
	complete_schedule_item();
	complete_mode_transition();
	update_display();
	delay(IFACE_MS);
}
