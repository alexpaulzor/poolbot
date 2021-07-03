import sqlite3
from flask import g, Flask, jsonify
from datetime import datetime
import logging

DATABASE = 'database.db'
POOL = 0
SPA = 1
MIN = 0
LOW = 1
HIGH = 2
MAX = 3

PIN_STOP = 5
PIN_STEP1 = 6
PIN_STEP2 = 12
PIN_HEATER = 13
PIN_IN_VALVE = 19
PIN_OUT_VALVE = 16
PIN_CLEANER = 26
PIN_VALVE_CURRENT = 20
PIN_FLOW_SWITCH = 21

app = Flask(__name__)
app.logger.setLevel(logging.DEBUG)

# Events use datetimes, but RecurringEvents only have times.
# A "current" Event is one whose start/end encompass now.
# A "current" Event may not yet be "activated," meaning that the system
# has asserted that Event's state on all fronts (valves, pumps, heater).

class Event(dict):
    """
    Events have a start and end datetime, and those time blocks are lazy
    (an overlapping event does not start until the end of the earliest-starting
    event).
    """
    def __init__(
            self, start_date, end_date, id=None, in_valve=POOL, out_valve=POOL, 
            speed=MIN, cleaner=False, heater=False, recurring_source_id=None,
            activated=False):
        self['start_date'] = start_date
        self['end_date'] = end_date
        self['in_valve'] = in_valve
        self['out_valve'] = out_valve
        self['speed'] = speed
        self['cleaner'] = cleaner
        self['heater'] = heater
        self['recurring_source_id'] = recurring_source_id
        self['activated'] = activated
        self['id'] = id

class RecurringEvent(dict):
    """
    RecurringEvents are like Events, except they don't have specific dates,
    only times and skip_days.
    """
    def __init__(self, start_time, end_time, id=None, in_valve=POOL, out_valve=POOL, 
            speed=MIN, cleaner=False, heater=False, skip_days=0):
        self['start_time'] = start_time
        self['end_time'] = end_time
        self['in_valve'] = in_valve
        self['out_valve'] = out_valve
        self['speed'] = speed
        self['cleaner'] = cleaner
        self['heater'] = heater
        self['skip_days'] = skip_days
        self['id'] = id

    def should_skip_today(self):
        if self['skip_days'] == 0:
            return False
        days_since_previous_event = query_db(
            "select julianday('now') - julianday('start_date') "
            "from events where recurring_source_id = ? "
            "order by start_date desc limit 1", one=True)
        if days_since_previous_event is None:
            return False
        return days_since_previous_event > self['skip_days']

DEFAULT_RECURRING_EVENTS = [
    RecurringEvent(
        start_time="04:00",
        end_time="08:00",
        in_valve=POOL,
        out_valve=POOL,
        speed=MAX,
        cleaner=False,
        heater=False,
    ),
    RecurringEvent(
        start_time="08:00",
        end_time="10:00",
        in_valve=POOL,
        out_valve=POOL,
        speed=MAX,
        cleaner=True,
        heater=False,
        skip_days=7,
    ),
    RecurringEvent(
        start_time="10:00",
        end_time="16:00",
        in_valve=POOL,
        out_valve=SPA,
        speed=MIN,
        cleaner=False,
        heater=False,
    ),
    # TODO: remove test events
    RecurringEvent(
        start_time="16:00",
        end_time="22:00",
        in_valve=SPA,
        out_valve=SPA,
        speed=HIGH,
        cleaner=False,
        heater=True,
    ),
    RecurringEvent(
        start_time="22:00",
        end_time="23:59",
        in_valve=POOL,
        out_valve=SPA,
        speed=MIN,
        cleaner=False,
        heater=False,
    ),
    RecurringEvent(
        start_time="00:00",
        end_time="04:00",
        in_valve=POOL,
        out_valve=POOL,
        speed=MIN,
        cleaner=False,
        heater=False,
    ),
]

def get_db():
    db = getattr(g, '_database', None)
    if db is None:
        db = g._database = sqlite3.connect(DATABASE)
        db.row_factory = make_dicts
    return db

def make_dicts(cursor, row):
    return dict((cursor.description[idx][0], value)
                for idx, value in enumerate(row))

def query_db(query, args=(), one=False):
    app.logger.debug("query={}, args={}, one={}".format(query, args, one))
    with app.app_context():
        cur = get_db().execute(query, args)
        rv = cur.fetchall()
        cur.close()
    return (rv[0] if rv else None) if one else rv

def write_db(query, args=(), one=False):
    app.logger.debug("query={}, args={}, one={}".format(query, args, one))
    with app.app_context():
        db = get_db()
        db.execute(query, args)
        db.commit()
    
