#!/bin/bash -ex

rm -f database.db

flask run &
PID=$!
sleep 1
curl -f -X POST localhost:5000/v0/reset \
    && curl -f localhost:5000/v0/events/recurring \
    && curl -f localhost:5000/v0/events/current \
    && curl -f -X POST localhost:5000/v0/tick \
    && curl -f localhost:5000/v0/events/current \
    || echo quit with rc=$?

kill $PID
