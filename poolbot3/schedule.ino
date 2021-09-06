#include "poolbot3.h"

void complete_schedule_item() {
	// Due to rollover nonsense, we have to check for overflow
	long time_since = millis() - schedule_until;
	long time_until = schedule_until - millis();
	if (abs(time_since + time_until) > 2) {
		Serial.println("Resetting schedule_until from " + String(schedule_until) + " since=" + String(time_since) + " until=" + String(time_until));
		schedule_until = min(schedule_until, millis());

	}
	if (schedule_until > millis())
		return;
	Serial.println("schedule_until=" + String(schedule_until) + " reached");
	

	set_speed(SPEED_OFF);
	schedule_until = millis() + (1000l * 60l * DEFAULT_DURATION_M);
	Serial.println("No schedule. Sleeping 1h.");
	Serial.println("schedule_until=" + String(schedule_until));
	
	lcd.clear();
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
}

void sort_schedule() {
	t_schedule_item orig_schedule[SCHED_SLOTS];

	for (int i = 0; i < SCHED_SLOTS; i++) {
		orig_schedule[i] = schedule[i];
	}

	/*
	selection sort by (start_time, end_time)
	*/
	int min_idx = -1;
	
	for (int i = 0; i < SCHED_SLOTS; i++) {
		// Find the earliest-starting item still in 
		// orig_schedule, and assign it to schedule[i],
		// then unset it in orig_schedule.
		for (int j = 0; j < SCHED_SLOTS; j++) {
			if (orig_schedule[j].start_time_m != orig_schedule[j].end_time_m) {
				if (min_idx < 0)
					min_idx = j;
				else if (orig_schedule[j].start_time_m <= orig_schedule[min_index].start_time_m &&
						 orig_schedule[j].end_time_m < orig_schedule[min_index].end_time_m)
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
	}
}

void save_schedule() {
	sort_schedule();
	for (int i = 0; i < SCHED_SLOTS; i++) {
		EEPROM.put(i*SCHED_ITEM_BYTES, schedule[i]);
	}
}

void load_schedule() {
	for (int i = 0; i < SCHED_SLOTS; i++) {
		EEPROM.get(i*SCHED_ITEM_BYTES, schedule[i]);
	}
}

int get_next_schedule_item_idx(int current_idx, int now_m) {
	for (int di = 1; di <= SCHED_SLOTS; di++) {
		int idx = (current_idx + di) % SCHED_SLOTS;
		// Schedule is sorted on save, so it should be safe to be stupid
		if (schedule[idx].start_time_m != schedule[idx].end_time_m) {
			return idx;
		}
	}
	return -1;
}
