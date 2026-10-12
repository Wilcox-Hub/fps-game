from pathlib import Path
import copy
import hashlib
import json
import shutil

task = Path(__file__).parent
workspace = task.parents[1]
records = workspace / 'research/launcher-delivery-20261012/readiness-publication/docs/beta-readiness'
game = Path('C:/Users/YaYa/Documents/Codex/2026-09-22/connect-this-github-repo-to-a')
sid = '2026-10-12-launcher-smartscreen'
read = lambda p: json.loads(p.read_text(encoding='utf-8-sig'))

def sha(p):
    with p.open('rb') as f:
        return hashlib.file_digest(f, 'sha256').hexdigest().upper()

def write(p, value):
    p.write_text(json.dumps(value, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')

prior = read(records / 'latest.json')
assert prior['snapshot_id'] == '2026-10-12-launcher-public-alpha', 'Preserve any newer assessment.'
facts = read(task / 'publisher-inspection.json')
owner = read(task / 'user-report.json')
check = read(task / 'game-input-check.json')
assert facts['setup']['matches_published']
assert facts['setup']['signature_status'] == 'NotSigned'
assert facts['installed_launcher']['signature_status'] == 'NotSigned'
assert check['reusable_input_evidence'] and check['observed_input_count'] == 607
assert not owner['first_run_accepted']
snapshot = records / 'snapshots' / sid
assert not snapshot.exists(), 'Never overwrite a dated snapshot.'
evidence = snapshot / 'evidence'
evidence.mkdir(parents=True)
(snapshot / '.gitattributes').write_text('* -text\n', encoding='utf-8')
inputs = read(records / prior['input_manifest'])
for item in inputs:
    assert sha(game / 'unreal/Ridgefire' / item['path']) == item['sha256']
write(snapshot / 'input-manifest.json', inputs)
write(evidence / 'prior-latest.json', prior)
for name in ['publisher-inspection.json', 'user-report.json', 'game-input-check.json', 'signing-options.md', 'Record-Smartscreen.py']:
    shutil.copyfile(task / name, evidence / name)

write(snapshot / 'test-results.json', {
    'recorded_on': '2026-10-12',
    'assessment_kind': 'Owner-reported first-run failure with exact artifact identity and signature inspection',
    'game_inputs_exact_match': True,
    'game_input_count': 607,
    'game_tests_repeated': False,
    'artifact_identity_and_signature_inspection': facts,
    'owner_windows_first_run': owner,
    'smartscreen_first_run_accepted': False,
    'public_trusted_publisher_accepted': False,
    'previous_defender_scan_reused': {
        'snapshot': prior['snapshot_id'],
        'recorded_on': '2026-10-12',
        'scope': 'Same setup/native byte identities; historical clean scan only. Does not prove safety or SmartScreen reputation.'
    },
    'fix_implemented': False,
    'code_signing_account_created': False,
    'paid_code_signing_approved': False,
    'windows_protection_changed': False,
    'scope': 'No automated security UI repro, new installer build, gameplay tests or measured score credit. Signing still requires approved paid onboarding, identity verification, signed new release and actual first-run validation.'
})
write(snapshot / 'visual-check.json', {
    'recorded_on': '2026-10-12',
    'source': 'Owner supplied warning text, not agent screenshot',
    'publisher_displayed': 'Unknown publisher',
    'smartscreen_first_run_accepted': False,
    'agent_rendered_ui_accepted': False,
    'scope': 'No new website/native/game visual review or automated interaction with Windows security.'
})
(snapshot / 'report.md').write_text('''# IRON SUN launcher SmartScreen first run — October 12, 2026

The owner reports that Microsoft Defender SmartScreen prevented IRON-SUN-Setup (2).exe from starting, showing Unknown publisher. This changes the pending Windows first-run check to an owner-reported failure. Public internal-alpha downloads remain available, but a smooth protected friend-PC installation is not accepted.

The exact downloaded file is 4,025,552 bytes and SHA256 62C789A0ABD26F2CA127635AD61FDE7B41537AAD31C4C29A2D54FF754464C9F8, matching the immutable published setup. Get-AuthenticodeSignature reports NotSigned, with no publisher. The installed launcher also reports NotSigned and matches native1.0.1 SHA256 C293A7D60887B2D719C753E25E4AE8083A4299513A55FE9C23AD2249AF6378B4. A certificate-store inspection found no code-signing certificate in the CurrentUser/My or LocalMachine/My stores at inspection time; that is a limited local inventory, not a survey of all possible external signing identities.

The prior exact-byte Defender scans and analyst removal of the malware detection retain their recorded scope. The last portal observation still had In progress / Pending fields. No fresh portal check or malware scan was necessary for this unchanged artifact. Microsoft explicitly treats SmartScreen reputation separately from Defender Antivirus. The RSA signatures already on our release feeds protect updater integrity; they do not establish a Windows Authenticode publisher. See the cited signing-options evidence for official Microsoft guidance.

The intended remedy is a legitimately validated public publisher identity and a new signed release. Research recommends Azure Artifact Signing Basic for an eligible US/Canada individual, currently listed at US$9.99/month for 5,000 signatures. A paid Azure subscription, matching legal identity and mobile/government-ID verification are required. Recurring spending and legal identity onboarding have not been approved or completed. Signing would establish a validated publisher but cannot guarantee immediate SmartScreen reputation or an initially warning-free launch. No certificate, account, paid resource or purchase was created.

The release plan must sign the launcher, Inno setup and uninstaller with SHA256 and timestamping; bump the launcher's three version declarations, record final artifact hashes after signing, publish a new immutable tag, and sign new updater envelopes. Existing self-update byte/hash handling can carry signed payloads. Final signature, install/uninstall/save retention, self-update and actual protected friend-PC first-run checks remain required. The existing unsigned artifacts must not be silently replaced.

Diagnosis scope: the owner captured the real Windows symptom; the agent verified exact bytes and absent Authenticode signatures through read-only commands. SmartScreen reputation itself was not reproduced unattended. The Windows security UI cannot be automated in this workflow; a signature check is not a substitute for that first-run result. The platform/UI reproduction and green-validation phases are deferred for that reason. No speculative game changes, bypasses, exclusion, trust-root installation or Windows protection changes were made.

All 607 current game input hashes and the project descriptor still match. No engine/gameplay tests were repeated or new fun/beta credit awarded. The October 4 low-confidence 69% gameplay estimate, open 75% target and NOT_BETA_READY verdict remain. Original snapshots remain unchanged.
''', encoding='utf-8')
latest = copy.deepcopy(prior)
latest.update({
    'snapshot_id': sid,
    'previous_snapshot': prior['snapshot_id'],
    'previous_test_snapshot': prior['snapshot_id'],
    'report': f'snapshots/{sid}/report.md',
    'test_results': f'snapshots/{sid}/test-results.json',
    'visual_check': f'snapshots/{sid}/visual-check.json',
    'input_manifest': f'snapshots/{sid}/input-manifest.json',
    'assessment_kind': 'Owner-reported SmartScreen first-run failure; exact current installer and launcher unsigned',
})
latest['internet_launcher'].update({
    'status': 'PUBLIC_INTERNAL_ALPHA_UNSIGNED; OWNER_REPORTED_SMARTSCREEN_WARNING',
    'smartscreen_first_run_status': 'OWNER_REPORTED_UNRECOGNIZED_APP',
    'smartscreen_first_run_accepted': False,
    'windows_publisher_signature_status': 'NotSigned',
    'trusted_publisher_accepted': False,
    'friend_pc_accepted': False,
    'smartscreen_evidence': f'snapshots/{sid}/evidence/user-report.json',
    'publisher_signature_evidence': f'snapshots/{sid}/evidence/publisher-inspection.json',
    'publisher_signing_next_step': 'Approve paid verified publisher onboarding, then sign a new immutable release and validate actual protected first run. Valid signatures do not guarantee initial reputation.',
    'live_download_install_scope': 'Prior protected CLI production install/open/uninstall/reinstall and live backend checks retain scope; owner now reports actual browser-downloaded setup blocked by SmartScreen. Smooth normal friend-PC first run is not accepted.',
})
write(records / 'latest.json', latest)
overview = records / 'README.md'
text = overview.read_text(encoding='utf-8-sig')
heading, rest = text.split('\n', 1)
overview.write_text(heading + '\n\nOctober12 Windows first run: **owner reports SmartScreen blocking the current setup with Unknown publisher.** The exact downloaded installer matches the published hash; installer and launcher are unsigned. Previous clean Defender scans do not grant publisher trust or clear SmartScreen. Public internal alpha remains available; smooth friend-PC delivery is unaccepted. Verified signing requires approved paid identity onboarding and a new release, followed by actual first-run checks. No protection changes, gameplay retesting or score credit. See [SmartScreen assessment](snapshots/' + sid + '/report.md).\n\n' + rest.lstrip('\r\n'), encoding='utf-8')
write(snapshot / 'evidence-manifest.json', {
    'files': [{'path': p.relative_to(snapshot).as_posix(), 'sha256': sha(p), 'size_bytes': p.stat().st_size}
              for p in sorted(snapshot.rglob('*')) if p.is_file() and p.name != 'evidence-manifest.json']
})
print(json.dumps({'prepared': True, 'snapshot': sid, 'inputs_verified': len(inputs),
                  'signature_status': facts['setup']['signature_status'],
                  'owner_first_run_accepted': False, 'game_tests_repeated': False}))
