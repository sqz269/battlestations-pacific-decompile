# Native archive audio provider binding review

The assigned caller's constructed object is the reader passed to `008D6DC0`
on normal return. Its constructor installs table `00CE44FC`, whose actual
`+0Ch` cell contains `00BD68D0`. That dispatcher directly selects `00BD61C0`
or `00BD63B0`; both tag-2 branches contain a four-byte x87 destination store.
Consequently, the reviewed local store at settings `+20h` ends at `+23h`
and does not overlap the sound-state byte at `+24h`.

This closes the previous candidate-provider and local write-width questions.
It does not establish all-path archive preservation: delegated effects,
table rebinding, other settings reader regions, and invocation before
`0073DAEE` remain unproved. No zero selector, Source change, build, native ABI
replacement, or startup/gameplay credit follows from this review.

## Actual construction and caller stack

`004425D7` copies incoming `ECX` to `ESI`. `004425E6` writes `00CE44FC` to
`[ESI]`; the constructor zeroes dwords at `this+8/+0C/+10` while leaving
`this+4` without an explicit local initialization. It passes the first word
of its incoming stack argument area to `00442220` with `ECX=this+4`, then
passes that argument's address to `00B67700`. Those callees remain unopened.
The normal epilogue returns `EAX=this` and executes `RET 14h` at `00442626`.
The consumed stack argument area is 20 bytes; the Source interpretation is
a by-value Lua object. Stored Ghidra prototypes remain `undefined(void)`.

Let `B` be the assigned caller's `ESP` just before `004425C0` is called.
The frozen `008D79DA` instruction supplies `ECX=B+20h`. After the constructor's
return consumes its return address and 14h argument bytes, caller `ESP` is
`B+14h`. `008D79E3` then forms `(B+14h)+0Ch = B+20h`, and `008D79E7` pushes
that same address for `008D6DC0`. This equality comes from actual instructions
and `RET 14h`, without relying on a Source constructor label.

The separately leased 16-byte table range `00CE44FC..00CE450B` matches the
original PE and live capture. Its four little-endian words are `00441A70`,
`00BD8E20`, `00BD7A20`, and `00BD68D0`. Thus the constructor-installed table's
`+0Ch` cell at `00CE4508` positively selects the reviewed dispatcher. The first
three provider bodies were not opened. The prior audio reader reloads its
current table for each call; arbitrary delegated mutation or rebinding is
outside this normal installed-table contract.

## Dispatcher and tag-2 stores

`00BD68D0` receives the reader in `ECX` and three eight-byte stack pairs:
key at entry `ESP+4`, field at `+0C`, and fallback at `+14`. Its epilogue is
`RET 18h`, balancing the audio caller's three eight-byte records. It addresses
the reader's last 20-byte object, calls `00BD5790`, and calls `00B65FB0` on
the temporary result. A nonzero returned `AL` selects the default branch;
zero selects the value branch. The predicate's *nil* interpretation comes
from the existing provider contract; its body was not reopened.

The default call at `00BD6956` supplies `ECX=&field` and one pointer argument
to the fallback. The value call at `00BD6965` supplies `EDX=&field` and
`ECX=&temporary`. These frame expressions describe the ordinary balanced
return path; delegated cleanup implementations and nonlocal/exceptional
transfers remain outside this review. After a successful branch the wrapper
destroys the temporary through `00B67700`.

| Tag-2 branch | Destination and input | Local output instruction | Return |
| --- | --- | --- | --- |
| Default `00BD61C0` | `EDX=[field+4]`; `FLD m32real [fallback+4]` | `00BD6235: D9 1A`, `FSTP m32real [EDX]` | `RET 4` |
| Value `00BD63B0` | `EDI=[field+4]`; `ST0` after `00B66270` | `00BD645A: D9 1F`, `FSTP m32real [EDI]` | `RET` |

The field tag alone selects the default arm; that arm does not inspect the
fallback's tag and contains no delegated call. The value arm holds its
destination in `EDI` across the conversion call under that callee's ordinary
nonvolatile-register contract. `00B66270` remains an effects frontier. Both
output operands are exactly 32 bits. The default's x87 load/store is not a
claim of raw-bit copying, bitwise NaN preservation, matched floating-point
control/status, or guaranteed completion when an exception occurs.

Composed with the previously accepted four descriptors, the local output
ranges are `00F889A0..A3`, `00F889A8..AB`, `00F889AC..AF`, and `00F889B0..B3`.
None includes `00F889A4`. This statement concerns these destination stores;
it does not exclude Lua/parser/metamethod/global effects, constructor and
destructor effects, invalid-reader handling, earlier or later archive work,
or externally scheduled callbacks. No new startup-order evidence was acquired.

## Evidence and verification boundaries

The packet owns four function entries and the explicitly approved 16-byte
table range. It preserves twelve complete saved exports while decoding only
eight finite constructor/dispatcher/tag-2 slices: 400 bytes, 123 instructions.
Each slice and the table match both the retained complete original PE
(SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`)
and typed read-only live captures. No other provider, EH handler, data cell,
listing repair, prototype repair, flow repair, or GPR access was performed.
Each live query verified configured project `bsp`, program
`/battlestationspacific.exe`, x86 language, and image base `00400000`, with
autostart disabled. `C:/Users/sqz269/bsp.gpr` is the configured full path;
the bridge does not independently expose that path.

The report retains 440 complete current Source/Git files from 54 seeds,
803 quoted-include edges, four accepted reports/documents, and 1,080 prior
artifacts. Of the prior 430 Source/Git inputs, 429 retain equal LF content.
The sole change is the already accepted copy-direction correction in
`docs/OPTIONS_MENU_SCREENS.md`; there is no C++ or compiler-input drift.
Ten whole-file-backed Source excerpts provide context, including the current
native storage providers. They add no compiler or binary replacement evidence.

Run `python local/archive_audio_provider_binding_verify.py` for portable
artifact verification. It reads retained inputs and performs no Git, live
Ghidra, compiler, test, SDK, or game operation. It checks pin identity, embedded
Git blob identities, include closure, prior artifact mappings, original PE,
live bytes, listings, table values, and the recomputed semantic audit.

Run the same command with `--original-worktree` only in the producing checkout
for the additional Git-object and owned-output checks against frozen commit
`b852eb6e0`. This mode explicitly rejects another worktree. A portable artifact
pass is not presented as accepting another checkout's HEAD or dirty scope.
The previous packet's original-worktree verifier is retained unchanged and is
not represented as rerun under this packet's expanded Git scope.
