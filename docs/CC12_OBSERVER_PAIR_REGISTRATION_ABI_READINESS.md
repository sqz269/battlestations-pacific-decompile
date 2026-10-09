# Observer pair registration: native boundary and actual Source provider

The complete `00694A60..00694AE6` body is **135 bytes / 43 instructions**. Live
Ghidra bytes, the original PE and all 43 saved instruction starts agree. Root
Astra independently read, decoded and PE-replayed the full body. There is no
missing continuation, padding claim or listing repair. Its SHA-256 is
`8db8ecccd279bc880f420cc66f00d039fec9230a09617f239d94b97e51f3cc11`.

The existing Source provider is the **free function**
`register_observer_pair_00694a60(first, callback_owner, lifetime)` in
`observer_edges.hpp/.cpp`; it is not a `NativeObserverLifetime` member method.
It takes actual endpoint references and an explicit retained lifetime. Its
current object defines the public symbol and is present byte-for-byte in the
current Core library. This supports a later qualified Source caller. It does
not recover the selected member's complete type, Native unwind policy or a
binary-compatible entry ABI.

This packet changes only this document and its report. No Source, CMake, ledger,
GPR state, build, test, probe or consumer is changed. New Source, Original ABI,
startup, runtime and gameplay credit are all zero.

## Scope and physical entry

Baseline: `e585bda8178d152fdda52c69de6d5ecfbace4336`. Every live query verifies the
configured `bsp` project and `/battlestationspacific.exe`, x86 little-endian 32-bit
at base `00400000`. The configured `C:/Users/sqz269/bsp.gpr` exists, and the live
function count remains 64,729. The report records the original-image hash,
physical offsets, complete bytes, all instructions and instruction-start context.
No stronger server-reported absolute project-file identity is asserted.

Let S be entry ESP, A entry ECX and M entry EDX. The body captures A in EBX and
M in EDI. It reads no public stack argument and ends with plain RET. Incoming
EAX is replaced by the prior FS head before the first child call. Incoming ESI
is saved, then replaced by a captured section pointer. EBP is neither written
nor saved locally; compatible child preservation is required.

Saved metrics count three direct calls, eight blocks and ten block edges.
Physical decoding finds **five calls**: three direct children and two indirect
IAT calls, plus three conditional branches, one internal JMP and one RET.
The permissive local flow model reaches all 43 instructions / 135 bytes and has
45 instruction-level edges. It allows children to return; it proves no runtime
termination, exception behavior or corrupt-backing recovery.

| Normal address relative to S | Stored role |
| --- | --- |
| `S-4` | Full DWORD state, initially `FFFFFFFF` |
| `S-8` | Handler immediate `00C7E998` |
| `S-0Ch` | Captured prior FS head; registration is installed at this address |
| `S-10h` | Published captured section pointer |
| `S-14h` | Guard profile immediate `00CE37FC` |
| `S-18h`, `S-1Ch`, `S-20h` | Saved EBX, ESI and EDI |

Steady body ESP is `B=S-20h`. The handler, guard-profile contents, children and
IAT-slot values were not opened. Their immediates and call operands come only
from the owned 135-byte span. The saved pseudocode names the indirect calls as
Enter/LeaveCriticalSection; that does not prove current target-pointer values.

## Complete normal sequence

1. `00694A60..A7D` installs the FS registration, reserves eight local bytes,
   saves three nonvolatile registers and captures `(A,M)` into `(EBX,EDI)`.
2. `00694A7F` calls `00694280` with no pushed arguments. After its return,
   `00694A84` loads L from current DWORD `[EAX+4]` into ESI. There is no null
   check on the returned owner before this dereference.
3. TEST L precedes the two guard-local stores. MOV profile `00CE37FC` to
   `[B+0Ch]` and MOV L to `[B+10h]` preserve TEST flags for `00694A95`.
   If L is nonzero, push L, call through `[00CE2218]`, then increment current
   DWORD `[L+18h]` modulo 2^32. Null L skips both enter and depth increment.
