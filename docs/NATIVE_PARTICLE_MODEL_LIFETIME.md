# Native particle-model terminal lifetime

Addresses: `00AF6C50`, `00AF7F40`, `00AFD370`, `00AF6180`, `00AF6B70`,
`00AFD0F0`, `00AFD1E0`, `00AFCDA0`, `00AF6DA0`, `00AF5AF0`, `00AF5B00`.

The actual `D5DA50` particle profile now has a complete terminal sequence over
the existing `NativeModelOwner`, its sole native reference count and canonical
node/scene binding. Scalar deletion returns the actual `2E0h` slot to `F8D2D0`.
Every reached emitter, variant or mesh zero-reference dispatch still requires
its real implementation in the owner's existing `NativeRenderActualOwners`.
This packet creates no alternative owner lookup, model, emitter, definition or
pool domain. Descriptive names are hypotheses; the interfaces are new C++ ABI.

| Routine, inclusive span | Original ABI | Coverage |
| --- | --- | --- |
| AF6C50..AF6D91 | ECX model; RET; two FH3 unwind states | complete through required canonical actual owner terminals |
| AF7F40..AF7F5F | ECX model; stack DWORD flags; EAX original model; RET4 | complete |
| AFD370..AFD405 | ECX separate18h arrays owner; RET; one FH3 state | complete for native cookie domain |
| AF6180..AF61CF | ECX0Ch pointer header; stack signed count; RET4 | complete for valid native pointer-array domain |
| AF6B70..AF6B86 | ECX0Ch pointer header; RET | complete |
| AFD0F0..AFD126 | ECX8h byte-array header; RET | complete for native cookie domain |
| AFD1E0..AFD219 | ECX8h108h-record-array header; RET | complete for native cookie domain |
| AFCDA0 | ECX byte; RET | complete, literal RET |
| AF6DA0..AF6DC7 | ignores ECX; stack token; AL bool; RET4 | complete |
| AF5AF0..AF5AF5 | ignores ECX; EAX current own type ID; RET | complete |
| AF5B00..AF5B05 | ignores ECX; EAX current native name address; RET | complete |

## Identity, producer and dispatch

AF74A0 constructs `2DCh` payload storage. The canonical174h node and model tail
174..183 belong to the SAME `NativeModelOwner`. Particle184 is a scalar, not
the ordinary model pool's slab ID. AF5C20 writes this pool's ID at2DC, and
AF60B0 returns its2E0h slot under the actual pool critical section. AF7F40 calls
the full derived/base destructor before testing flags bit0; flags2 does not
free. It returns the original address even after a physical return.

AFD2E0 is the producer of the separate18h model190 owner: model00 is borrowed,
bytes04 and records0C are8h headers, and word14 is initialized independently.
AFD130/AFD220 allocate DWORD count cookies followed by byte/108h elements.
Destruction uses the cookie, not either descriptor count, and visits elements
backwards through the concrete literal-RET AFCDA0 and existing AFD9F0. The
valid cookie domain is nonnegative signed counts with actual allocated extents.
This is unrelated to the emitter's lazy30h container or its6Ch state records.

CD7850 produces the actual five type DWORDs: name F8D318=D5DA3C, inherited model,
node and root IDs atF8D30C/310/314 after canonical B74F90, and own ID atF8D308
from the existing6FAC20 counter. The three reconstructed leaves read those
current words without initializing, checking the guard or copying a descriptor.
AF6DA0 short-circuits across exactly four IDs; the name word is not a fifth ID.
The producer was inspected only; its startup and shared counter remain required.

`NativeParticleModelReference` can be created in the constructor's required
`bind_particle_profile` hook immediately after actualD5DA50 publication. It
borrows the existing atomic04 without retaining it and binds itself in the SAME
`GeneratedModelLifetimeRuntime`. Its owner must also bind this reference in the
same canonical `NativeRenderActualOwners` lookup used for all retained fields.
No `NativeModelReference` may coexist: its terminal path uses the wrong188h pool.

The reference replaces the existing scene binding's type/attach/remove/change
callbacks without changing its context (which remains `NativeModelOwner`),
transform, hierarchy or native bytes. The type callback validates currentD5DA50
slot0C=AF6DA0 and reads the actual descriptor. The inherited scene callbacks use
canonical B6ED80/B6EE10/B6DBE0; logical release uses canonical B6F310, current04
and the same point-light list. Slot0=BD30E0/slot4=AF7F40 are required at final
release. B750C0 later reinstalls its own base callbacks and B6F440 the node ones.

