// #include <stdbool.h>

#ifndef POOLBOT_H
#define POOLBOT_H

#define PIN_CLEANER_PUMP 0  // TODO

#define PIN_HEAT 0 // TODO

#define PIN_VALVE_IN_SPA 0  // TODO
#define PIN_VALVE_OUT_SPA 0  // TODO
// #define PIN_FLOW_SWITCH 0  // TODO
// #define PIN_VALVE_CURRENT 0  // TODO
#define PIN_PUMP_STOP 0 // TODO
#define PIN_PUMP_SPEED_STEP_1 0 // TODO
#define PIN_PUMP_SPEED_STEP_2 0 // TODO

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

// #define CURRENT_MAX 1024
// #define CURRENT_MIN 0
// #define CURRENT_ZERO ((CURRENT_MAX+CURRENT_MIN)/2)

typedef enum {
	IN,
	OUT
} t_valve;

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


# endif // POOLBOT_H