4. `00694AA2/AA4` put captured M/A into EDX/ECX, **then** `00694AA6` stores
   full DWORD zero into `[B+1Ch]=[S-4]`. `00694AAE` calls `006949D0` without
   pushed arguments. A state-store alias can alter backing after the register
   values are formed, but the owned code does not recapture them.
5. TEST the returned EAX edge. If zero, reload EDX/ECX from EDI/EBX and call
   `00694850` at `00694ABB`, again without stack arguments. At this call EAX
   is still zero and flags still come from TEST zero. If nonzero, the only
   owned edge-relative operation is DWORD `[EAX+0Ch] += 1` at `00694AC2`.
   That addition wraps, including `FFFFFFFF -> 0`; there is no saturation,
   count validation or extra owned append.
6. Test current ESI, conventionally captured L under compatible children.
   If nonzero, decrement current DWORD `[L+18h]` modulo 2^32, push L, and call
   through `[00CE2210]`. The saved guard-local section is **not reloaded** on
   this normal release path. There is no new singleton lookup here.
7. Load the current saved FS word from `[B+14h]` **before** popping current
   saved EDI/ESI/EBX. Restore FS from that captured word, add `14h` to ESP and
   RET through the current return slot. The full state zero remains stored
   through normal release and epilogue; no owned reset to -1 occurs.

The owned body never directly reads or writes A/M fields. The direct subordinate
footprints are a DWORD owner+4 read, section-depth DWORD read/modify/writes at
L+18h, and the selected edge DWORD increment at E+0Ch. These support minimum
8-byte owner, 1Ch-byte tracked-section and 10h-byte edge address extents where
used; they do not recover either endpoint's layout or the enclosing member type.
All 13 explicit memory operands, including FS, stack and indirect-call slots,
are listed in the report. Implicit PUSH/POP/CALL/RET accesses are modeled separately.

## Call, register and result boundaries

| Site / target | Child entry ESP | Required normal callee cleanup | Selected delivery |
| --- | --- | --- | --- |
| `00694A7F -> 00694280` | `S-24h` | 0 | ECX=A, EDX=M remain physically present; EAX is prior FS; EBX=A, EDI=M |
| `00694A98 -> [00CE2218]` | `S-28h` | 4 | One stack word L; EAX is getter result; ECX/EDX are getter residuals |
| `00694AAE -> 006949D0` | `S-24h` | 0 | ECX=A, EDX=M, ESI=L; state is now full DWORD zero |
| `00694ABB -> 00694850` | `S-24h` | 0 | ECX=A, EDX=M, EAX=0; TEST-zero flags survive into the call |
| `00694ACF -> [00CE2210]` | `S-28h` | 4 | One stack word L; flags follow the depth decrement |

These are requirements for the owned normal frame. Subordinate Native queries
were metadata only: `00694280` is 189 bytes / 51 saved instructions; `006949D0`
is 143 / 61; `00694850` is 369 / 126. No hidden-input independence, child RET
instruction, child lock identity or Native call-target value is newly proved.
Children must preserve the live nonvolatile captures and return/frame backing.
The existing Source contracts are a separate evidence domain.

If L is null, normal EAX remains either the nonzero lookup result or the create
call's residual. If L is nonnull, the leave call can replace it. No common semantic
EAX result is promised. EDX is the latest child residual. Final ECX is the late
saved-chain value. Incoming arithmetic flags are not consumed before an owned
definition. Final flags come from `ADD ESP,14h`, not the edge increment or TEST:
for x=`S-14h`, CF iff x>=`FFFFFFEC`, OF iff `7FFFFFEC<=x<=7FFFFFFF`, AF iff
`(x&0F)>=0C`, ZF iff S=0, SF=bit31(S), PF=even low-byte parity. RET preserves them.

## Actual Source storage, locks and services

