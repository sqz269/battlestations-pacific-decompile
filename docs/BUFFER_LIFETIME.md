# Physical buffer registry and frame boundary

The initial lifetime investigation was analysis-only. The rewind and explicit
registry port is described below. Names are inferred
from the executable. All eleven annotated function ranges match the installed
binary byte-for-byte; see `reports/buffer_lifetime_evidence.json`.

The physical wrapper array at +8h holds raw logical-stream pointers. Registration
at 00b4b1e0 suppresses duplicates without retaining the stream. Removal helper
00b4b2e0 searches for a pointer, replaces a matching entry with the last entry,
and decrements the count. It neither releases the pointer nor clears the old last
slot. Array order is therefore not stable. Its thiscall argument is a pointer to
the value to remove, and its result is a boolean in AL.

The index and vertex unregister wrappers at 00b4b390 and 00b4b3f0 use an optional
renderer guard and pass physical+8h to that helper. Both are thiscall routines
with one stack argument (RET 4); pseudocode omits the ECX receiver. Assembly at
00b4b73c/00b4b751 and 00b4b645/00b4b649 establishes the physical receiver and
logical-stream argument.

Logical index destructor 00b4b6f0 and vertex destructor 00b4b5d0 unregister when
the stream flags satisfy `(flags & 0xf000) == 0x1000`, before releasing their
physical reference. Vertex destruction additionally holds the renderer's tracked
critical section around unregister. Renderer callbacks 00b26900/00b268e0 and
base destruction remain dependencies; the current C++ shared ownership model
now exposes a raw registry with explicit registration/removal, but does not
reproduce this native destructor path.

Physical vtable +10h targets 00b4b480 (index) and 00b4b520 (vertex) are single
RET no-ops. They do not rewind the cursor. Vtable +18h targets 00b4b800 and
00b4b9b0 return byte capacity from +18h. Zero stores in 00b49490 and 00b4a040
instead clear logical index CPU-shadow pointers after free; they are not evidence
of physical cursor rewind. Those functions also retain truncated Ghidra bodies
after free, so their pseudocode needs further repair before reconstruction.

Renderer vtable +10h resolves through newly defined JMP thunk 00b2f4b0 to
00b2d8e0, provisionally named EndFrameAndPresent. Assembly establishes an active
field +1998h gate, texture/stream/index unbinding, unresolved callbacks,
XLiveRender, a conditional device EndScene call (+A8h), cache clearing, and a
conditional Present call (+44h). Present's 0x88760868 result sets device-lost
global 0108d4b9. The routine increments renderer+14h. This supplies a concrete
frame-boundary route for continued tracing, but is not a reconstructed frame
loop. Physical cursor rewind is now recovered below; the complete frame callback
dependencies remain unresolved.


## Rewind port

Physical vertex rewind 00b232b0 and index rewind 00b231c0 each take their physical
wrapper in ECX and return with RET. Under the optional renderer guard, they call
virtual +8h on every registered logical stream in array order, then write zero
to cursor +1Ch and dynamic-lock count +24h. Lock depth +20h, capacity, flags and
COM ownership are unchanged. There is no null-COM gate or implicit Unlock.
Logical vtable targets 00b48d40/00b48dd0 write offset 0xffffffff.

EndFrame assembly at 00b2d9a5/00b2d9ab passes renderer+1974h to vertex rewind
under the tracked renderer critical section. At 00b2d9bb/00b2d9c1 it passes
renderer+1978h to index rewind. This occurs after unbinding streams/index and
before debug draw callbacks, EndScene and Present. A future frame-loop port
must preserve that ordering; the standalone rewind methods do not unbind.

`D3D9StateCache` implements both rewinds for the current typed logical-stream
projections, plus explicit unique registration and swap-last removal. The vector
holds non-owning pointers, preserving the absence of native AddRef on registration.
Callers must unregister before destroying a stream, coordinate shared-buffer
access, and unbind before reuse. Native constructors/destructors, arbitrary virtual
stream subclasses, allocator layout and SEH are not supplied by this interface.
The helper's unused boolean removal result is omitted in C++.

The existing D3D9 probe draws green, unbinds, registers and rewinds both buffers,
checks invalid offsets/zero cursors and counters, uploads blue geometry into the
same buffers, rebinds and reads back blue inside/black outside. It also checks
duplicate suppression and explicit removal. This is a real-device buffer reuse
check, not the original game's complete frame loop or gameplay equivalence.
Build and the two existing CTest targets passed; no test target was added.
See `reports/buffer_rewind_probe.txt` and `reports/buffer_rewind_evidence.json`.
