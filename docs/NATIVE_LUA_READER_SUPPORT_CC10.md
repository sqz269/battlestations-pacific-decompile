# Actual Lua reader support (cc10)

This packet implements the complete normal bodies of B66FA0 (85 bytes,
through B66FF4 inclusive) and BD5790 (204 bytes, through BD585B inclusive).
It reuses the existing actual 14h `NativeLuaObjectStorage`, 4C8h state owner,
Lua 5.1.1 library and native object providers. These are source interfaces;
the existing registry-reference GUI adapter remains a separate projection.

## Copy construction

B66FA0 takes ECX destination and a stacked source pointer, returns the same
destination in EAX, and uses RET4. Four DWORD copies transfer owner, kind,
index and opaque0C in order. It then reads the source tracked BYTE, captures
the destination's current owner, and writes that byte to destination+10.
Destination padding11..13 is untouched. This differs from B67690 assignment,
which releases an existing destination and does not copy opaque0C.

A nonzero tracked byte adds the destination's actual address to the captured
owner's slot indexed by DWORD-wrapped `stack_offset + index`. The signed
high-water comparison and pointer/count publication follow the native body.
No Lua value is pushed or referenced through the registry. Kind does not gate
registration, and there is no self-copy guard. Source requires the existing
valid tracking domain (50 slots, five pointer positions), without adding a
new owner, counter, bounds fallback or reference policy.

## Tagged child lookup

BD5790 is fastcall with output in ECX and parent in EDX, two stacked DWORDs
for tag and value bits, EAX output, RET8. It first constructs an unbound
output through B65F50. Tag zero calls B67800 with the value as a C-string
pointer; tag one calls B67720 with the signed integer bits; tag two converts
the float32 bits using the existing CVTTSS2SI provider, then calls B67720.
Other tags leave the fresh output unbound. The integer provider's existing
kind2/table and non-kind2/call-frame distinction is retained, not normalized.

The returned temporary is assigned to output using B67690 and destroyed
using B67700. Assignment preserves output opaque0C and padding. Temporary
tracking uses its real source address; destroying it leaves the returned
output as the remaining tracked reference. The raw Lua lookup providers
retain their nonlocal-transfer contract. No protected Lua frame, error
fallback, additional metamethod call or registry-reference adapter is added.

The FH3 descriptor DFFBDC names four unwind states at DFFBBC. State0 uses
CC5970 to destroy the output only when its construction flag is set.
States1/2/3 use CC5989/CC5991/CC5999 to destroy the local temporary, then
transition to state0. The source C++ catch preserves that cleanup order for
C++ exceptions. Native Lua longjmp, SEH, FH3 frame layout and exception identity
are not replaced by this catch. The original helper is tested only normally.
The previously undefined CC59A1 handler is ten bytes ending CC59AA; the
primary defines it under the write lock and preserves the evidence record.

## Verification

All 289 bytes of the two game bodies match the live saved Ghidra program
and installed PE. The strict Win32 build and all three existing checks pass.
One ignored /MD, /MANIFEST:EMBED probe executes the original instruction
bodies. B66FA0 is unchanged; BD5790's six direct calls are rebound to the same
production object primitives used by source. Four copy pairs exercise
untracked/tracked values, existing slot references/high-water, and self-copy.
Eight lookup pairs cover present/missing string keys, integer and float keys,
negative truncation, NaN conversion and an unsupported tag, using actual Lua.

The probe compares complete output bytes, actual Lua values, stack height,
tracked counts/addresses, cleanup state and MXCSR. Private temporary addresses
in stale owner cells are normalized after cleanup; their active pointers are
checked before destruction. No original FH3 or complete reader execution is
claimed. Source and native whole-reader allocation behavior is not tested.

## Remaining reader dependency

004425C0 constructs a 14h reader: profile CE44FC, untouched proxy word+4,
and a LuaObject vector at +8/+C/+10. Its by-value 14h root is copied through
00442220 and destroyed through B67700 before RET14. 00441A20 is stack-reader
destruction; 00441A70 is the flags-dependent scalar-deleting wrapper, despite
an older no-argument ledger description. BD8E20 enters a key through BD5790
and pushes the result; BD7A20 adjusts to the vector and pops via BD7130.

Those raw reader/container operations and the property visitor are still
dependencies. This packet does not duplicate STL vector internals, replace
the raw reader with the logical GuiLuaReader, wire AC6600, or claim application
or gameplay validation. The new copy and lookup leaves supply its real object
lifecycle and key-dispatch prerequisites.
