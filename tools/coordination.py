"""Cross-harness coordination: address/file leases and a global Ghidra write lock.

Several agents (Codex, Claude, workers in git worktrees) share one Ghidra project and one
history. Leases say who is working on which addresses and files; the Ghidra lock serializes
mutations of the shared project. Both live OUTSIDE any worktree so every checkout sees the
same registry: `coordination_dir` in config/target.json, else $BSP_COORDINATION_DIR, else
~/.bsp/. Owner identity is $BSP_AGENT, else the current git branch (worktrees use agent/<name>).

  leases.jsonl  one lease per line: id, owner, packet, addresses, ranges, files, claimed,
                expires, status (active|released), worktree
  ghidra.lock   JSON {owner, pid, claimed, expires}; created O_EXCL, expired locks are reclaimed
"""
import json
import os
import socket
import subprocess
import time
from datetime import datetime, timedelta, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class LeaseConflict(RuntimeError):
    pass


def coordination_dir():
    env = os.environ.get('BSP_COORDINATION_DIR')
    if env:
        return Path(env)
    config = ROOT / 'config/target.json'
    if config.exists():
        configured = json.loads(config.read_text(encoding='utf-8')).get('coordination_dir')
        if configured:
            return Path(configured)
    return Path.home() / '.bsp'


def leases_path():
    return coordination_dir() / 'leases.jsonl'


def lock_path():
    return coordination_dir() / 'ghidra.lock'


def now():
    return datetime.now(timezone.utc)


def iso(moment):
    return moment.isoformat(timespec='seconds')


def parse(text):
    return datetime.fromisoformat(text)


def owner_name():
    env = os.environ.get('BSP_AGENT')
    if env:
        return env
    try:
        branch = subprocess.run(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], cwd=ROOT,
                                capture_output=True, text=True).stdout.strip()
    except OSError:
        branch = ''
    return branch or socket.gethostname()


# ---------------------------------------------------------------- leases

def load_leases():
    path = leases_path()
    if not path.exists():
        return []
    rows = []
    with path.open(encoding='utf-8') as handle:
        for line in handle:
            if line.strip():
                rows.append(json.loads(line))
    return rows


def save_leases(rows):
    path = leases_path()
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_suffix('.tmp')
    with tmp.open('w', encoding='utf-8', newline='\n') as handle:
        for row in rows:
            handle.write(json.dumps(row) + '\n')
    os.replace(tmp, path)


def active_leases(rows=None):
    rows = load_leases() if rows is None else rows
    current = now()
    return [r for r in rows if r.get('status') == 'active' and parse(r['expires']) > current]


def parse_ranges(texts):
    ranges = []
    for text in texts or []:
        if isinstance(text, dict):
            lo, hi = text.get('start'), text.get('end')
        else:
            lo, hi = str(text).replace('..', '-').split('-', 1)
        ranges.append([f'{int(lo, 16):08x}', f'{int(hi, 16):08x}'])
    return ranges


def covers(lease, address):
    """True when an integer address is inside the lease's explicit addresses or ranges (end inclusive)."""
    if any(int(a, 16) == address for a in lease.get('addresses', [])):
        return True
    return any(int(lo, 16) <= address <= int(hi, 16) for lo, hi in lease.get('ranges', []))


def overlap_detail(a, b):
    """Human-readable description of what two leases share, or '' when they do not overlap."""
    parts = []
    files = sorted(set(a.get('files', [])) & set(b.get('files', [])))
    if files:
        parts.append('files ' + ' '.join(files[:6]) + (' ...' if len(files) > 6 else ''))
    addrs = sorted({addr for addr in a.get('addresses', []) if covers(b, int(addr, 16))}
                   | {addr for addr in b.get('addresses', []) if covers(a, int(addr, 16))})
    if addrs:
        parts.append('addresses ' + ' '.join(addrs[:8]) + (' ...' if len(addrs) > 8 else ''))
    ranges = [f'{lo}-{hi}' for lo, hi in a.get('ranges', []) for lo2, hi2 in b.get('ranges', [])
              if int(lo, 16) <= int(hi2, 16) and int(lo2, 16) <= int(hi, 16)]
    if ranges:
        parts.append('ranges ' + ' '.join(ranges[:4]) + (' ...' if len(ranges) > 4 else ''))
    return '; '.join(parts)


def overlaps(a, b):
    if set(a.get('files', [])) & set(b.get('files', [])):
        return True
    for addr in a.get('addresses', []):
        if covers(b, int(addr, 16)):
            return True
    for addr in b.get('addresses', []):
        if covers(a, int(addr, 16)):
            return True
    for lo, hi in a.get('ranges', []):
        for lo2, hi2 in b.get('ranges', []):
            if int(lo, 16) <= int(hi2, 16) and int(lo2, 16) <= int(hi, 16):
                return True
    return False


