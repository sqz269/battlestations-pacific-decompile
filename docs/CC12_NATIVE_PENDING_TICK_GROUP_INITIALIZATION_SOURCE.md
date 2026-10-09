# Pending tick group sentinel initialization: Source candidate

This unregistered candidate supplies the missing borrowed link-initialization
operation identified by `CC12_PENDING_GROUP_SOURCE_BACKING_LIFECYCLE_READINESS.md`.
It takes the actual first group head in ECX and performs exactly twenty ordered
DWORD stores across five groups. It creates no backing storage or production
call. Compilation, emitted-code review, and any Source fragment admission remain
with the integrator.

Candidate baseline: `ac9205e2864b6d8604955dcd16a7032b8cc6d313`. The public API is:

```cpp
void __fastcall initialize_native_pending_tick_group_sentinels_00cd27c0(
    void* actual_first_group_head);
```

The MSVC Win32 definition is naked and contains only its inline assembly. It
does not declare `noexcept`. The header rejects other target/compiler pairs;
the source also requires four-byte pointers. There is no new complete group
type, static array, owner, default object, allocator, guard, child call, local
exception handler, iterator, flush implementation, or caller.

## Retained Native evidence

The accepted entry receipt
`reports/cc12_pending_group_initializer_entry_readiness.json` covers
`00CD27C0..00CD2858`: 153 Original bytes, 22 operations, SHA-256
`ef403713f0319a915381e64216765bf0c561637508f0519db8e3cea0031649c2`.
It records no saved Ghidra function or listing at the entry or return. This
candidate retains that bounded fragment qualification; it neither creates a
Ghidra function nor claims to have checked a newer saved-analysis state.
The descriptive Source symbol is not a recovered Native symbol.

The Native fragment starts with `XOR EAX,EAX`, performs twenty absolute DWORD
stores, and ends in plain `RET`. Its five heads are
`00F876C0 + index*0x68`, with each tail at `head+0x34`. It has no input argument,
calls, object reads, local stack frame, or local EH setup. Its accepted normal
path leaves ECX/EDX and the nonvolatile registers untouched, returns EAX zero,
and retains the XOR arithmetic flags through the MOVs and RET. These are prior
receipt facts; no Native bytes, code, artifacts, or Ghidra queries were opened
by this candidate packet.

The accepted newer CRT-route receipt
`reports/cc12_pending_group_crt_initializer_route_readiness.json` places the
selected initializer at DWORD slot `00CE3020`, index 571 in the half-open
1004-slot void table `00CE2734..00CE36E4`. The accepted PE entry/startup route
reaches that walk through `00BFBC47` only after the integer initializer walk
returns zero. Reaching and calling the selected word also requires normal
earlier callback returns, preserved ESI/EDI and usable stack, and the freshly
read word still naming `00CD27C0`. This static conditional route precedes the
Native WinMain call on the accepted normal path; it establishes no Source
startup call or execution. Earlier entry-receipt table uncertainty is resolved
by that newer receipt only to this extent.

## Source write schedule

Let `B` be the actual borrowed first head, `H_i = B + i*0x68`, and
`T_i = H_i+0x34`, using 32-bit x86 effective-address arithmetic. Each group is
completed before the next. The table is in actual store order.

| Group | Native store instruction | Source destination | Source DWORD value |
| --- | --- | --- | --- |
| 0 | `00CD27C2` | `B+0x004` | `0` |
| 0 | `00CD27C7` | `B+0x008` | actual `B+0x034` |
| 0 | `00CD27D1` | `B+0x03C` | `0` |
| 0 | `00CD27D6` | `B+0x038` | actual `B` |
| 1 | `00CD27E0` | `B+0x06C` | `0` |
| 1 | `00CD27E5` | `B+0x070` | actual `B+0x09C` |
| 1 | `00CD27EF` | `B+0x0A4` | `0` |
| 1 | `00CD27F4` | `B+0x0A0` | actual `B+0x068` |
| 2 | `00CD27FE` | `B+0x0D4` | `0` |
| 2 | `00CD2803` | `B+0x0D8` | actual `B+0x104` |
| 2 | `00CD280D` | `B+0x10C` | `0` |
| 2 | `00CD2812` | `B+0x108` | actual `B+0x0D0` |
| 3 | `00CD281C` | `B+0x13C` | `0` |
| 3 | `00CD2821` | `B+0x140` | actual `B+0x16C` |
| 3 | `00CD282B` | `B+0x174` | `0` |
| 3 | `00CD2830` | `B+0x170` | actual `B+0x138` |
| 4 | `00CD283A` | `B+0x1A4` | `0` |
| 4 | `00CD283F` | `B+0x1A8` | actual `B+0x1D4` |
| 4 | `00CD2849` | `B+0x1DC` | `0` |
| 4 | `00CD284E` | `B+0x1D8` | actual `B+0x1A0` |

