

create table if not exists recurring_events (
    id integer primary key,
    start_time text not null,
    end_time text not null,
    in_valve integer not null default 0,
    out_valve integer not null default 0,
    speed integer not null default 0,
    heater integer not null default 0,
    cleaner integer not null default 0,
    skip_days integer not null default 0
);

create table if not exists events (
    id integer primary key,
    start_date text not null,
    end_date text not null,
    in_valve integer not null default 0,
    out_valve integer not null default 0,
    speed integer not null default 0,
    heater integer not null default 0,
    cleaner integer not null default 0,
    recurring_source_id integer default null 
        references recurring_events (id) on delete set null,
    activated integer not null default 0
);
