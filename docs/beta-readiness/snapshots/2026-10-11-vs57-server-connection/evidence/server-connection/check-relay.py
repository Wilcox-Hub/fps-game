"""One-shot nonce probe of the approved UDP game tunnel; no game acceptance claim."""
from pathlib import Path
import datetime, json, secrets, socket, subprocess, time

folder = Path(__file__).parent
nonce = ('IRON-SUN-PROBE-' + secrets.token_hex(24)).encode()
remote = f'''
import socket, time
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(('127.0.0.1',7777))
s.settimeout(1)
deadline = time.monotonic()+35
expected = {nonce!r}
print('READY',flush=True)
while time.monotonic()<deadline:
    try: data, peer = s.recvfrom(128)
    except socket.timeout: continue
    if data == expected:
        s.sendto(data,peer)
        print('MATCHED',flush=True)
        break
s.close()
'''
worker = subprocess.Popen([
    r'C:\Windows\System32\OpenSSH\ssh.exe','-T','-oBatchMode=yes',
    '-oClearAllForwardings=yes','-oConnectTimeout=8','-oStrictHostKeyChecking=yes',
    'sidequest','python3 -u -'
],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
worker.stdin.write(remote)
worker.stdin.close()
udp = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
udp.settimeout(3)
result = {'passed':False,'time_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
          'endpoint':'cairo-ferry.tun.ply.gg:42527','origin':'127.0.0.1:7777',
          'scope':'Exact one-shot nonce UDP round trip through Playit to Lenovo. Windows test client is on the owner home network; no game, external-Wi-Fi, latency or performance acceptance.'}
try:
    if worker.stdout.readline().strip() != 'READY':
        raise RuntimeError('Remote probe did not bind; an existing service may own port 7777')
    target = (socket.gethostbyname('cairo-ferry.tun.ply.gg'),42527)
    for attempt in range(6):
        started = time.monotonic()
        udp.sendto(nonce,target)
        try: data, peer = udp.recvfrom(128)
        except socket.timeout: continue
        if data != nonce or peer != target: continue
        result.update(passed=True,attempt=attempt+1,probe_roundtrip_ms=round((time.monotonic()-started)*1000,1))
        break
    worker.wait(timeout=40)
    result['remote_matched'] = 'MATCHED' in worker.stdout.read()
    result['remote_exit_code'] = worker.returncode
    if not result['remote_matched'] or worker.returncode: result['passed'] = False
finally:
    udp.close()
    if worker.poll() is None:
        worker.wait(timeout=40)
    (folder/'relay-roundtrip-result.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
print(json.dumps(result,indent=2))
if not result['passed']: raise SystemExit(1)
