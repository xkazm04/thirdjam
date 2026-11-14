#!/bin/sh
# lane A162: the two trims at 2,000 (tunes on build 1; a tune overrides the large offset), then law 3 with pro's candidate, then the fallback candidates
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 --check closing > /dev/null; }
run G-rk-large-red114 novice rookie large 2000 "red_skill=1.14"
run G-pro-reg-red109 regular pro regular 2000 "red_skill=1.09"
sh law3.sh law3-1 "pro:-|pro:red_skill=1.09|all_star:-"
run G-pro-reg-red110 regular pro regular 2000 "red_skill=1.10"
run G-rk-large-red1135 novice rookie large 2000 "red_skill=1.135"
