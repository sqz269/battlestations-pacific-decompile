# Raw mission Lua payload vector: 006B88E0

The complete `006B88E0..006B8983` body is 164 bytes and 58 instructions. Fresh
live bytes match the installed executable; all six branch destinations, two
direct calls, one indirect call, the single-byte NOP, and final `RET 4` are
retained in the report. The metadata size passed the authorized 300-byte gate
before complete inspection. No x87 expression, listing repair, or bespoke incoming
register requirement was found in the owned body. Child register contracts and
the local exception handler remain outside this audit.

This closes two uncertainties in the earlier payload-prefix evidence: this
vector entry consumes its one stack argument and preserves EDI on normal return.
It does not close Source readiness. No exact-address Source implementation was
found for this operation, its reserve helper `006B87F0`, or its element initializer
`008849B0`. The actual element virtual slot 0 and EH/lifetime/allocation contracts
remain qualified. No `std::vector`, callback, owner, or RAII substitute is proposed.

The published and worker baseline is
`365083d632ee8aaff7095a02292766b1f5271051`. The primary agent's new report of a
148-byte outer destructor through `00887743` is contextual only here; its separate
proof was not yet part of this packet's published inputs. This worker reads no
new bytes from that destructor, its continuation, handlers, callers, or tables.

## Raw header, stride, and call ABI

ECX supplies the actual header H, delivered as `payload+10h` by the accepted
destructor prefix. The single stack DWORD is the requested size R. The function
captures R in EBX and H in ESI. The three directly accessed DWORDs establish a
minimum 12-byte header footprint, without establishing a complete C++ class:

| Offset | Observed role | Local accesses |
| --- | --- | --- |
| H+0 | current data pointer | reread for each growth element and each retirement |
| H+4 | current count, compared as signed | load after reserve; reload before/after retirement; decrement before each retirement; final store of R |
| H+8 | capacity, compared as signed | read for the initial reserve gate |

Element address arithmetic is `data + ((index + 4*index) * 4)`, with x86 DWORD
wrapping: stride 14h, or 20 bytes. This establishes the pitch, not the complete
element layout. Retirement reads an element's DWORD at +0 as its vptr and that
table's slot 0. The initializer's remaining field requirements are unaudited.

All normal paths restore EBX, EBP, ESI, EDI, and the prior `FS:[0]` chain head,
then return with `RET 4`. EDI is restored **before** the retirement loop. No typed
return value is established. The owned instructions do not consume incoming EDX
as a value; direct descendants were not inspected, so their possible register
requirements are not inferred from that observation.

## Reserve and growth schedule

`006B8901/08` compares captured R with current capacity using signed JLE. If
R is greater, the body pushes R and calls `006B87F0`; ECX still holds H. The
fixed subsequent frame accesses and normal epilogue require a compatible reserve
target to consume that four-byte argument. H's count is loaded into EDI only
after the helper returns, so helper changes to the header are observed.

EBP becomes `FFFFFFFF` at `006B8914`. The initial signed growth comparison is
`index < R`; the current index is also written over the original request argument
slot on the native stack. If growth is needed, the exact `90` NOP at `006B891F`
executes once. Loop back-edges target `006B8920`, after the NOP.

Each growth iteration rereads H's data pointer, computes `data + 20*index`, and
stores that computed address in a local stack slot. TEST sets the null-result
flags; the following MOV sets exception state to 0 while preserving those flags
for JZ. A zero computed address skips `008849B0`; otherwise ECX is that address and the initializer is
called with no explicit stack arguments. This is a guard on the complete computed
address, not a guard on H's data pointer alone.

The local index is incremented with wrapping ADD. After comparing it with captured
R, the body resets state to -1 and writes the updated index into the original stack
argument slot; these MOVs preserve the comparison flags for JL. H's count is not
locally incremented during growth. Constructor callbacks may modify H's data or
count: the data is reread next iteration, while the local EDI index remains the
loop progression under the callee-saved register contract.

## Retirement and final count publication

At `006B894C`, the body compares R with H's **current** count, then pops/restores
EDI without changing those flags. If signed R is lower, each retirement does:

1. Add EBP (-1) to the current DWORD at H+4, then reload that decremented count.
2. Compute its 20-byte scaled index and separately reload H's current data pointer.
3. Form the element address, load its current vptr and current slot-0 target.
4. Push DWORD 0, place the element address in ECX, and call the target normally.
5. Reread H+4 and repeat only while signed R is still lower.