After actual successful scalar destruction/pool return, the reference unbinds
from that lifetime runtime and invokes the required host disposal callback.
Constructor unwind calls `retire_after_failed_construction` only after real
base cleanup; this retires host bindings without returning the raw slot. The
constructor caller still performs AF62F0. Neither path accesses a companion
after its disposal callback. The caller must remove its canonical lookup entry.

## Terminal order and current reads

1. PublishD5DA50. Capture model190; if nonnull, destroy its records and bytes,
   then free the captured18h allocation. Preserve the dangling190 field.
2. Walk emitters in ascending signed index order. Capture each nonnull actual
   cell, decrement its sole atomic04 and dispatch current virtual0 only on zero.
   Reload the current194 backing and198 count after each terminal callback.
   Do not clear or release the pointer cells a second time.
3. Capture/release/clear variant18C, then current mesh1C8, then current mesh1B4.
   Read each next field after preceding callbacks. Clearing happens after the
   release returns, including if that callback replaced the source field.
4. Capture current F8C274 manager atAF6D3F, **before** decrementing current
   F8D2C8 atAF6D45. Call the root's real AF0AE0 removal on that captured manager
   and actual model; disregard its AL return. Manager membership is weak.
5. Arm state0, resize emitters194 to zero, then free its current backing.
   Preserve the stale pointer and capacity. Finally arm state-1 and call full
   B750C0 on the existing model owner, including its retained174/geometry180 and
   complete node cleanup. Geometry180 may share mesh1B4; its separate retain is
   consumed only in the base destructor.

AF6180 compares signed requested count to current capacity, calling the existing
AF6120 reserve if needed. It zeroes new pointer cells, conditionally skipping
an actual zero computed cell address, decrements count for shrinking, and
publishes the requested count. It never releases a cell's emitter. AF6B70 is
the exact resize0/free wrapper. The constructor's existing private unwind
helper remains untouched; it already implements that valid-header path.

AFD370 first frees records0C using its captured cookie, then stores pointer0C=0
before count10=0. It next frees bytes04 and stores count08=0 before pointer04=0.
The standalone AFD0F0 and AFD1E0 helpers both store count before pointer. All
preserve the containing owner model00/word14 and do not free the18h owner.

## Calls, stack and exception evidence

The report enumerates every direct and indirect CALL in all11 bodies with
containing function and exact site. Each numeric row can be checked by
`tools/verify_report_calls.py` after saved-analysis repairs. The dynamic release
rows explicitly remain canonical current virtual0 calls, not invented terminals.

| Sites | Native target | Body-established contract and cleanup |
| --- | --- | --- |
| AF6C8C | AFD370 | complete actual18h member destruction; RET |
| AF6C92 | BF65AC | returning CRT free of captured18h allocation; ADD ESP4 atAF6C97 |
| AF6CBD/CE7/D09/D2B | captured CE2220 | actual Win32 InterlockedDecrement, callee pops4 |
| AF6CCA/CF3/D15/D37 | current owner virtual0 | actual zero-reference terminal; ECX owner, no stack argument |
| AF6D4D | AF0AE0 | weak actual manager array erasure; RET4, AL discarded |
| AF6D60, AF6B75 | AF6180 | actual pointer header resize0; RET4 |
| AF6D68, AF6B7D | BF6989 | returning array-free thunk BF65AC -> BF9DC8; ADD ESP4 |
| AF6D7A | B750C0 | full same model/node base cleanup, no pool return; RET |
| AF7F43/AF7F55 | AF6C50/AF60B0 | derived destruction then actual pool return iff bit0; latter RET4 |
| AFD3AE/3E0, AFD109/1FC | BF7C6E | vector destructor iterator, four stack words(data,stride,cookie,dtor), RET10h |
| AFD3B4/3E6, AFD10F/202 | BF6989 | free captured cookie address; ADD ESP4 after each |
| AF618E | AF6120 | minimum1 reserve preserving current array metadata ordering; RET4 |

