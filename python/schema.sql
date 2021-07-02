create table if not exists events (
    id integer primary key,
    start_time text not null default "00:00",
    end_time text not null default "00:00",
    in_valve integer not null default 0,
    out_valve integer not null default 0,
    speed integer not null default 0,
    heater integer not null default 0,
    cleaner integer not null default 0
);

create table if not exists recurring_events (
    id integer primary key,
    start_time text not null default "00:00",
    end_time text not null default "00:00",
    in_valve integer not null default 0,
    out_valve integer not null default 0,
    speed integer not null default 0,
    heater integer not null default 0,
    cleaner integer not null default 0
);