At the virtual boundary, EDX holds the current vptr and EAX the current target.
A compatible target consumes four argument bytes and preserves the native
callee-saved registers, including EBX=R, EBP=-1, ESI=H, and restored incoming EDI.
The zero flag is observed; a particular destructor or deallocation policy is not
established by the table lookup alone. No cached original count, data pointer,
element, or target controls a later iteration. No element dereference follows
its virtual call before the next iteration selects an element anew.

On exit, `006B896D` first loads the saved previous exception-chain head into ECX.
`006B8971` then overwrites H+4 with captured R, even if callbacks left a different
count. Saved registers are popped, `FS:[0]` is restored, 14h local/frame bytes are
discarded, and `RET 4` consumes the request word. Capacity and data have no local
stores in this body; that does not prevent child services or callbacks changing
them. There is no direct backing-buffer free in this entry.

## Request zero and malformed states

When R=0 and the observed capacity and count are nonnegative, reserve and growth
are skipped. Positive counts retire in reverse index order in the absence of
callback mutations, with count reduced before each virtual slot-0(0) call; zero
count performs no element call. The final count store is zero in both cases.
This path preserves the data pointer locally so the outer destructor can reload
its current value and free it afterward, as established in the accepted prefix.

Zero request is not an unconditional shortcut: negative signed capacity selects
reserve(0), and a negative signed count selects growth from negative indices toward
zero. The native code contains no range, capacity-consistency, multiplication,
underflow, or allocation-size check of its own. The arithmetic and comparisons
must retain their exact wrapping and signed behavior. H is dereferenced at +8
even for R=0, so a null header is not accepted as an empty vector.

Growth only skips the constructor when the **computed** address is zero. A null
data pointer with a nonzero index can produce a nonzero address that is passed
onward. Retirement has no element null guard before its vptr load. Header, data,
elements, and callback targets may alias; the concrete load/store order governs
the result. Invalid bounds, incompatible lifetimes, or callbacks that continually
raise the count can fault or prevent termination. No general fixed-work or
once-per-element guarantee follows under arbitrary mutation.

## Native frame and failure limits

With S as entry ESP, the body pushes state -1 at S-4, handler immediate `00C80E29`
at S-8, and the previous `FS:[0]` at S-12, publishing S-12 as the new chain head.
It reserves two local DWORDs, saves EBX/EBP/ESI, and later saves EDI. The header
pointer is stored at S-20; the current constructor address is stored at S-16.
During growth, state is at `[ESP+20h]` and the overwritten original argument word
is at `[ESP+28h]`. The native frame exists even on the ordinary zero-request path.

The handler immediate and state transitions are byte evidence only. Handler body,
tables, unwind cleanup, throwing children, and fault transfer were not inspected.
The old count is not locally advanced as each element is initialized; the state
and stack slots may matter to failure handling and cannot be replaced with an
invented C++ exception/RAII policy. Retirement changes count before callbacks and
has no visible local rollback. Native EH, SEH, hardware faults, Lua execution, and
production ownership remain unproven despite a complete normal-path epilogue.

## Source search and next boundaries

Address-index queries found no reconstruction provider for `006B88E0`,
`006B87F0`, or `008849B0`; case-insensitive address searches in `include/` and
`src/` also found no matches. The reserve helper has metadata extent 227 bytes
(`006B87F0..006B88D2`, 72 metadata instructions); the initializer has 27 bytes
(`008849B0..008849CA`, eight metadata instructions). Their native bodies remain
unread. Each can be assigned a separate small metadata-gated audit; the 27-byte
initializer is the smallest direct leaf candidate.

Existing `MissionLuaArgument`, `MissionLuaDeferredCall`, and
`MissionLuaResultVariant` use owning C++ strings/vectors or projected values.
The `GuiLuaVariant` used by the GUI reader is an eight-byte tag/value pair. None
supplies the verified 20-byte-stride raw element constructor, reserve behavior,
or current virtual-retirement schedule. Their current Source files and the
accepted payload-prefix/caller reports are pinned; no substitution is made.

This audit writes only its document and report. It changes no Source, CMake,
ledger, or Ghidra state and runs no builds, tests, probes, or native code. It
claims no new Original, Source, ABI, runtime, or gameplay credit.
