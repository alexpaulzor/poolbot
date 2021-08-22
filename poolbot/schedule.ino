#include "poolbot.h"

byte mode_to_nibble(t_mode md) {
	byte nib = 0;
	if (md == MODE_SPA) nib = 1;
	if (md == MODE_SPILL) nib = 2;
	if (md == MODE_CLEAN) nib = 3;
	return (nib << 4);
}

t_mode nibble_to_mode(byte nib) {
	nib = nib >> 4;
	if (nib == 1) return MODE_SPA;
	if (nib == 2) return MODE_SPILL;
	if (nib == 3) return MODE_CLEAN;
	return MODE_POOL;
}

byte speed_to_nibble(t_speed spd) {
	byte nib = 0;
	if (spd == SPEED_MIN) nib = 1;
	if (spd == SPEED_LOW) nib = 2;
	if (spd == SPEED_HI) nib = 3;
	if (spd == SPEED_MAX) nib = 4;
	return nib;
}

t_speed nibble_to_speed(byte nib) {
	nib = nib & 0x0f;
	if (nib == 1) return SPEED_MIN;
	if (nib == 2) return SPEED_LOW;
	if (nib == 3) return SPEED_HI;
	if (nib == 4) return SPEED_MAX;
	return SPEED_OFF;
}

void set_schedule_item(t_schedule_item &item, unsigned short start_time_m, t_mode md, t_speed spd, byte duration_m) {
	item.start_time_m = start_time_m;
	item.duration_5m = duration_m / 5;
	item.mode_speed = mode_to_nibble(md) | speed_to_nibble(spd);
}

void reset_to_defaults() {
	// Rewrite non-volatile memory with default schedule
	set_schedule_item(schedule[0],  9*60, MODE_SPILL, SPEED_LOW, 15);
	set_schedule_item(schedule[1], 10*60, MODE_CLEAN, SPEED_HI,  60);
	set_schedule_item(schedule[2], 11*60, MODE_POOL,  SPEED_HI,  4*60);
	set_schedule_item(schedule[3], 18*60, MODE_SPILL, SPEED_MIN, 120);
	set_schedule_item(schedule[4], 21*60, MODE_CLEAN, SPEED_HI,  60);
	
	for (int i = 5; i < SCHED_SLOTS; i++) {
		set_schedule_item(schedule[i], 0, MODE_POOL, SPEED_OFF, 0);
	}

	save_schedule();
}

void sort_schedule() {
	byte sorted_schedule_idx[SCHED_SLOTS];

	/*
	selection sort
	*/
	int last_index = -1;
	int next_index = -1;
	int last_start_time_m = 0;
	int next_start_time_m = 24 * 60 + 1;
/*
	for (int i = 0; i < SCHED_SLOTS; i++) {
		next_start_time_m = 24 * 60 + 1;
		for (int j = 0; j < SCHED_SLOTS; j++) {
			if (schedule[j].start_time_m > last_start_time_m || (
					schedule[j].start_time_m == last_start_time_m && 
					j > last_index)) {
				if (schedule[j].duration_5m > 0) {
					next_index = j;
					next
				}
			}
		}
		if (next_index >= 0) {
			sorted_schedule_idx[i] = next_index;
			last_index = next_index;
		}

	}*/

	// TODO: implement
}

void save_schedule() {
	// TODO: sort schedule
	// TODO: zero out fields of duration=zero items
	for (int i = 0; i < SCHED_SLOTS; i++) {
		EEPROM.put(i*SCHED_ITEM_BYTES, schedule[i]);
	}
}

void load_schedule() {
	for (int i = 0; i < SCHED_SLOTS; i++) {
		EEPROM.get(i*SCHED_ITEM_BYTES, schedule[i]);
	}
}

void complete_schedule_item() {
	if (schedule_until > millis())
		return;
	byte next_item_idx = get_next_schedule_item_idx(
		current_schedule_item_idx, get_now_m());
	t_schedule_item next_item = schedule[next_item_idx];
	
	if (next_item.duration_5m == 0) {
		// No schedule, set off for 1h.
		set_speed(SPEED_OFF);
		schedule_until = millis() + (1000l * 60l * DEFAULT_DURATION_M);
	} else if (next_item.start_time_m <= get_now_m()) {
		// item is current but has not been switched to yet
		schedule_until = millis() + (1000l * 60l * 5l * next_item.duration_5m);
		set_mode(nibble_to_mode(next_item.mode_speed));
		set_speed(nibble_to_speed(next_item.mode_speed));
		current_schedule_item_idx = next_item_idx;
	} else {
		// Wait until next schedule item
		set_speed(SPEED_OFF);
		int time_until_m = next_item.start_time_m - get_now_m();
		if (time_until_m < 0)
			time_until_m = abs(time_until_m + 24 * 60);
		schedule_until = millis() + (time_until_m * 60l * 1000l);
	}
	
	lcd.clear();
}

unsigned short get_now_m() {
	// TODO: port to use RTC
	// bool am_pm_enabled;
	// bool pm_time;

	// byte hour = clock.getHour(am_pm_enabled, pm_time) % 24;
	// byte minute = clock.getMinute() % 60;

	return ((millis_offset_now + millis()) / 1000l / 60) % (60*24);
}

byte get_next_schedule_item_idx(byte current_idx, unsigned short now_m) {
	// Returns current or next-upcoming shedule item
	unsigned short next_start_time_m = 24 * 60;
	byte next_item_idx = current_idx;
	for (int i = 0; i < SCHED_SLOTS; i++) {
		// byte idx = (current_idx + i) % SCHED_SLOTS;
		if (schedule[i].duration_5m > 0 &&
				i != current_idx && 
				schedule[i].start_time_m <= now_m && 
				schedule[i].start_time_m + schedule[i].duration_5m * 5 >= now_m) {
			// current item
			return i;
		}
		if (schedule[i].duration_5m > 0 &&
				i != current_idx &&
				schedule[i].start_time_m >= now_m && 
				schedule[i].start_time_m < next_start_time_m) {
			next_item_idx = i;
			next_start_time_m = schedule[i].start_time_m;
		}
		// TODO: handle wraparound midnight better
	}

	return next_item_idx;
}
