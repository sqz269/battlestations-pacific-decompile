# Text copy after base construction and cursor production

`GuiTextLifetime(GuiTextAfterBaseCopy00aa9520{}, ...)` implements only the
derived field-copy portion of `00ABB2C0`. `GuiTextCopyContinuation` then calls
`00AB8910`, `00AB8530`, and `00ABB1D0` in that order, retaining any pending
content continuation. The new cursor producer uses the existing canonical
Model, mesh, section, material, parenting, and resource owners.

This does **not** enable `GuiTextRuntimeFactory` copy construction or complete
`00AAB4C0` subtree cloning. The destination must first have been produced by
actual `00AA9520` base-copy construction. Integration now checks the canonical
base owner's completion predicate; a default-constructed owner is rejected.
The actual Text identity/refcount binding required by the cursor material is
also mandatory and is not supplied by `GuiWidgetOwner`'s semantic count.

## Evidence and ABI

The read-only Ghidra wrapper verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, before each live batch. Both complete bodies and
their complete assembly were read. `00ABB2C0..00ABB623` contains 195 listed
instructions, ending with `RET 4` at `00ABB621`. Its original ABI is destination
in ECX, source on the stack, destination in EAX. `00AB8910..00AB8C24` contains
236 instructions, ending with `RET`; ECX is Text and there are no stack inputs.
Flow inspection found zero gaps in both bodies. No Ghidra changes were made.

The producer at `00AA13BA` calls ABB2C0 with the nonnull source captured from
factory ECX and a destination allocated by AB79E0. The null-source arm calls
AB9650 at AA13DD instead. ABB2E5 calls AA9520 before any derived work. AA9520's
source-primary-node virtual10 call is **AA96FC**, not AA96E4 (which loads ECX).
Its second argument is zero, pushed early at AA9640. The first argument comes
from the current DWORD table at D5C0B8 indexed by source type+60. The observed
Text3 entry D5C0C4 is **clone flags 3Eh**, not a type-name pointer. This differs
from the existing Text auxiliary Model clone's flags 26h. A 26h clone cannot
satisfy the required AA9520 base-copy contract.

AA9520 copies the same base's fields, makes a fresh empty general child list,
resets its entries, forces byte76 to one, and publishes the returned primary
Model at +4C, clearing that Model's +138 bits 0/1 when nonnull. Native allocation
failure/null-clone branches, native Text pool/vtable/SEH construction, and full
canonical base-owner admission are outside this packet's implementation.

## Derived admission and pending work

The admission requires distinct, registered Text3 owners and distinct nonnull
primary nodes in the same runtime. The source companion must be live and any
earlier copied-source continuation must be complete. Destination association
must be empty. Buffer and glyph-child service identities must match the source.
These checks establish owner identity and require the separately reconstructed
AA9520 producer to have completed. The copied companion is published once, without calling the
default Text lifetime constructor or prematurely invoking AB8530.

The native order is UTF16 text EC, source string F4, fields FC..190, empty glyph
vector 198 and optional wide string 1A4, null 1AC/1B0, byte1B4, shader/font names
1C0/1C8, default shadow1D0, font scale1D8, null cached shader1EC, byte1F0, then
the four state colors 11C..158. The implementation preserves that order among
the copied values and uses x87 FLD/FSTP for 10C/114/164/178/18C/190/1D8. Shadow
and state color DWORDs copy without floating-point conversion. Font108 and
pointer180 remain borrowed; cursor184 and shadow188 are never shared from source.

The field-copy path uses the existing typed strings and containers. The full
4C8DD0 body was inspected: it zeroes the destination wide header, resizes, and
copies through the first UTF16 terminator. The three narrow copies resize with
41DD40, then call the existing `_memcpy` for **length bytes**; the resize supplies
the terminator. C++ allocation/string-header ABI and allocator callback timing
are excluded. Supported strings are null-free and fit the native signed length;
the existing semantic bool fields represent 0/1. No CRT/STL replacement is added.

Native-unwritten 194, 1B8/1BC, 1D4 and 1DC..1E8 are not copied or read. The
existing semantic alpha-texture default is not claimed to be a native store.
Glyph-offset validity remains false until the real offset producer writes it.

