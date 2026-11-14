#!/bin/sh
# lane A162: pro regular red candidates at 2,000 (two workers) beside law 3 over every candidate (one process): three processes
cd "$(dirname "$0")"
sh law3.sh law3-2 "pro:-|pro:red_skill=1.13|pro:red_skill=1.15|all_star:-" &
node curve.mjs H-pro-reg-red113 regular pro regular 2000 "red_skill=1.13" --workers 2 --check closing > /dev/null
node curve.mjs H-pro-reg-red115 regular pro regular 2000 "red_skill=1.15" --workers 2 --check closing > /dev/null
wait