@app.teardown_appcontext
def close_connection(exception):
    db = getattr(g, '_database', None)
    if db is not None:
        db.close()

def init_db():
    with app.app_context():
        db = get_db()
        with app.open_resource('schema.sql', mode='r') as f:
            db.cursor().executescript(f.read())
        db.commit()


@app.route("/")
def index():
    return "<p>Hello, World!</p>"

@app.route("/v0/reset", methods=['POST'])
def reset():
    app.logger.warning("Resetting DB to hardcoded defaults")
    init_db()
    query_db("delete from recurring_events")
    query_db("delete from events")
    for event in DEFAULT_RECURRING_EVENTS:
        app.logger.info("Inserting default recurring event {}".format(event))
        write_db(
            "insert into recurring_events "
            "(start_time, end_time, in_valve, "
            "out_valve, speed, heater, cleaner, "
            "skip_days) "
            "values (time(?), time(?), ?, ?, ?, ?, ?, ?)", args=(
                event['start_time'],
                event['end_time'],
                event['in_valve'],
                event['out_valve'],
                event['speed'],
                event['heater'],
                event['cleaner'],
                event['skip_days']))
    return jsonify(query_db(
        "select * from recurring_events "
        "order by start_time"))
    

@app.route("/v0/tick", methods=['POST'])
def tick():
    """Main loop, externally triggered every minute"""
    active_event = get_current_event()
    # response = dict(
    #     active=active_event,
    #     next=next_event)
    return jsonify(active_event)


@app.route("/v0/events/current")
def current_event():
    app.logger.info("checking for current events...")
    event = get_current_event()
    return jsonify(event)

@app.route("/v0/events/recurring")
def recurring_events():
    r_events = query_db("select * from recurring_events")
    return jsonify(r_events)

# @app.route("/v0/events/next")
# def active_events():
#     event = get_next_event()
#     return jsonify(event)

def get_current_event(check_recurring=True):
    event = query_db(
        "select * from events "
        "where datetime(start_date) <= datetime('now') "
        "and datetime(end_date) >= datetime('now') "
        "order by datetime(start_date) limit 1", 
        one=True)
    if not event:
        app.logger.info("Found no current event, checking for recurring events")
        r_event = get_current_recurring_event()
        if r_event:
            event = create_event_from_recurring(r_event)
    return event

def get_current_recurring_event():
    r_events = query_db(
        "select * from recurring_events "
        # "where time(start_time) <= time('now') "
        # "and time(end_time) >= time('now') "
        "order by time(start_time)")
    for r_event in r_events:
        app.logger.info("Checking skip_days for {}".format(r_event))
        r_event = RecurringEvent(**r_event)
        if not r_event.should_skip_today():
            app.logger.info("Found current recurring event {}".format(r_event))
            return r_event
    return None

def create_event_from_recurring(r_event):
    write_db(
        "insert into events "
        "(start_date, end_date, in_valve, out_valve, "
        "speed, heater, cleaner, recurring_source_id) "
        "values ("
        "date('now') + time(?), date('now') + time(?), "
        "?, ?, ?, ?, ?, ?)", args=(
            r_event['start_time'],
            r_event['end_time'],
            r_event['in_valve'],
            r_event['out_valve'],
            r_event['speed'],
            r_event['heater'],
            r_event['cleaner'],
            r_event['id']))
    event = query_db(
        "select * from events "
        "where recurring_source_id = ? "
        "order by datetime(start_date) desc limit 1", 
        args=(r_event['id'],), one=True)
    event = Event(**event)
    return event

# def get_next_event():
#     now = datetime.now()
#     now_str = now.strftime("%H:%M")
#     event = query_db(
#         "select * from events "
#         "where start_date >= ? "
#         "order by start_date limit 1", 
#         args=(now_str,), one=True)
#     if not event and lazy_load:
        
#         return get_next_event(False)
#     return event

# def get_recurring_events_after(when="now"):
#     events = query_db(
#         "select * from recurring_events "
#         "where time(start_time) >= time(?) "
#         "order by start_time", args=(when,))
#     return events

# def fill_recurring_events():
#     recurring_events = get_recurring_events_after("now")
#     for event in recurring_events:
#         app.logger.info("Inserting recurring event: {}".format(event))
#         query_db(
#             "insert into events "
#             "(start_date, end_date, in_valve, out_valve, speed, heater, cleaner) "
#             "values (date('now') + time(?), date('now') + time(?), ?, ?, ?, ?, ?)",
#             args=(event['start_time'], event['end_time'], event['in_valve'], event['out_valve'], event['speed'], event['heater'], event['cleaner'])
#         )
