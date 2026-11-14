#!/bin/sh
# lane A162: pro regular, levers the 1v1 drill does not read (600, two workers) beside law 3 over the same tunes (one process): three processes
cd "$(dirname "$0")"
sh law3.sh law3-3 "pro:opp.hk_pressure_s=0.2|pro:opp.deke=1|pro:opp.shoot_extra_m=45|pro:opp.pass_out=1" &
run() { node curve.mjs "$@" --workers 2 --check closing > /dev/null; }
run K-pro-reg-hkp02 regular pro regular 600 "opp.hk_pressure_s=0.2"
run K-pro-reg-deke regular pro regular 600 "opp.deke=1"
run K-pro-reg-se45 regular pro regular 600 "opp.shoot_extra_m=45"
run K-pro-reg-passout regular pro regular 600 "opp.pass_out=1"
wait
