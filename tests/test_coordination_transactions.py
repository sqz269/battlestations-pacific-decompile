"""One regression scenario for the observed concurrent lease-registry corruption.

Every process uses a fresh BSP_COORDINATION_DIR; this never exercises ~/.bsp.
"""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import coordination


def worker(mode, number=0):
    # Refuse a standalone invocation that could fall back to the live registry.
    directory = Path(os.environ['BSP_COORDINATION_DIR']).resolve()
    if not directory.name.startswith('bsp-lease-transactions-'):
        raise RuntimeError('worker requires the isolated fixture directory')
    if mode == 'reader':
        reads = 0
        while not (directory / 'stop-reader').exists():
            coordination.load_leases()
            reads += 1
            time.sleep(0.001)
        print(json.dumps({'reads': reads}), flush=True)
        return
    if mode in ('holder', 'rewrite'):
        with coordination.lease_transaction():
            rows = coordination.load_leases()
            print('ready', flush=True)
            sys.stdin.readline()
            if mode == 'rewrite':
                print('saving', flush=True)
                coordination.save_leases(rows)
        return
    print('ready', flush=True)
    sys.stdin.readline()
    owner = f'worker-{number}'
    won = False
    try:
        coordination.claim('contended', files=['shared-contention.cpp'], owner=owner)
        won = True
    except coordination.LeaseConflict:
        pass
    for packet in range(10):
        address = 0x100000 + number * 0x1000 + packet * 0x10
        first = coordination.claim(str(packet), addresses=[f'{address:x}'],
                                   files=[f'{owner}/{packet}.cpp'], owner=owner)
        extended = coordination.claim(str(packet), addresses=[f'{address + 1:x}'],
                                      ranges=[f'{address + 2:x}-{address + 3:x}'],
                                      files=[f'{owner}/{packet}.hpp'], owner=owner)
        if extended['claimed'] != first['claimed']:
            raise AssertionError('extending a lease replaced its claimed timestamp')
        if packet % 2 == 0:
            released = coordination.release(packet=str(packet), owner=owner)
            if len(released) != 1:
                raise AssertionError('release lost its lease')
    print(json.dumps({'won': won}), flush=True)


class LeaseTransactionTests(unittest.TestCase):
    def test_concurrent_registry_transactions_and_failure_recovery(self):
        processes = []
        with tempfile.TemporaryDirectory(prefix='bsp-lease-transactions-') as temporary:
            with patch.dict(os.environ, {'BSP_COORDINATION_DIR': temporary,
                                         'BSP_AGENT': 'isolated-fixture'}):
                directory = Path(temporary)

                def start(mode, number=0):
                    process = subprocess.Popen(
                        [sys.executable, str(Path(__file__).resolve()), '--worker', mode, str(number)],
                        env=os.environ.copy(), stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE, text=True)
                    processes.append(process)
                    return process

                def finish(process):
                    output, errors = process.communicate(timeout=30)
                    self.assertEqual(process.returncode, 0, errors)
                    return output

                def proceed(process):
                    process.stdin.write('go\n')
                    process.stdin.flush()

                try:
                    reader = start('reader')
                    writers = [start('writer', number) for number in range(10)]
                    for process in writers:
                        self.assertEqual(process.stdout.readline().strip(), 'ready')
                    for process in writers:
                        proceed(process)
                    results = [json.loads(finish(process)) for process in writers]
                    (directory / 'stop-reader').touch()
                    self.assertGreater(json.loads(finish(reader))['reads'], 0)
                    self.assertEqual(sum(result['won'] for result in results), 1)

                    rows = coordination.load_leases()
                    self.assertEqual(len(rows), 101)
                    by_id = {row['id']: row for row in rows}
                    self.assertEqual(len(by_id), 101)
                    for number in range(10):
                        for packet in range(10):
                            owner = f'worker-{number}'
                            row = by_id[f'{owner}:{packet}']
                            address = 0x100000 + number * 0x1000 + packet * 0x10
                            self.assertEqual(row['addresses'], [f'{address:08x}', f'{address + 1:08x}'])
                            self.assertEqual(row['ranges'], [[f'{address + 2:08x}', f'{address + 3:08x}']])
                            self.assertEqual(row['files'], [f'{owner}/{packet}.cpp', f'{owner}/{packet}.hpp'])
                            self.assertEqual(row['status'], 'active' if packet % 2 else 'released')
                            self.assertEqual('released' in row, packet % 2 == 0)

                    # A timeout cannot replace a live holder; killing the holder
                    # releases its OS handle without removing the persistent file.
                    holder = start('holder')
                    self.assertEqual(holder.stdout.readline().strip(), 'ready')
                    before = (directory / 'leases.jsonl').read_bytes()
                    with self.assertRaises(TimeoutError):
                        with coordination.lease_transaction(wait_seconds=0.08):
                            self.fail('entered another process\'s live transaction')
                    self.assertIsNone(holder.poll())
                    self.assertTrue((directory / 'leases.lock').exists())
                    self.assertEqual((directory / 'leases.jsonl').read_bytes(), before)
                    holder.kill()
                    holder.communicate(timeout=10)
                    with coordination.lease_transaction(wait_seconds=2):
                        # Nested normal API calls use this same transaction.
                        coordination.claim('after-crash', owner='recovery')
                        coordination.release(packet='after-crash', owner='recovery')
                    with self.assertRaisesRegex(RuntimeError, 'requires lease_transaction'):
                        coordination.save_leases([])

                    if os.name == 'nt':
                        # A real deny-sharing handle blocks replacement after the
                        # writer has read its snapshot. Releasing it permits retry.
                        rewrite = start('rewrite')
                        self.assertEqual(rewrite.stdout.readline().strip(), 'ready')
                        close = coordination._open_lease_lock(directory / 'leases.jsonl')
                        try:
                            proceed(rewrite)
                            self.assertEqual(rewrite.stdout.readline().strip(), 'saving')
                            time.sleep(0.15)
                            self.assertIsNone(rewrite.poll())
                        finally:
                            close()
                        finish(rewrite)

                    self.assertEqual(len(coordination.load_leases()), 102)
                    self.assertEqual(list(directory.glob('.leases-*.tmp')), [])
                    # Failed serialization leaves the published snapshot intact.
                    before = (directory / 'leases.jsonl').read_bytes()
                    with self.assertRaises(TypeError):
                        with coordination.lease_transaction():
                            coordination.save_leases([{'invalid': object()}])
                    self.assertEqual((directory / 'leases.jsonl').read_bytes(), before)
                    self.assertEqual(list(directory.glob('.leases-*.tmp')), [])
                    # Existing corruption is surfaced, never silently truncated.
                    malformed = before + b'{"unfinished":'
                    (directory / 'leases.jsonl').write_bytes(malformed)
                    with self.assertRaises(json.JSONDecodeError):
                        coordination.claim('must-not-repair', owner='recovery')
                    self.assertEqual((directory / 'leases.jsonl').read_bytes(), malformed)
                finally:
                    for process in processes:
                        if process.poll() is None:
                            process.kill()
                        process.communicate(timeout=10)


if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == '--worker':
        worker(sys.argv[2], int(sys.argv[3]))
    else:
        unittest.main()
