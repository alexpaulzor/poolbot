import sqlite3
from flask import g, Flask, jsonify

DATABASE = 'database.db'
POOL = 0
SPA = 1
MIN = 0
LOW = 1
HIGH = 2
MAX = 3

DEFAULT_RECURRING_EVENTS = [
    dict(
        start_time="04:00",
        end_time="08:00",
        in_valve=POOL,
        out_valve=POOL,
        speed=MAX,
        cleaner=False,
        heater=False,
    ),
    dict(
        start_time="08:00",
        end_time="10:00",
        in_valve=POOL,
        out_valve=POOL,
        speed=MAX,
        cleaner=True,
        heater=False,
    ),
    dict(
        start_time="10:00",
        end_time="16:00",
        in_valve=POOL,
        out_valve=SPA,
        speed=MIN,
        cleaner=False,
        heater=False,
    ),
]

app = Flask(__name__)

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
    cur = get_db().execute(query, args)
    rv = cur.fetchall()
    cur.close()
    return (rv[0] if rv else None) if one else rv

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

@app.route("/reset", methods=['POST'])
def reset():
    init_db()
    query_db("delete from recurring_events")
    for event in DEFAULT_RECURRING_EVENTS:
        query_db(
            "insert into recurring_events "
            "(start_time, end_time, in_valve, out_valve, speed, heater, cleaner) "
            "values (?, ?, ?, ?, ?, ?, ?)", args=(
                event["start_time"],
                event["end_time"],
                event.get("in_valve", POOL),
                event.get("out_valve", POOL),
                event.get("speed", MIN),
                event.get("heater", False),
                event.get("cleaner", False)))
    return jsonify(query_db("select * from recurring_events"));
