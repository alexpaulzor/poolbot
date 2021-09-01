// #include <stdbool.h>

#ifndef POOLBOT_H
#define POOLBOT_H
#include <DS3231.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

#define IFACE_MS 100
#define BACKLIGHT_TIMEOUT_MS 120000l
#define DEFAULT_DURATION_M 60l
#define STARTUP_MS 10000l

#define MAX_VALVE_MOVE_TIME_MS 40000l
#define MIN_VALVE_MOVE_TIME_MS 35000l


// #define PIN_FLOW_SWITCH 0  // TODO
#define PIN_VALVE_CURRENT A1
#define PIN_PUMP_STOP 2 // RELAY4IN1
#define PIN_PUMP_SPEED_STEP_1 3 // RELAY4IN2
#define PIN_PUMP_SPEED_STEP_2 4 // RELAY4IN3
#define PIN_HEAT 5 // RELAY4IN4
#define PIN_VALVE_IN_SPA 6  // RELAY0IN1
#define PIN_VALVE_OUT_SPA 7  // RELAY0IN2
#define PIN_CLEANER_PUMP 8  // RELAY2IN

#define PIN_BUTTON_MENU_UP 13
#define PIN_BUTTON_MENU_OK 12
#define PIN_BUTTON_MENU_DOWN 11

#define PIN_BUTTON_SPEED_OFF 53
#define PIN_BUTTON_SPEED_MIN 51
#define PIN_BUTTON_SPEED_LOW 49
#define PIN_BUTTON_SPEED_HI 47
#define PIN_BUTTON_SPEED_MAX 45

#define PIN_BUTTON_MODE_SPA 52
#define PIN_BUTTON_MODE_SPILL 50
#define PIN_BUTTON_MODE_POOL 48
#define PIN_BUTTON_MODE_CLEAN 46


#define NUM_BUTTONS 12

const byte INPUT_BUTTON_PINS[] = {PIN_BUTTON_MENU_UP, PIN_BUTTON_MENU_OK, PIN_BUTTON_MENU_DOWN, PIN_BUTTON_SPEED_OFF, PIN_BUTTON_SPEED_MIN, PIN_BUTTON_SPEED_LOW, PIN_BUTTON_SPEED_HI, PIN_BUTTON_SPEED_MAX, PIN_BUTTON_MODE_SPA, PIN_BUTTON_MODE_SPILL, PIN_BUTTON_MODE_POOL, PIN_BUTTON_MODE_CLEAN};

#define CURRENT_MAX 1024l
#define CURRENT_MAX_MA 30000l
//#define CURRENT_ZERO ((CURRENT_MAX+CURRENT_MIN)/2)

typedef enum {
	PORT_POOL,
	PORT_SPA,
	PORT_UNKNOWN
} t_port;

typedef enum {
    MODE_SPA,
    MODE_SPILL,
    MODE_POOL,
    MODE_CLEAN,
    MODE_UNKNOWN
} t_mode;

typedef enum {
    SPEED_OFF,
    SPEED_MIN,
    SPEED_LOW,
    SPEED_HI,
    SPEED_MAX
} t_speed;

typedef struct {
    unsigned short start_time_m;
    byte mode_speed;
    byte duration_5m;
} t_schedule_item;

#define SCHED_ITEM_BYTES 4
//#define EEPROM_SIZE (2 * 1024)  // 2 kB
#define SCHED_SLOTS 10

/* MENUS */
void menu_edit_schedule();
void menu_edit_schedule_item(byte sched_row);
void menu_edit_schedule_item_show(byte sched_row, byte row_index);
void menu_edit_schedule_show(byte row_index);
short menu_edit_schedule_item_handle_button(byte sched_row, byte button_pin);
void menu_root();
void menu_root_show(byte row_index);
void menu_set_time();
void handle_input();
byte wait_button_release();
char * get_mode_str(t_mode md);
char * get_speed_str(t_speed spd);
byte poll_buttons();
void update_display();
void schedule_row_to_buf(char *buf, t_schedule_item item);

/* SCHEDULE */
void reset_to_defaults();
void sort_schedule();
void save_schedule();
void set_schedule_item(t_schedule_item &item, unsigned short start_time_m, t_mode md, t_speed spd, byte duration_m);
void load_schedule();
void complete_schedule_item();
byte mode_to_nibble(t_mode md);
byte speed_to_nibble(t_speed spd);
t_mode nibble_to_mode(byte nib);
t_speed nibble_to_speed(byte nib);
int get_next_schedule_item_idx(byte current_idx, unsigned short now_m);
unsigned short get_now_m();

/* HARDWARE */
void set_mode(t_mode md);
void set_speed(t_speed spd);
void complete_mode_transition();
void stop_cleaner();
void stop_heater();
void stop_pumps();
void unstop_pump();
bool needs_valve_transition(t_mode from_mode, t_mode to_mode);
bool start_cleaner();
bool start_heater();

// TODO: helper function to handle millis() rollover

# endif // POOLBOT_H

