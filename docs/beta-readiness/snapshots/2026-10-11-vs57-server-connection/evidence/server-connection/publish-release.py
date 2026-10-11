from pathlib import Path, PurePosixPath
import hashlib, json, os, stat, sys, zipfile

root=Path('/home/caiden/iron-sun-server')
if root.is_symlink() or root.resolve()!=root: raise SystemExit('Unexpected server root')
if len(sys.argv)!=3: raise SystemExit('Usage: publish-release.py archive.zip expected_sha256')
archive=Path(sys.argv[1]).resolve()
if archive.parent!=(root/'incoming').resolve(): raise SystemExit('Archive must be in the server incoming directory')
with archive.open('rb') as stream: digest=hashlib.file_digest(stream,'sha256').hexdigest()
if digest!=sys.argv[2].lower(): raise SystemExit('Archive checksum mismatch')
destination=root/'releases'/digest[:16]
if destination.exists(): raise SystemExit('Release already exists; preserve it')
with zipfile.ZipFile(archive) as bundle:
    members=bundle.infolist()
    if not members or len(members)>100000 or sum(item.file_size for item in members)>12*1024**3: raise SystemExit('Archive size invalid')
    names=set()
    for item in members:
        name=PurePosixPath(item.filename)
        mode=item.external_attr>>16
        if name.is_absolute() or '..' in name.parts or '\\' in item.filename or stat.S_ISLNK(mode) or not name.parts: raise SystemExit('Unsafe archive member')
        if str(name) in names: raise SystemExit('Duplicate archive member')
        names.add(str(name))
    destination.mkdir(mode=0o700)
    bundle.extractall(destination)
launcher=destination/'TP_FirstPersonServer.sh'
executables=list(destination.glob('*/Binaries/Linux/TP_FirstPersonServer'))
if not launcher.is_file() or len(executables)!=1: raise SystemExit('Dedicated server files are missing; release remains unactivated')
with executables[0].open('rb') as stream:
    if stream.read(4)!=b'\x7fELF': raise SystemExit('Not a native Linux server executable')
metadata=json.loads((destination/'IronSunServer.json').read_text(encoding='utf-8-sig'))
if metadata.get('protocol')!=57 or metadata.get('engine')!='5.8.3' or metadata.get('platform')!='Linux' or metadata.get('target')!='Server': raise SystemExit('Server build metadata mismatch; release remains unactivated')
for file in destination.rglob('*'):
    if file.is_file() and (file.suffix=='.sh' or file==executables[0]): file.chmod(0o755)
current=root/'current'
if current.exists() and not current.is_symlink(): raise SystemExit('Current release path is not a managed symlink')
previous=os.readlink(current) if current.is_symlink() else None
link=root/'current.next'
if link.exists() or link.is_symlink(): raise SystemExit('A pending release pointer exists')
link.symlink_to(destination.relative_to(root),target_is_directory=True)
os.replace(link,current)
result={'release':destination.name,'sha256':digest,'previous_release':previous,'activated_files':True,'process_restarted':False}
(root/'release-state.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result))
