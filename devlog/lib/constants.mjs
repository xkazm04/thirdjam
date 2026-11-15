// The one place that knows when the jam starts. Every hour number in the dev log is
// "hours since H0". Change it here and nowhere else.
//
// H0 = Thursday 2026-11-12 06:00 CET (+01:00; daylight saving ended 2026-10-25).
export const H0_ISO = '2026-11-12T06:00:00+01:00';
export const H0_MS = Date.parse(H0_ISO);

// --project-only (lane A233, 15 Nov): the first instant of this jam's own sessions (the Director's start, Wed 11 Nov 20:40 CET, before H0).
export const PROJECT_FROM_ISO = '2026-11-11T20:40:00+01:00';
export const PROJECT_FROM_MS = Date.parse(PROJECT_FROM_ISO);

// The window is 92 h 59 m long (Thu 06:00 to Mon 02:59 CET). The ruler shows H0..H93.
export const RULER_HOURS = 93;

// Wall-clock offset of the machines' clocks and of screenshot file names (CET, fixed).
export const CLOCK_OFFSET_MIN = 60;
export const CLOCK_LABEL = 'CET';

export const DEVICES = ['A', 'B'];
export const MS_PER_HOUR = 3600000;
