# summ.sh <name>: one line per take from verify.json
f=C:/Users/kazda/jam/skater-lab/skh/logs/verify_$1.log
grep "^TAKE" $f | sed -E 's/"(pelvis_xy_cm|root_keyed|object_keyed|lowest_body_cm|frames)": [^,]*(, [^"]*)?, //g' | sed 's/TAKE //' | cut -c1-330
