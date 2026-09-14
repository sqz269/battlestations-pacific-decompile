"""Bring the live Ghidra server back up when the application is not running.

The MCP HTTP server (config/target.json ghidra_url) belongs to the CodeBrowser tool, so a
reachable server needs three things at once: the Ghidra front end running, the project open,
and the target program open in a CodeBrowser. A reboot that takes Ghidra down with it leaves
none of them, and relaunching by hand is the only thing this module replaces.

  python tools/bsp.py ghidra ensure                 probe; launch and wait when nothing answers
  python tools/bsp.py ghidra ensure --status        probe only, never launch
  python tools/bsp.py ghidra ensure --restart       also relaunch a front end with no server
  python tools/bsp.py ghidra autostart --install    run the same check at every logon

Evidence for the restore path, checked on this machine against Ghidra 12.0.4:
  - The saved CodeBrowser tool carries <EXTENSION NAME="GhidraMCP"/>
    (AppData/Roaming/ghidra/ghidra_12.0.4_PUBLIC/tools/_code_browser.tcd), so a restored
    CodeBrowser loads the plugin and the server starts with it.
  - WorkspaceImpl writes <RUNNING_TOOL TOOL_NAME="CodeBrowser"> into <project>.rep/projectState
    only on a clean exit. After a hard shutdown the file has none, so the next launch restores
    the front end alone and no server ever listens. That is the failure this module repairs.
  - The program is reopened from the ProgramManagerPlugin data state. The key names below are
    copied from a real clean-exit projectState (C:/Users/sqz269/wows.rep/projectState), not
    guessed; ghidra-mcp-setup.ps1's OPEN_FILE/TOOL_INSTANCE shape is not what Ghidra 12 reads.
  - Ghidra 12 locks a project through a file channel on <project>.lock~, which the OS releases
    on reboot, so stale lock files are not cleaned up here and must not be deleted by hand.

Known limit: if the previous session died with unsaved program changes, Ghidra opens a crash
recovery prompt and waits for an answer. Nothing here can dismiss it; `ensure` reports the
server as still down and the prompt has to be answered in the Ghidra window.
"""
import json
import os
import shutil
import subprocess
import sys
import time
import xml.etree.ElementTree as ET
from datetime import datetime, timedelta, timezone
from pathlib import Path
from urllib.error import URLError
from urllib.parse import urlencode
from urllib.request import urlopen

sys.path.insert(0, str(Path(__file__).resolve().parent))
import coordination  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
LAUNCH_LOCK_TTL = timedelta(minutes=12)  # a cold start with a large program can take minutes


def load_config(path=None):
    return json.loads(Path(path or ROOT / 'config/target.json').read_text(encoding='utf-8'))


# ---------------------------------------------------------------------- probe

def _get(config, endpoint, timeout, **params):
    url = config['ghidra_url'].rstrip('/') + '/' + endpoint + ('?' + urlencode(params) if params else '')
    with urlopen(url, timeout=timeout) as response:
        return response.read().decode('utf-8', 'replace')


def probe(config, timeout=2.0):
    """Return (state, detail) where state is up | no-program | wrong-program | down."""
    try:
        _get(config, 'check_connection', timeout)
    except (URLError, OSError, TimeoutError) as error:
        return 'down', getattr(error, 'reason', error)
    try:
        info = json.loads(_get(config, 'get_current_program_info', timeout, program=config['program']))
    except Exception as error:  # server answers but holds no program
        return 'no-program', error
    path = info.get('path') if isinstance(info, dict) else None
    if path != config['program_path']:
        return 'wrong-program', path
    return 'up', path


# ------------------------------------------------------------------ processes

def _powershell(command, timeout=40):
    return subprocess.run(['powershell', '-NoProfile', '-NonInteractive', '-Command', command],
                          capture_output=True, text=True, timeout=timeout)


def processes():
    """Running Ghidra JVMs as [{'pid': int, 'command': str}]."""
    done = _powershell(
        "Get-CimInstance Win32_Process -Filter \"Name='javaw.exe' OR Name='java.exe'\" | "
        "Where-Object { $_.CommandLine -match 'ghidra' } | "
        "Select-Object ProcessId,CommandLine | ConvertTo-Json -Compress")
    text = (done.stdout or '').strip()
    if not text:
        return []
    try:
        rows = json.loads(text)
    except json.JSONDecodeError:
        return []
    rows = rows if isinstance(rows, list) else [rows]
    return [{'pid': r['ProcessId'], 'command': r.get('CommandLine') or ''} for r in rows]


def close(pids, grace=60, force=False):
    """Ask Ghidra to close, then wait. Never force-kills unless asked: a forced exit
    discards unsaved annotations and can leave the project database mid-write."""
    ids = ','.join(str(p) for p in pids)
    _powershell(f"Get-Process -Id {ids} -ErrorAction SilentlyContinue | ForEach-Object "
                "{ $null = $_.CloseMainWindow() }")
    deadline = time.time() + grace
    while time.time() < deadline:
        time.sleep(2)
        if not processes():
            return True
    if not force:
        return False
    _powershell(f"Stop-Process -Id {ids} -Force -ErrorAction SilentlyContinue")
    time.sleep(3)
    return not processes()


