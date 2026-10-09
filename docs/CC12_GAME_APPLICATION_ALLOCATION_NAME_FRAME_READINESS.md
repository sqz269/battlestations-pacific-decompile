# Application Game allocation, name and frame readiness (CC12)

The bounded normal caller can be implemented from existing genuine allocation,
raw-string and Game runtime services. Its contract is now precise, but **full
application execution is not admitted**: the current startup still lacks the
actual duplicated-input/publication bindings and fully composed Game
construction/lifetime contexts. A retained provider must require those inputs;
it cannot manufacture a name, stack preimage, child table or successful callback.
This audit adds no Source or Original-function credit.

## Exact fragment and normal schedule

The leased interior fragment is `0073E150..0073E1D9`, **138 bytes /39 instructions**
inside Application Init `0073D410`. `0073E1DA` starts the next consumer and is
outside the fragment. This is not a new Original function. The enclosing entry
and fragment were unleased when the bounded range was claimed.

| Sites | Native effect |
|---|---|
| `0073E150..E168` | Allocate 71A0h through BF55BE, retain EAX in ESI, memset all 71A0h bytes to EDI=0, then clean the four stacked words. |
| `0073E16B..E17C` | Retain the allocation in the caller frame, compare it with zero and set EH state27h. |
| `0073E17E..E188` | Read current E1AE78 once and construct the actual 8-byte temporary header through 41E870. |
| `0073E18D..E1A1` | Set temporary-name bit2, pass its address with ECX=allocation, set EH state28h and call 4DDB90. |
| `0073E1A6..E1B9` | Select the constructor return, test the temporary flag, write result to Application+14, reset EH state to -1, then branch around absent cleanup. |
| `0073E1BB..E1D5` | Capture current name.data; if nonnull, capture length+1 and return it through current 419CC0/BD1510. |

The complete caller listing has one EDI write before this fragment: XOR EDI,EDI
at 73D433. EBX is restored to -1 at 73D88D and not changed before the state reset.
The caller's temporary mask starts at zero; the earlier name uses bit0 and
clears it. Bit2 is introduced only after this 41E870 returns. These facts do not
require an invented caller stack preimage.

The allocation is cleared **before** its null comparison. A replacement
allocator returning null cannot be treated as a successful null-Game path:
the preceding nonzero-size memset would already access it. The existing
canonical allocator's nonnull-or-throw contract is appropriate for the valid
normal Source path.

## The real input is the duplicated Init mode/command line

The native global producer is explicit. At `0073D4C2`, Init captures its second
stack argument in EBP. At `0073D933..D94A`, it passes that pointer to 438E40,
publishes the returned duplicate in E1AE78, and calls the command-line parser.
EBP is later reloaded with the Application receiver at 73DBCD. The Game fragment
therefore reads the retained duplicate, not a newly selected Game name.

The actual WinMain caller at `008F841E..842D` passes flags0 and the literal at
CE8168; live and installed-PE bytes confirm `cachedload\0`. Current
`src/winmain_startup.cpp` passes the same named initialization constant to
`GameStartupHost::application_initialize`. Nevertheless, the bounded fragment
must read its **current E1AE78-equivalent cell at the native point**, after
allocation/zeroing, rather than hard-code that default or recopy a cached mode.

The genuine `duplicate_native_string_00438e40` implementation already exists
and owns its result in the canonical allocation domain. Its producer belongs
to the earlier bootstrap phase. The bounded Game fragment borrows the current
duplicate and does not free or replace it. Native later frees and clears that
global at `0073E45C..E473`, outside this packet.

Current `GameStartupHost` validates nonnull mode and parses it directly; its
current startup does not compose this retained raw E1AE78 duplicate, actual
71A0h caller or `GameNativeGameRuntime`. A std::string parser input is not proof
that the native publication/ownership has been reproduced.

## Existing storage and string primitives

`NativeGameStorage` supplies the exact 71A0h extent and trivial storage shape.
`singleton_lifetime_allocate({object, 0x71a0, 0x71a0})` uses the real Source
malloc/new-handler loop. A new provider can begin the trivial object's lifetime
in that storage and clear all 71A0h bytes before invoking the runtime. It should
not infer an allocation from the runtime's borrowed reference or substitute a
stack object that will later receive a freeing scalar call.

The final `delete_native_game_004de270` calls the selected lifetime service's
`free_00bf6989` when flags bit0 is set; its concrete default is `std::free`.
That default pairs with the canonical malloc allocation. A different lifetime
free override needs its own pairing qualification. Scalar flags0 destroys the
graph without freeing the outer allocation; the runtime then forbids another
scalar call, so subsequent outer storage retirement must be an explicit owner
action, never a second scalar deletion. The scalar result is the original
pointer even after free and must not be dereferenced.

For the temporary, reuse `construct_native_string_header_0041e870` with actual
8-byte header storage and `NativeStringRawPoolContext`. The callee itself zeros
length/data before strlen; no guessed old bytes or caller-initialized header
contents are required. The constructor copies the captured NUL-terminated input
through the actual raw pool. Game construction copies that header into Game+7164.
Normal caller cleanup can use the raw-pool overload of
`destroy_native_string_header_0041dd20`: it captures data and length+1, obtains
the current pool and returns the block while leaving header bytes unchanged.

