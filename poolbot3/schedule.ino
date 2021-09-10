#include "poolbot3.h"

void complete_schedule_item() {
	// Due to rollover nonsense, we have to check for overflow
	/*long time_since = millis() - schedule_until;
	long time_until = schedule_until - millis();
	if (abs(time_since + time_until) > 2) {
		Serial.println("Resetting schedule_until from " + String(schedule_until) + " since=" + String(time_since) + " until=" + String(time_until));
		schedule_until = min(schedule_until, millis());

	}*/
	if (schedule_until > millis())
		return;
	Serial.println("schedule_until=" + String(schedule_until) + " reached");
	
	// TODO: get next schedule item
	int now_m = get_now_m();
	int next_idx = get_next_schedule_item_idx(now_m);
	if (next_idx >= 0 && schedule[next_idx].start_time_m != schedule[next_idx].end_time_m) {
		Serial.println("Enqueueing schedule item " + String(next_idx));
		if (now_m >= schedule[next_idx].start_time_m && now_m < schedule[next_idx].end_time_m) {
			// active now
			activate_schedule_item(next_idx);
		} else {
			// wait until next item
			int minutes_until = schedule[next_idx].start_time_m - now_m;
			if (minutes_until < 0)
				minutes_until += DAY_M;
			Serial.println("Waiting " + String(minutes_until) + "m until schedule item " + String(next_idx));
			set_speed(SPEED_OFF);
			schedule_until = millis() + minutes_until * 1000l * 60l;
		}
	} else {
		Serial.println("No schedule. Sleeping 1h.");
		set_speed(SPEED_OFF);
		schedule_until = millis() + (1000l * 60l * DEFAULT_DURATION_M);
	}
	Serial.println("schedule_until=" + String(schedule_until));
	
	lcd.clear();
}

void activate_schedule_item(int idx) {
	current_schedule_item_idx = idx;
	int minutes_until = schedule[idx].end_time_m - get_now_m();
	if (minutes_until < 0)
		minutes_until = 0;
	Serial.println("Activating sched item " + String(idx) + " for " + String(minutes_until) + "m");
	set_mode(nibble_to_mode(schedule[idx].mode_speed));
	set_speed(nibble_to_speed(schedule[idx].mode_speed));
	schedule_until = millis() + minutes_until * 1000l * 60l;
}

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

void set_schedule_item(t_schedule_item &item, unsigned short start_time_m, unsigned short end_time_m, t_mode md, t_speed spd) {
	item.start_time_m = start_time_m;
	item.end_time_m = end_time_m;
	item.mode_speed = mode_to_nibble(md) | speed_to_nibble(spd);
}

void reset_to_defaults() {
	// Rewrite non-volatile memory with default schedule
	for (int i = 0; i < SCHED_SLOTS; i++) {
		set_schedule_item(schedule[i], 0, 0, MODE_POOL, SPEED_OFF);
	}

	set_schedule_item(schedule[1], 9*60,  10*60, MODE_SPILL, SPEED_LOW);
	set_schedule_item(schedule[3], 10*60, 11*60, MODE_CLEAN, SPEED_HI);
	set_schedule_item(schedule[5], 11*60, 15*60, MODE_POOL,  SPEED_HI);
	set_schedule_item(schedule[7], 15*60, 21*60, MODE_SPILL, SPEED_MIN);
	set_schedule_item(schedule[9], 21*60, 22*60, MODE_CLEAN, SPEED_HI);

	save_schedule();
	load_schedule();
}

void sort_schedule() {
	t_schedule_item orig_schedule[SCHED_SLOTS];
	char buf[21];

	for (int i = 0; i < SCHED_SLOTS; i++) {
		orig_schedule[i] = schedule[i];
		schedule_row_to_buf(buf, orig_schedule[i]);
		Serial.println("unsorted[" + String(i) + "]=" + buf);
	}

	/*
	selection sort by (start_time, end_time)
	*/
	int min_idx = -1;
	
	for (int i = 0; i < SCHED_SLOTS; i++) {
		min_idx = -1;
		// Find the earliest-starting item still in 
		// orig_schedule, and assign it to schedule[i],
		// then unset it in orig_schedule.
		for (int j = 0; j < SCHED_SLOTS; j++) {
			if (orig_schedule[j].start_time_m != orig_schedule[j].end_time_m) {
				if (min_idx < 0)
					min_idx = j;
				else if (orig_schedule[j].start_time_m <= orig_schedule[min_idx].start_time_m &&
						 orig_schedule[j].end_time_m < orig_schedule[min_idx].end_time_m)
					min_idx = j;
			}
		}
		if (min_idx >= 0) {
			schedule[i] = orig_schedule[min_idx];
			// disable in orig_schedule, now that its sorted into real schedule
			orig_schedule[min_idx].end_time_m = orig_schedule[min_idx].start_time_m;
		} else {
			// No more non-disabled items.
			set_schedule_item(schedule[i], 0, 0, MODE_POOL, SPEED_OFF);
		}
		schedule_row_to_buf(buf, schedule[i]);
		Serial.println("sorted[" + String(i) + "]=" + buf);	
	}
}

void save_schedule() {
	sort_schedule();
	for (int i = 0; i < SCHED_SLOTS; i++) {
		EEPROM.put(i*SCHED_ITEM_BYTES, schedule[i]);
	}
}

void load_schedule() {
	stop_pumps();
	for (int i = 0; i < SCHED_SLOTS; i++) {
		EEPROM.get(i*SCHED_ITEM_BYTES, schedule[i]);
	}
	current_schedule_item_idx = -1;
	schedule_until = millis();
	valves_moving_until = millis() + MAX_VALVE_MOVE_TIME_MS;
	complete_schedule_item();
}

int get_next_schedule_item_idx(int now_m) {
	
	for (int idx = 0; idx < SCHED_SLOTS; idx++) {
		// Schedule is sorted on save, so it should be safe to be stupid
		if (schedule[idx].start_time_m != schedule[idx].end_time_m) {
			if (schedule[idx].end_time_m > now_m % DAY_M)
				return idx;
		}
	}
	// No later items today, let's try for tomorrow
	for (int idx = 0; idx < SCHED_SLOTS; idx++) {
		// Schedule is sorted on save, so it should be safe to be stupid
		if (schedule[idx].start_time_m != schedule[idx].end_time_m) {
			if (schedule[idx].end_time_m + DAY_M > now_m % DAY_M)
				return idx;
		}
	}
	return -1;
}

int get_now_m() {
	bool am_pm_enabled;
	bool pm_time;

	byte hour = clock.getHour(am_pm_enabled, pm_time) % 24;
	byte minute = clock.getMinute() % 60;

	return hour * 60 + minute;
}