# ------------------------------------------------------------- project restore

def ghidra_home(config):
    for candidate in (config.get('ghidra_home'), os.environ.get('GHIDRA_INSTALL_DIR')):
        if candidate and (Path(candidate) / 'ghidraRun.bat').exists():
            return Path(candidate)
    lastrun = Path(os.environ.get('APPDATA', '')) / 'ghidra' / 'lastrun'
    if lastrun.exists():
        recorded = Path(lastrun.read_text(encoding='utf-8').strip())
        if (recorded / 'ghidraRun.bat').exists():
            return recorded
    raise RuntimeError('No Ghidra install found; set ghidra_home in config/target.json')


def project_state_path(config):
    gpr = Path(config['project_file'])
    return gpr.with_suffix('.rep') / 'projectState'


def inject_running_tool(config):
    """Make the next launch restore CodeBrowser with the target program open.

    Returns 'already' when Ghidra's own clean-exit state is present (which is richer than
    anything written here, including the window layout), 'injected', or 'no-state-file'.
    """
    path = project_state_path(config)
    if not path.exists():
        return 'no-state-file'
    tree = ET.parse(path)
    root = tree.getroot()
    manager = root.find('TOOL_MANAGER')
    if manager is None:
        manager = ET.SubElement(root, 'TOOL_MANAGER', {'ACTIVE_WORKSPACE': 'Workspace'})
    active = manager.get('ACTIVE_WORKSPACE', 'Workspace')
    workspace = manager.find(f"WORKSPACE[@NAME='{active}']") or manager.find('WORKSPACE')
    if workspace is None:
        workspace = ET.SubElement(manager, 'WORKSPACE', {'NAME': active, 'ACTIVE': 'true'})
    if workspace.find('RUNNING_TOOL') is not None:
        return 'already'

    gpr = Path(config['project_file'])
    tool = ET.SubElement(workspace, 'RUNNING_TOOL', {'TOOL_NAME': 'CodeBrowser'})
    plugin = ET.SubElement(ET.SubElement(tool, 'DATA_STATE'), 'PLUGIN', {'NAME': 'ProgramManagerPlugin'})
    for name, kind, value in (
            ('CURRENT_FILE', 'string', config['program_path'].rsplit('/', 1)[-1]),
            ('LOCATION_0', 'string', '/' + gpr.parent.as_posix().rstrip('/') + '/'),
            ('NUM_PROGRAMS', 'int', '1'),
            ('PATHNAME_0', 'string', config['program_path']),
            ('PROJECT_NAME_0', 'string', config['project']),
            ('VERSION_0', 'int', '-1')):
        ET.SubElement(plugin, 'STATE', {'NAME': name, 'TYPE': kind, 'VALUE': value})

    shutil.copyfile(path, path.with_suffix('.bak'))  # old value kept next to the file we rewrite
    ET.indent(tree, space='    ')
    tree.write(path, encoding='UTF-8', xml_declaration=True)
    return 'injected'


def launch(config):
    home = ghidra_home(config)
    subprocess.Popen(['cmd', '/c', str(home / 'ghidraRun.bat'), str(Path(config['project_file']))],
                     cwd=str(home), stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                     stderr=subprocess.DEVNULL, creationflags=subprocess.CREATE_NO_WINDOW)
    return home


def wait_up(config, seconds, report=print):
    """Poll until the server answers with the target program, or the budget runs out."""
    deadline = time.time() + seconds
    last = None
    while time.time() < deadline:
        state, detail = probe(config, timeout=3.0)
        if state == 'up':
            return state, detail
        if state != last:
            report(f'  waiting: {state} ({int(deadline - time.time())}s left)')
            last = state
        time.sleep(4)
    return probe(config, timeout=3.0)


# --------------------------------------------------------------- launch mutex

def _launch_lock_path():
    return coordination.coordination_dir() / 'ghidra-launch.lock'


def _take_launch_lock():
    """One launcher at a time: several agents share this Ghidra and two front ends would
    fight over the project lock. Returns True when this process owns the launch."""
    path = _launch_lock_path()
    record = {'owner': coordination.owner_name(), 'pid': os.getpid(),
              'expires': (datetime.now(timezone.utc) + LAUNCH_LOCK_TTL).isoformat()}
    try:
        path.parent.mkdir(parents=True, exist_ok=True)
        with open(os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY), 'w', encoding='utf-8') as handle:
            json.dump(record, handle)
        return True
    except FileExistsError:
        pass
    try:
        held = json.loads(path.read_text(encoding='utf-8'))
        if datetime.fromisoformat(held['expires']) > datetime.now(timezone.utc):
            return False
    except (OSError, ValueError, KeyError):
        pass
    path.write_text(json.dumps(record), encoding='utf-8')  # expired or corrupt: reclaim
    return True


