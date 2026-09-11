"""Cross-harness coordination: address/file leases and a global Ghidra write lock.

Several agents (Codex, Claude, workers in git worktrees) share one Ghidra project and one
history. Leases say who is working on which addresses and files; the Ghidra lock serializes
mutations of the shared project. Both live OUTSIDE any worktree so every checkout sees the
same registry: $BSP_COORDINATION_DIR, else `coordination_dir` in config/target.json, else
~/.bsp/. Owner identity is $BSP_AGENT, else the current git branch (worktrees use agent/<name>).

  leases.jsonl  one lease per line: id, owner, packet, addresses, ranges, files, claimed,
                expires, status (active|released), worktree
  leases.lock   persistent file; an OS-held exclusive handle serializes lease transactions
  ghidra.lock   JSON {owner, pid, claimed, expires}; created O_EXCL, expired locks are reclaimed
"""
import errno
import json
import os
import socket
import subprocess
import tempfile
import threading
import time
from contextlib import contextmanager
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

_lease_transaction_state = threading.local()


def _windows_sharing_error(error):
    # Access denied can also be a replacement denied by a Windows open handle.
    # Python's CRT-backed open can omit winerror and expose only EACCES/EPERM.
    # Retry for a bounded time; an actual permissions error still propagates.
    return (getattr(error, 'winerror', None) in (5, 32, 33) or
            (os.name == 'nt' and error.errno in (errno.EACCES, errno.EPERM)))


def _retry_registry_io(operation, wait_seconds=5.0, retry_if=_windows_sharing_error):
    deadline = time.monotonic() + wait_seconds
    while True:
        try:
            return operation()
        except OSError as error:
            remaining = deadline - time.monotonic()
            if not retry_if(error) or remaining <= 0:
                raise
            time.sleep(min(0.02, remaining))


def _open_lease_lock(path):
    """Return a closer for an OS-held lock. The lock file is never removed."""
    if os.name == 'nt':
        import ctypes
        from ctypes import wintypes

        kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)
        create_file = kernel32.CreateFileW
        create_file.argtypes = (wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD,
                                wintypes.LPVOID, wintypes.DWORD, wintypes.DWORD,
                                wintypes.HANDLE)
        create_file.restype = wintypes.HANDLE
        close_handle = kernel32.CloseHandle
        close_handle.argtypes = (wintypes.HANDLE,)
        close_handle.restype = wintypes.BOOL
        # OPEN_ALWAYS, no sharing, non-inheritable handle. A process exit closes
        # the handle; there is no guessed PID/expiry or stale-file reclamation.
        handle = create_file(str(path), 0xc0000000, 0, None, 4, 0x80, None)
        if handle == ctypes.c_void_p(-1).value:
            raise ctypes.WinError(ctypes.get_last_error())
        return lambda: close_handle(handle)

    import fcntl

    descriptor = os.open(path, os.O_CREAT | os.O_RDWR, 0o600)
    try:
        fcntl.flock(descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except BaseException:
        os.close(descriptor)
        raise
    return lambda: os.close(descriptor)


@contextmanager
def lease_transaction(wait_seconds=30.0):
    """Serialize the complete read/check/modify/save, including same-thread nesting.

    All writers must participate. Older worktrees using the former unlocked
    save_leases implementation must be upgraded before this protects the registry.
    """
    path = leases_path().resolve()
    token = (os.getpid(), path)
    active = getattr(_lease_transaction_state, 'token', None)
    if active == token:
        yield
        return
    if active is not None and active[0] == os.getpid():
        raise RuntimeError('cannot switch registries inside a lease transaction')
    path.parent.mkdir(parents=True, exist_ok=True)
    lock = path.with_suffix('.lock')

    def busy(error):
        return _windows_sharing_error(error) or isinstance(error, BlockingIOError)

    try:
        close = _retry_registry_io(lambda: _open_lease_lock(lock), wait_seconds, busy)
    except OSError as error:
        if not busy(error):
            raise
        raise TimeoutError(f'timed out waiting for lease transaction lock {lock}; '
                           'the existing lock was not removed or reclaimed') from error
    _lease_transaction_state.token = token
    try:
        yield
    finally:
        del _lease_transaction_state.token
        close()


def load_leases():
    path = leases_path()

    def read():
        try:
            with path.open(encoding='utf-8') as handle:
                return [json.loads(line) for line in handle if line.strip()]
        except FileNotFoundError:
            return []

    # Readers need no lock: replacement publishes one complete snapshot. Do not
    # suppress malformed JSON or attempt to repair data left by an older writer.
    return _retry_registry_io(read)


def save_leases(rows):
    """Publish rows read and modified inside the current lease_transaction."""
    path = leases_path().resolve()
    if getattr(_lease_transaction_state, 'token', None) != (os.getpid(), path):
        raise RuntimeError('save_leases requires lease_transaction around read and write')
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(mode='w', encoding='utf-8', newline='\n',
                                         prefix=f'.leases-{os.getpid()}-', suffix='.tmp',
                                         dir=path.parent, delete=False) as handle:
            temporary = Path(handle.name)
            for row in rows:
                handle.write(json.dumps(row) + '\n')
            handle.flush()
            os.fsync(handle.fileno())
        _retry_registry_io(lambda: os.replace(temporary, path))
        temporary = None
    finally:
        if temporary is not None:
            try:
                _retry_registry_io(lambda: temporary.unlink(missing_ok=True))
            except OSError:
                # Preserve the original failure. This unique orphan is never read
                # as a registry and cannot collide with another writer's file.
                pass


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
    with lease_transaction():
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
    with lease_transaction():
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
