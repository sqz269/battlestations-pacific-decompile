# Locale startup constructor and owner readiness

The next bounded implementation is a **dormant raw locale constructor/base** for
`0073C960` and `00736540`, using the existing `LocaleTableManagerImage` layout.
The current application still creates a semantic `GameLocaleHost`; it has no
equivalent native publication, registration, or deletion contract. This packet
does not authorize or implement application activation. It changes no C++,
Ghidra listing, build output, tests, or runtime state.

## Native boundary and captured allocation

The explicitly assigned `0073E030..0073E064` caller window is 53 bytes: 51 bytes
of complete instructions and the first two bytes `89 9C` of the instruction at
`0073E063`. Root subsequently admitted exactly `0073E065..0073E069`; the extended
58-byte window decodes completely. The original 53-byte capture remains retained
separately. No instruction beyond `0073E069` or later font startup was admitted.

| Address | Established behavior |
|---|---|
| `0073E030` | Push `4040h` as the allocator argument. |
| `0073E035` | Clear the preceding construction state with captured `EBX=-1`. |
| `0073E03C` | Call `00BF681B`, then discard its four-byte argument. |
| `0073E044` | Save returned `EAX` at `[ESP+10h]`. |
| `0073E048..0073E055` | Compare against `EDI=0`, set caller state `24h`, and skip construction on null. |
| `0073E057..0073E059` | Move that captured allocation into `ECX`, then call **`0073C960`**. |
| `0073E05E` | Both paths continue with `ECX=00F88980`. The constructor return is not consumed in this window. |
| `0073E063` | Clear caller construction state using `EBX=-1`; the extended window ends after this instruction. |

The accepted enclosing `0073D410` prologue/register evidence supplies `EDI=0`,
`EBX=-1` and the EH-frame relationship; these are labeled retained prior evidence,
not fresh full-caller recovery. Its accepted handler/FuncInfo lead to unwind map
`00DB60C0`. Fresh entry `00DB61E0` for state `24h` is `{-1,00C86C16}`. The cleanup
loads the captured allocation from `[EBP-138h]`, calls `00BF65AC` (`free`), pops
the argument and returns. It does not read the current locale publication, call
a locale destructor, undo registry changes, or repair a publication.

Ghidra currently describes `00C86C16..00C86C21` as a 12-byte function. The
necessary finite 14-byte raw span includes `POP ECX; RET` at `00C86C22..23`.
That metadata gap remains explicit; no function-boundary repair was performed.
Allocator-body behavior and exceptional allocator return policy were not freshly
reconstructed. A constructor failure is distinct from an allocator failure.

## Complete constructor and base

Fresh live bytes, typed metadata/decompilation, and register-dependent assembly
cover the entire `0073C960..0073C9EC` body: 141 bytes, 39 instructions. `ESI`
captures the incoming `ECX`, and the base call at `0073C97B` receives the same
receiver. On successful return, `EAX=ESI`, followed by plain `RET`. No stack
argument or semantic EDX input appears in the body. The decompiler's inferred
`__fastcall` is not a recovered declaration; live typed metadata still reports
an untyped no-parameter function. Added C++ interfaces cannot be called native
ABI-compatible on that basis.

`00736540..007365D0` is also fully captured: 145 bytes, 40 instructions. Its
observable normal order is:

1. Capture `ECX` in `EDI`, arm state 0, and store base profile `00CFEA5C`.
2. Call `00415350` and capture that manager's raw section pointer at `+10h`.
3. If nonnull, enter the captured critical section and increment its current
   physical DWORD at `+18h`; then arm state 1.
4. Publish the captured receiver in `00F8BC4C`.
5. Call `00415350` again, **then reload the mutable `00F8BC4C` cell** as the
   object argument to `00BD0C30` on the returned manager. The registry argument
   is not necessarily the originally captured receiver under external mutation.
6. If the originally captured section is nonnull, decrement its current `+18h`
   word and leave that same section. Return the captured receiver in `EAX`.

The fresh base FuncInfo at `00DB5388` points to two unwind entries at `00DB5378`:
state 1 invokes `00C85F48`, then state 0 invokes `00C85F40`. The first tail-jumps
to the existing `00411EE0` guard destructor using the actual stack guard. It
captures guard `+4`, stores guard profile `00CE37FC`, decrements the captured
section's current counter, and leaves it if nonnull. The second tail-jumps to
`00412430`, which resets the receiver's profile to `00CE3818`. Neither cleanup
clears `00F8BC4C`, removes a registered slot, or frees the caller allocation.
The 25-byte guard and 7-byte root-reset bodies were freshly captured and decoded.

After the base returns, the derived constructor writes:

| Offset | Initialization |
|---|---|
| `+0000` | Derived profile `00CFF140`. |
| `+0008,+000C,+0010` | Zero table-name vector pointer triple. |
| `+0014..+4013` | `memset` of exactly `4000h` bytes: 1000h four-byte bucket heads. |
| `+4014` | Zero entry count. |
| `+4018,+401C` | Zero language string size/data pair. |
| `+4024,+4028,+402C` | Zero first sidecar vector pointer triple. |
| `+4034,+4038,+403C` | Zero second sidecar vector pointer triple. |

The allocation's words at `+4`, `+4020`, and `+4030` remain untouched. A whole
object zero-fill would change the recovered behavior. Native derived field order
is preserved in the retained full instruction listing, including the EH-link
load between the first and second sidecar triples.

