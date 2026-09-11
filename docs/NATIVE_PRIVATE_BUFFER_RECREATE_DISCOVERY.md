# Private physical-buffer device-method discovery

Both excluded private-profile +20 methods are complete, dependency-free no-ops:

| Profile and slot cell | Entry | Complete bytes | Following boundary |
| --- | --- | --- | --- |
| D61E10 +20 = D61E30 | B4B810 | `C2 04 00` — `RET 4` | 13 bytes of `CC` through B4B81F |
| D61E34 +20 = D61E54 | B4B9C0 | `C2 04 00` — `RET 4` | 13 bytes of `CC` through B4B9CF |

These instructions consume the return address and discard one stacked DWORD.
They do not read ECX, EDX, the device argument value, object memory, globals or
COM storage. They do not call anything, allocate, retain, release or establish
an exception frame. All general registers, EFLAGS and floating-point state are
untouched. The existing renderer B1FD90 call site supplies the current device
as that stacked DWORD; this caller establishes its role, not a guessed prototype.

At capture time Ghidra had no defined function at either entry. Its only listed
xref to each was the corresponding DATA reference above. Seven fresh guarded
queries against `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` matched
246 installed-PE bytes: the two padded entry spans, both full nine-DWORD private
profiles, two inline private allocation/initialization spans, and two destructor
profile stores. Only six bytes constitute the proposed function bodies.
The installed binary SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Actual private-profile provenance

D61E10 is written at B4C137 inside B4BF30 logical-index-stream construction.
The captured B4C111..B4C153 sequence pushes 2Ch, calls actual `operator_new`
Bf681B, then initializes the returned object inline and installs D61E10.
D61E34 is similarly written at B4BE38 inside B4BC00 logical-vertex-stream
construction. B4BE14..B4BE54 contains its 2Ch `operator_new` call and inline
initialization. These branches do not obtain storage from the pooled wrapper
allocators used by the renderer's normal dynamic-wrapper producer.

Private profile identity alone still does not establish allocation provenance:
complete destructor B4B900 installs D61E10 at B4B91D, and complete destructor
B4BAB0 installs D61E34 at B4BACD, including when destroying a pooled object.
The current implementations are in `src/native_physical_buffer_owner.cpp`.
This distinction is why no pooled allocator or terminal-lifetime assumption
is transferred to the private cases.

None of those constructor, allocation, destructor, resource-support or COM
dependencies is reached by either `RET 4` method. No named-but-incomplete
prerequisite blocks the two leaves. This does not promote the enclosing logical
constructors or prove whole private-wrapper creation/destruction.

## Ready primary integration scope

The smallest addition is the two exact three-byte naked leaves. To fit the
existing raw recreation ABI, each can expose ECX receiver, an explicit unused
EDX parameter, and one unused stacked device pointer. A bare `RET 4` preserves
the original register/flag behavior; a C++ empty function is insufficient
evidence for incidental register preservation without inspecting its object.

Primary can define/name just B4B810..B4B813 and B4B9C0..B4B9C3, implement the
leaves, and separately widen the renderer source's admitted profile context to
the actual D61E10/D61E34 profiles. The existing two-profile parent proof does not
already establish that expanded source interface. Preserve its current profile
and +20 selector reload order and dispatch only to the captured established
target. The two no-op methods require no additional allocator, device policy,
owner context or terminal provider. Whole original/COFF comparison of six bytes
is sufficient leaf-code evidence; primary owns validation of any parent change.

This packet contains discovery only. No source, Ghidra definitions/names,
shared metadata, build files, tests or installed-game files were changed.
The tracked report pins all fresh bytes, original data-reference observations,
bounded writer assembly, and the current source-contract files. No runtime or
general private-owner compatibility claim is made.


Primary review verified all eleven worker pins and reread all seven guarded
spans, 246 bytes. Both complete three-byte leaves and the distinct private
profile provenance agree. The read-only primary bundle is
`local/private_recreate_discovery_primary/`. This review adds no source or
Ghidra definition; parent profile expansion remains a separate change.
