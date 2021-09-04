# Poolbot2

## Features

* 5 speed buttons (off, min, low, hi, max)
* 4 mode buttons (spa, spill, pool, clean)
* 3 menu buttons (ok, +, -)
* 20x4 LCD display
* Real time clock module with battery backup
* persistent+editable schedule
* menu interface
  * set time
  * reset defaults
  * edit schedule
    * edit schedule item
* 7 relay control
  * in valve
  * out valve
  * stop
  * spd1
  * spd2
  * heat
  * clean
* flow sensor
* valve motor current sensor
* safety features
  * heater only when in+out are spa and flow is present and pump is >= HI
  * cleaner only when in+out are pool and flow is present and pump is >= HI




## Program design

Timers:
* valves_moving_until
* schedule_until
* last_button_pressed

State:
* mode
* speed
* stopped
* heat_on
* cleaner_on

## Program flow

* handle_input
    * menu button -> root menu
    * +/- -> adjust schedule_until
    * mode buttons -> switch mode
        * stop pump
        * delay 5s (stopping)
        * move valves
        * delay 35s (monitor valve current, multisampled)
        * start pump
        * delay 10s (starting)
        * start cleaner or heater (if applicable)

    * speed buttons -> switch speed
