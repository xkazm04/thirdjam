#!/bin/sh
# lane A162: pro regular, levers the 1v1 drill does not read, each with red_skill=1.07 (build 2 carries 1.13), 600, three workers, one at a time
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 --check closing > /dev/null; }
run K2-pro-reg-base regular pro regular 600 "red_skill=1.07"
run K2-pro-reg-hkp02 regular pro regular 600 "red_skill=1.07;opp.hk_pressure_s=0.2"
run K2-pro-reg-deke regular pro regular 600 "red_skill=1.07;opp.deke=1"
run K2-pro-reg-se45 regular pro regular 600 "red_skill=1.07;opp.shoot_extra_m=45"
run K2-pro-reg-passout regular pro regular 600 "red_skill=1.07;opp.pass_out=1"
