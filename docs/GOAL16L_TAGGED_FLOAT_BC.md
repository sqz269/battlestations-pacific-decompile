# GOAL16L: three complete physical x87 getters

Status: **READY_WHOLE_REGISTERED_NORMAL_BUILD_CORE_VERIFIED**. B4 authored
and froze the strict-prepared Source; the warm successor ran the actual normal
CMake build and existing checks, verified the entire registered object/Core
member, and only then recorded the three owned ledger entries. Independent
review, Original ABI/declaration recovery, caller adoption and game validation
remain separate.

| Original entry | Complete Native and strict body | Read offset |
| --- | --- | --- |
| 00BC3010 | `D9 41 2C C3` — `FLD m32real [ECX+2Ch]; RET` | 2Ch |
| 00BC7AA0 | `D9 41 34 C3` — `FLD m32real [ECX+34h]; RET` | 34h |
| 00BC7AB0 | `D9 41 1C C3` — `FLD m32real [ECX+1Ch]; RET` | 1Ch |

Fresh whole typed single ranges, live bytes/listings and the installed I386 PE
agree on all three four-byte bodies: 12 bytes, six instructions. The saved
Ghidra project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Current names `TRIV_body_00bc3010`,
`BSP_CloudPuff_GetSortKey` and `BSP_CloudPuff_GetSize` remain unchanged.
Existing `undefined name(void)` prototypes do not establish Original ABI.

The new C++17 interfaces take a raw backing address in ECX and expose ST(0)
through a float return declaration. This is a descriptive callable Source
projection, not recovery of Original float-versus-double spelling, class,
receiver extent or calling-convention declaration. Native comparator callers
consume the result from ST(0), including a subsequent double store; that
consumer also does not settle the Original formal return spelling.

Each naked MSVC Win32 body performs the literal complete `FLD` and `RET`.
It adds no bit normalization, helper, environment reset, stack-empty check,
extra load or `FSTP`. The first instruction reads one four-byte real from the
wrapping effective address ECX plus the stated offset. The inherited x87
control/status/tags/TOP/stack, SNaN and denormal processing, pending exceptions,
masked/unmasked exceptions and stack-fault behavior remain the actual `FLD`
instruction behavior. Exceptional cases are not reduced to normal-value
semantics or assumed always to push. EAX/EBX/ECX/EDX/ESI/EDI/EBP and EFLAGS are preserved, with plain
`RET` consuming the return slot on normal completion (ESP +4).

The first-instruction memory and FP fault schedule is retained statically.
Runtime exceptional and inaccessible-memory behavior was not probed. Absolute
fault EIP and saved FIP follow relocated code placement: byte identity does
not establish equality of absolute saved FP-environment pointers, placement,
full ABI or game behavior. Intel's [instruction reference](https://cdrdv2-public.intel.com/868137/325462-089-sdm-vol-1-2abcd-3abcd-4.pdf)
defines the retained operation; Microsoft's [fastcall documentation](https://learn.microsoft.com/en-us/cpp/cpp/fastcall?view=msvc-160)
documents the new declaration's register argument mapping.

The full existing Source/header search found caller inline approximations and
an abstract interface, not these complete standalone getters. In particular,
`water_weather_cc13.cpp` uses `w2_x87_reload_bits` for the BC3010 caller; that
helper quiets signaling-NaN bits in software and cannot stand for the complete
physical load/return, inherited x87 state or exception prefixes. Those prior
files and all earlier review artifacts remain unchanged.

Strict compilation used `/std:c++17 /MD /O2 /W4 /WX /fp:strict /Gy` and passed
in 1.6912838 seconds. Independent full COFF inspection found exactly three
four-byte executable sections, six decoded instructions, zero relocations in
all sections, zero undefined externals and zero extra helpers. Strict object
SHA-256: `b8e46da80db1f8158097082d7241678b06691553df783571ff01b4ce4d6ced22`.
Source and evidence hashes, complete bodies and capture details are recorded in
`reports/goal16l_tagged_float_bc.json` and
`local/goal16l_tagged_float_bc/strict_proof.json`.

