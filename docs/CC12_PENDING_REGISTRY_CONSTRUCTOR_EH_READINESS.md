# CC12 pending registry constructor unwind readiness

Baseline: `15cc2cc1e9c1403c83618589333d1b42a998305f`. This read-only packet follows
handler `00C963E8`, independently captures its FuncInfo/unwind map, and captures
only the directly named action. It adds no Source, ledger/CMake or Ghidra change.
Evidence: [cc12_pending_registry_constructor_EH_readiness.json](../reports/cc12_pending_registry_constructor_EH_readiness.json).

## Result and remaining boundary

The constructor has **one encoded unwind action**: state index zero has next
state `-1` and action `00C963E0`. That complete eight-byte action executes
`MOV ECX,[EBP-10h]` followed by a tail `JMP 008748F0`.

**The cleanup effects remain held.** No current concrete Source binding was
found for actual `008748F0`, and its native body is outside this packet. The
wrapper establishes the frame-slot read and transfer, not publication clearing,
profile restoration, section release, owner free or any RAII policy. Production
Source binding remains unready until that specific cleanup boundary is known.

The full bounded capture is **68 file-backed bytes: 62 code/data bytes plus six
alignment bytes**. All match the installed PE. All scope limits passed; no map
or action expansion beyond the requested bounds was needed.

## Handler and absolute x86 record

The constructor's pinned ordinary body pushes handler `C963E8`. At that address,
ten physical bytes execute `MOV EAX,DC84DC`; `JMP BF6B43`. Six following `CC`
bytes are alignment, not handler instructions. The 16-byte capture fits the
32-byte limit. Ghidra has no function starting at `C963E8`; this packet did not
create one or repair its enclosing listing.

The handler replaces incoming EAX with the FuncInfo address. It does not alter
ECX/EDX/EBP/ESP, push a return address or establish the shared helper's frame
protocol. `BF6B43` remains an unexpanded runtime boundary.

The 36-byte absolute x86 record at **`00DC84DC`** contains:

| Field | Encoded value |
| --- | --- |
| Magic / upper BBT bits | `19930522` / zero |
| maxState | `1` |
| Unwind-map pointer | `00DC84D4` |
| Try count / map pointer | zero / zero |
| IP-map count / pointer | zero / zero |
| Exception-type-list pointer | zero |
| EHFlags | `1` |

The eight-byte map at `DC84D4` is the single pair `{toState=-1,
action=C963E0}`. It fits the four-entry limit. No constructor try/catch or IP map
is encoded in this record. Encoded flags are retained as data; they do not prove
the original runtime's hardware-fault policy.

The current installed MSVC `ehdata.h` supplies corroborating absolute x86
layouts: signed `__ehstate_t`, an eight-byte state/action pair, and the 36-byte
FuncInfo record. Its exact path, SHA-256 and inspected ranges are in the report.
This is a layout reference, not original compiler/runtime compatibility proof.
The Application `CC7260` audit supplied that reference and qualification example;
none of its native states or actions are used as this constructor's evidence.

## Complete action and physical frame inputs

`C963E0..C963E7` is a complete listed function: **eight bytes, two instructions,
zero physical CALLs**. It first loads the **current** DWORD at `[EBP-10h]` into
ECX and then jumps to `8748F0`. It has no push/pop, return instruction, ESP/EBP
write, owner/global store or local catch. The tail transfer leaves its incoming
stack unchanged, including any return address supplied by the runtime helper.
That helper's actual invocation protocol remains unproved.

Let `S` be ESP on entry to the already audited ordinary constructor `874BC0`.
Its actual pushes and stores establish these slots:

| Slot | Address | Ordinary evidence |
| --- | --- | --- |
| State | `S-04h` | Initial `FFFFFFFF`; full zero at `874BDD` |
| Handler | `S-08h` | Pushed `C963E8` |
| Previous exception chain | `S-0Ch` | Pushed old `FS:[0]` |
| Receiver spill | `S-10h` | Pushed ECX; explicit captured ESI store at `874BD9` |
| Saved caller ESI | `S-14h` | Push at `874BD6` |

If the shared helper supplies **EBP = S**, the action's `[EBP-10h]` reads the
constructor receiver spill. This required equality is not independently proved
by the unexpanded `BF6B43` body; the ordinary constructor never sets EBP. The
action rereads the current slot, so a frozen receiver, a fresh `F878CC` lookup
or an assumed immutable spill would be a different contract.

## State timing and cleanup qualifications

The constructor arms full state zero **before** writing profile `D0DEA0` and
calling `BD1860`. The section field `+04` is written only after that service
returns normally. No lower-state write occurs before the section store or
ordinary epilogue. There is no additional encoded constructor state/action for
a returned section.

The map supplies the pair for state zero; it does not by itself prove the
runtime's ordering of state publication, action invocation and nested failure
handling. `8748F0` may have further effects or fail nonlocally. Original exception
preservation/replacement, termination behavior, remaining unwinds and hardware
fault compatibility are unresolved. No `noexcept` or successful-cleanup promise
is introduced.

Current `.hpp/.cpp/.inc` searches under `include/` and `src/` found no binding for
`8748f0`, `c963e0`, `c963e8`, `dc84dc` or `dc84d4`. Therefore this audit pins no
concrete cleanup service. Another owner's same-layout reset, generic base reset
or guard release cannot substitute for the actual target. The ordinary section
creation service remains prior constructor evidence, not cleanup evidence.

The next bounded prerequisite is a separately size-gated `008748F0` cleanup
audit. Getter handler `C96433`, owner retirement, callable table delivery,
canonical `F878CC` ownership and prior pending/group composition contracts
remain separate. Absence of free/unlock instructions in this eight-byte wrapper
does not establish absence of cleanup elsewhere in the call chain.

## Verification

Queries verified existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 LE32 image base `00400000`. Live and saved total
function counts both remained 64,729. The complete installed PE SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Four fresh windows independently verify the handler, record, map and action.
Capstone covers all 18 semantic code bytes: four instructions, zero CALLs, two
tail JMPs. Six current prior-file pins, nine inherited constructor pins and the
current SDK header pin pass. The prior constructor's full 69-byte window was
rechecked against the PE without querying its live body again.

JSON parsing and owned-file diff checks complete the audit. No shared runtime,
cleanup descendant, getter EH, unrelated FH3, caller, table or group-bootstrap
body was expanded. No build, test, probe, runtime run, Source implementation or
new Original credit is claimed.
