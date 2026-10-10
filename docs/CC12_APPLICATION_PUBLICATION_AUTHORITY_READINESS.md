# Application publication authorities: Source readiness

**NO-GO for connecting `GameNativeGameApplicationFrame` to startup, and no new
C++ implementation is admitted by this packet.** Neither a genuine enclosing
raw Application owner nor a retained `00E1AE78` publication owner is established
in the frozen Source. The frame accepts references supplied by a caller; those
parameters do not establish who owns either cell or authorizes its contents.

Several useful components already exist: the actual application `+08` vector,
the process current-game cell, the canonical raw string domain, a standalone
duplicate leaf, and the retained game/frame operations. The missing work is
their concrete authority and lifetime contract, not a replacement local pointer.

## Current evidence and the older frame review

The immutable Source base is `ef15bbd5da9390cb75b691efae685b4360c69113`: Main
`98b5113614afb4b54aa16a469bfdd16befc656f3` merged into the worker with its closed
grid-default commit `bf21d1264bb15a3375bc1f0bfc4c1d373800cb8e` preserved. All
4,208 tracked `src/` and `include/` files have complete actual Git and raw working
archives. The two grid files are the only Source difference from that Main.

The accepted frame report `6a80adb7f` and primary review `ac580812a` are retained
whole, including the primary's actual review and guarded-replay records. Their
two missing-publication findings still hold. Six Source files changed since
the frame freeze: two particle files, two grid files and two prepared-registry
files. The frame/publication/string/startup files examined here are unchanged.

The older reports' grid allocator warnings are historical. The current worker
has the two canonical grid defaults from `bf21d1264`; this Source-only packet
does not repeat their builds or claim a new primary build. Numeric raw-grid
table ownership and the other game-composition obligations remain separate.

## Application `+14`: components are not the enclosing owner

| Existing Source | Established fact | Remaining boundary |
| --- | --- | --- |
| `winmain_startup.hpp:168` | The five application steps describe the same `1Ch` stack object. | This is retained reconstruction evidence, not an instantiated raw Source object or fresh Native verification. |
| `native_shader_preload.hpp:9` | `NativeApplicationPointerVectorStorage` is an actual `0Ch` header at application `+08`, with signed count/capacity. Preload consumes that exact subobject. | It borrows an application address; it creates no enclosing application. |
| `app_frame.hpp:6` | `ApplicationFrameState` projects first-update `+18` and input-latch `+19`. | It has no `+14` pointer and is explicitly not the full layout. |
| `game_hosts.cpp:1835` | `application_construct()` resets that projection and sets `constructed_`. | It performs no raw Application construction. |
| `game_hosts.cpp:2130` | Window setup assigns `request.application = this`. | This is the `GameStartupHost` object. Reinterpreting its address as a raw `1Ch` application would invent a layout. |
| `platform_services.cpp:74` | `ApplicationFrameService` checks a supplied application's pointer identity before using separate frame state. | Pointer equality does not establish the pointee's raw layout or a `+14` field. |
| `app_shutdown.hpp:182` | `ApplicationShutdownState` stores systems, count and game **by value**. | Its `game` is a projection, not a reference into an authoritative raw application. |
| `game_native_game_application_frame.cpp:15` | The frame retains the caller's `void* volatile&`. | Its only constructor guard is an exclusive fresh runtime; it validates neither the reference's address nor publication ownership. |

The old startup comment describing `+04..+14` as unreconstructed is too broad
when applied to the entire current repository: the `+08` vector body exists.
It remains accurate that this startup host has not composed the enclosing raw
object. A vector helper or a freshly allocated buffer alone does not close that
gap.

The smallest future raw-owner contract is one stable, explicitly owned `1Ch`
application instance, retained through normal shutdown and any failed frame.
Its proposed layout must establish the existing vector at `+08`, the one game
cell at `+14`, and separate flag bytes `+18/+19/+1A`; it must preserve declared
preimages for `+00/+04` and padding. The current Source comments establish the
constructor's `1,0,0` flag stores, not permission to zero all `1Ch` bytes.
`GameNativeGameApplicationFrame`'s separate zeroing of its `71A0h` game allocation
does not authorize application zeroing.