def claim(packet, addresses=(), ranges=(), files=(), ttl_hours=8.0, owner=None, note=''):
    """Create or extend the (owner, packet) lease; refuse when it overlaps another owner's active lease."""
    owner = owner or owner_name()
    wanted = {'id': f'{owner}:{packet}', 'owner': owner, 'packet': packet,
              'addresses': sorted({f'{int(a, 16):08x}' for a in addresses}),
              'ranges': parse_ranges(ranges), 'files': sorted(set(files)),
              'claimed': iso(now()), 'expires': iso(now() + timedelta(hours=ttl_hours)),
              'status': 'active', 'worktree': str(ROOT), 'note': note}
    rows = load_leases()
    for row in active_leases(rows):
        if row['id'] == wanted['id']:
            # extending our own lease: merge the coverage
            wanted['addresses'] = sorted(set(wanted['addresses']) | set(row.get('addresses', [])))
            wanted['ranges'] = row.get('ranges', []) + [r for r in wanted['ranges'] if r not in row.get('ranges', [])]
            wanted['files'] = sorted(set(wanted['files']) | set(row.get('files', [])))
            wanted['claimed'] = row['claimed']
            continue
        if row['owner'] != owner and overlaps(wanted, row):
            raise LeaseConflict(f"overlaps active lease {row['id']} on {overlap_detail(wanted, row)} "
                                f"(until {row['expires']}, worktree {row.get('worktree', '?')}); drop those from the claim or treat them as external")
    rows = [r for r in rows if r['id'] != wanted['id']]
    rows.append(wanted)
    save_leases(rows)
    return wanted


def release(lease_id=None, packet=None, owner=None):
    owner = owner or owner_name()
    lease_id = lease_id or f'{owner}:{packet}'
    rows = load_leases()
    released = []
    for row in rows:
        if row['id'] == lease_id and row.get('status') == 'active':
            row['status'] = 'released'
            row['released'] = iso(now())
            released.append(row)
    save_leases(rows)
    return released


def holders(address):
    return [r for r in active_leases() if covers(r, address)]


def check_writable(addresses, owner=None, force=False):
    """Return other owners' active leases covering any of the addresses; raise unless force."""
    owner = owner or owner_name()
    conflicts = []
    for address in addresses:
        for lease in holders(int(address, 16)):
            if lease['owner'] != owner:
                conflicts.append((address, lease))
    if conflicts and not force:
        detail = '; '.join(f"{a} leased by {l['id']} until {l['expires']}" for a, l in conflicts[:5])
        raise LeaseConflict(f'refusing write: {detail} (release the lease, or --force with a reason)')
    return conflicts


# ---------------------------------------------------------------- Ghidra write lock

class ghidra_lock:
    """Context manager serializing Ghidra mutations across agents and worktrees."""

    def __init__(self, owner=None, ttl_seconds=600, wait_seconds=120, purpose=''):
        self.owner = owner or owner_name()
        self.ttl = ttl_seconds
        self.wait = wait_seconds
        self.purpose = purpose
        self.path = lock_path()

    def __enter__(self):
        self.path.parent.mkdir(parents=True, exist_ok=True)
        deadline = time.time() + self.wait
        while True:
            try:
                fd = os.open(self.path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
                with os.fdopen(fd, 'w', encoding='utf-8') as handle:
                    json.dump({'owner': self.owner, 'pid': os.getpid(), 'purpose': self.purpose,
                               'claimed': iso(now()), 'expires': iso(now() + timedelta(seconds=self.ttl))}, handle)
                return self
            except FileExistsError:
                try:
                    current = json.loads(self.path.read_text(encoding='utf-8'))
                except (OSError, ValueError):
                    current = {}
                stale = not current or parse(current['expires']) < now() or not pid_alive(current.get('pid'))
                mine = current.get('owner') == self.owner and current.get('pid') == os.getpid()
                if stale or mine:
                    try:
                        self.path.unlink()
                    except FileNotFoundError:
                        pass
                    continue
                if time.time() > deadline:
                    raise TimeoutError(f"Ghidra write lock held by {current.get('owner')} (pid {current.get('pid')}, "
                                       f"{current.get('purpose', '')}) until {current.get('expires')}")
                time.sleep(2)

    def __exit__(self, *exc):
        try:
            current = json.loads(self.path.read_text(encoding='utf-8'))
            if current.get('owner') == self.owner and current.get('pid') == os.getpid():
                self.path.unlink()
        except (OSError, ValueError):
            pass
        return False


def pid_alive(pid):
    """True when the process exists (or cannot be inspected); a lock whose holder died is stale."""
    if not isinstance(pid, int) or pid <= 0:
        return False
    if os.name == 'nt':
        import ctypes
        kernel32 = ctypes.windll.kernel32
        handle = kernel32.OpenProcess(0x1000, False, pid)  # PROCESS_QUERY_LIMITED_INFORMATION
        if not handle:
            return ctypes.get_last_error() == 5  # access denied: exists, owned by someone else
        try:
            code = ctypes.c_ulong()
            if kernel32.GetExitCodeProcess(handle, ctypes.byref(code)):
                return code.value == 259  # STILL_ACTIVE
            return True
        finally:
            kernel32.CloseHandle(handle)
    try:
        os.kill(pid, 0)
        return True
    except ProcessLookupError:
        return False
    except PermissionError:
        return True


def lock_status():
    path = lock_path()
    if not path.exists():
        return None
    try:
        current = json.loads(path.read_text(encoding='utf-8'))
    except (OSError, ValueError):
        return {'owner': '?', 'expires': '?'}
    current['expired'] = parse(current['expires']) < now()
    return current
