// #include <stdbool.h>

#ifndef POOLBOT_H
#define POOLBOT_H
#include <DS3231.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define IFACE_MS 100
#define BACKLIGHT_TIMEOUT_MS 45000l
#define DEFAULT_DURATION_M 60l

#define MAX_VALVE_MOVE_TIME_MS 40000l
#define MIN_VALVE_MOVE_TIME_MS 35000l


// #define PIN_FLOW_SWITCH 0  // TODO
// #define PIN_VALVE_CURRENT A1
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

/* MENUS */
void menu_root();
void menu_root_show(byte row_index);
void menu_set_time();
void handle_input();
byte wait_button_release();
char * get_mode_str(t_mode md);
char * get_speed_str(t_speed spd);
byte poll_buttons();
void update_display();

/* SCHEDULE */
void complete_schedule_item();

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