A copied lifetime admits one `GuiTextCopyContinuation`. Its `run_derived_00abb2c0`
executes cursor, draw-section, and current-text rebuild in order. Pending content
is retained with its original lifetime and service domains. `resume_after_child`
only resumes the exact child boundary accepted by the existing submission
continuation. It never repeats cursor creation, section initialization, or the
ABB1D0 cache-clearing prefix. Owners and this frame must remain alive while
pending; destroying a frame is not native completion or cleanup.

The copied lifetime stays in the existing `constructing` phase until content
fully completes. Existing Text scalar deletion therefore rejects it before
phase/flag writes. `has_incomplete_copy()` remains true across failure/pending;
direct derived teardown rejects it and destructor-time disposal terminates.
The shared owner must also consult that query in its existing
`require_no_active_owned_operation()` preflight, covering external retirement,
update and source reuse; the parent integrator coordinates that shared-file
addition with the independent base-copy packet. Completed destruction is unchanged.

Source inspection found no conflict with the required internal operations:
cursor/section construction, direct current-text submission, canonical glyph
append, glyph recompose/bounds, and base clip entry do not consult the external
owner-operation guard. New glyph children have independent live lifetimes.
This route remains unexercised with an actual copied Text implementation. The
existing `GuiTextRuntimeImplementation` constructor always makes a default
lifetime; a future explicit adoption path must install this same copied lifetime
and retained continuation without invoking that constructor.

## Cursor sequence and required actual retention

AB8910 returns immediately for a nonnull cursor184. Otherwise it allocates and
constructs the Model named `gui_cursor`, publishes it before name release, clears
Model138 bits 0/1, and parents it under the current Text4C node. Model current38
must be B6DB10. Its matrix uses one live D7A24C load for four diagonal entries,
zero elsewhere except element14 from a separate live D7A260 load. Visibility
factor becomes zero without recursion.

The actual cursor receives a new mesh through B75170, passing zero and two copies
of a fresh x87-loaded D7A260 sentinel. Current renderer38 creates a declaration
from `simplecolor.mvfm`; renderer is reloaded after name cleanup before virtual5C
creates a four-vertex stream with flags1. Stream0 is installed. The declaration
creator is released **before** the vertex creator; AB8400's order differs.
There is no index-stream allocation.

A new section receives primitive5 and raw range words `{0,4,0,2}`. The actual
material factory resolves `GuiCursor.mshd`. After its name cleanup, B18A40 must
retain this same Text object, then the section retains the material. The material
creator is released, actual layout is rebuilt, the section is appended, and
section/mesh creators are released in that order. All final releases use the
canonical actual owners and current terminal dispatch.

`GuiTextCursorParameterOwnerServices::bind_retained_text_00b18a40` is required.
It must resolve an actual Text identity with atomic refcount+04 and a canonical
current virtual0 binding, and perform the existing actual B18A40 body with flag1.
That body releases an old retained owner first, clears after its callback,
publishes the new owner+0C and byte10D, then retains the incoming owner. Equal
identity still releases/reacquires. Passing a `GuiWidgetOwner*`, a semantic
`references_04` address, a COM texture, or a null successful fallback is invalid.
The service and owner must survive the material's eventual release.

The cursor slot is a borrowed canonical `NativeNodeBinding*`. Its initial Model
reference follows the native parented-node ownership contract; AB8250 does not
separately release cursor184. No second widget/model hierarchy is created.

`GuiTextCursorAcquired` retains native caller temporaries and creator references
on an interrupted call; published state is left visible and the frame is marked
failed. A pointer is cleared before any possibly terminal release to prevent
double release on callback failure. Live temporary-name headers are explicitly
marked. These are diagnostic C++ frame fields, not another resource owner.
Automatic rollback, interrupted-call retry, native faults, unmasked x87 exceptions
and SEH unwind equivalence are not claimed. AB8530/ABB1D0 retain their documented
normal-return and pending-boundary contracts.

## Validation

See `reports/gui_text_copy.json` for every CALL instruction, its containing body,
callee implementation or required binding, producer evidence and cleanup.
`scripts/build.ps1` and `tools/verify_report_calls.py` are the scoped checks.
The available pool and Section-UV fixtures do not construct the canonical Text
and native renderer/resource domain needed to execute this new cursor path;
no synthetic ownership fixture or permanent tests are added. Build and existing
tests do not establish native ABI compatibility, full factory copy, rendering,
or in-game behavior.
