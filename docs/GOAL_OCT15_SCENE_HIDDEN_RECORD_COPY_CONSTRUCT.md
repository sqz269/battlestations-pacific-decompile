# Raw hidden-record copy constructor

Addresses: 00469240, 00C61A90.

**General Source-ready remains false.** This packet supplies whole ordinary
normal control-flow code and a qualified cleanup-action mapping. It inherits
both the published raw bag-clone admission and the raw string-duplicate
admission. It does not close their failure or Original unwind gaps.

| Routine | Coverage | Observed native ABI | New Source |
| --- | --- | --- | --- |
| 00469240 | Whole 181B / 58 instructions | ECX record, one LIVE source-pointer stack word; RET4, EAX captured record | Explicit input4/saved-this4 and two concrete contexts |
| 00C61A90 | Qualified whole 11B / 3 instructions | CURRENT[EBP-10], add54, tail cleanup | Explicit current saved-this4 reader; ordinary cleanup transport |

The older Seg6Host projection remains separate. Live caller and reference
queries found no references to this constructor; no incoming-call coverage,
application adoption or native-entry execution is inferred.

The new frame contains stable/disjoint binding metadata for actual input4 and
saved-this4 backing. The actual record, cells and payloads retain their reached
raw read/write schedules. Implicit compiler/private-state/outgoing argument
homes are excluded; the new C++ frame is not an Original stack-frame mapping.

Native PUSH ECX first saves this. The constructor computes record+54, writes
the borrowed actual image word CE5640 to record0, stores saved-this again,
then zeroes header54 and header58. CE5640 is only a literal raw image tag:
there is no Source vtable, virtual dispatch or nominal class-ownership claim.
Only then does it read the LIVE source-pointer input once, retaining that
captured source for the rest of the constructor.

It reads current source+4 before arming state0 and invoking the real whole bag
clone. It publishes the returned bag at destination+4, reads current source+8,
duplicates it and publishes destination+8, then reads current source+C and
duplicates it. It computes source+14, publishes the class result at
destination+C, **then** reads current source+10 and publishes destination+10.
The real existing x87 matrix provider receives the captured source+14 and
destination+14; its sixteen FLD/FSTP pairs retain their actual progressive
overlap schedule. No matrix snapshot, finite guard or FP reset is introduced.

After matrix return, captured source advances to its header+54. An exact
destination-header self match skips resize and all header reads. Otherwise
the code reads current source length, calls raw resize with preserve=true,
rereads current source length, and only if it is nonzero reads current
destination length, current source data, then current destination data.
It copies that captured DESTINATION length, retaining overlap and a reached
zero-count/null call through a Source-only x86 SDK memmove tail adapter.
The adapter supplies current SDK transport, with no Original helper-frame
identity or extra native reconstruction credit. Its emitted edge awaits build.

| Native transfer | Role |
| --- | --- |
| 00469284 CALL 008F41F0 | Bag clone |
| 0046928F CALL 00438E40 | Name duplicate |
| 0046929A CALL 00438E40 | Class duplicate |
| 004692AF CALL 004134F0 | Matrix |
| 004692C2 CALL 0041DD40 | Parent resize |
| 004692D7 CALL 00BF7680 | Parent bytes |
| 00C61A96 JMP 0041DD20 | Qualified cleanup |

The final copy call has three pushes and ADD ESP,0Ch. The matrix provider
returns with RET4, resize with RET8, and the enclosing constructor with RET4.
Each typed row is attributed to its actual containing function. The cleanup
JMP's existing flow override is retained rather than repaired to fit a claim.

Handler C61A9B loads FuncInfo D890C8 and transfers to the actual FH3 service.
FuncInfo magic19930522/maxstate1 points to D890C0: state0 goes to -1 through
the sole C61A90 action. That action rereads CURRENT saved-this4, adds54 with
x86 wrapping arithmetic and destroys only the current pooled parent header.
It never frees the bag, either duplicated raw string or the record. The Source
guard is armed before bag cloning and disabled on normal return; the native
epilogue unlinks its handler without a state-minus-one store. Source guard
transport and secondary cleanup throws are not Original FH3/SEH equivalence.

The real bag provider and concrete NativeScenePropertyCloneContext are reused
from native_scene_property_clone.cpp. Its documented successful canonical
domain requires explicit startup, stable finite/unshared/acyclic owning graphs,
closed NUL-free ASCII keys/current locale, valid selected record/payload/count
domains, stable canonical memberships and no invalid alias, mutation, trim,
overflow, reentry or provider failure. Type1 conversions require an empty x87
stack at each conversion, all six exception masks already set, valid PC24/53/64
and RC and no pending unmasked exception; all binary32 patterns remain allowed.
No new graph owner or validation layer is invented. The clone context and raw
pool context borrow the same actual live canonical string process and the
clone's actual physical process.

The real duplicate provider is also reused, including null input and its
normal successful, readable contiguous NUL-text/nonwrapping len+1 and disjoint
current-allocation admission. Failure/new-handler/reentry and naked/private
unwind paths remain unproved. Both provider contracts apply where reached;
whole parent code does not widen either admission.

The matrix provider is the already accepted exact 103B raw x87 implementation.
The original copy service is overlap-capable; the current SDK memmove binding
retains that needed behavior. The prior constructor's installed x86 SDK
zero-count-path review is hash-pinned, with a fresh emitted edge check pending.
Original FP instruction-address/fault identities and native differential
execution are not claimed by compilation or the existing math checks.

Guarded analysis verifies C:/Users/sqz269/bsp.gpr and program
/battlestationspacific.exe. The report pins all 192 original live/installed
bytes, 61 instructions, seven typed transfers, actual provider Source hashes,
zero incoming references and the genuine one-action unwind table. Correct
native names/comments will be preserved under the write lock, with prior
values saved, project save, export refresh and readback. Draft registration and
normal MSVC Win32 build follow primary complete-Source review. No new tests,
Original binary ABI, native entry, application or game validation is claimed.
General Source-ready remains false after a successful build.