`register_observer_pair_00694a60` receives two `NativeObserverOwnerStorage&`
arguments and a `NativeObserverLifetime&`. Its Source guard captures
`lifetime.lock_owner_00694280()->section_04` once. A nonnull section is entered
before its depth is incremented; destruction decrements depth before leaving
that same captured section. Depth arithmetic uses a uint32 copy to preserve
DWORD wrap. Null sections skip those operations. A nonnull section must be an
actual initialized `TrackedCriticalSection`, whose depth is at +18h and size is
1Ch. A synthetic pointer with a similar integer count is insufficient.
The Source guard has its own C++ object storage and compiler-generated cleanup;
it does not reproduce the Native profile, FS registration or state-slot layout.
Arbitrary Native frame aliases therefore remain outside that Source comparison.

The Source then calls the lifetime's independently locked `find_pair_006949d0`.
That method compares the two current unsigned array counts, scans the smaller
endpoint array (the callback array on ties), and matches the opposite endpoint
by address in each edge. It requires valid live pointer arrays and edge objects;
it adds no null-edge, bounds or corruption recovery. A found edge's uint32
reference count is incremented. An absent edge invokes the free creation helper,
which independently obtains and captures another section. Do not remove those
nested lookups/locks or require their captured pointers to equal the outer one
without an explicit stable-publication premise.

The real owner prefix is 10h bytes: profile at +0 and data/count/capacity at
+4/+8/+0Ch. The real edge is 10h bytes: profile, first endpoint, callback owner,
reference count. These existing Source layouts are not proof of a new 18h-byte
member class. Endpoint addresses are identities: pass the actual bases, retaining
the real pointer-array storage. A copy with equal fields does not register the
same object. No A!=M condition is added; the Source creation helper can append
the same new edge twice when both endpoint references identify the same array.

`NativeObserverLifetime` retains a borrowed `SoundLifetimeAccess`, references to
the lock and dispatch publication cells, and an `ObserverLifetimeServices&`.
The access object identifies its raw publication **cell**, or its semantic
domain, rather than equating current pointer values. Its lazy lock-owner getter
uses the existing manager/critical-section/allocation/registration services and
reloads publication after unlocking. This packet opens none of their Native bodies.

The existing `GameObserverRuntime` owns the Source publication cells and lifetime
object. `GameSingletonHost` retains that runtime and binds it into deletion
services; its application manager publication is borrowed from the existing
process domain. Those objects and cells must outlive registration, later removal
and manager drain. Source fields named after Original globals do not establish
new bindings to Original global addresses. The selected registration code does
not call the runtime's invalid-parameter or noncanonical edge-deletion service;
those services remain relevant to later teardown, not new calls in this body.

## Failure and a later Source92 initializer

Native state is -1 during getter, section capture, guard publication, enter and
depth increment. It becomes full DWORD zero before lookup and remains zero
during creation, count increment, normal unlock and exit. The current Source
guard is armed only after its constructor completes. A getter/enter failure,
a lookup failure, a partial creation failure and a failure during release are
different stages; no uniform rollback or cleanup conclusion follows from them.

The Source allocation service uses current `std::malloc`, calls `_callnewh` on
failure, retries when it returns nonzero and otherwise throws `std::bad_alloc`.
Creation initializes one edge and appends to the first endpoint before the
callback endpoint. Each full-array growth stores wrapped `2*capacity+2` before
allocation, then uses current count/data after allocation. Consequently a throw
can leave a changed capacity, allocated edge or completed earlier append. The
registration wrapper supplies no transactional rollback or edge cleanup.
Source C++ unwinding releases completed nested/outer guards. Guard destructors'
implicit noexcept, current Win32/CRT behavior and `/EHsc` build policy do not
prove Native FH3/SEH identity, hardware-fault handling, second-exception behavior
or action order in unopened handler `00C7E998`. The older edge document's funclet
claims remain inherited context, not a fresh handler or interpreter audit.

