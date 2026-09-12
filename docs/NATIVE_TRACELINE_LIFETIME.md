# Actual Traceline and payload lifetime

Addresses: `00AF1980`, `00858380`, `0086AD50`, `0086ADE0`.

This packet closes the lifetime of the actual `1BCh` Traceline slot allocated
from `F8C288`, and its separate `80h` payload authored inline in `B0B6A0`.
It reuses the existing `NativeModelOwner`, node hierarchy, scene binding,
point-light backlinks, native `+04` reference count and actual retained-owner
registry. It introduces no payload constructor or secondary ownership domain.
The `BAD6F0` tracer with `7B0h` storage is a different type.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| AF1980..AF19A3 | ECX Traceline; tail B750C0; RET | complete through existing actual model destructor |
| 858380..85839B | ECX Traceline; flags on stack; EAX same slot; RET4 | complete, including same-pool return |
| 86AD50..86ADDF | ECX payload; RET | complete ordinary body and C++ state0 cleanup effects |
| 86ADE0..86ADFD | ECX payload; flags on stack; EAX same payload; RET4 | complete, including matching scalar free |

All four bodies exist in the verified `C:/Users/sqz269/bsp.gpr` project,
program `/battlestationspacific.exe`. The last instructions are respectively
`AF199F JMP B750C0` (5 bytes), `858399 RET4` (3), `86ADDF RET` (1), and
`86ADFB RET4` (3). Inclusive ends include the complete instruction. No Ghidra
functions, prototypes, names, comments or saved analysis were changed here.

## Destruction and returning-free continuations

`AF1980` captures `+188` before publishing base profile `D0C8C8`, frees the
nonnull captured segment array, retains its dangling pointer, restores ECX,
and tail-jumps to the full `B750C0`. The base destructor publishes its own
profile, releases current `+174` then current geometry `+180`, and completes
the real node/scene/hierarchy/string/backing cleanup. Native `AF1999 ADD ESP,4`
is a returning-free gap omitted by Ghidra's pseudocode.

`858380` then returns the original slot through `AF1EA0 -> AF1D30` using the
same actual `F8C288` pool if flags bit 0 is set. Upper flag bits have no effect.
`D0C8C8` instead points at `858330`, whose return helper is `B748C0` for the
normal Model pool. That separate route is not implemented: an actual `1BCh`
Traceline slot must never be returned to the `188h` Model pool.

`86AD50` publishes `D0D4A4`; captures current `+40`; atomically decrements
captured owner `+04`; and invokes captured owner's current virtual 0 only
when the result is zero. It clears `+40` after the callback, then repeats the
sequence for newly loaded `+44`. A callback can replace `+44`; the new value
is released. The canonical actual-owner service validates the exact atomic
and performs the real terminal destructor. Missing owners throw on zero.

The nonnull current `+4C` backing is freed and then cleared. `+50/+54` and
all other payload fields remain unchanged. The producer `B0B96C..B0B9ED`
allocates `80h`, publishes the profile and initializes those pointer/count
fields inline; later `B0BBF4..B0BC70` retains actual resources into `+40/+44`.
`86ADC7 ADD ESP,4; 86ADCA MOV [ESI+4C],EBX` is another omitted continuation.
The whole function listing establishes EBX=0, ESI=payload, and EBP=current
InterlockedDecrement import; terminal calls preserve those registers.

`86ADE0` frees the payload with the matching `BF65AC` service only after
successful destruction and only for flags bit 0. `86ADF5 ADD ESP,4` reconnects
to `MOV EAX,ESI`, so it returns the original address even after free.

The FH3 state0 map `DC740C = {FFFFFFFF,C95460}` referenced by `DC7414`
routes exceptions to `C95460`: ECX=`[EBP-10]+4C`, tail `869B10`.
Its returning-free gap `869B1F..869B27` clears the current array pointer.
The C++ catch performs exactly that cleanup and propagates the error; it
does not clear the throwing reference or release an unvisited reference.
The original SEH/FH3 ABI is not transplanted.

## Canonical derived reference

`NativeTracelineReference` binds after `B0B6A0` publishes `D0C928`, before
`AF3440`. Its constructor changes only existing host bindings: it borrows the
same actual `+04` and node lifetime runtime, preserves the binding's
`NativeModelOwner` context, and installs current-profile type/scene callbacks.
The caller also registers this object in the same `NativeRenderActualOwners`.
`model_owner()` and `lifetime_access()` expose those exact borrowed objects
for integration identity checks. A normal `NativeModelReference` cannot
replace this derived companion.

| Current operation | Proven native body |
| --- | --- |
| virtual0 -> scalar delete4 | BD30E0 -> 858380 |
| type predicate0C | 6EF860 with the same initialized ModelTypeBootstrap |
| logical release18 | B6F310 with actual point-light backlinks and hierarchy |
| world changed40 | B6DBE0 on the same binding |
| set scene50 / remove scene54 | B6ED80 / B6EE10 with the same real scene runtime |

Terminal release destroys the actual object, returns its physical slot,
unbinds the lifetime, then invokes `disposal.retire`. The callback removes
the same actual-owner registry entry and may dispose both companions; no
access follows it. `retire_after_failed_construction` only removes host
bookkeeping after the owner has already reached `dead`, without a pool
return. The caller still owns raw storage on that failure path.

The borrowed pool/table views and actual services must remain stable.
Membership inspection proves current slab table, exact `1BCh` boundary and
retained `+1B8` ID agreement, not whether that slot is allocated or live.
The caller supplies a live constructed owner and externally coordinates use.

## Verification and limits

The ignored local fixture at `C:/Users/sqz269/bsp-as-traceline-lifetime`
compiles with MSVC Win32 `/MD /W4 /WX /O2 /fp:strict`. Its neutral `probe.exe`
has an embedded manifest; `/SAFESEH:NO` applies only to this byte fixture.
It verifies live Ghidra bytes against the installed disk image before
relocating native operands, including every returning-free continuation.

Ten original-byte/C++ Traceline scalar comparisons cover null/non-null
segment arrays and flags `0,1,100h,101h,FFFFFFFFh`, comparing all `1BCh`
bytes, stale tail, pool free indices/depth, allocation effects and return
address. Two payload comparisons cover empty resources and real canonical
render-context destruction whose first retirement replaces the second
reference; five payload scalar comparisons cover the same flag values.
Native bytes execute the four routines and pool return. Both sides delegate
`B750C0` to the existing complete actual-owner implementation; this is an
explicit differential boundary, not independent base verification.

Focused C++ checks exercise missing-owner state0 cleanup and the canonical
Traceline terminal path with a real retained scene and actual point-light
forward/reverse backlink. Logical release removes the backlink, full base
cleanup releases the scene, the slot is returned, and the runtime is unbound
before disposal. No successful fake terminal callback substitutes for an
unimplemented native owner. Original native SEH unwinding is not executed;
its scope map and helper are checked statically and its C++ cleanup effects
are checked separately.

The normal repository `scripts/build.ps1` Release build passed, including
the Win32 executable and `reconstructed_math` (1/1). The new source is
compiled directly in the strict fixture; the root integrator owns its CMake
registration and final integrated checks. The live report verifier checked
8 direct call rows with 0 failures; four indirect rows remain explicit.
Detailed byte hashes,
call-site evidence and final check status are in
`reports/native_traceline_lifetime.json`. No permanent tests were added.
These are new C++ interfaces, not binary drop-in replacements or gameplay
validation. AF3440 attachment and AF26A0/B0A110 updates belong to other
packets, and this packet makes no rendering or executable frame-timing claim.
