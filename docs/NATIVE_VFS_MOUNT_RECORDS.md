# Actual VFS mount records and right trim (AW)

`src/native_vfs_mount_records.cpp` reconstructs four complete stored bodies. The
interfaces take actual storage pointers and the existing `NativeStringStorage`.
Names are hypotheses, not recovered symbols. This worker used read-only Ghidra
queries against `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; the CLI
verified the target on every live batch. No analysis definitions, names, comments,
or flow were changed. The installed PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Entry and inclusive end | Bytes | Original ABI |
| --- | ---: | --- |
| `00584110..00584165` | 86 | ECX actual 8-byte string; stacked C-string character set; RET 4 |
| `00BDCE40..00BDCE9B` | 92 | ECX destination; stacked source record pointer; EAX destination; RET 4 |
| `00BDCFC0..00BDD01C` | 93 | ECX destination; stacked priority-DWORD pointer then payload pointer; EAX destination; RET 8 |
| `00BDEEC0..00BDEF43` | 132 | ECX destination, EDX full priority bits, stacked **16-byte by-value payload**; EAX captured destination; RET 10h |

The record producers establish a **14h-byte record**: priority DWORD at +0,
actual string length/data at +4/+8, raw provider DWORD at +Ch, and one byte at
+10h. Bytes +11h..+13h are untouched. The **10h-byte payload** contains string
length/data at +0/+4, provider at +8, byte at +Ch and untouched padding +Dh..+Fh.
There is no invented host record type, provider reference acquisition, provider
deletion, private pool, or additional lifetime domain. Sibling `00BDEE60` is a
distinct copy producer owned by the insertion packet, not counted here.

`00584110` checks the trim-set pointer and first byte before touching the string.
Null data or zero length then returns. It captures the initial data and length,
walks backward using unsigned address comparisons, and calls the existing CRT
`_strchr` at `00BF86F0` with the sign-extended current byte. After each match it
reloads the header's current data for the loop comparison; the final length also
uses current data. The first byte is always retained (`"///"` becomes `"/"`).
It calls actual `0041DD40` with preserve=1 even for unchanged length. High-bit
bytes and embedded NUL follow CRT character-set search semantics; no slash-only
or ASCII-only restriction is introduced.

`00BDCE40` captures source priority and compares the actual source/destination
string-header addresses before storing priority and zeroing the destination
string. Identical headers skip string copying **after** zeroing; self-copy does
not release the old block. `00BDCFC0` follows the same schedule while capturing
`*priority_dword` before any destination store, including when that pointer
aliases destination storage. Both call resize with current source length and
preserve=1, then reload source length. A nonzero reloaded length causes current
destination length, current source data and current destination data to be
captured in that order for `00BF7680`. Provider and byte are reloaded after the
string operation and stored in order. The complete native `_memcpy` body has a
backward overlap path; the source calls standard `memmove`, retaining the correct
native library name in evidence. A zero-byte library call is omitted after the
same field reads, consistent with the existing actual string source contract.

`00BDEEC0` captures EDX into a local priority DWORD, initializes its completion
bit to zero and arms state 1 before calling `00BDCFC0`. The source's explicit
payload pointer denotes consumed argument storage. On successful construction,
the native body loads the argument's current data, marks the destination complete,
sets state 0, and, for nonnull data, captures current length+1 before the getter
`00419CC0` and return `00BD1510`. Cleanup leaves argument bytes untouched.
The explicit source API does not reproduce mutable native EH spill identity.

FuncInfo `00E00A6C..00E00A8F` points at the two-entry unwind map
`00E00A5C..00E00A6B`:

| State | Next | Native action |
| --- | --- | --- |
| 1 | 0 | `00CC6520..00CC6527`: use argument at EBP+4, tail-call `00BDB550` |
| 0 | -1 | `00CC6528..00CC6540`: test/clear completion bit at EBP-14h; if set use captured destination at EBP-10h and tail-call `00BDB570` |

`00BDB550..00BDB56C` releases current argument data with current length+1;
`00BDB570..00BDB58D` releases the record string at +4/+8. Both capture the block
and size before the pool getter. Thus construction failure releases the consumed
argument and skips incomplete destination cleanup. Native argument-cleanup
failure after construction would release the completed destination. Existing
`NativeStringStorage::release` and the source string destructor are `noexcept`;
throwing native getters/cleanup and simultaneous cleanup exceptions remain
outside that source contract. The raw handler `00CC6541..00CC654A` loads FuncInfo
then jumps from `00CC6546` to `00BF6B43`; it is explicitly
`no_ghidra_function`, not a newly defined or reconstructed routine. No gaps were
promoted to functions or extra routine counts.

All 11 trim call sites in ten current stored callers were checked, plus the
`00BE1740` calls to the two record constructors and the internal `00BDEEFF` call.
These are bounded call-context checks, not whole-caller reconstruction claims.
The four complete bodies contain nine direct calls. Six additional direct/tail
rows cover the consumed cleanup and unwind helpers; the raw handler transfer is
reported separately with its inclusive byte range. Running
`tools/verify_report_calls.py` checked 28 numeric rows: 27 passed, while the exact
`0058413C -> 00BF86F0` call exposed a verifier/body-range issue. The live `_strchr`
body minimum is `00BF86E0`, with its callable prologue at `00BF86F0`; the checker
uses body minimum as function start. The original numeric row is retained and
the failed log is preserved for primary review. No library rename or function
boundary mutation was made by this worker.

Strict Win32 `scripts/build.ps1` passed with `MSBUILDDISABLENODEREUSE=1`, the
project's `/W4 /WX /fp:strict`, verified seeds, and both existing CTests. No
permanent test was added. The ignored focused fixture is
`local/mount-records-aw/attempt01`: nine pairs of physical native/source result
files compare normalized record/string/padding state, pool state, and ordered
string allocation/return traces. Cases cover ordinary/right-edge/high-bit trim,
early trim guards, callback-driven source length/provider/byte reload, self-copy,
aliased priority capture, and nonempty/empty consumed by-value arguments. One
source-only allocation failure verifies argument cleanup and untouched incomplete
destination fields. All 711 checks passed, including complete fixture-owned CRT
allocation/free bookkeeping and raw canonical singleton cleanup.

The probe executes all four unchanged original bodies, 403 bytes shifted by
`30000000h`; nine calls and 21 total relative transfers were checked before
execution. Five external bridges call actual resize, CRT search/copy, and the
actual canonical raw-domain string pool getter/return source. The internal call
to `00BDCFC0` executes its unchanged shifted original body. Native FH3/SEH is not
executed. All 11 captured spans (557 bytes including EH/data) matched live Ghidra
and the installed PE. There are no numeric table pages or launcher in this probe.

Before execution, 518 physical inputs were copied and sealed read-only, including
actual source, recursive BSP headers, 120 linked object copies checked byte-for-
byte against their archive members, all linked BSP archives, native captures,
probe source/object/executable, map and build records. Every before hash was
rechecked after execution. Manifest SHA-256:
`de7e9e16c77dc82fb4b545143f7b42369ab31b94db6b2ddf3fe274850a627db8`.
Probe SHA-256:
`0ae6e64d1122f3e17e8f3be43f96d5d68f0f23452750ff890aef4bd8d74b10e4`.
The one failed caller-context capture is preserved as preparation evidence; it
failed before fixture compilation/execution and was repaired using exact stored
function listings. No fixture attempt failed or was overwritten.

Relink using `python local/mount-records-aw/run.py --repo <exact-checkout>
--attempt <new-absolute-directory>`. It refuses an existing attempt, freezes the
selected checkout's actual linked inputs, verifies original bytes, compiles with
an embedded manifest/fixed high base, seals before execution, and compares the
results and hashes afterward. Whole archive hashes and unlinked code are not
runtime coverage. These are source and bounded native-body composition proofs,
not drop-in original ABI/FH3 compatibility or `bsp_game`/gameplay validation.
