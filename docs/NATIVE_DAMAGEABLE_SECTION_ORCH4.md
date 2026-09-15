# Damageable section copy and handle cleanup

## Completed source

`src/native_damageable_section.cpp` supplies three complete routines against
borrowed actual Win32 storage. The section description and function names are
hypotheses, not recovered symbols. Source reconstruction does not complete the
separate `0087CA80` reader or establish binary replacement/gameplay equivalence.

| Entry | Inclusive end | Bytes | Original ABI |
| --- | --- | ---: | --- |
| `00878B40` | `00878BAB` | 108 | ECX destination, stacked source, EAX destination, RET4 |
| `00878EF0` | `00878F20` | 49 | ECX actual `30h` record, RET |
| `0041DE40` | `0041DE68` | 41 | ECX actual one-DWORD handle slot, RET |

All 198 bytes match the installed executable and saved live Ghidra program.
Live batches verify project `bsp`, program `/battlestationspacific.exe`, language,
and image base through `Client.verify`. The configured project file remains
`C:/Users/sqz269/bsp.gpr`. All three exports were refreshed without Ghidra edits;
their complete listings require no flow or body repair.

## Copy schedule and x87 effects

`00878B40` first stores the actual `00D0DF04` vtable identity, passed as a stable
DWORD to the source interface. It then copies DWORDs `+4` and `+8` in order.
The next six transfers are individual x87 `FLD m32real; FSTP m32real` pairs at
`+0C,+10,+14,+18,+1C,+20`. Each source load occurs after the previous destination
store. The implementation uses those same ordered instructions rather than
staging source fields or treating the payload as an integer byte copy.

It clears destination+24 **before** loading source+24. If the loaded owner is
nonnull, publish it into destination+24 and atomically increment owner+4. The
final x87 pairs transfer `+28` and `+2C` after that retain. No old destination
handle is released, no identity check is added, and no constructor EH handler
is present. Self-copy therefore zeros the handle without retaining or releasing
its old owner. Partially overlapping storage likewise follows the individual
loads and stores; callers remain responsible for valid pointers after those
writes. An overlap does not make an invalid resulting handle safe.

The x87 instructions preserve the current control word and naturally affect
status flags, stack tags, and NaN representations. In particular masked signaling
NaNs become quiet, and invalid/denormal flags remain raised. The source adds no
FPU reset, masking, SSE substitute, or blanket save/restore. It does not claim
identical instruction/data pointers in the x87 environment or a binary ABI.

## Release schedule

`0041DE40` captures the current slot once. A null owner returns without any
store. For a nonnull owner, atomically decrement owner+4; when the result is zero,
reload that captured owner's vtable and call its current slot0 with ECX=owner
and no stacked arguments. Clear the original slot only after the call returns.
The callback may replace the slot; that replacement is cleared without a retain
or second release. There is no synthetic callback dispatch layer in the source.

`00878EF0` stores the actual `00D0DF04` identity before loading its embedded
handle at +24, then follows exactly the same release sequence. The source
composes the complete `0041DE40` implementation for this inlined native sequence.
All other record fields remain untouched. These functions neither free the
record nor invent a base destructor, exception cleanup, or `noexcept` wrapper.

The PE import table maps `00CE221C` to `KERNEL32.dll!InterlockedIncrement` and
`00CE2220` to `KERNEL32.dll!InterlockedDecrement`. Source uses the corresponding
Win32 atomic intrinsics; virtual calls use the captured owner's actual vtable.
There are five indirect call sites and no direct call or tail-jump sites in
these three bodies. The report records each call instruction and its contract.

## Validation and limits

One ignored fixture, `local/damageable_section_probe.cpp`, executes copied
original versions of all three complete bodies and the reconstructed source.
Its original-code IAT operands point at real Windows interlocked exports. The
fixture owner is a concrete C++ object with refcount at +4 and a real virtual
method that checks the old slot and publishes a replacement. This tests dispatch
and store order; it does not validate the game's terminal release implementation.

For the copy, complete destination/backing bytes, reference count, x87 CW/SW,
and MXCSR match with separate storage, self-copy, and an eight-byte forward
overlap. The fixture runs two masked rounding modes, leaves one live x87 stack
entry across each call, and includes signaling/quiet NaNs, a denormal, signed
zero, finite data, and infinities. Signaling NaNs quiet and the expected invalid
and denormal flags are present in both executions. x87 FIP/FDP are excluded.

For both cleanup entries, null handles, nonterminal decrements, and terminal
virtual dispatch match. The terminal callback overwrites the slot, and both
executions clear it after the callback without changing the replacement owner's
reference count. There are no new tracked tests or test framework additions.
The strict Win32 build, existing CTests, and report-verifier results are recorded
in `reports/native_damageable_section_orch4.json`.

Unmasked hardware exceptions, arbitrary SEH, real owner destruction, concurrent
mutation, the full Lua reader, and gameplay are not validated. Raw-byte parity,
source compilation, and the bounded differential fixture remain separate evidence.