Before each head-to-tail store, a Source-only `LEA EDX,[ECX+tail_offset]`
computes the actual tail. Before each tail-to-head store, another LEA computes
the actual head. Group zero uses `LEA EDX,[ECX]` for its head. Thus the source
contains 32 operations: one XOR, twenty MOVs, ten LEAs, and one plain RET.
The extra LEAs touch no memory and change no flags. No ADD, loop, condition,
stack argument, or saved-register slot is introduced. Every MOV preserves
the accepted store order; the zero values come from EAX throughout.

The highest store is the group-four tail-next DWORD at `B+0x1DC..B+0x1DF`.
The subsequent final store is at `B+0x1D8..B+0x1DB`. The selected writable fields
are sparse: `0x1E0` is a contiguous access bound measured from B, not proof of
a complete `5*0x68 == 0x208` object, and not a request to clear the intervening
bytes. No profile, count, payload, other field, or general zero-fill is added.
The caller must provide usable writable addresses for those fields and retain
the actual identities for later consumers. There is no null check, wrap check,
alignment normalization, alias check, copying of backing, or fault recovery.
The code reads no old field value and does not reorder stores to accommodate
aliases. On a fault, earlier writes can already have been published.

## New Source interface and machine boundary

This Source entry deliberately takes one actual pointer in ECX. The Native
fragment instead has no argument and embeds the fixed addresses and pointer
words. When B equals the accepted Native base in usable backing, the selected
store destinations and values coincide; arbitrary borrowed Source backing is
a relative translation and is not Native canonical-address ownership.

For an unfaulted pass through the declared assembly, EAX is zero, ECX retains
B, and EDX ends at `B+0x1A0` (the last head). EBX, ESI, EDI and EBP are not
touched. There is no local stack adjustment; plain RET consumes the return
word, giving entry ESP+4 on a normal return with intact control-stack backing.
The Native fragment preserves incoming EDX, so this scratch use alone prevents
an Original-entry ABI claim. Source ECX now has an input meaning even though
the assembly retains it. The C++ return type is void; zero EAX is an assembly
effect, not a public return-value API.

XOR establishes `CF=OF=SF=0`, `ZF=PF=1`, and undefined AF. The following LEAs,
MOVs and RET leave those arithmetic flags unchanged. The body does not modify
the direction flag, x87 state, or SIMD state. There is no LOCK or concurrency
protocol. Ordinary storage and intact return backing remain caller
requirements; hardware faults, asynchronous observation and reinitialization
of live links are not made equivalent or safe by this interface.

These statements describe the Source assembly schedule. No object, Core
library, linker map, instruction encoding, relocation, import, generated EH
section, Original caller, startup execution, or gameplay has been inspected
for this candidate. In particular, 153 is the accepted Native fragment size,
not an emitted Source size.

## Current backing and production gaps

The accepted Source audit at
`b5b3773626bd37c32fae56f6336de65e04ec7f89` remains unchanged in the inspected
providers at this candidate baseline. Its exact excerpts show that
`GameMissionFrameHost::Impl` constructs and owns
`GameFixedStepHost(log,dynamics)`, with no group backing member or borrow.
`next_element` discards its arguments and always returns false. Queue/dispatch
and the pending-registration flush count/log work without providing the missing
group splice. The actual `FixedStepBinding` delegates to that same host.

The current abstract interface is `UnitInstanceCreationHost`. The scoped audit
found no implementing host or caller of its generic `create_unit_instance`.
The admitted raw tick-registration constructor has no production Source caller
and borrows its separately supplied pending tail. `get_tick_group` and
`get_tick_group_storage` are absent from current `include/bsp` and `src`.
The new initializer's only Source symbol matches are this header and definition;
the accepted older absence search for `CD27C0` is expressly pre-candidate.

`GameNativeReadOnlyData` covers `00CE2000..00E07B23`, returns const pointers,
and protects its copied data as read-only; it cannot supply the F8 group base.
`GameNativeMutableCrtData` commits only the three pages at `00E15000`,
`00E16000`, and `0109E000`. Their joint canonical owner adds no group backing.
This candidate does not change any of those providers or cast their storage.

The unresolved integration still needs actual writable storage and its owner
lifetime, shared group identities across producer/flush/iterator, the separate
`00E0B6D0`/`00E0B704` pending pair and registry binding, and a production call
before first use with a supported repeat-initialization policy. Replacing the
logging/always-false boundaries and validating startup/gameplay remain separate
work. The Native CRT table does not select that Source owner or call.

## Candidate validation and credit

The report pins the three candidate text files, unchanged baseline inputs,
the accepted initializer/CRT-route/backing reports, and the exact Source
assembly-to-store mapping. It replays the preceding Source audit's 27 direct
pins and 188 historical receipt references, including the current 61 build
pins and historical 57/38 pin sets. The two older CMake differences, historical
ledger drift and CRT-route document CRLF/LF qualification remain explicit;
none is reclassified as current build or execution proof.

Only the two new C++ files, this document, and the candidate report change.
There is no CMake registration, ledger edit, Ghidra mutation, build, probe,
test or production call. Static report validation checks the complete 32
operations against the accepted twenty writes and ten address-forming LEAs;
it does not establish compiled or runtime behavior. All Source admission,
Original ABI, startup and gameplay credit remains zero pending integrator
review. The 153 accepted Native bytes are referenced once as fragment evidence.
