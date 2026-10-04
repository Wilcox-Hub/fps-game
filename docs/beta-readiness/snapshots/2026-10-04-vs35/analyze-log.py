"""Summarize this owner's Editor session; never infer shot success from input logs."""
import collections
import datetime as dt
import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
LOG = Path('C:/Users/YaYa/Documents/Codex/2026-09-22/connect-this-github-repo-to-a/unreal/Ridgefire/Saved/Logs/Ridgefire.log')
raw = LOG.read_bytes()
lines = raw.decode('utf-8', errors='replace').splitlines()
pattern = re.compile(r'^\[(\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}:\d{3})\]\[\s*(\d+)\]')
events = []
for line in lines:
    match = pattern.match(line)
    if match and 'LogTemp:' in line and 'RIDGEFIRE' in line:
        events.append((dt.datetime.strptime(match[1], '%Y.%m.%d-%H.%M.%S:%f'), match[2], line))
start = dt.datetime(2026, 10, 4, 20, 23, 55)
end = dt.datetime(2026, 10, 4, 20, 31, 43)
events = [event for event in events if start <= event[0] <= end]
waves, deaths, fire_inputs, clusters = [], [], [], []
by_frame = collections.defaultdict(list)
for event in events:
    timestamp, frame, line = event
    wave = re.search(r'Starting wave (\d+)\.', line)
    if wave:
        waves.append({'wave': int(wave[1]), 'started_utc': timestamp.isoformat(), 'deaths': 0})
    if 'mechanical death hides inherited skeletal mesh and ragdoll:' in line:
        deaths.append(event)
        if waves:
            waves[-1]['deaths'] += 1
            waves[-1]['last_death_utc'] = timestamp.isoformat()
    if 'Fire input received for ' in line:
        name = line.split('Fire input received for ', 1)[1].split(' (', 1)[0]
        ammo = int(re.search(r'\((\d+) rounds', line)[1])
        fire_inputs.append({'timestamp_utc': timestamp.isoformat(), 'weapon': name, 'ammo_before_input': ammo})
    # Frame counter wraps. Timestamp second plus frame makes a local session key.
    by_frame[(timestamp.replace(microsecond=0), frame)].append(event)
for group in by_frame.values():
    fallen = [e for e in group if 'mechanical death hides inherited skeletal mesh and ragdoll:' in e[2]]
    inputs = [e for e in group if 'Fire input received for ' in e[2]]
    if len(fallen) >= 2 and inputs:
        clusters.append({'timestamp_utc': inputs[0][0].isoformat(), 'weapon': inputs[0][2].split('Fire input received for ', 1)[1].split(' (', 1)[0], 'same_frame_death_count': len(fallen), 'actors': [e[2].rsplit(': ', 1)[1] for e in fallen]})
for wave in waves:
    if 'last_death_utc' in wave:
        wave['start_to_last_recorded_death_seconds'] = round((dt.datetime.fromisoformat(wave['last_death_utc']) - dt.datetime.fromisoformat(wave['started_utc'])).total_seconds(), 3)
arena_clear = next(e[0] for e in events if 'RIDGEFIRE RUN arena_cleared=0 ' in e[2])
result = {
    'scope': 'Matching owner Editor session log, 2026-10-04. Input events are not successful-shot counts. Same-frame deaths corroborate clustering but do not establish hitbox cause, damage radius, or intent.',
    'log_sha256': hashlib.sha256(raw).hexdigest().upper(),
    'session_window_utc': [start.isoformat(), end.isoformat()],
    'first_arena_start_to_clear_seconds': round((arena_clear - dt.datetime.fromisoformat(waves[0]['started_utc'])).total_seconds(), 3),
    'waves_started': len(waves),
    'waves': waves,
    'mechanical_deaths_logged': len(deaths),
    'fire_input_events': len(fire_inputs),
    'fire_inputs_by_weapon': dict(collections.Counter(e['weapon'] for e in fire_inputs)),
    'empty_magazine_fire_inputs': sum(e['ammo_before_input'] == 0 for e in fire_inputs),
    'same_frame_multi_death_groups_with_fire_input': clusters,
    'multi_death_group_count': len(clusters),
    'maximum_same_frame_deaths_with_fire_input': max((e['same_frame_death_count'] for e in clusters), default=0),
    'limitations': ['No shot/hit telemetry or hardware key log.', 'UTC log timestamps are not asserted to align exactly to video presentation timestamps.', 'One solo Editor run, ending during the second arena; not a complete run or packaged/co-op acceptance.', 'No cause or fix validated.'],
}
(ROOT / 'log-metrics.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
(ROOT / 'gameplay-events.log').write_text('\n'.join(e[2] for e in events) + '\n', encoding='utf-8')
print(json.dumps({key: result[key] for key in ['first_arena_start_to_clear_seconds', 'waves_started', 'mechanical_deaths_logged', 'fire_input_events', 'fire_inputs_by_weapon', 'empty_magazine_fire_inputs', 'multi_death_group_count', 'maximum_same_frame_deaths_with_fire_input']}, indent=2))
