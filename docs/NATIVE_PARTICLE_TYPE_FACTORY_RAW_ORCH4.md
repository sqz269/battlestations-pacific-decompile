# Raw particle type factory

## Scope and evidence

`B00CE0..B00ED8` is reconstructed by the raw context overload in
`native_particle_type_factory_raw.cpp`. The old host binding overload remains
historical source; this overload uses the genuine raw constructors and all five
raw parsers. Names are descriptive hypotheses, not recovered symbols.

The 505 installed PE bytes match saved Ghidra in `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. All 16 calls, all five eleven-byte cleanup actions,
the ten-byte dispatcher, and complete FH3 map/flags are recorded in
`reports/native_particle_type_factory_raw_orch4.json`. Original ABI: ECX points
to the actual eight-byte kind header, EDX to the eight-byte name header; the
stack holds parent and TextBuffer; EAX returns the child and `RET 8` removes
those two arguments.

## Construction and current dispatch

Case-insensitive comparisons select Sprite, Axial, Floating, Object or Tracer.
Allocation sizes are respectively 90h, A4h, 8Ch, 98h and E8h. The real object
allocator is `singleton_lifetime_allocate`, with identical native/host sizes.
The current parent+10 word is read **after allocation** and supplied with the
name and parent to the raw constructor. Sprite and Axial receive final numeric
profiles D5DD18 and D5DCC0 after their base constructors; the other constructors
install their final profiles themselves. Unwritten owner bytes stay unwritten.
The same actual string publications, actual D3DX import, current numeric cells
and explicit record+18 residue flow through construction.

The factory reads the owner's current profile, then reads its current slot+08
through borrowed actual table cells. It dispatches the captured target, rather
than the selected kind or the profile's default meaning:

| Captured target | Genuine source provider |
| --- | --- |
| B08AC0 | Sprite raw parser |
| B064A0 | Axial raw parser |
| B07D60 | Floating raw parser |
| AF8BD0 | Object raw parser |
| B0AD50 | Tracer raw parser |

All contexts must borrow the same actual string/runtime domains, shared scratch
and current profile cells. This is a caller contract, not synthetic duplicated
services. Actual particle table cells are supplied by the common property
context, and actual emitter table cells are explicit factory context inputs.

## Unknown kinds have a different native failure boundary

An unknown kind reuses the original parent and still captures its current
profile/slot+08. The three known callers are the SmartArea B02210, Sphere B02FD0
and Cone B03EC0 emitter parsers. Their final emitter tables are D5DE48, D5DE88
and D5DEBC. Their slot+08 targets are respectively B01CE0, B02BC0 and B03970:
these create 108h particle records through AFE0A0; they are **not** the emitter
parsers in slot+14.

Those record constructors expect ECX owner and two stacked arguments
`(model,time)` with `RET 8`. B00CE0 pushes only the TextBuffer and expects a
particle parser's `RET 4`. An unknown kind under an emitter parent therefore
reaches an incompatible native call, not a harmless no-op or parser fallback.
The new source captures that actual target and reports an explicit boundary
at B00EC2. It does not invent the absent argument, invoke another parser,
allocate a substitute record, or report success. A reused parent whose current
profile/target is a supported particle parser follows that parser normally.
An unknown profile is a boundary at B00EBC before an unavailable cell read.

Null allocation still reaches a null owner dereference. No null-success return
or synthetic recovery was introduced. Hardware-fault behavior is not reproduced
by the source exception interface.

## Exception ownership and retained invocation

DF341C has magic 19930522, maxState 5, map DF3440, no try/IP/ESType maps and
flags 1. Each state 0..4 transitions directly to -1 and runs its own action:
CBB3D0, CBB3DB, CBB3E6, CBB3F1 or CBB3FC. Every action reads the allocation at
EBP-10, calls BF65AC, adjusts ESP and returns. CBB407 loads DF341C and jumps
to BF6B43 (`__CxxFrameHandler3`).

Allocation happens before a state is armed. A constructor failure first finishes
the callee's cleanup, then the factory frees only the raw allocation. Success
sets state -1 before virtual dispatch. Parser failure consequently does not
destroy or free the child. The one-shot acquired frame retains its owner,
captured dispatch identity and optional concrete parser child. Failed children
must remain alive while their referenced provider storage has obligations;
there is no destructor rollback or replay. Axial's incoming builder residue is
explicit in its context; the other parser frames consume the factory frame's
explicit incoming builder kind.

An Object model child can retain a failed VFS invocation whose existing
destructor terminates. No discharge API exists for that state: keep the entire
factory/parser/context graph alive for process life. The factory does not
invent a recovery operation for it.

## Validation and limits

Validation results and exact artifact hashes are recorded in the report. The
focused ignored probe executes the complete copied original factory body and
bridges dependencies to the same genuine raw constructors and parsers used by
the source overload. It does not replace parsing with a capture callback. The
supporting providers' original-byte validation remains their own evidence.

The strict MSVC Win32 build and all three existing CTests passed. The final
library probe passed five original/source factory cases, comparing defined
owner fields, names, all 28 record bytes, runtime parameters and text cursors.
It also checked native `RET 8`, the precise unsupported Sphere-parent target,
and source parser-failure allocation/child retention. No permanent tests were
added. Uninitialized owner bytes, native FH3 execution and resource cache/model
loads were not part of that comparison.

This is a new C++ composition interface. It is not an original register/FH3/SEH
binary replacement. Native exception dispatch, allocation failure, arbitrary
hardware faults, concurrent mutation, unrestricted unknown targets and gameplay
are unvalidated. The unsupported emitter record path is documented and captured,
not executed or claimed complete. Prior host evidence is preserved in the report.
