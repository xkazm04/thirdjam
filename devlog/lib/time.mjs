import { H0_MS, MS_PER_HOUR, CLOCK_OFFSET_MIN, CLOCK_LABEL } from './constants.mjs';

const DAYS = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
const pad = (n) => String(n).padStart(2, '0');

/** Hours since H0 for a millisecond timestamp (may be negative or fractional). */
export function hoursSinceH0(ms) {
  return (ms - H0_MS) / MS_PER_HOUR;
}

/** Millisecond timestamp for an hour number since H0. */
export function msAtHour(h) {
  return H0_MS + h * MS_PER_HOUR;
}

/** "Thu 06:00 CET" for a millisecond timestamp. */
export function clockLabel(ms, withZone = true) {
  const d = new Date(ms + CLOCK_OFFSET_MIN * 60000);
  const s = `${DAYS[d.getUTCDay()]} ${pad(d.getUTCHours())}:${pad(d.getUTCMinutes())}`;
  return withZone ? `${s} ${CLOCK_LABEL}` : s;
}

/** "06:42" for a millisecond timestamp, in jam clock time. */
export function minuteLabel(ms) {
  const d = new Date(ms + CLOCK_OFFSET_MIN * 60000);
  return `${pad(d.getUTCHours())}:${pad(d.getUTCMinutes())}`;
}

/** "H12" or "H12.5" style label. */
export function hourLabel(h) {
  const r = Math.round(h * 100) / 100;
  return `H${Number.isInteger(r) ? r : r}`;
}

/** Parse "YYYYMMDD-HHMMSS" as CET wall-clock time. Returns ms or undefined. */
export function parseShotStamp(date, time) {
  const y = +date.slice(0, 4), mo = +date.slice(4, 6), da = +date.slice(6, 8);
  const h = +time.slice(0, 2), mi = +time.slice(2, 4), s = +time.slice(4, 6);
  if (mo < 1 || mo > 12 || da < 1 || da > 31 || h > 23 || mi > 59 || s > 59) return undefined;
  const utc = Date.UTC(y, mo - 1, da, h, mi, s) - CLOCK_OFFSET_MIN * 60000;
  return Number.isNaN(utc) ? undefined : utc;
}

/** The inverse, for tools that write screenshot names: "YYYYMMDD-HHMMSS" in CET. */
export function shotStamp(ms) {
  const d = new Date(ms + CLOCK_OFFSET_MIN * 60000);
  return `${d.getUTCFullYear()}${pad(d.getUTCMonth() + 1)}${pad(d.getUTCDate())}-${pad(d.getUTCHours())}${pad(d.getUTCMinutes())}${pad(d.getUTCSeconds())}`;
}
