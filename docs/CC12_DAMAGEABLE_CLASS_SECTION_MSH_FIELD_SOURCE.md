# Retained MshCategory prefix and release Source

`NativeDamageableClassSectionMshFieldFragment` implements two disjoint retained
regions of `0087CA80`: `[0087CEBB,0087CF35)` (122 bytes, 35 instructions) and
`[0087D181,0087D1A0)` (31 bytes, 10 instructions). It is an ordinary MSVC Win32
owner, not an original entrypoint or the excluded 741-byte field lifetime.
The implementation has only a constructor, destructor, `open`, and `close`.
It is fixed, noncopyable and nonmovable. No shared provider was changed.

The selected evidence was already frozen and accepted in the retained readiness
report. This packet hashes that 741-byte input and decodes only the selected
153 bytes / 45 instructions. It does not reopen the installed executable, a live
Ghidra listing, callee, constant/name bytes, handler, or excluded middle.
`MshCategory` / `00D0E190` is retained key metadata; the Source literal is not
new proof of the original address's bytes. Earlier typed parent snapshots remain
historical after later annotation edits.

## Storage and lifetime

Construct the owner before `open`, inside the already successful state13
`NativeDamageableClassSectionIteratorSetupFragment`, with the same actual
iterator scratch. The enclosing owner still owns Sections S80, key S58 and value
S2C. The row-begin state14 temporary has already closed and back-row selection
has returned. The owner borrows fresh aligned14h Lua E0 storage and aligned8h
raw S18 string storage (length DWORD at0, pointer at4). It starts no typed
`NativeString` lifetime and never copies a tracked Lua object.

`open` returns with state17 retained across every excluded CF35..D180 inner field.
Each inner owner must have restored17 before the one explicit normal `close`.
The destructor supplies exceptional cleanup; successful scope exit must not
substitute it for normal close. The same fixed owner cannot be reopened, replayed
or closed twice. Iterator witnesses introduce no new accessor or owner checks.
No row, vector, cursor, effect wrapper, continuation service or parent loop is
implemented or invoked.

Bindings, actual Lua owner and valid tracked indices/capacity must remain stable
under the existing providers' contracts. The inherited error handler stays below
every consumed or removed Lua slot. Scratch/header/buffer and canonical pool
publication cells must not alias the owner's private state, captures or Source
frames. Callbacks may make valid changes to the actual header and buffers; they
may not reenter or change bindings. Captured values survive those header changes.
The converting getter must yield a valid nonnull NUL-terminated span; copy ranges
must support the DWORD count, including the wrapping-zero case. Unsupported Lua
values still have no substituted string/default; null/fault identity is held.

## Ordered behavior and failure ownership

| Stage | Operation and active owner |
| --- | --- |
| Entry13 | Real protected named lookup from actual iterator value S2C into fresh E0. Arm15 only after it succeeds. |
| State15 | Real protected converting Lua string getter, then zero S18 length, then data. Walk bytes through the NUL; derive the DWORD length without `strlen`. |
| State15 | Real raw `0041DD40` resize with preserve=true. Capture data once, evaluate presence, and capture current length unconditionally, even for null data. |
| State15 | For nonnull captured data, call host `memmove` with captured length+1, including count0; no zero-count branch is added. |
| Transition17 | Arm17 before destroying actual E0 through `00B67700`. Return the live retained owner. |
| Exceptional15 | Lower13 before E0 destruction. Do not destroy the string, including partially changed/allocated storage. |
| Exceptional17 | Lower13 before real raw `0041DD20` on the current S18 header. Do not retry Lua cleanup. |
| Normal17 | Test saved data, lower13, and skip every pool operation if saved data is null. Otherwise compute saved length+1 before the current pool getter and return that saved block. |

The retained EH map contains state16, but neither selected region activates it;
this Source has no state16. The Source owner state is private and does not write
an original parent's FH3 frame. After normal lowering13, a getter exception does
not cause a destructor retry. Current header contents and callback writes remain
untouched by normal close. Saved-null/current-nonnull skips normal return while
exceptional17 returns the current block; saved-nonnull/current-null does the
opposite. No rollback or pointer/length reread replaces either contract.

The protected getter is the real Lua5.1.1 conversion path: numeric values may
change their actual TValue before later GC/finalizer failure. Its adapter restores
stack height on error while retaining completed value changes. It is not an
exact-string predicate. Existing tracking releases can change remaining indices;
the enclosing iterator owns the same actual objects through propagation.

## Raw pool provider

The owner borrows `NativeStringRawPoolContext` with canonical AA8 publication,
AA4 small-return gate and AA0 raw manager cells. Normal nonnull close always calls
the actual raw `00419CC0` overload before `00BD1510`, including large sizes and
disabled small returns. It reads the actual gate through the existing return
provider; the original unused PUSH1 argument is not a replacement gate value.
Current-header exceptional cleanup delegates to the existing raw `0041DD20`.
No `NativeStringStorage` bridge, private pool/publication cell or fabricated
callback is introduced. Getter exceptions remain capable of escaping `close`.