`game_native_string_process()` already retains both `raw_context()` and
`strings()` against the same pool, return gate and manager cells. The VFS raw
service exposes the canonical `ActualNativeStringPoolStorage` wrapper and its
application exposes a raw context over the same cells. Game construction's
string storage, the temporary's raw context and eventual lifetime contexts must
share that canonical domain; no CRT-string fallback or second pool is justified.
The raw cleanup overload propagates getter exceptions. The older storage
interface's noexcept release has a different exception boundary.

## Minimum retained provider contract

A bounded new interface could accept an already composed, stable
`GameNativeGameRuntime&`, the actual Application+14 publication cell, the current
E1AE78-equivalent input cell and the canonical raw string context, plus a stable
caller-owned operation frame. It would call the existing runtime's `construct`
with its newly allocated/zeroed actual storage and retained actual name header.
The runtime supplies the genuine Game-specific Source table; the new provider
must not stamp a table or publish a Game pointer ahead of the native stores.

The frame must retain at least the allocation pointer, actual 8-byte name header,
whether name construction completed, whether the constructor returned, whether
Application+14 was written, whether the name return completed, current stage and
failure state. It must retain enough state to distinguish a partial constructor
from a live Game with failed name cleanup. It owns no duplicate input and may
not use automatic cleanup to infer destruction of a partial Game graph.

`GameNativeGameRuntime` already owns persistent construction/destruction
operations and copies context objects containing borrowed references. The
runtime, every borrowed service/context/owner/publication and the new frame must
remain stable and alive through normal teardown and any failed operation. Its
external 71A0h allocation ownership must be explicit and retired exactly once,
either by the qualified final scalar or by an explicitly resolved owner path.
No `unique_ptr` default delete or frame destructor may double-free it.

Full construction still requires the actual profile, embedded/array constants,
live constant cells, canonical Dyn owners, unit/rank tables, global config/load,
resource parsers, grids and their genuine descriptor preimages. Lifetime still
requires the matching graph contexts, same call service and allocation domains.
The unit/rank provider specifically requires an explicit opaque local-word
preimage. Neither that value nor the grid preimages may be defaulted to zero.
These contexts must already be qualified before allowing the frame's construct
method to run. Existing primitive availability does not establish that complete
caller composition.

## Publication and failure boundaries

The internal Game global and Application field are different stores.
`4DDB90` publishes E188A8 at `004DE105`, before Dyn construction completes.
The outer fragment publishes Application+14 only at `0073E1AF`, after the
constructor returns and before temporary-name cleanup. A new provider must
preserve both positions. It must not publish the allocation before construction,
defer the internal global store until return, or roll back publications on an
unqualified failure path.

The narrowly inspected native FH3 metadata at DB609C points to unwind map
DB60C0. Its rows at DB61F8 describe:

- State27h -> -1: C86C3A loads the captured allocation and jumps through 42B100
  to BF6989. The raw 42B100 tail is POP ECX/RET despite its saved listing stopping
  after the call; no Ghidra repair was made.
- State28h ->27h: C86C45 tests and clears temporary-name bit2, then invokes
  41DD20 on that frame header. Thus the parent cleanup order is name, then outer
  allocation, after the Game constructor's own native unwind.

Those observations are not a port of the parent/constructor FH3 machinery.
The current Source Game constructor and runtime deliberately retain failed
operations and prohibit replay. Their destructors reject unresolved failure;
diagnostic acknowledgement does not free storage or undo publications. Running
only the native outer free against a retained Source partial graph is invalid.

| Failure point | Required Source distinction |
|---|---|
| Allocation or name construction | The Game runtime has not constructed the graph; retain the exact allocation/header stage for explicit resolution. |
| Game constructor | Keep storage, completed name, partial graph and all contexts/operations alive. Application+14 has not been assigned here; E188A8 may already have been assigned inside the constructor. |
| Normal name cleanup after constructor return | The Game is live and Application+14 already contains its result. Retain pending name-return state; do not mark the Game constructor failed or undo/free the live owner. |

Before normal name cleanup, native has already reset parent EH state to -1.
The state27h/28h cleanup actions therefore cannot be blindly applied to a later
pool-getter exception. Source failure reporting/retention is viable; native
FH3/SEH equivalence and automatic failure cleanup remain outside admission.

## Verification and next boundary

Fifteen live-Ghidra/installed-PE instruction and data windows, 356 bytes total,
cover the fragment, input producer/literal, register origins, internal Game
publication and the narrow EH evidence. Relevant current Source is pinned in
the [report](../reports/cc12_game_application_allocation_name_frame_readiness.json).
No Source, build, test, probe, native execution or Ghidra mutation occurred.

The next possible work is a retained ordinary C++ frame with explicit complete
inputs, followed by independent registration/build/review. Connecting it to the
current application requires the missing actual input/publication and full Game
context composition first. This report grants no full Game, startup or gameplay
readiness and supplies no new Original function.