The preparation worktree was cold and ran no normal build, tests, probes or
Native execution. Its exact preparation commit
3825b0b29567da39813e21bffa1230f07b7117a2 was cherry-picked alone onto the
genuinely warm H baseff4f6b0d3da512726037e131b33865e1be146b39 as77c174bb6.
Source/header remain byte-identical modulo line endings to the prepared blobs;
no Source or header changes followed successful compilation/build.

The CMake append conflict was preserved in all three stages. The prescribed
merge_resolve tool refuses cherry-picks, so Root was informed and the exact
warm file plus B4's ONE original registration line was retained manually.
An initial guard assumed another registration spelling and failed before a
write; reading the actual frozen registration fixed the guard. No unrelated
Main registrations or Sources entered this warm build. Initial output lease
overlap was queued until B4 explicitly released; no addresses/files were
claimed beforehand. All transfer/classification/wait costs are included.

One normal warm scripts/build.ps1 invocation passed in17.2918530s
(2026-10-16T08:08:08.5373803Z..2026-10-16T08:08:25.8319642Z); existing reconstructed_math0.20s and
tool_tests6.91s passed. No new tests/probes or getter/Native execution. Native
reference seeds are not enabled in this worker; Root's later three-check build
is separate. The actual C++17/O2/Oy-/MD/W4/WX/fp:strict/EHsc command and complete
TLOG/project/Source/header inputs are frozen.

The actual registered object is993B with exactly THREE complete executable
sections4B each, all public symbols at0: entire12B/six instructions equals the
strict/live/installed bytes. All object sections, raw symbols/aux/string table,
metadata and directives are retained. There are ZERO relocations in ANY
section, ZERO undefined symbols, ZERO extra helpers and ZERO writable globals.
The strict object is965B; the registered debug metadata section is208B rather
than180B, accounting for the28-byte size difference. Code remains exactly12B.

Actual registered SHA256 `6b2c2f2537af1683747858422854fb2878a37728061dff03c0fc6b15e93a04da`.
Actual whole Core archive SHA256 `eefc40b60a65269028c1d62e674105f7c2be1fb2c044d8b8dc55da100237c3bf`. The entire993B member
occurs uniquely at headeroffset20054476; BOTH MS archive linker dictionaries
independently map all three exact public names to that one payload. All2397
prior Core objects remain whole-byte unchanged; only this new object was added.
This proves actual registered archive inclusion, not that unused getter code
was adopted into a final executable or that callers executed it.

All H/F/Z prior625 receipt artifacts/all629 H-directory files, the actual prior
Core, all2397 objects and11 tracking logs were preserved before build. B4's
complete preparation directory, five exact commit blobs and41-file manifest
are frozen. Initial lease wait/CMake refusal/guard correction are retained;
there were no Source/strict/normal-build rejections or reruns in warm closure.

B4 preparation ran07:43:27..08:00:53.091542Z, inclusive1046.091542s, including
strict1.6912838s. Warm author closure starts07:57:57Z beforebrief/preservation
and ends at final receipt sealing after commit/release. These intervals
overlap176.091542s; do not add them as serial throughput. Final HANDOFF/receipt
record exact separate clocks and overall pipeline elapsed. Administrative
B4 release/dispatch after its seal was unmeasured by B4 but lies within the
warm closure interval. No speedup claim.

Only three exact owned name/function ledger appends follow actual build/Core
proof, retaining TRIV_body_00bc3010/BSP_CloudPuff_GetSortKey/BSP_CloudPuff_GetSize.
Physical-only Original prototype/comment proposals are ignored local evidence;
Root owns Ghidra writes, preserved originals, independent review and integration.
No Original class, float-versus-double spelling or full physical ABI is recovered.

Ledger bookkeeping: ordinary add-name refused the two existing reviewed names;
CLI --append-evidence retained their exact names and original hypothesis text.
An initial diagnostic import also encountered the already-added first name;
there are exactly three new function records, with no duplicate function rows
or Source/build changes. This correction is included in closure timing.
