# CC12 model storage, vehicle class constructor and CRT unlock Source review

Primary admission accepts the ordinary Source implementations of `00749050` and
`00BF9E1E`, plus the retained model/base descriptor composition in the existing
production `GameNativeTypeStorage`. These interfaces remain separate from original
binary ABI and gameplay equivalence. The authoritative receipt is
`reports/cc12_model_class_unlock_source_primary_review.json`.

## Source and emitted evidence

The normal MSVC Win32 build passed all three enabled existing checks. The selected
input capture contains 534 project inputs, four artifacts, 56 Core objects and two
application objects. It is a selected dependency union, not a complete compiler,
SDK or build-input manifest. All prior 44 selected Core objects remain byte-identical.
The preceding 511-input capture and its four artifacts are separately retained.

The unlock helper has the complete original nine-byte/four-instruction schedule:
`PUSH 4`, real canonical `__unlock` provider, `POP ECX`, `RET`. Its current 810-byte
object has one indexed REL32 relocation to the genuine 21-byte/seven-instruction
canonical provider and its `LeaveCriticalSection` import. Both definitions occur
once in the actual Core archive, with object-identical member payloads. This
requires an already initialized, acquired lock descriptor at the canonical fixed
address; it establishes no lock initialization, acquisition, whole free provider,
heap provenance, original unwind or fault equivalence.

The vehicle class base constructor reproduces all 30 ordered stores after the
genuine damageable child: transient profile, final profile pair, 27 other stores
including a byte-only write at `+120h`, and `+80h` last. The current constructor
COMDAT is 305 bytes/42 instructions, with one actual child relocation. Its whole
object also contains an unreferenced 17-byte/eight-instruction store helper.
The real reused damageable provider's code, indexed relocations, ordinary C++
catch/unwind graph and EH data are retained. This is a borrowed-storage Source
interface, with valid aligned raw backing, stable profiles/string domains and
no alias with the child's allocation. Child failure performs no parent stores.
Original register ABI, FS/FH3 metadata and hardware-fault behavior remain unproved.

The constructor's own stores require access through `+133h`, while the `138h`
allocation contract comes from the existing derived-class/layout and Shipyard
allocation evidence. It does not follow from the highest store. Holes at
`110h..113h`, `121h..127h` and `134h..137h` remain untouched by the parent stores.
Older Ghidra prose is retained verbatim and this refinement is appended.

The model composition retains actual backing cells and both borrowed views for
the existing production owner's lifetime. Its initializer passes the same
counter/common bootstrap and orders camera, model, model base, then animation.
The 312-byte/91-instruction initializer and both seven-byte views preserve the
worker code. Anonymous namespace spellings are compared through physical indexed
targets; different raw objects are not declared byte-identical. Guard/counter
partial initialization and the existing failure-before-drain policy are retained.
Full CRT initializer coverage, absolute original IDs, fixed storage mapping and
the native class/model object consumer remain outside this admission.

## Runtime boundary

The exact combined executable exited zero in a three-tick startup smoke using
the genuine private SDK and ordinary isolated windowed settings. It created the
640x480 device and presented twice, with one reset skip. It remained at
`PressStartPoll` state 2, injected no input, ran zero mission frames and logged
88 `UNIMPLEMENTED` rows. There was no visual observation or execution proof for
the newly retained constructor/helper. Faithful startup and gameplay remain open.

Ghidra changes are descriptive names/evidence only, under the write lock with
old values retained, a saved project and refreshed affected exports. The original
installation is unchanged; no body, flow, prototype or missing-listing repair is
part of this batch.
