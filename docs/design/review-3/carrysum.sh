#!/usr/bin/env bash
# Lane A169 (REVIEW-3): the summary lines of a Bar.Movement.CarryTrace log, cut to what the review quotes. bash carrysum.sh <log>
grep "CARRYTRACE" "$1" | grep -v "CARRYTRACE match" | sed 's/^.*LogAutomationController: //' | sed -E 's/ \| median ([0-9.]+ s) \|.*a pass open +([0-9.]+ %).*the stop on +([0-9.]+ %).*$/ | median \1 | pass open \2, stop on \3/' | sed -E 's/(frames each) \| closing ([0-9.]+ m\/s) \(10 or more in +([0-9.]+ %)\).*the puck his for ([0-9.]+ s).*$/\1 | closing \2, 10+ in \3 | puck his \4/'
