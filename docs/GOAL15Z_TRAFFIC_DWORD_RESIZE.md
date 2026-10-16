# Whole raw traffic DWORD resize

GOAL15Z reconstructs only 00495AE0..00495B92, all 179 bytes/78 instructions.
Coverage: complete owned routine. The retained library/tag name
STL_Vector_Resize_00495ae0 is descriptive evidence, not a recovered class or
Original C++ declaration. Source lives in src/native_traffic_dword_resize.cpp;
the physical interface is include/bsp/native_traffic_dword_resize.hpp.

| Native routine | Coverage | Physical inputs / cleanup |
| --- | --- | --- |
| 00495AE0..00495B92 | complete179B/78 instructions | ECX raw header, two actual stack DWORDs, RET8 |

## Physical inputs, return and actual frame

Original receives the actual borrowed header in ECX and TWO actual stacked
DWORDs {requested count,value}. No incoming EDX is read. Header words at+4/+8
are begin/end; whole insert also uses capacity+0C, establishing the borrowed
raw16-byte minimum. Word0 remains opaque; no whole producer/class is recovered.
Both normal exits use RET8. Source reserves EDX only to express that physical
stack shape in a naked fastcall envelope. Do not add EDX to Original metadata.

The chosen uint32 return exposes physical EAX only. No-op exits leave a size;
growth leaves insertion's result; shrink leaves the internal output-area
address after its stack storage is released. Neither Native caller consumes
that result. No Original semantic return type or usable output lifetime follows.

SUB ESP,8 preserves the actual uninitialized output-area preimage. Four saved
registers follow. The count is captured from actual caller stack+4 before any
diagnostic. Wrapped SUB/SAR2 and unsigned comparisons remain literal. The
genuine diagnostic may return and change actual cells; no guard, repair,
snapshot, new class, exception conversion or RAII was added.

Growth captures old size and end before its diagnostic. It then passes the
address of the CURRENT actual caller value word (stack+8), wrapped requested
minus captured size, captured end and actual receiver to whole insert. The
callee retires four words with RET10; this parent restores its four saved
words and eight-byte area before RET8.

Shrink captures old end, diagnoses malformed begin/end, reloads actual begin,
compares it with current end, and diagnoses again when needed. Only then does
it seed the second output DWORD with captured begin. Cut is wrapped begin+
captured_count*4. It checks cut against CURRENT end/begin and may diagnose a
third time. It passes the actual output area, receiver owner, captured cut,
receiver owner and the OLD captured end to whole erase. That child uses RET14.
The actual output address/preimages are not narrowed to initialized C++ locals.

## Six true Native calls

At 00495B1B it calls 00BF6713.

At 00495B2C it calls 00493120.

At 00495B51 it calls 00BF6713.

At 00495B5E it calls 00BF6713.

At 00495B74 it calls 00BF6713.

At 00495B84 it calls 00488FF0.

The four diagnostics have no parent stack arguments or cleanup; their own
complete Source entry pushes/discards five unused Source padding words around
the actual selected SDK zero-argument API. Insert receives four actual words
and RET10; erase receives five actual words and RET14. Incoming EDX is not
invented as a child semantic input.

## Both actual callers and named but incomplete parents

At 0049EB4C it calls 00495AE0. Owner 0049E560 pushes value0 and count from the
actual [EBP] word; ECX is EBX-40h. The following instruction overwrites EAX.
Typed body has2829 bytes in two ranges; the2833-byte captured span includes
the excluded four-byte alignment instruction at0049EB0C..0049EB0F. That
padding is not represented as executed body, and no listing/body repair was made.

At 004A5A36 it calls 00495AE0. Owner 004A5850 loads count from actual stack+0x24,
pushes value0 then count, and forms receiver from [EBP+4] plus the current
stack+0x34 word after those pushes. RET8 retires both arguments. Its loop/result
schedule does not consume EAX. Whole591-byte caller span is retained.

Both observed callers use zero, but the whole callee borrows and passes the
actual value word. Source accepts its full DWORD domain rather than baking in0.
Existing traffic_layers_rebuild_0049e560 and buoy_field_construct_004a5850
records in cc13_left8b_seg8 use Left8bHost virtual service boundaries. Those
names/build records do not establish actual Native provider/global/x87/member
closure. Both larger Native parents, their producers and game adoption remain OPEN.

## Real children and qualified service boundaries

