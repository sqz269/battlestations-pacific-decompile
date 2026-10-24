"""Rebuild a game run from the manifest tools/run_game.ps1 wrote next to its log.

Usage: python tools/run_replay.py <log>.manifest.json [--log local/replay.log] [--launch]

Prints the BSP_* environment and the run_game.ps1 command that repeats the recorded run, with
every file-valued game argument (--helm-orders, ...) pointed at the copy archived in
<log>.inputs/. With --launch it runs that command (BSP_* variables not in the manifest are
removed from the child's environment). Warns when the executable is missing or its sha256
no longer matches the recorded one; the slot, instance tag and processor are not replayed,
because run_game.ps1 assigns those per run.
"""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def sha256(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for block in iter(lambda: f.read(1 << 20), b''):
            h.update(block)
    return h.hexdigest()


def ps_quote(text):
    return "'" + str(text).replace("'", "''") + "'"


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('manifest')
    ap.add_argument('--log', help='log path for the replay (default local/replay_<original log name>)')
    ap.add_argument('--launch', action='store_true', help='run the command instead of printing it')
    args = ap.parse_args(argv)

    manifest_path = Path(args.manifest).resolve()
    m = json.loads(manifest_path.read_text(encoding='utf-8-sig'))
    inputs_dir = Path(str(manifest_path)[:-len('.manifest.json')] + '.inputs')
    launcher = m['launcher']
    full = list(m['arguments'])
    game_args = list(launcher['game_args'])
    prefix = len(full) - len(game_args)
    warnings = []

    def archived(entry):
        path = inputs_dir / entry['archived']
        if not path.is_file():
            warnings.append(f"archived input missing: {path}")
        elif sha256(path) != entry['sha256']:
            warnings.append(f"archived input {path} no longer matches its recorded sha256")
        return str(path)

    xlive = launcher['xlive_dll']
    for entry in m.get('inputs', []):
        index = entry['arg_index']
        if index >= prefix:
            k = index - prefix
            copy = archived(entry)
            game_args[k] = f"{entry['option']}={copy}" if game_args[k].startswith(entry['option'] + '=') else copy
        elif entry['option'] == '--xlive-dll':
            # The stand-in is a build product: keep the original when it is unchanged.
            original = Path(entry['source'])
            if not original.is_file() or sha256(original) != entry['sha256']:
                xlive = archived(entry)
            else:
                xlive = str(original)

    exe = m['exe']
    exe_path = Path(exe['path'])
    if not exe_path.is_file():
        warnings.append(f"executable missing: {exe_path} (built at {exe['build_checkout'].get('head')}, "
                        f"dirty={exe['build_checkout'].get('dirty')}); rebuild that commit")
    elif sha256(exe_path) != exe['sha256']:
        warnings.append(f"executable {exe_path} has changed since the run (recorded sha256 {exe['sha256']})")

    log = args.log or str(Path('local') / ('replay_' + Path(m['log']).name))
    command = ['-Exe', str(exe_path), '-GameRoot', launcher['game_root'], '-XLiveDll', xlive, '-Log', log, '--'] + game_args
    env = dict(m.get('env', {}))
    if 'difficulty' not in m:
        # A manifest from before the difficulty pin (cc15-prof2) records the run's own game
        # args with no --difficulty; replay them as they were, without the launcher's pin.
        env.setdefault('BSP_DIFFICULTY_PIN', 'off')

    print(f"# run started {m.get('started_utc')} in {m.get('cwd')}, host {m.get('host', {}).get('head')}"
          f" (dirty={m.get('host', {}).get('dirty')}), exit {m.get('exit_code')}")
    for w in warnings:
        print(f"# WARNING: {w}")
    for name in sorted(os.environ):
        if name.startswith('BSP_') and name not in env:
            print(f"Remove-Item Env:{name}")
    for name, value in env.items():
        print(f"$env:{name} = {ps_quote(value)}")
    rendered = ' '.join(a if a.startswith('-') and ' ' not in a and "'" not in a else ps_quote(a) for a in command)
    print('./tools/run_game.ps1 ' + rendered)

    if not args.launch:
        return 0
    child_env = {k: v for k, v in os.environ.items() if not k.startswith('BSP_')}
    child_env.update(env)
    shell = shutil.which('pwsh') or 'powershell'
    # -File would bind `--` and the game's --options as script parameters; -Command parses
    # the line exactly as printed above.
    script = ps_quote(ROOT / 'tools' / 'run_game.ps1')
    return subprocess.call([shell, '-NoProfile', '-Command', f'& {script} {rendered}; exit $LASTEXITCODE'],
                           env=child_env)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