The constructor has an FH3 handler and five unwind-map entries, but its complete
body retains EH state `-1`: no instruction arms those five states. The entries
were retained as data; their action bodies were not admitted or reconstructed.
Do not infer a derived member cleanup or base unregister from the mere presence
of that table. Ordinary C++ throw edges through the base, original FH3 stack
aliases, arbitrary SEH faults, concurrency, and second-unwind exceptions remain
distinct claims.

## Current Source and compiled evidence

`include/bsp/gui_startup.hpp` already fixes the exact `4040h` layout with offset
and size assertions. `construct_locale_table_manager_0073c960` in
`src/gui_startup.cpp` initializes the derived profile and fields only. It is a
`void noexcept` helper: it does not call the base, publish, register, or return
the receiver. Its comment acknowledging the native base does not implement it.
The current Core object contains a complete 146-byte helper section whose only
relocation is `_memset`. This remains fragment credit.

The active `GameStartupHost::run_initialize_phases` creates
`new GameLocaleHost(log_)` immediately after the newly activated particle owner,
then calls its semantic `initialize`. `GameLocaleHost` owns a `LocaleTables`
projection plus a lazy GUI refresh object; it has no raw locale lifetime binding.
The actual compiled initializer pushes **`4064h`**, calls C++ `operator new`,
inlines the semantic constructor, and later calls `GameLocaleHost::initialize`.
That allocation size and standard-container layout are not the native `4040h`
receiver contract.

The semantic initializer performs setter, `globals` registration, and reload
in that order. Existing menu/text consumers use its `LocaleTables`, and host
destruction deletes the semantic locale after the singleton drain. No behavior
credit for those semantic paths supplies a native `00CFF140` deletion binding.
The current raw singleton dispatcher has no admitted locale profile case.

The portable replay reparses the entire frozen Core archive (1991 entries,
1988 COFF objects) and all 70 frozen application object symbol indices. It finds
one Core definition of the existing field helper and no mention of that helper
in those 70 App symbol tables. Whole selected objects, symbols, sections and
relocations are retained. Five complete code sections are decoded, rather than
using a matching prefix or merely a symbol hit:

| Object / section | Bytes |
|---|---:|
| `gui_startup`: existing locale field helper | 146 |
| `game_hosts`: standalone semantic `GameLocaleHost` constructor | 248 |
| `game_hosts`: application initializer wrapper | 141 |
| `game_hosts`: complete `run_initialize_phases` | 12748 |
| `game_hosts_vfs`: semantic locale initializer | 544 |

These are current Source compiler artifacts, not an Original-versus-Source
runtime fixture or a native calling-convention proof.

## Source epoch, replay, and qualification

The exact accepted epoch is **`Source3352-particle-startup`**, from Root's
`local/cc12_particle_startup_Source_primary/Source3352_particle_startup_frozen.json`.
All 3352 selected inputs and four artifacts were checked against both the frozen
bytes and the live integrator before retention. The four artifacts are the whole
Core library, game executable, map and prior `LastTest.log`; retaining that log
does not represent a new test run.

The worker and committed Git cohort are separately qualified. All 3352 committed
Git blobs match the worker byte-for-byte; 3345 match the compiler-input Source
cohort byte-for-byte. Seven differ only by CRLF/LF: `cmake/startup.cmake`, the
sound configuration/startup headers and sources, `src/game_sound_runtime.cpp`,
and an older pending-registry readiness report. All seven byte variants and the
full Git blob identities are retained. They are not silently relabeled as the
Root compiler-input bytes.

The bundle retains 3522 payloads, including 15 freshly captured Native spans
(623 retained bytes, with intentional overlaps), all Source inputs, all four
artifacts, all 70 application objects and full COFF indices. Every fresh Native
span also matched the configured original PE's disk bytes. Typed `bsp.py`
queries verified project `bsp`, program `/battlestationspacific.exe`, x86 language
and base before each query; configured project remains `C:/Users/sqz269/bsp.gpr`.
Supporting tool/config source is retained. No Original executable was run.

Evidence: `local/cc12_locale_startup_constructor_owner_readiness/evidence.zip`,
43,819,024 bytes, SHA-256
`6462b203010cb44e7f5a23362d10eeac297c14466778ea06ad0dea750a03a903`.
`portable_replay.py evidence.zip` passes in the packet directory and in a
separate physical copy. An audit guard denies every external file read after
runtime imports and forbids process/network operations; a deliberate live-source
read is rejected in both runs. The replay checks retained bytes and capture
receipts; it does not claim the live checkout remains unchanged later.

## Minimal next frontier

1. **Ready for a separately authorized dormant implementation:** raw constructor
   and registration base over the existing `LocaleTableManagerImage`, with a
   stable binding to the actual manager and locale publication cells. Preserve
   receiver capture, two manager lookups, publication reload, captured-section
   cleanup, untouched words, receiver return and the base's failure side effects.
   Reuse already admitted actual guard/reset/registry providers and qualify their
   contracts; do not add an application owner or a destructor to this packet.
2. **Required before owner activation:** recover and implement the locale raw
   unregister/destructor/scalar-delete and profile dispatch, establish context
   lifetime through the drain, then replace or bridge current setters/loaders,
   lookup and menu/text consumers onto the same actual storage. Running a raw
   singleton alongside an unrelated semantic locale would not establish parity.
3. **Required before caller activation:** prepare the complete domain before
   allocation; implement the `4040h` allocation/null/constructor handover and
   captured-allocation-only failure cleanup at this exact startup position.
   Preserve publication/registry failure effects and contain unsafe later drain
   paths rather than repairing them speculatively. Later locale startup calls
   and font construction remain separately bounded work.

Readiness is established for the first item only. No C++ implementation,
activation, new build/CTest, Original fixture, ABI, startup process or gameplay
validation is claimed by this review.
