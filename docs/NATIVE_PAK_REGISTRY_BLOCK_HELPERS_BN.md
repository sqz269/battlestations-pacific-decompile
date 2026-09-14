# Native PakRegistry block helpers BN

Five complete source bodies provide the package-name producer and raw VFS
field operations needed by actual PakRegistry block observers. Source lives
in [native_pak_registry_block_helpers.cpp](../src/native_pak_registry_block_helpers.cpp);
the [report](../reports/native_pak_registry_block_helpers_bn.json) records
complete native spans, direct calls, FH3 support, exact hashes and validation.
The worker base is `942f454c003868062d01e3320b2fb2e164b228cb`.

| Entry / complete span | Original ABI | Source operation |
|---|---|---|
| `BB5670..BB5760`, 241 bytes | ECX output8h header, EDX input8h header; EAX output, RET | Construct block package name |
| `BD9200..BD920E`, 15 bytes | ECX actual VFS; AL predicate, RET | Decrement counter+1C, then recompute byte+20 |
| `BD9210..BD9219`, 10 bytes | ECX actual VFS, low stack byte; AL byte, RET4 | Copy low argument byte to+20 |
| `BD9220..BD922A`, 11 bytes | ECX actual VFS; AL predicate, RET | Recompute byte+20 from current counter |
| `BD9FA0..BD9FAB`, 12 bytes | ECX actual VFS; EAX1, RET | Increment counter+1C and write byte+20=1 |

## Name construction

`BB5693` reads the actual input data pointer. A null pointer produces position
`FFFFFFFF`; otherwise the first case-sensitive `strstr` of `.mpak` at
`CEBA84` is used. A successful search subtracts the current input data pointer
at `BB56AF`. The comparison uses current stored length minus5 with DWORD
wrap, not C-string length and not a suffix predicate.

Consequently `a.mpak` is copied, `a.mpak.mpak` gains another `.mpak`,
`A.MPAK` gains `.mpak`, and the four-character string `file` is copied:
both its absent-match position and length-5 equal `FFFFFFFF`. Search stops
at embedded NUL even when the stored length extends farther.

Equality uses the same actual-header schedule as existing `00426060`:
compare addresses, clear output length then pointer, and only then branch
on identity. Nonidentity resizes using current source length, rechecks current
source length after allocation, and copies current output length from current
source data into current output data. No old output buffer is released by
the initial clear. Identical input/output therefore becomes empty on equality.

Otherwise the helper constructs a native local `.mpak` header through
`0041E870`, calls actual `004261A0` with `(left=input, output, right=suffix)`,
then destroys the current local header. That concat also clears output before
the identity branch, so identical input/output becomes just `.mpak` on this
path. Existing actual headers and `NativeStringStorage` are borrowed directly;
no `std::string`, copied input header, or private registry is introduced.

## Cleanup evidence

The fresh handler `CC43C8..CC43D1` loads `DFDCBC` and jumps to `BF6B43`.
The 36-byte FH3 FuncInfo has magic `19930522`, maxState1, unwind map
`DFDCB4`, no try blocks, and flags1. Its only entry is
`state0 -> -1, action CC43C0`. That complete 8-byte action executes
`LEA ECX,[EBP-14h]; JMP 41DD20`, destroying the current suffix header.

State0 is armed at `BB56D2`, after suffix construction and before concat.
It is disarmed at `BB56E5` before normal temporary return. A suffix
construction failure has no outer cleanup. A concat failure destroys the
suffix; the existing concat dependency independently destroys its output
only after its initial copy completes. There is no new cleanup on the
equality copy path. Normal or exceptional destruction leaves header fields
untouched, including stale pointers, following existing `41DD20` behavior.

## Actual VFS storage

Each helper receives the actual captured manager address and reads/writes
its DWORD+1C and byte+20 in place. Decrement and increment use unsigned
wrap. Decrement rereads the current counter before testing signed positivity;
recompute performs the same test (`0 < count < 80000000h`). Increment sets
byte1 even when the wrapped counter is zero or negative. The setter copies
any low byte without Boolean coercion. Surrounding manager bytes survive.

The caller remains responsible for reloading current `0109CEEC` separately
before each native-equivalent manager call. These leaves do not capture the
publication cell or add another counter. `BB5770`, `BB5910`, unmount, gate
list, FileBlock construction and production observer dispatch remain outside
this packet; their composition must keep the actual registry and VFS storage.

## Verification and limits

Every live batch used `bsp.py ghidra`, whose client verifies project `bsp`,
program `/battlestationspacific.exe`, x86 language and image base `00400000`.
The configured project file is `C:/Users/sqz269/bsp.gpr`; autostart was
disabled. No scripts, restarts, annotations or function repairs were used.
All five complete bodies (289 bytes) and the five FH3/literal support spans
(68 bytes) match fresh live Ghidra and the installed PE byte-for-byte.
The seven direct calls in `BB5670` were decoded from those matching bytes.

The changed TU passed MSVC Win32 `/std:c++17 /EHsc /O2 /MD /W4 /WX
/fp:strict`. One ignored actual-header fixture linked that new object against
a frozen copy of the orchestrator checkout's existing `bsp_core.lib`, with source,
copy and source-again SHA256 agreement. The library hash is
`134578367593f74415bb6fd12b51d103dfc6575dcb94d2f692640dde69c78251`;
this is historical fixture input, not an integrated build claim.
Its exact source is `J:/PROG/battlestations-pacific-decompile-orch4-20260910/build/win32/Release/bsp_core.lib`.
An earlier main-checkout library/run is
preserved separately in ignored evidence and is not the final fixture input.

The fixture passed first-match/case/length4/null-data cases, both identity
branches, source-header mutation during allocation, allocation/release order,
each of the three allocation failure points, VFS wrapping/sign boundaries,
arbitrary setter bytes, and unchanged surrounding bytes. Its executable
embeds a manifest; the ignored compile/fixture/artifact hashes are in the report.

Source interfaces add explicit string storage and discard native EAX/AL
residue from void leaves. The reused string layer's omitted zero-byte copies,
unsupported overlap, bound allocator, and noexcept release remain explicit
boundaries. Hardware SEH, raw-pool getter exceptions, native ABI/FH3 transport,
complete observer integration and gameplay have not been validated. CMake,
shared ledgers, runtime composition and Ghidra annotation are integrator work.
