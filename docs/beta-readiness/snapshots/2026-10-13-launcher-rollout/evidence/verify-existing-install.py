from pathlib import Path
import datetime
import hashlib
import json
import shutil
import subprocess
import tempfile

support = Path(__file__).resolve().parent
old = Path('C:/Users/YaYa/Documents/ChatGPT/FPS-1/research/launcher-delivery-20261012/fresh-live-install')
source = Path('C:/Users/YaYa/Documents/ChatGPT/FPS-1/distribution/iron-sun-launcher')
repo = Path('C:/Users/YaYa/Documents/Codex/2026-09-22/connect-this-github-repo-to-a')
run = support / ('existing-install-' + datetime.datetime.now().strftime('%Y%m%d-%H%M%S'))
# Classic .NET Framework file APIs need a short installation path. The support
# folder is deeply nested; a normal per-user installation is much shorter.
root = Path(tempfile.gettempdir()) / ('is-rollout-' + datetime.datetime.now().strftime('%H%M%S'))

def sha(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest().upper()

def verify_files(directory, record):
    for entry in record['files']:
        file = directory / entry['path']
        assert file.stat().st_size == entry['size'], entry['path']
        assert sha(file) == entry['sha256'].upper(), entry['path']
    return len(record['files'])

def call(action):
    process = subprocess.run(
        [str(exe), '--install-root', str(root), '--no-shortcut', '--update-test', action],
        capture_output=True, text=True, timeout=300,
    )
    if process.returncode:
        raise RuntimeError(process.stderr or process.stdout or f'Launcher exit {process.returncode}')
    return json.loads(process.stdout)

run.mkdir(parents=True, exist_ok=False)
feeds_process = subprocess.run(
    ['powershell.exe', '-NoProfile', '-NonInteractive', '-File', str(support / 'check-public-feeds.ps1')],
    capture_output=True, text=True, timeout=150,
)
assert feeds_process.returncode == 0, feeds_process.stderr
feeds = json.loads(feeds_process.stdout.lstrip('\ufeff'))
(run / 'public-feeds.json').write_text(json.dumps(feeds, indent=2), encoding='utf-8')

inputs = json.loads((repo / 'docs/beta-readiness/snapshots/2026-10-12-launcher-public-alpha/launcher-input-manifest.json').read_text(encoding='utf-8-sig'))
for entry in inputs['files']:
    assert sha(source / entry['path']) == entry['sha256'], entry['path']
old_record = json.loads((old / 'Game/installed.json').read_text(encoding='utf-8-sig'))
assert old_record['protocol'] == 60 and old_record['version'] == '20261012-034847-088'
old_count = verify_files(old / 'Game', old_record)
exe = old / 'Launcher/IronSunLauncher.exe'
binary_hash = sha(exe)
assert binary_hash == 'C293A7D60887B2D719C753E25E4AE8083A4299513A55FE9C23AD2249AF6378B4'
assert binary_hash == feeds['launcher']['sha256'].upper()

# Exercise an existing installation without changing the owner's install or old evidence.
root.mkdir()
(root / 'managed-install.json').write_text(json.dumps({
    'app_id': 'IRON-SUN-LAN', 'root': str(root), 'schema_version': 1,
}), encoding='utf-8')
shutil.copytree(old / 'Game', root / 'Game')
saves = root / 'Saves'
saves.mkdir()
sentinel = saves / 'rollout-preservation-control.bin'
sentinel.write_bytes(b'IRON SUN updater preservation control; not a playable Unreal save.\n')
sentinel_hash = sha(sentinel)
check = call('Check')
assert check['protocol'] == 61 and check['version'] == feeds['game']['version'], check
print('Existing VS60 install detected the live VS61 release; downloading and upgrading.', flush=True)
update = call('Update')
assert update['status'] == 'UPDATED' and update['protocol'] == 61, update
new_record = json.loads((root / 'Game/installed.json').read_text(encoding='utf-8-sig'))
assert new_record['version'] == feeds['game']['version']
new_count = verify_files(root / 'Game', new_record)
assert (root / 'Launcher/IronSunLauncher.exe').is_file()
assert sha(root / 'Launcher/IronSunLauncher.exe') == binary_hash
assert sha(sentinel) == sentinel_hash
assert json.loads((root / 'Previous/installed.json').read_text(encoding='utf-8-sig'))['version'] == old_record['version']
previous_count = verify_files(root / 'Previous', old_record)
current = call('Update')
assert current['status'] == 'CURRENT' and current['protocol'] == 61, current

result = {
    'passed': True,
    'tested_utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
    'from_game': old_record['version'], 'from_protocol': 60,
    'to_game': new_record['version'], 'to_protocol': 61,
    'launcher_version': '1.0.1', 'launcher_sha256': binary_hash,
    'isolated_install_root': str(root),
    'launcher_binary_unchanged': True, 'installer_rerun_required': False,
    'signed_public_feeds_verified': True,
    'source_inputs_verified': len(inputs['files']),
    'previous_install_files_verified': old_count,
    'updated_install_files_verified': new_count,
    'rollback_files_verified': previous_count,
    'root_save_control_preserved': True,
    'check': check, 'update': update, 'repeat_update': current,
    'scope': 'Unchanged production launcher backend, actual HTTPS upgrade of a copy of the earlier complete VS60 install to VS61, all 32 current and previous file hashes. Source inspection confirms automatic startup and ten-minute idle game updates. No new launcher binary, normal GUI acceptance, actual Unreal saved-run test, friend-PC acknowledgement or closed-app background service claim.',
}
(run / 'result.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
(support / 'latest-run.json').write_text(json.dumps({'run': str(run)}, indent=2), encoding='utf-8')
print(json.dumps(result), flush=True)