The platform publication, application callbacks, preload borrower, frame and
shutdown must use that same instance. The frame must borrow the actual `+14`
field. Shutdown and frame consumers must use live views of that field, rather
than copies in `ApplicationShutdownState`, `GameStateSlot` or another pointer.
No callable application table, allocation/constructor policy or startup routing
is supplied by this report.

## `E1AE78`: the producer and publication are missing

The current entry records the OS command-line pointer in `WinMainArguments`,
but `run_win_main` explicitly ignores those arguments. It calls initialization
with flags `0` and the literal mode `"cachedload"`. At the documented duplication
point, `GameStartupHost` directly calls `parse_command_line_0073ce20(mode)`.
It does not duplicate the bytes or store a retained publication. Separately,
`BootstrapHost::command_line()` returns `std::string`; that interface also does
not own or publish `E1AE78`.

All seven case-insensitive `E1AE78` hits in the complete frozen Source occur in
bootstrap comments or the frame's reference declaration, binding and read.
The frame type appears only in its own declaration/definition files. The
canonical duplicate leaf has other Source consumers, but no startup caller.
These are bounded lexical findings, including comments; they do not prove
universal absence under arbitrary other names or in a foreign binary.

`duplicate_native_string_00438e40` is an existing positive Source boundary:
null returns null; a valid nonnull text scan requests exactly length plus one
bytes from canonical allocation and copies the terminator. Its contract admits
normal success with readable, nonwrapping input and a later matching canonical
free. It explicitly leaves failure unwind and class ownership unadmitted.
The older plain `duplicate_00438e40` uses direct `std::malloc`; it does not supply
the same new-handler contract or establish publication ownership.

A future duplicate authority must be one retained process-lifetime
`char* volatile` cell, owned alongside or explicitly bound to the existing
process domain. A new member would be a declared Source authority, not a claim
that a local variable reproduces the original global. Its initial preimage,
single-admission policy, exact producer input and successful publication point
must be explicit. Publish only the returned owned copy; do not substitute the
OS command line, the mode literal, `std::string::c_str()`, the frame's temporary
buffer or the game's copied buffer. The parser and frame must observe this same
cell. A fresh host must not silently reset or replace an existing process
authority.

No current Source owner establishes an overwrite, final free, publication-clear
or retry schedule for this cell. The broad retirement query's two matches are
ordinary parser/search return expressions, not deallocations. Retaining the
cell does not by itself prove the pointed-to allocation remains live. A future
normal retirement must wait for the final parser, frame and diagnostic borrow,
use the matching canonical free once, and define clear timing explicitly.
Neither automatic application-destructor cleanup nor original FH3 behavior is
inferred here.

## Three separate name lifetimes

| Storage | Allocation/return domain | Ownership and timing |
| --- | --- | --- |
| Standalone duplicated bytes published at `E1AE78` | Canonical duplicate allocation; eventual matching canonical free | Future process owner. The frame captures only a borrowed pointer and never frees this allocation. Its diagnostic status can still expose that pointer. |
| Frame's raw eight-byte temporary header | Raw pool context over the same `01090AA0` manager, `01090AA8` pool and `01090AA4` return gate | The callee initializes the header, prepares a separate buffer, and the frame returns it after application publication. Returning the header leaves its bytes untouched. |
| Game's header at `+7164`, data at `+7168` | Construction's string provider and lifetime's matching raw pool bridge | Game construction makes a distinct copy. Game cleanup returns that buffer independently. It is not the standalone duplicate or the frame temporary. |

`GameNativeStringProcess` already retains the canonical three cells and both
the raw context and `ActualNativeStringPoolStorage` bridge over them.
`GameSingletonHost` borrows its manager cell. Future composition must use these
same references for the frame, game construction and profile/game cleanup;
separate pools or equal-valued substitute cells are insufficient. Existing
frame/runtime guards do not comprehensively validate those string identities.

