# Native settings reader post-audio byte review

The instruction-aligned remainder `008D70A1..008D76B6` has no local operand
addressing settings `+24h`. Its explicit receiver stores zero bytes at `+84h`
and `+85h`, then zero a dword through a cached pointer whose last local origin
is settings `+90h`. The cache conclusion uses the ordinary balanced-call
frame model; delegated stack or alias mutation remains unexcluded.

The remainder supplies sixteen receiver descriptors and one stack-local
string descriptor. Its two tag-2 receiver destinations, `+64h` and `+80h`,
reuse the accepted four-byte Native provider proof and do not overlap `+24h`.
Other descriptor widths and all delegated effects remain unreviewed here.
There is no local block-copy instruction. These facts do not establish
whole-archive preservation, a zero selector, or archive timing before the
startup sound-state comparison.

## Scope and evidence epochs

This packet owns function `008D6DC0` and two documentation/report outputs.
It newly reviews `008D70A1..008D76A1` and refreshes the already admitted
epilogue `008D76A2..008D76B6`. No boundary expansion was needed. The current
seven physical windows total 1,558 bytes and 455 instructions, including the
21-byte/eight-instruction epilogue.

Current Source/Git is frozen at `7af8a0ac6`. The earlier prefix/audio/caller
receipt retains its original `1a53da357` epoch. Provider binding retains
worker base `b852eb6e0`, worker commit `00124fb1e`, and the primary review's
`0ebcc4745` verification epoch; its acceptance was integrated in `7af8a0ac6`.
The pending statistics packet is not substituted for accepted main.

Accepted evidence supplies settings receiver `00F88980`, reader register
`ESI`, settings register `EDI`, and inherited `EBX=1`. It also supplies the
constructor/caller stack identity and table `00CE44FC`, whose `+0Ch` entry is
`00BD68D0`; the admitted tag-2 stores in `00BD61C0/00BD63B0` are four bytes.
Those bodies/table cells were not reopened. Combining retained prefix/audio
bytes with this remainder covers the reader's 2,295 physical bytes and 661
instructions across their stated epochs; that is not a new whole-reader
effects or absence proof.

## Local writes and descriptor destinations

All 455 instructions are accounted for. There are 171 explicit memory-write
operands: 167 stack writes, two direct settings-byte writes, one cached-pointer
dword write, and one `FS:[0]` exception-chain restoration. Nine `PUSH`
instructions and 33 call return-address pushes are separately classified as
stack writes. The classification preserves byte-store widths: a Boolean
fallback initializes only its low byte, leaving upper bytes unspecified.

At `008D732B`, `LEA ECX,[EDI+90h]` forms the cached target. At `008D734B`,
after 18h bytes of descriptor arguments have been allocated, `[ESP+44h]`
is normal frame `B+2Ch`. This slot previously held argument-record pointers;
the report retains all 27 local assignments. The `+90h` assignment is last.
No later local write targets `B+2Ch` under the declared balanced-call model.
`008D768A` reloads `[ESP+2Ch]`, and `008D769C` writes a zero dword through it.
The physical instruction's target is the loaded pointer; this local provenance
does not exclude a delegate changing that stack cell or violating the assumed
normal stack/register contract.

The three stores occur only when the renderer call's returned object's dword
at `+28h` is unsigned-below `200h`. Otherwise `008D7688` branches straight to
the epilogue. The other two stores are `008D768E`, byte `[EDI+84h]=0`, and
`008D7695`, byte `[EDI+85h]=0`. The admitted local ranges `+84h`, `+85h`, and
the normally cached `+90h..+93h` do not overlap `+24h`.

| Saved pseudocode key | Field tag | Receiver offset | Call site |
| --- | --- | --- | --- |
| `rumbleOff` | 3 | `40h` | `008D70E6` |
| `swapSticks` | 3 | `41h` | `008D712D` |
| `invertCameraY` | 3 | `42h` | `008D7173` |
| `invertPlaneY` | 3 | `43h` | `008D71B9` |
| `swapMapSticks` | 3 | `44h` | `008D7200` |
| `XboxCompatibilityMode` | 3 | `B0h` | `008D724A` |
| `HardwareReported` | 3 | `95h` | `008D72CA` |
| `waterDrops` | 3 | `7Ch` | `008D7310` |
| `oldFilmEffect` | 1 | `90h` | `008D7355` |
| `MotionBlur` | 3 | `8Dh` | `008D739E` |
| `gamma` | 2 | `64h` | `008D73E9` |
| `markerAlpha` | 2 | `80h` | `008D7437` |
| `ShowSafeArea` | 3 | `B1h` | `008D7481` |
| `CockpitMode` | 3 | `B2h` | `008D74CA` |
| `MotionBlur`, repeated | 3 | `8Dh` | `008D750D` |
| `ClanText`, conditional | 0 | `B4h` | `008D757A` |

