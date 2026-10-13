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
  waitlist.jsonl  one waiter per (owner, packet) in queue order: the lease fields plus queued,
                expires (default 24 h) and note; a claim over a resource someone queued for
                earlier is refused unless it jumps with a recorded reason
  messages.jsonl  mailbox for harnesses that cannot message each other: id, from, to (owner,
                prefix ending in - / or *, or all), text, sent, read_by; read mail is pruned after 7 days
All three .jsonl files are rewritten only inside lease_transaction (the one leases.lock).
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
    return _load_jsonl(leases_path())


def _load_jsonl(path):
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
    _save_jsonl(leases_path(), rows)


def _save_jsonl(path, rows):
    """Atomic replace of one registry file; every registry file shares the lease transaction lock."""
    path = path.resolve()
    if getattr(_lease_transaction_state, 'token', None) != (os.getpid(), leases_path().resolve()):
        raise RuntimeError(f'saving {path.name} requires lease_transaction around read and write')
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(mode='w', encoding='utf-8', newline='\n',
                                         prefix=f'.{path.stem}-{os.getpid()}-', suffix='.tmp',
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


def normalised_path(path):
    """Repo-relative, forward-slash, lower-case form of a leased path (a directory has no trailing slash)."""
    text = str(path).replace('\\', '/').strip()
    candidate = Path(text)
    if candidate.is_absolute():
        try:
            text = candidate.resolve().relative_to(ROOT).as_posix()
        except ValueError:
            text = candidate.as_posix()
    while text.startswith('./'):
        text = text[2:]
    return text.rstrip('/').lower()


def paths_overlap(x, y):
    """Equal paths, or one a parent directory of the other (packet cc9_lease_path_overlap)."""
    a, b = normalised_path(x), normalised_path(y)
    return a == b or a.startswith(b + '/') or b.startswith(a + '/')


def file_overlaps(a, b):
    """The paths of lease a that overlap some path of lease b, directory-aware."""
    return sorted({x for x in a.get('files', []) for y in b.get('files', []) if paths_overlap(x, y)})


def path_holders(path, rows=None):
    """Active leases holding a file path under the same directory-aware rule as a claim refusal."""
    rows = active_leases() if rows is None else rows
    return [row for row in rows if any(paths_overlap(path, f) for f in row.get('files', []))]


def overlap_detail(a, b):
    """Human-readable description of what two leases share, or '' when they do not overlap."""
    parts = []
    files = file_overlaps(a, b)
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
    if file_overlaps(a, b):
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


def claim(packet, addresses=(), ranges=(), files=(), ttl_hours=8.0, owner=None, note='', jump=''):
    """Create or extend the (owner, packet) lease; refuse when it overlaps another owner's active lease,
    or when another owner queued earlier for a resource this claim newly takes (unless jump gives a reason)."""
    owner = owner or owner_name()
    with lease_transaction():
        wanted = {'id': f'{owner}:{packet}', 'owner': owner, 'packet': packet,
                  'addresses': sorted({f'{int(a, 16):08x}' for a in addresses}),
                  'ranges': parse_ranges(ranges), 'files': sorted(set(files)),
                  'claimed': iso(now()), 'expires': iso(now() + timedelta(hours=ttl_hours)),
                  'status': 'active', 'worktree': str(ROOT), 'note': note}
        rows = load_leases()
        held = [row for row in active_leases(rows) if row['owner'] == owner]
        fresh = unheld_part(wanted, held)
        for row in active_leases(rows):
            if row['id'] == wanted['id']:
                # extending our own lease: merge the coverage
                wanted['addresses'] = sorted(set(wanted['addresses']) | set(row.get('addresses', [])))
                wanted['ranges'] = row.get('ranges', []) + [r for r in wanted['ranges'] if r not in row.get('ranges', [])]
                wanted['files'] = sorted(set(wanted['files']) | set(row.get('files', [])))
                wanted['claimed'] = row['claimed']
                if 'jumped' in row:
                    wanted['jumped'] = row['jumped']
                continue
            if row['owner'] != owner and overlaps(wanted, row):
                raise LeaseConflict(f"overlaps active lease {row['id']} on {overlap_detail(wanted, row)} "
                                    f"(until {row['expires']}, worktree {row.get('worktree', '?')}); "
                                    f"drop those from the claim, treat them as external, or queue: "
                                    f"python tools/bsp.py lease queue --packet {packet} --files ...")
        waiters = live_waiters()
        ahead = waiters_ahead(fresh, owner, waiters)
        if ahead and not jump:
            detail = '; '.join(f"{w['id']} ({overlap_detail(fresh, w)}, queued {w['queued'][:16]})" for w in ahead[:4])
            raise LeaseConflict(f"earlier waiters for now-free resources: {detail}"
                                + (f' ... {len(ahead) - 4} more' if len(ahead) > 4 else '')
                                + "; let them claim first, or pass --jump '<reason>' (recorded and mailed to them)")
        if ahead:
            wanted['jumped'] = wanted.get('jumped', []) + [{'over': [w['id'] for w in ahead], 'reason': jump, 'at': iso(now())}]
        rows = [r for r in rows if r['id'] != wanted['id']]
        rows.append(wanted)
        save_leases(rows)
        _save_jsonl(waitlist_path(), trim_waiters(waiters, owner, wanted))
        for w in ahead:
            send_message(w['owner'], f"{owner} claimed {packet} ahead of your queued {w['packet']} "
                                     f"({overlap_detail(fresh, w)}): {jump}", sender=owner)
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


# ---------------------------------------------------------------- lease waitlist

def waitlist_path():
    return coordination_dir() / 'waitlist.jsonl'


def live_waiters(rows=None):
    """Unexpired waiter rows in queue order (file order is queue order)."""
    rows = _load_jsonl(waitlist_path()) if rows is None else rows
    current = now()
    return [r for r in rows if parse(r['expires']) > current]


def path_within(path, parent):
    a, b = normalised_path(path), normalised_path(parent)
    return a == b or a.startswith(b + '/')


def unheld_part(wanted, held):
    """The files, addresses and ranges of wanted that none of the held leases already covers."""
    return {'files': [f for f in wanted.get('files', []) if not any(path_within(f, g) for r in held for g in r.get('files', []))],
            'addresses': [a for a in wanted.get('addresses', []) if not any(covers(r, int(a, 16)) for r in held)],
            'ranges': [[lo, hi] for lo, hi in wanted.get('ranges', [])
                       if not any(int(l2, 16) <= int(lo, 16) and int(hi, 16) <= int(h2, 16) for r in held for l2, h2 in r.get('ranges', []))]}


def waiters_ahead(resources, owner, waiters):
    """Other owners' waiters overlapping resources that no waiter of owner precedes on the same resource."""
    mine = [i for i, w in enumerate(waiters) if w['owner'] == owner]
    ahead = []
    for i, w in enumerate(waiters):
        if w['owner'] == owner or not overlaps(resources, w):
            continue
        if any(j < i and overlaps(waiters[j], w) and overlaps(waiters[j], resources) for j in mine):
            continue
        ahead.append(w)
    return ahead


def trim_waiters(waiters, owner, lease):
    """Drop what lease now covers from owner's waiter rows; a row with nothing left leaves the queue."""
    kept = []
    for w in waiters:
        if w['owner'] == owner:
            w = dict(w, **unheld_part(w, [lease]))
            if not (w['files'] or w['addresses'] or w['ranges']):
                continue
        kept.append(w)
    return kept


def queue(packet, addresses=(), ranges=(), files=(), ttl_hours=24.0, owner=None, note=''):
    """Add or refresh the (owner, packet) waiter; a refresh keeps its place and merges the coverage."""
    owner = owner or owner_name()
    with lease_transaction():
        waiters = live_waiters()
        row = {'id': f'{owner}:{packet}', 'owner': owner, 'packet': packet,
               'addresses': sorted({f'{int(a, 16):08x}' for a in addresses}),
               'ranges': parse_ranges(ranges), 'files': sorted(set(files)),
               'queued': iso(now()), 'expires': iso(now() + timedelta(hours=ttl_hours)),
               'worktree': str(ROOT), 'note': note}
        for i, old in enumerate(waiters):
            if old['id'] == row['id']:
                row['addresses'] = sorted(set(row['addresses']) | set(old.get('addresses', [])))
                row['ranges'] = old.get('ranges', []) + [r for r in row['ranges'] if r not in old.get('ranges', [])]
                row['files'] = sorted(set(row['files']) | set(old.get('files', [])))
                row['queued'] = old['queued']
                row['note'] = note or old.get('note', '')
                waiters[i] = row
                break
        else:
            waiters.append(row)
        _save_jsonl(waitlist_path(), waiters)
    return row


def unqueue(packet, owner=None):
    owner = owner or owner_name()
    with lease_transaction():
        waiters = live_waiters()
        gone = [w for w in waiters if w['id'] == f'{owner}:{packet}']
        _save_jsonl(waitlist_path(), [w for w in waiters if w not in gone])
    return gone


def path_waiters(path, waiters=None):
    """Waiters for a file path in queue order, directory-aware like path_holders."""
    waiters = live_waiters() if waiters is None else waiters
    return [w for w in waiters if any(paths_overlap(path, f) for f in w.get('files', []))]


def resource_waiters(lease, waiters=None):
    """Waiters overlapping any resource of a lease, in queue order."""
    waiters = live_waiters() if waiters is None else waiters
    return [w for w in waiters if w['owner'] != lease.get('owner') and overlaps(lease, w)]


def your_turn(owner=None):
    """(waiter, target) pairs where owner is first in line for a resource no other owner holds.
    target is a file path, or 'addresses' for the row's address/range part."""
    owner = owner or owner_name()
    leases = active_leases()
    waiters = live_waiters()
    others = [l for l in leases if l['owner'] != owner]
    turns = []
    for w in waiters:
        if w['owner'] != owner:
            continue
        for f in w.get('files', []):
            if not path_holders(f, others) and path_waiters(f, waiters)[0]['owner'] == owner:
                turns.append((w, f))
        part = {'addresses': w.get('addresses', []), 'ranges': w.get('ranges', []), 'files': []}
        if (part['addresses'] or part['ranges']) and not any(overlaps(part, l) for l in others) \
                and not waiters_ahead(part, owner, waiters):
            turns.append((w, 'addresses'))
    return turns


# ---------------------------------------------------------------- cross-harness mailbox

MESSAGE_KEEP_READ_DAYS = 7
MESSAGE_KEEP_UNREAD_DAYS = 30
MESSAGE_MAX_CHARS = 4000


def messages_path():
    return coordination_dir() / 'messages.jsonl'


def load_messages():
    return _load_jsonl(messages_path())


def addressed_to(message, owner):
    """Exact owner, 'all', or a prefix ending in '-', '/' or '*' (the '*' is dropped)."""
    to = message['to']
    if to == 'all' or to == owner:
        return True
    return to.endswith(('-', '/', '*')) and owner.startswith(to.rstrip('*'))


def _prune_messages(rows):
    current = now()
    return [m for m in rows
            if not (current - parse(m['sent']) > timedelta(days=MESSAGE_KEEP_READ_DAYS) and m.get('read_by'))
            and current - parse(m['sent']) <= timedelta(days=MESSAGE_KEEP_UNREAD_DAYS)]


def send_message(to, text, sender=None):
    sender = sender or owner_name()
    text = text.strip()
    if not text or not to.strip():
        raise ValueError('a message needs a recipient and text')
    if len(text) > MESSAGE_MAX_CHARS:
        raise ValueError(f'message is {len(text)} chars; the mailbox caps a message at {MESSAGE_MAX_CHARS}, '
                         'so point at a committed doc instead')
    with lease_transaction():
        rows = _prune_messages(load_messages())
        row = {'id': max((m['id'] for m in rows), default=0) + 1, 'from': sender, 'to': to.strip(),
               'text': text, 'sent': iso(now()), 'read_by': []}
        rows.append(row)
        _save_jsonl(messages_path(), rows)
    return row


def mailbox(owner=None, include_read=False, rows=None):
    """Messages addressed to owner (not ones it sent to a group it belongs to), oldest first."""
    owner = owner or owner_name()
    rows = load_messages() if rows is None else rows
    return [m for m in rows if addressed_to(m, owner) and not (m['from'] == owner and m['to'] != owner)
            and (include_read or owner not in m.get('read_by', []))]


def mark_read(ids, owner=None):
    """Mark message ids read for owner; returns the ids that were addressed to owner and unread."""
    owner = owner or owner_name()
    wanted = {int(i) for i in ids}
    with lease_transaction():
        rows = _prune_messages(load_messages())
        marked = []
        for m in rows:
            if m['id'] in wanted and addressed_to(m, owner) and owner not in m.get('read_by', []):
                m['read_by'] = m.get('read_by', []) + [owner]
                marked.append(m['id'])
        _save_jsonl(messages_path(), rows)
    return marked


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