Accepted `007F0F80` member92 supplies this call with captured A in ECX and M in
EDX after six **mixed-width** stores and state zero. In order, it zeros DWORDs
M+4/+8/+0Ch, then reads the current incoming endpoint word, compares that captured
A to zero, stores state zero, writes DWORD profile `00CF6494` at M+0, writes
DWORD A at M+14h and writes **one byte** 1 at M+10h. Its null-A arm still makes
all six member writes. The nonzero call tests the captured A, not a fresh M+14h
read; the early zero stores can alias the incoming word before its capture.
That accepted caller body was not reopened here.

A later qualified Source initializer therefore needs:

- The actual M backing through M+17h, with its actual 10h owner prefix available
  to the existing API. Preserve the byte store at +10h and the untouched bytes
  +11h..+13h; do not invent or default-initialize an enclosing 18h type.
- The ordered endpoint-word read after the three zero stores, or an explicitly
  qualified nonaliasing caller premise. Pass that captured actual A and actual
  M to the free function with the retained lifetime. Do not copy endpoints,
  reload A from M+14h or create a replacement lifetime domain.
- Valid live owner/edge/array backing, an initialized or null section according
  to the existing service contract, and publication/manager lifetime through
  registration and subsequent removal. If paired cleanup must find the same
  pair, later current M+14h and M must still identify that registered pair;
  aliases or callbacks are not proved unable to change them.
- An explicit Source failure policy that preserves prior writes and partial
  registration effects. Neither the normal cleanup provider nor this guard
  proves what unopened member handler `00C8F398` does on construction failure.
  A catch that unregisters, rebuilds arrays or repeats complete teardown needs
  its own justification.

The owned plain RET satisfies member92's zero-argument cleanup requirement
locally, conditional on all child contracts and live saved slots. It does not
establish a drop-in Native register ABI or Native exceptional composition.

## Current provider registration and verification boundary

`cmake/startup.cmake:1356` registers `src/observer_edges.cpp` in `bsp_core`, and
the current generated Win32 project has its ClCompile entry. The current
`observer_edges.obj` is **7,776 bytes**, SHA-256
`95b7c531e24e4985348de31a74973aedc45fc49ca4cab069dd93228db103d275`.
Its COFF table defines the exact public free-function symbol in section 16.
Section metadata describes 302 raw bytes; those compiled bytes and its compiler
EH payloads were not decoded or claimed as a new complete-object review here.
The object is byte-identical to its sole member in the current Source97 Core
library, whose hash is
`84ad4366435f91988d8953eb063b2bf9ad98496ecd17d27f0ba5515510902d27`.

The newer Root build ran from `2026-10-09T17:59:46.563985Z` through
`18:00:05.036630Z`, passed three existing checks and captures 97 inputs,
23 reviewed/replayed objects and 27 public Core roots. Root's pinned capture
reports all 19 prior Source88 objects unchanged. The unchanged **67-function
object is `observer_lifetime`, not `observer_edges`**. Source88's CMake and
artifact snapshots are now historical.

`observer_edges` is outside the current 23-object review; its header and
implementation are outside the 97-input set. This audit freshly pins the actual
provider and retained services: 12 Source/configuration Git pins, seven bounded
context Git pins, and the current Root CMakeLists snapshot. Eight relevant
Source/configuration files match the unchanged Source88 and current Source97
inputs; the refreshed CMakeLists snapshot matches a ninth Source97 input.
The newer build/capture receipts are pinned separately. Only the selected
current Core library and member identity are replayed, not all 97 inputs,
all 23 graphs or an application link map. There is no new build, fixture,
runtime or gameplay proof by this worker.

The normal registration boundary and actual Source API are ready as qualified
inputs to a later Source92 decision. Member backing/lifetime, its failure policy,
unopened Native children/handlers/IAT targets and Original ABI remain explicit
boundaries. The project and original installation are unchanged.
