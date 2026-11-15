#!/usr/bin/env bash
# Lane A219: the before / after sheets of docs/experiments/bar from the film runs under tools/qa/eye2/out (see BAR.md).
#   bash tools/qa/eye2/make-pairs.sh <after tag for clay 720> <after tag for the other looks>      e.g.  a3 a2
# Before: b-<look>-<size> (package H39's Development twin; clay 720: h39-clay-hand). A missing run is skipped with a line.
set -u
cd "$(dirname "$0")/../../.."
T=tools/qa/eye2/out; O=docs/experiments/bar; P="python tools/qa/eye2/pair.py"
A1=${1:-a3}; A2=${2:-a2}
pair() { # before-run after-run out from to step cols w crop prefix title
  if [ ! -d "$T/$1/hits" ] || [ ! -d "$T/$2/hits" ]; then echo "SKIP $3 ($1 or $2 missing)"; return; fi
  $P "$T/$1/hits" "$T/$2/hits" "$O/$3" "$4" "$5" "$6" "cols=$7" "w=$8" "crop=$9" "prefix=${10}" "title=${11} BEFORE = package H39 (yellow), AFTER = lane A219's package (green). Same sim frames." bl=H39 al=A219
}
C=280,120,1000,660; G=200,60,1080,720
pair h39-clay-hand $A1-clay-720  pair-01-clay-720-tap-f391-427.jpg    391 427 3 7 272 $C a196-cross-000391 "CLAY 1280x720, LMB tapped (press f391, lands f399)."
pair h39-clay-hand $A1-clay-720  pair-02-clay-720-hold-f430-518.jpg   430 518 8 6 320 $C a196-brace-000430 "CLAY 1280x720, LMB HELD (braced f403-490, the braced hit f490, his pass f503)."
pair b-clay-1080 $A2-clay-1080   pair-03-clay-1080-tap-f391-427.jpg   391 427 3 7 272 $C a196-cross-000391 "CLAY 1920x1080, LMB tapped (press f391, lands f399)."
pair b-clay-1080 $A2-clay-1080   pair-04-clay-1080-hold-f430-518.jpg  430 518 8 6 320 $C a196-brace-000430 "CLAY 1920x1080, LMB HELD (braced f403-490, the braced hit f490)."
pair b-night-720-d2 $A2-night-720-d2    pair-05-night-720-tap-f920-956.jpg   920 956 3 7 272 $G a196-cross-000920 "NIGHT 1280x720, LMB tapped (the drill, press f920)."
pair b-night-720-d23 $A2-night-720-d23  pair-06-night-720-hold-f130-286.jpg  130 286 12 7 272 $G a196-brace-000130 "NIGHT 1280x720, LMB HELD (the drill, braced from f130)."
pair b-night-1080-d2 $A2-night-1080-d2   pair-07-night-1080-tap-f920-956.jpg  920 956 3 7 272 $G a196-cross-000920 "NIGHT 1920x1080, LMB tapped (the drill, press f920)."
pair b-night-1080-d23 $A2-night-1080-d23 pair-08-night-1080-hold-f130-286.jpg 130 286 12 7 272 $G a196-brace-000130 "NIGHT 1920x1080, LMB HELD (the drill, braced from f130)."
pair b-glow-720 $A2-glow-720     pair-09-glow-720-tap-f391-427.jpg    391 427 3 7 272 $G a196-cross-000391 "GLOW ICE 1280x720, LMB tapped (press f391, lands f399)."
pair b-glow-720 $A2-glow-720     pair-10-glow-720-hold-f6604-6756.jpg 6604 6756 14 6 320 $G a196-brace-006604 "GLOW ICE 1280x720, LMB HELD (braced f6604-6729)."
pair b-glow-1080 $A2-glow-1080   pair-11-glow-1080-tap-f391-427.jpg   391 427 3 7 272 $G a196-cross-000391 "GLOW ICE 1920x1080, LMB tapped (press f391, lands f399)."
pair b-glow-1080 $A2-glow-1080   pair-12-glow-1080-hold-f6604-6756.jpg 6604 6756 14 6 320 $G a196-brace-006604 "GLOW ICE 1920x1080, LMB HELD (braced f6604-6729)."
pair h39-clay-hand $A1-clay-720  pair-13-clay-720-tap-that-floors-him-f1274-1304.jpg 1274 1304 3 6 320 240,100,1040,700 a196-cross-001274 "CLAY 1280x720, LMB tapped, the man goes down (f1274)."
python tools/qa/eye2/hashes.py $T/h39-clay-hand $T/$A1-clay-720 $T/b-clay-1080 $T/$A2-clay-1080 $T/b-night-720-d2 $T/$A2-night-720-d2 $T/b-night-720-d23 $T/$A2-night-720-d23 \
  $T/b-night-1080-d2 $T/$A2-night-1080-d2 $T/b-night-1080-d23 $T/$A2-night-1080-d23 $T/b-glow-720 $T/$A2-glow-720 $T/b-glow-1080 $T/$A2-glow-1080