Keep the sizes and gates separate. Standalone duplication of an empty string
owns one NUL byte, while fresh pooled construction of that text leaves a
zero-length/null-data header. A null duplicate is permitted by the leaf but
violates the frame constructor helper's nonnull `strlen` input contract; no
empty-string fallback is admitted. Raw header return captures nonnull data and
the current DWORD length-plus-one, resolves the pool getter, then returns the
buffer. Large returns (`size >= 150`) free directly; small returns read the live
disable gate. The getter still precedes either branch. No cached pool or gate
is interchangeable with that schedule.

## Publications, failure and retirement

The frame records parent state `27h` after allocating/zeroing the game, captures
the current duplicate pointer, and prepares the temporary name. It records
mask bit `2` and parent state `28h` after preparation. Game construction can
publish the real current-game cell **before** its later worker/engine/world
calls return. Only after construction returns does the frame write Application
`+14`, record parent state `-1`, and attempt temporary-name return. Frame phase
`live` is reached only after that return succeeds. A nonnull application or
current-game publication therefore does not establish successful completion.

`GameNativeSettingsProcess::game_00e188a8()` is the already established current
game authority, shared with its profile context. It remains separate from
Application `+14`. Future admission must check completed frame/runtime phases,
resolved temporary-name ownership and the expected same game allocation in
both actual cells; it must not create another current-game pointer.

On failure the frame records progress and rethrows. It does not roll back
publication, return a partial name, clear a header, free the duplicate or replay
an operation. Diagnostic acknowledgment performs no cleanup. A failed name
return can leave a published application and nonnull historical header bits;
those bits cannot authorize another return. Original hardware-fault delivery,
FH3 state and unwind equivalence remain separate from these Source phases.

Scalar deletion forwards the full flags word. Bit zero controls game-allocation
freeing; clear-bit success retains raw storage. A throwing free can make the
allocation disposition indeterminate. These scalar flags, the parent cleanup
state, the name mask, initialization flags `0`, and application byte flags are
different state. None authorizes freeing the standalone duplicate.

The frame never clears Application `+14`. The current shutdown projection calls
`game_on_destroy_004dc5c0(state.game)` unconditionally at its step, then guards
the separate delete step and clears `state.game` only after `delete_game`
returns. Its abstract delete service does not establish a scalar flags value.
The system-array loop separately rereads its count, decrements references,
destroys at zero and clears each nonnull slot after returning calls. Its unsigned
projection count must not silently replace the actual vector's signed header.
Copying the projection into and out of an authoritative owner would obscure
live reads, callback changes and interruption timing.

Current startup does not bind that shutdown driver. Its destructor deletes
frame callbacks before draining the raw manager and retaining services through
that drain, but has no new application/frame/name retirement path. Future
composition must quiesce producers and consumers, detach any fixed-step runtime,
resolve the exact frame/game/name obligations, and retire before the borrowed
renderer, VFS, Lua, observer and raw string services become unusable. The
original application/name clear and free schedule must be established rather
than added as an inferred rollback.

## Finite proof and next packet

The report retains 18 complete searches, 58 bounded excerpts and positive Source
witnesses. Ten dependency Git blobs and two actual primary frame records are
copied whole. The earlier frame and grid `Source4208` corpora remain distinct.
Seven historical selected-input/build epochs are also retained separately:
Source3352-wide, Source3352-worker-free, Source3354-node, Source3354-primary-free,
Source3354-particle-primary, Source3354-worker-grid and the prepared-registry
primary epoch. Their four build artifacts each receive no new or unchanged
build credit. All 125 pins in the closed grid proof are preserved unchanged.

This packet makes zero C++ changes, Native queries, Ghidra/ledger writes, builds,
tests, probes, game executions or OS changes. Portable, actual-Git and physical
relocation verification concern retained Source evidence only.

**Next packet: `cc12_application_raw_storage_constructor_readiness`.** Bound it
to the same-receiver `1Ch` application contract, constructor `00737970`, the
existing `+08` vector, `+14` publication, `+18/+19/+1A` stores, unknown bytes and
normal/failed-startup lifetime. Its deliverable is an explicit preimage/write
and owner/borrow contract that can admit a real Source storage owner without
invented initialization or a second authority. Keep the duplicate's process
publication and normal retirement contract separate as specified above; the
next packet must name any additional evidence needed for its unresolved clear,
free and failure schedule. No owner implementation or frame activation begins
until that admission is reviewed.
