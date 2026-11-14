#!/bin/sh
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 > /dev/null; }
run T1-pro-reg-full regular pro regular 600 "opp.sprint_share=0.7;opp.lift_cooldown_frames=45"
run T1-pro-large-full regular pro large 600 "opp.sprint_share=0.7;opp.lift_cooldown_frames=45"