AF6C50 handlerCBAD96 loads descriptorDF2C10, whose mapDF2C00 has state1 ->0
CBAD88 -> AF6B70(model194), then state0 ->-1 CBAD80 -> B750C0. An exception
during array destruction or a retained-owner release performs only that member
and base cleanup; it does not retry earlier releases or invent rollback of live
count, manager membership, variant or extra raw meshes. An exception in ordinary
state0 backing cleanup calls only the base. A base exception is not retried.
Cleanup throwing during another exception terminates in the source interface.

AFD370 handlerCBB21B loads descriptorDF31E4/mapDF31DC. Its sole state0 action
CBB210 calls AFD0F0 on owner04. The concrete record destructor is literal RET
and CRT free is nonthrowing, so no C++ catch is required to execute this native
member-unwind path in the supported domain. Native FH3 dispatch is not replaced.

Direct callers/xrefs were checked in the live project: AF7F40 is D5DA50+04;
AF6C50 is called by AF7F40; AFD370 by AF5B30 and AF6C50; AF6180 by AF6B70 and
AF6C50; AF6B70 by the destructor and constructor unwind actions. The three
type leaves are installed in the same D5DA50 profile. Other callers of the two
array-member cleanup helpers are retained in the report's capped xref lists.

## Saved-analysis repairs and verification limits

Every live batch used the guarded `bsp.py ghidra` wrapper, verifying existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. This worker made no
Ghidra mutation, import, annotation, save, ledger or shared build edit. Eleven
complete owned spans and ten supporting spans match live and installed PE bytes;
SHA256s and prototype/xref preimages are in the report.

| Required repair | Inclusive end and final instruction |
| --- | --- |
| AF6C50 missing AF6C97..99 ADD ESP4 and AF6D6D..AF6D91 full continuation | AF6D91; RET atAF6D91 length1 |
| AFD370 missing AFD3B9..BB and AFD3EB..ED ADD ESP4 | AFD405; RET atAFD405 length1, existing body end already correct |
| Define type query AF6DA0 | AF6DC7; RET4 atAF6DC5 length3 |
| Define type ID AF5AF0 | AF5AF5; RET atAF5AF5 length1 |
| Define type name AF5B00 | AF5B05; RET atAF5B05 length1 |
| Define FH3 dispatcher CBAD96 | CBAD9F; JMP BF6B43 atCBAD9B length5 |
| Define FH3 dispatcher CBB21B | CBB224; JMP BF6B43 atCBB220 length5 |

AF6B70's earlier missing suffix is already repaired throughAF6B86. Existing
AFD0F0/AFD1E0 free continuations are repaired. Existing unwind entries CBAD80,
CBAD88 and CBB210 must be preserved, not duplicated. The false pseudocode
returns after free neither terminate AF6C50 early nor omit B750C0 legitimately.

Strict MSVC Win32 `/std:c++20 /EHsc /MD /W4 /WX /O2 /Gy /fp:strict /permissive-`
compilation and the focused fixture passed, linking the current integrator's
manager and model-owner objects before a frozen core library. The call audit
checked19 numeric rows; its only failure is AF6D7A outside Ghidra's truncated
AF6C50 body. Eight indirect rows are explicitly outside the mechanical check.
The integrator must repair that body and repeat the audit.

The repository script was run
after eight successful `verify-seeds` matches; its new worker build failed with
MSB3191 creating `bsp_lua511.dir/Release/bsp_lua511.tlog` (access denied). The
integrator reported shared J: capacity pressure; this worker removed only its
verified generated build/win32 and moved strict compilation/fixture output to C:.
The integrator owns the combined build and current CTest run.

The ignored fixture passed after relocating actual AF7F40/AF6C50/AFD370/AF6180 bytes, using
real CRT allocation/free, Windows InterlockedDecrement, the full existing base
destructor, real manager erasure and actual F8D2D0 pool return. Its iterator
adapter invokes the concrete two literal-RET element destructors. The source
trajectory uses the actual canonical zero-reference model lookup and derived
reference retirement. Only the two deliberately dangling allocation identities
are normalized in the full2E0h comparison. The emitter/variant/mesh references
are backed fixture headers with count2; their zero-reference implementations
are not reached or supplied by synthetic successful callbacks.

This is bounded byte/fixture/source evidence. Complete application binding,
particle emitter/variant terminal lifetime, native binary/C++ exception ABI,
throwing original FH3 paths, successful AF74A0 composition, rendering and game
behavior remain unvalidated. No permanent tests were added.