The existing complete invoke_native_invalid_parameter_00bf6713 is genuine:
Native16B/9 instructions map to Source17B/9 instructions, differing in its
selected SDK IAT call encoding and policy. It calls actual declared
_invalid_parameter_noinfo(void), permits return and supplies no fabricated
five-argument API. Its five zero words are unused Source API padding. Native
BF66EF's private encoded-handler/debugger/Watson selection is not reconstructed
by that SDK call. The older NativeInputDeviceRuntime ledger alias is not used
as proof of transport or private CRT closure.

Erase is the complete accepted90-byte entry with current SDK diagnostic/copy
boundaries. Insert is accepted pin822facbc's complete436-byte entry and its
real Source children, under current malloc/new-handler/copy/free and owning28h
NativeSingletonVectorLengthError/host-EH policies. There is no fake provider,
global substitute, restricted allocator domain or std::length_error replacement
for that vector failure. T/X Main acceptance does not prove current C++
exception propagation through either naked parent.

## Whole emission and one concrete real-handler fixture

Strict and registered own COFF each contain one complete179-byte executable
section, public symbol at offset0, all78 instructions:155 identical fixed bytes
and24 bytes of SIX genuine zero-addend CALL rel32 operands. All internal
branches, flags schedules, stack offsets/cleanup and actual memory operands
match fresh live/installed Native. No NativeSize prefix or byte embedding is used.

Complete own/provider CODE/data/symbol/relocation objects, actual Core archive,
whole linked section extents and resolved operands are retained. Providers
total9613 executable bytes in12 real objects. Thirteen entire archive members
match actual objects. All selected sections are compared to the fixture PE
after every relocation is resolved; unselected sections are retained without
claiming execution. Compiler-generated BSS has no fabricated raw payload.
The fresh worktree changes MSVC anonymous-namespace/lambda identity hashes.
Ten accepted predecessor objects retain exactly the same executable bytes;
only recorded compiler source-local identity tokens in relocation names differ.
All public bindings stay unchanged and all CURRENT targets are resolved to
actual linked code. That supplementary normalization is not a runtime provider.

The single focused case addresses a concrete borrowed-cell/preimage risk. It
installs a REAL thread-local current SDK returning handler through the actual
SDK API, restores it afterwards, and reaches it through the genuine archive
diagnostic. No diagnostic provider or handler event is substituted for that call.
The actual two incoming cells start {count2,value13579BDF}; Source and captured
whole Native179 each use the same setup. The actual future eight-byte output
area is preseeded11223344/55667788 and the handler observes those unchanged
preimages, proving no C++ value initialization was inserted.

Backed header begins at data+5 with end+4. The real handler changes begin to
data, CURRENT end to data+3 and actual count/value cells to7/17. Resize must
retain captured count2 and OLD end+4 while reloading corrected begin. Cut is
data+2. The genuine erase sees suffix SAR2((data+3)-(data+4))=-1 and publishes
wrapped data+1 without copying; output words are actual receiver and cut+2.
The case checks those actual cells, unchanged backing payload/capacity/opaque,
physical EAX=actual output area and observed RET8 stack retirement. It executes
the first shrink diagnostic and erase composition with genuine current providers.

PASS applies only to that one returning-diagnostic shrink path. Growth/insert,
no-op paths, other diagnostic sites, positive/error copies, allocation/free,
overflow and unwinding are unexecuted by this fixture. X/T standalone fixtures
do not prove them here. Current C++ exception propagation through this naked
resize or insert remains unproved. Native private CRT/FH3/RTTI/ThrowInfo/SEH,
arbitrary hardware-fault/unwind-frame/full flags ABI and live game behavior
remain outside the Source service qualification.

Strict MSVC Win32 W4/WX compilation passed first attempt. Actual registered
scripts/build.ps1 and its available existing checks passed; full timing/logs
are retained. The unrelated existing LNK4006 spawn duplicate warning remains.
No tracked tests, framework, Ghidra write, game install or deployment was added.
Only own name/reconstruction evidence and one append-only CMake line change.
Primary owns Ghidra annotation/save/export and independent integration checks.

Reproduce with local/goal15z_traffic_dword_resize/build_probe.ps1 (-CompileOnly
or -FixtureOnly), scripts/build.ps1, and local/goal15z_traffic_dword_resize/audit.py.
Full setup/reject/correction/build/fixture/docs/commit/release/handoff timing and
immutable Source/artifact pins are in ignored receipt.json and HANDOFF.md.
