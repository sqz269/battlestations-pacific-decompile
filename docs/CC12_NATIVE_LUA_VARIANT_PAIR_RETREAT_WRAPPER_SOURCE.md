Primary compiled review: Root read every 12 emitted bytes/6 operations and the sole indexed REL 32 edge. Every byte matches Native 006EDE90 outside the operand at 4, which targets the actual admitted Source 137 retreat helper. Physical post-child ESI moves to EAX before saved ESI restoration; result remains conditional on child preservation/control backing. No EH, guard or new consumer is added.

Normal MSVC Win 32 build 2026-10-09T 19:35:55.717080+00:00 to 2026-10-09T 19:36:12.082131+00:00 passed all three existing checks. The combined evidence pins 113 Source/build inputs and four artifacts; all 30 prior objects and the existing 20-function raw-guard provider object are unchanged. The provider is newly included in the capture domain, giving 33 captured/replayed whole objects and 37 selected positive Core definitions. Only the provider's selected 31-byte destroy was newly instruction-reviewed. Both new public roots are absent from the application map. Original ABI, startup and gameplay remain unproved. The worker candidate section below is an immutable Source 109 snapshot; its build artifacts are historical.

# Lua pair retreat wrapper Source candidate

The complete `006EDE90..006EDE9B` wrapper is 12 bytes and six instructions.
This candidate preserves its physical register and stack sequence through
a new naked fastcall entry, calling the actual admitted
`retreat_native_lua_variant_pair_006edd80` provider. Its result is physical
post-child ESI copied to EAX before the wrapper restores its own saved word.

Only these four files are owned:

- `include/bsp/native_lua_variant_pair_retreat_wrapper.hpp`
- `src/native_lua_variant_pair_retreat_wrapper.cpp`
- this document
- `reports/cc12_native_lua_variant_pair_retreat_wrapper_source.json`

Root owns CMake registration, the normal Win32 build, emitted-code review and
Core admission. No wrapper build, test, probe or consumer was added here.

## Gate and actual dependency

The approved Native body is:

```asm
006EDE90  56              PUSH ESI
006EDE91  8B F1           MOV ESI,ECX
006EDE93  E8 E8 FE FF FF  CALL 006EDD80
006EDE98  8B C6           MOV EAX,ESI
006EDE9A  5E              POP ESI
006EDE9B  C3              RET
```

Its SHA-256 is
`3d20fe5e8ca30c1a6d4bc3cef024ad8fdc9c398648954cd9368ea516b4fd9a3c`.
Root independently read all 12 bytes and six operations and checked live/PE
bytes and saved instruction starts. The primary gate is
`local/cc12_native_readiness_primary/Root_Astra_gate_approval.json`; the
report pins it with `wrapper_Root_listing.txt` and `wrapper_Root_live.txt`.
This worker replayed those files against the installed PE and six raw starts.

The actual child is declared in `native_lua_variant_pair_retreat.hpp` and
defined in `native_lua_variant_pair_retreat.cpp` as a naked, ECX-only
`void __fastcall` entry. It was admitted in `806294aa1` and is present in
published baseline `77da337c2b78aaae42a9009e8938ec5dd4b10a0e`. Its compiled
receipt is `reports/cc12_native_lua_variant_pair_retreat_primary_review.json`.
That receipt proves the complete 137-byte/48-operation Source body equals
Native outside its three REL32 operands to the actual Source17 validation
provider. The named dependency from the earlier readiness audit is closed.

The child still inherits its current CRT, raw backing and control-word
qualifications. This packet reads the actual Source provider and replays its
accepted evidence; it performs no new Native child audit or Ghidra mutation.

## Public interface and physical result

```cpp
void* __fastcall retreat_native_lua_variant_pair_wrapper_006ede90(void* actual_pair);
```

The definition is naked. The only parameter occupies ECX; there is no EDX
dummy, pushed argument, `noexcept`, generated owner or alternate call service.
The intended 12-byte body differs from Native only in the four-byte CALL
REL32 operand at wrapper offsets 4..7. That is a source intent awaiting the
Primary compiler/object review, not a claim about an unbuilt object.

With `S=entry ESP`, the wrapper saves incoming ESI at `S-4`, captures incoming
ECX into ESI, and calls the actual child with continuation `006EDE98` occupying
the Native `S-8` slot. For the Source object that continuation is its matching
instruction, not the original absolute address. After ordinary child return,
the wrapper copies current ESI to EAX, then reads the current wrapper save
word at `S-4` into ESI, then reads the return word at `S`.

The returned pointer bits therefore come from post-child ESI. Original-pair
identity is conditional on the child's ESI preservation and usable control
backing. A changed child ESI becomes EAX; a changed wrapper save word affects
ESI after that result copy. The code does not return a separately captured
C++ argument, the child's EAX, a pair field or a fresh ECX value.

Pair writes inside the child may alias saved ESI or return backing. The
wrapper adds no repair, rollback, validation, owner, extra frame or local EH
setup. Existing child/provider effects remain observable. The MOV/POP/RET
sequence after the child does not modify arithmetic flags, but Native fault,
flag, register, stack, CRT/handler, unwind and original caller ABI equivalence
are not established. Production lifetime, runtime use, startup and gameplay
remain unproved. The descriptive name does not recover a Native symbol/type.

## Current Source109 baseline and validation

The current child receipt describes the successful build from
`2026-10-09T19:04:50.472711+00:00` to
`2026-10-09T19:05:05.720846+00:00`: 109 inputs, four artifacts, three checks,
30 captured/replayed whole objects and 34 positive Core definitions. Its
29 previous objects were unchanged according to that accepted review.

All 109 input hashes match after canonical-LF normalization; 107 also match
raw bytes. The two known report line-ending differences are named in the
candidate report. All four artifacts match exact byte counts and SHA-256 at
the recorded replay time. The selected child's 971-byte Core member also
matches its accepted hash at the recorded offset. This is a byte replay of
the accepted Core evidence, not a new symbol-resolution or ABI proof.

These checks describe the Source109 snapshot at the report's replay time.
Later Primary builds may replace files at the same artifact paths. Source107
artifacts remain historical and are not substituted for this snapshot.
The report pins actual child files and accepted receipts, the Root gate,
and this candidate's three non-report files. Prior object graphs and the
full input manifest are referenced by hashes rather than copied.

No CMake, ledger, GPR, Ghidra, tests, probes or consumers changed in this
packet. The worker's code review checks the six-instruction source schedule;
Root must compile it, inspect its emitted bytes and sole actual-child REL32,
and decide admission. The prior successful build did not compile this wrapper.