The actual raw provider's established qualifications are retained: lower manager
and registration/allocation services must be genuinely bound, and the application
must install the correct shutdown deletion binding before first pool use. This
packet does not install or prove the DamageableClass production binding. The
existing resize provider omits its internal zero-count preserve copy; the selected
parent's nonnull final copy is separately present even when its count wraps to0.
Host CRT formatting, copy/fault behavior and Native binary identity remain held.

## Actual Release compilation and COFF review

Root's Core command tlog and project were frozen before the next Root build.
The candidate and four genuine provider translation units were compiled with its
Release settings: `/O2 /Ob2 /Oy- /MD /W4 /WX /fp:strict /EHsc /std:c++17`,
WIN32/_WINDOWS/_MBCS/NDEBUG/Release definitions, diagnostics, external-header and
include settings. Output/include roots were localized; listing and include
receipts were added. No normal build, link, new test, probe or runtime was run.

The final candidate has13 physical sections and5 code sections, totaling399
bytes: constructor41, destructor125, destructor handler29, close56, open148.
All code, physical section headers, raw symbols/AUX records, string table and
relocations are frozen. Four initially emitted unused template accessors were
removed; all five final method/handler byte sequences and named relocations are
identical to the initially reviewed versions. No helper API was added.
The unreferenced4-byte BSS COMDAT is the compiler header's AVX2 fallback cell,
not an introduced Native global.

Open is physical section9. Lookup is +19h, state15 +22h, protected string getter
+29h, length/data zeros +3Ah/+40h, byte loop +47h..+4Ch, resize +58h, captured
data read/store +5Dh/+63h, unconditional length read/store +66h/+68h, pointer
TEST +6Bh, conditional memmove +73h, state17 +7Eh and Lua destruction +85h.
The optimizer moves the pure pointer test after the unconditional length capture,
unlike Native TEST at CF0B before its length load CF0D. There is no intervening
callback; stable disjoint storage preserves the ordinary result. The report does
not claim identical instruction/fault/flag timing and adds no artificial volatile
or inline assembly to force it.

Close is section7: saved-pointer CMP+3, state13 store+7, null branch+Eh, saved
length load/increment+14h/+17h, real getter+1Dh and real return+2Dh. Saved data is
reloaded from the private owner after the getter, unlike the Native pre-call PUSH;
callbacks are forbidden from aliasing that private capture. Current S18 is never
loaded or cleared by close. `/Oy-` does not force an EBP frame for this function;
its actual emitted FPO receipt is retained.

Destructor section5 selects15 or17 and stores13 at+2Ch/+57h before the respective
real cleanup at+36h/+61h. Section11 is its36-byte FH3 FuncInfo: magic19930522h,
maxState0, no unwind/try maps, flags5 (ordinary synchronous/noexcept behavior).
Section10 SafeSEH contains physical symbol index35 for handler section6.
Open/close introduce no local unwind guard or FH3 owner frame. Correct enclosing
C++ scope must arrange destruction of this already-constructed retained owner;
no Native parent frame composition is claimed. Cleanup escaping the noexcept
destructor terminates under the ordinary C++ rules.

All seven project externals resolve by exact decorated names to the freshly
compiled real providers: Lua lookup49/125B, destruction38/39B, protected string
getter69/113B; raw string destruction56/51B and resize65/212B; raw pool
getter30/316B and return20/320B. These are physical provider section indices.
The complete objects and reference graph include actual callback, exception,
weak-AUX, pool constructor, allocation, return and `__finally` paths. All66 Lua
code sections match the previously retained `/Oy-` object in bytes and indexed
relocations. Unrelated sections are frozen as inventory, not new semantic credit.
Library Lua/CRT/Win32/MSVC symbols and the lower manager/allocation/registration
Source frontier are explicitly retained; this compile-only receipt is not a link
or production-service proof.

The JSON report records exact artifact and compiler pins, five complete objects,
current Source/dependency snapshots, baseline Git blobs, complete bounded searches,
header include closure, Native selected-byte replay and the accepted readiness
contracts. Earlier receipts are unchanged. Only the four assigned files are new;
Root owns any later registration, ledger edits, normal build and admission.

## Credit boundary

This is a compiled ordinary retained fragment Source proposal. It adds no complete
Native function or whole-parent implementation. Native ABI/FH3/SEH/longjmp,
fault/CRT identity, actual production/shutdown binding, excluded inner fields,
runtime/startup/gameplay and Root admission remain unproved here. No shared
CMake/ledger/Ghidra mutation, additional Native window, normal build, test, link,
probe, fixture execution, or runtime credit is taken.
