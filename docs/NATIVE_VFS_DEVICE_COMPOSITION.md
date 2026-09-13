# Focused original/source device-route comparison

Addresses: BDD850, BDBC70, BDB680, BDB670. Existing BDD0A0 and string/pool
implementations are shared source support, not new original-byte execution.

One differential fixture exercises eight distinct variants of the actual
device callback and owner path. It executes four original routines totaling
793 bytes, and independently compares the current C++ bodies. Fresh guarded
reads also verify the 12-byte D683F4 profile and two-byte slash literal against
the installed executable. The fixture runs in its own process with generated
headers and mount storage; it does not open the installed game or archives.

| Variant | Original provider result | Observed callback name | Final device |
|---|---|---|---|
| Empty mount success | raw 80h | `b` | 2222h |
| Named mount success | raw 80h | `m/b` | 2222h |
| Named mount failure | zero | `m/` | unchanged 3333h |
| Failure, output aliases visitor byte4 | zero, then current byte becomes `m` | `m/` | 2222h |
| Success, embedded-NUL output aliases byte4 | raw 80h, then current byte becomes zero | `\0/b` | unchanged 3333h |
| Empty mount failure | zero | previous `old` | unchanged 3333h |
| BDD850 success | raw 80h | temporary visitor discarded | returned 2222h |
| BDD850 failure | zero | temporary visitor discarded | returned FFFFFFFFh |

The provider instrumentation replaces the current payload's provider with a
second actual owner whose device is 2222h. This distinguishes the callback's
initial provider capture from its late provider/device reload. The two alias
variants point the visitor's name-data field at its own result byte and use a
matching recorded length; assignment then changes the result byte AFTER the
provider returned. This verifies both directions of the current-result test.
Those two visitors have externally backed output and are not destroyed by the
fixture; their four owned temporaries are still returned to the actual pool.

Each variant compares the complete 4,096-byte arena and the actual native pool
prefix through offset 8AD484h: 9,098,372 bytes containing owner, arena and ring
state. The single fixture function-pointer word is normalized because host
link addresses are not a native outcome. All other arena bytes, including
actual owner/string pointers, are compared unchanged at fixed process mappings.
The pool's critical section and trailing recursion word are outside this
comparison. Original/source output comparison uses full byte equality; stored
state fingerprints provide deterministic replay diagnostics.

The native callback calls current actual string construction/concatenation,
resize and pool services through explicit x86 bridges; BF7680 uses host
`memmove`. The original BDD850 calls the shared current BDD0A0 source traversal,
which uses the current source BDBC70 callback. Original BDBC70 is compared
separately. Thus this proves the owner and callback in that stated composition,
not original traversal or CRT/STL internals. Native BDB680 cleanup and BDB670
raw-byte reads execute directly in the callback comparisons. BDBE00 and
BF0FB0 are not newly executed by this fixture, and native exception unwinding
is outside the tested domain.

Use the portable helper after building the chosen candidate:

```powershell
python tools/native_vfs_device_fixture.py --repo . --output local/device-check-NEW --baseline reports/native_vfs_device_composition.json
```

`--output` must not already exist. `--baseline` is required and accepts this report's immutable
`fixed_baseline` or an earlier attempt directory. The helper never updates the
baseline. It writes original input fingerprints before native calls, writes
native outcomes before candidate calls, and writes candidate outcomes to a
different file. The report retains the fixed baseline text so replay does not
depend on keeping a worker worktree or ignored output directory.

The helper compiles only the fixture and freezes its observed candidate headers.
It freezes and links the exact existing candidate build objects
`bsp_core.dir/Release/native_vfs_device_route.obj` and
`bsp_core.dir/Release/native_vfs_lookup_routes.obj`, together with the candidate's
built libraries. Target source copies are retained as evidence and are not
recompiled. `/showIncludes`, `/MAP` and `/VERBOSE:LIB` records identify the
fixture dependencies, two exact objects and selected `bsp_core` string/pool
objects. Object and library hashes are checked before and after execution.
It embeds a manifest in the
neutral `device_probe.exe` and checks candidate HEAD before and after the run.
`replay.json` supplies current absolute input paths, attempt-relative frozen
copies, SHA-256 hashes, library identities and fixed functional results for
integration gates.

The baseline and a new-directory replay passed on the worker candidate; the
report distinguishes those initial source-compilation attempts from the final
exact-object replay against the same fixed input and outcome rows.
This supplements the earlier strict Win32/two-CTest implementation
check. Both implementation reports and the MPAK open packet remain unchanged.
No gameplay, binary replacement, compressed-entry or installed-archive result
is asserted here.