These calls reload the current reader table and select `+0Ch`. The addresses,
tags, and pointer expressions are Native operands; the key names come from
retained pseudocode. No key-string data cell was read. Only tag 2 has an
admitted Native output width here: `+64h..+67h` and `+80h..+83h`. No other
receiver descriptor begins at `+24h`, but provider writes through other
pointers, negative offsets, strings, heap storage, or aliases are not bounded
by that observation. The seventeenth descriptor is tag 0 with a stack-local
destination at normal frame `+14h`, supplied through reader slot `+10h`
inside the downloaded-content loop.

## Calls, paths, and remaining effects

There are seven direct calls: `008D44C0`, `005547D0`, `006ABA50`, `00427110`,
`004CDC20`, `00419CC0`, and `00BD1510`. In particular, `008D44C0` receives
`ECX=settings` and a value read from `settings+B0h`; its whole-object effects
remain open. The vector-related calls receive `settings+98h`. Their Source
roles are context, not newly recovered Native callee behavior.

The 26 indirect sites comprise sixteen reader `+0Ch` calls, two `+4` calls,
three `+8` calls, three `+14h` calls, one `+10h` call, and one renderer
`+104h` call. The renderer receiver is loaded through `00F8D394`; that data
cell was not fetched. All current-table rebinding and callee effects remain
frontiers, including Lua/parser/metamethod/global and callback effects.

Five conditional branches and one loop backedge account for all local
control transfers. A false `ClanText` predicate skips its descriptor. A false
`DownloadedContent` predicate skips its enter/loop/inner-leave path. Otherwise
the loop starts with index zero, obtains a stack-local string, delegates
insertion into `settings+98h`, conditionally returns temporary storage,
increments by inherited `EBX=1`, and repeats until the predicate is false.
No finite iteration bound is established. The outer reader leave always
follows normal completion. The renderer threshold controls the three final
stores. There is one physical return, `008D76B4: RET 4`. Exceptional/nonlocal
transfer, callee nonreturn, and error paths inside delegates are unreviewed.

The stack model records each assumed ordinary callee pop and checks branch
joins and loop frame balance. It does not newly establish those unopened
callee implementations. In particular, the temporary-release pair is modeled
with a combined 12-byte argument cleanup. Local alias classification must not
be promoted into proof against arbitrary delegated stack writes.

## Retention and replay

The report retains 445 complete Source/Git files from 62 seeds and all 815
quoted-include edges. All 440 prior provider Source/Git inputs retain equal
LF content. It preserves 1,615 prior worker artifacts, 1,619 primary-review
artifacts, four whole accepted reports/documents, complete saved exports,
and the complete original PE with SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Historical paths and epochs remain metadata; replay reads retained copies.

Eight typed read-only queries captured the current prototype and seven byte
windows. Each checked configured project `bsp`, program
`/battlestationspacific.exe`, x86 language and base `00400000`, with autostart
disabled. `C:/Users/sqz269/bsp.gpr` is the configured path; the bridge does not
independently expose the full path. No query error, GPR access, analysis
mutation, capability enabling, new provider/data inspection, Source edit,
compiler/test, SDK/game, or runtime-configuration action occurred.

Run `python local/settings_reader_post_audio_verify.py` for portable artifact
verification, and append `--original-worktree` only in the producing checkout
for its additional Git-object and two-output ownership checks. Portable mode
makes no claim about another checkout's HEAD or dirty state. The replay
recomputes the full local instruction audit from retained bytes and checks
prior receipts without running their now differently scoped Git verifiers.
Normal Source behavior, Native operand evidence, and runtime validation remain
separate; no startup or gameplay credit is added.