def _release_launch_lock():
    _launch_lock_path().unlink(missing_ok=True)


# ---------------------------------------------------------------------- ensure

def ensure(config=None, wait=300, restart=False, force_kill=False, status_only=False, report=print):
    """Return True when the Ghidra server is answering for the target program."""
    config = config or load_config()
    state, detail = probe(config)
    if state == 'up':
        report(f'ghidra: up  {detail}')
        return True
    if state == 'wrong-program':
        report(f'ghidra: up but holding {detail}, not {config["program_path"]}; open the target by hand')
        return False
    if status_only:
        report(f'ghidra: {state} ({detail}) -> python tools/bsp.py ghidra ensure')
        return False

    if not _take_launch_lock():
        report('ghidra: another agent is already starting it; waiting')
        state, detail = wait_up(config, wait, report)
        report(f'ghidra: {state}  {detail}')
        return state == 'up'

    try:
        if state == 'no-program':  # the tool is up; opening the program is a decision for the user
            report(f'ghidra: server answering with no program open; open {config["program_path"]} '
                   'in the CodeBrowser (a crash recovery prompt may be waiting)')
            return False
        running = processes()
        if running:
            if not restart:
                report(f'ghidra: running (pid {running[0]["pid"]}) but no server on {config["ghidra_url"]}. '
                       'The CodeBrowser tool is probably closed. Rerun with --restart to close and relaunch it.')
                return False
            report(f'ghidra: closing {len(running)} process(es) before relaunch')
            if not close([p['pid'] for p in running], force=force_kill):
                report('ghidra: it did not close (a save or confirmation dialog may be open); '
                       'close it by hand, or rerun with --restart --force-kill to discard that state')
                return False

        restored = inject_running_tool(config)
        report(f'ghidra: project state {restored}; launching {ghidra_home(config)}')
        launch(config)
        state, detail = wait_up(config, wait, report)
        report(f'ghidra: {state}  {detail}' if state == 'up' else
               f'ghidra: still {state} after {wait}s ({detail}); check the Ghidra window')
        return state == 'up'
    finally:
        _release_launch_lock()


# ------------------------------------------------------------------- autostart

def startup_cmd_path():
    return (Path(os.environ['APPDATA']) / 'Microsoft/Windows/Start Menu/Programs/Startup'
            / 'ghidra-bsp-autostart.cmd')


def autostart(install=True, delay=60, wait=420, report=print):
    """A logon entry, not a service: the Startup folder needs no elevation and the user can
    delete the .cmd themselves. The delay keeps Ghidra out of the logon disk storm."""
    path = startup_cmd_path()
    if not install:
        existed = path.exists()
        path.unlink(missing_ok=True)
        report(f'autostart: {"removed " + str(path) if existed else "was not installed"}')
        return True
    python = Path(sys.executable)
    pythonw = python.with_name('pythonw.exe')
    log = ROOT / 'local/output/ghidra_autostart.log'
    log.parent.mkdir(parents=True, exist_ok=True)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(  # text mode already writes CRLF on Windows; embedding \r would double it
        '@echo off\n'
        'rem Written by tools/ghidra_launch.py; delete this file to stop starting Ghidra at logon.\n'
        f'"{pythonw if pythonw.exists() else python}" "{ROOT / "tools/ghidra_launch.py"}" ensure '
        f'--delay {delay} --wait {wait} >> "{log}" 2>&1\n',
        encoding='utf-8')
    report(f'autostart: installed {path}\n  delay {delay}s, wait {wait}s, log {log}')
    return True


def main(argv=None):
    import argparse
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest='command', required=True)
    for name in ('ensure', 'status', 'restart'):
        p = sub.add_parser(name)
        p.add_argument('--wait', type=int, default=300, help='seconds to wait for the server')
        p.add_argument('--delay', type=int, default=0, help='sleep before probing (logon startup storm)')
        p.add_argument('--force-kill', action='store_true', help='restart: kill a front end that will not close')
        p.add_argument('--config')
    p = sub.add_parser('autostart')
    p.add_argument('--install', action='store_true')
    p.add_argument('--remove', action='store_true')
    p.add_argument('--delay', type=int, default=60)
    p.add_argument('--wait', type=int, default=420)
    args = parser.parse_args(argv)

    if args.command == 'autostart':
        if args.install == args.remove:
            parser.error('choose --install or --remove')
        return 0 if autostart(install=args.install, delay=args.delay, wait=args.wait) else 1
    if args.delay:
        time.sleep(args.delay)
    print(f'{datetime.now().isoformat(timespec="seconds")} ghidra_launch {args.command}')
    ok = ensure(load_config(args.config), wait=args.wait, restart=args.command == 'restart',
                force_kill=args.force_kill, status_only=args.command == 'status')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
