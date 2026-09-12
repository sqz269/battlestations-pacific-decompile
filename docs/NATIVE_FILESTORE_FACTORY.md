# Native FileStore factory singleton and lifetime

Addresses: `004FC150`, `00BE5320`, `00BE5340`, `00BE5350`, `00BE5380`, `00BE5790`.

`native_filestore_factory.hpp/.cpp` reconstructs six complete normal source
bodies over the actual 0Ch allocation and actual mutable publications
`01090AA0` and `0109DB68`. It composes the existing raw singleton getter,
registration, guard cleanup and genuine source CRT allocation services.
It does not create a second manager or a projected FileStore owner.
Descriptive names are provisional behavior hypotheses, not recovered symbols.

| Entry and inclusive end | Bytes | Original ABI | Coverage |
| --- | ---: | --- | --- |
| `004FC150..004FC216` | 199 | No input; EAX factory; RET | Complete source body |
| `00BE5320..00BE533D` | 30 | ECX raw0Ch; EAX original ECX; RET | Complete source body |
| `00BE5340..00BE5347` | 8 | ECX secondary; stacked flags; subtract4/JMP BE5790; inherited RET4 | Complete source body; raw undefined function |
| `00BE5350..00BE5371` | 34 | ECX primary; EAX scratch secondary; no semantic result; RET | Complete source body; raw undefined function |
| `00BE5380..00BE53A8` | 41 | ECX base allocation itself; stacked flags; EAX input; RET4 | Complete source body; raw undefined function |
| `00BE5790..00BE57C9` | 58 | ECX primary; stacked flags; EAX original primary; RET4 | Complete source body |

These new source interfaces add explicit bindings. They are not drop-in binary
ABI or original FH3 replacements. There is no game-validation claim. Original
hardware-fault cleanup, mutable EH stack aliases and original CRT exception
identity remain outside the source contract.

## Producer, tables and cache ownership

The complete constructor writes, in order, secondary+4=`D688AC`,
primary+0=`D688B4`, secondary+4=`D688B0`, and cache+8=0. Its first instruction
`MOV EAX,ECX` and final `RET` resolve the saved void pseudocode return.
The exact table bytes at `D688AC..D688BB` establish these slots:

| Slot | Target | Meaning |
| --- | --- | --- |
| `D688AC` | `BE5380` | Transient base scalar deletion; no secondary adjustment |
| `D688B0` | `BE5340` | Final lifetime-interface deletion; subtract4 before primary deletion |
| `D688B4` | `BE5790` | Final primary scalar deletion |
| `D688B8` | `BE8120` | Provider selection; sibling packet owns implementation |

`BE80B0`, inspected read-only, is the producer of a nonnull cache+8 through
`BE7FA0`. This agrees with `ARCHIVE_PROVIDER_ENTRY.md`. Those provider routines
and provider table `D689E8` are separate ownership. The factory destructors
contain no +8 access: they neither clear nor release the cached provider.
The source preserves this absence rather than introducing a release policy.
Provider shutdown coordination remains separate integration work.

The final primary deletion clears `0109DB68`, restores secondary `CE3818`
then primary `CFE9F4`, and optionally frees the entire allocation when low
flags bit0 is set. `BE5350` performs just those lifetime stores. `BE5340`
adjusts the registered secondary pointer to the primary. Transient-base
`BE5380` clears publication, writes only its own profile `CE3818`, and may
free its exact input pointer; it must not be dispatched on the final +4
subobject. None of the deletion bodies unregisters the pointer.

## Getter and register provenance

The fast path captures the first `0109DB68` read at `4FC165` and returns it
without reading the manager. The slow path gets the actual manager at
`4FC176`; `MOV ESI,[EAX+10h]` captures its real tracked critical section.
The complete listing has no later ESI write before its preserved-register
POP. Enter and the depth+18 increment precede arming guard state0.

The getter then rechecks the publication, allocates 0Ch, calls the raw
constructor with `ECX=EAX` and publishes the constructor result. It reloads
that publication and computes the optional secondary pointer. `PUSH EAX`
at `4FC1E6` occurs before the second manager getter at `4FC1E7`. That getter
consumes no input; `MOV ECX,EAX` then supplies the new/current manager to
registration, which consumes the already-pushed secondary argument with
RET4. The source captures the same pointer before its second lookup.
The slow return reloads publication after releasing the captured section.

## Direct calls and genuine service boundaries

The report retains every call-site `address`, `native` target and containing
function, including raw functions and external EH fragments. Target bodies
were inspected before assigning service meanings. Incoming xrefs are captured
for all six entries; the only constructor caller is the getter. The getter
has no consumed arguments at any caller; raw primary/secondary/base slots are
resolved from their producing constructor and table bytes.

| Containing function; call site | Target and contract | Cleanup/evidence |
| --- | --- | --- |
| `4FC150`; `4FC176`, `4FC1E7` | `415350`: same actual raw manager publication domain | No input; RET; second call leaves pushed secondary intact |
| `4FC150`; `4FC1AC` | `BF681B`: correct library `operator_new`, genuine source malloc/new-handler service | `PUSH 0Ch`; `ADD ESP,4` at `4FC1B1` |
| `4FC150`; `4FC1C3` | `BE5320`: complete raw constructor | `MOV ECX,EAX` at `4FC1C1`; RET |
| `4FC150`; `4FC1EE` | `BD0C30`: actual raw bounds validation and pointer registration | Captured object on stack; ECX second manager; RET4 |
| `BE5340`; `BE5343` | Tail `JMP BE5790` | `SUB ECX,4`; inherits RET4 |
| `BE5380`; `BE539B` | `BF65AC`: correct library `_free`, actual source `std::free` | `PUSH ESI`; `ADD ESP,4` at `BE53A0` |
| `BE5790`; `BE57BC` | `BF65AC`: free original primary | `PUSH ESI`; raw `ADD ESP,4` at `BE57C1` |
| `C686A0`; `C686A3` | EH tail `411EE0`: actual guard destructor | `LEA ECX,[EBP-14h]`; dependency only |
| `C686A8`; `C686AC` | EH `BF65AC`: free saved allocation | `[EBP-18h]`; `POP ECX` at `C686B1`, RET at `C686B2`; dependency only |
| `C686B3`; `C686B8` | EH tail `BF6B43`: library FH3 handler | EAX=`D917A4`; external, unimplemented handler |

Imports `CE2218` at `4FC18F` and `CE2210` at `4FC1FC` call genuine Win32
Enter/LeaveCriticalSection with the captured ESI section, stdcall RET4.
The source adjusts the actual DWORD at section+18 separately. Native
registration validates before testing null, appends without AddRef or an
internal lock, and can invoke a returning source CRT invalid handler.

## EH and saved-analysis boundaries

The raw `D91794` unwind map has state0 -> `C686A0` (guard), state1 ->
`C686A8` (saved allocation free) -> state0. State1 surrounds the leaf
constructor only. Registration runs at state0, so a registration failure
retains publication/allocation and cleans up the captured guard. The source
expresses this C++ cleanup with the already reconstructed `411EE0` service.
The leaf constructor makes no C++ throwing call; its possible hardware-fault
unwind and spill aliases are not implemented by source `/EHsc`.

No Ghidra mutations were made. All 370 owned body bytes, 64 data bytes and
29 external EH bytes were captured live and equal the installed PE. Raw
functions `BE5340[8]`, `BE5350[34]`, `BE5380[41]` need integrator definitions.
`BE5790` has a spurious no-return call-flow gap at `BE57C1..BE57C3`; full
live/PE decoding proves the missing cleanup and returned original ESI.
The raw external `C686B3[10]` handler is recorded without claiming ownership.
Its containing-function gap is separate from the owned definitions.

`verify_report_calls.py` checked 11 direct/tail rows and reported three
missing containing functions: `BE5343`, `BE539B`, `C686B8`. These rows remain
in the report with explicit raw-function metadata. The integrator must define
the relevant spans and rerun the check; this worker did not conceal the
known failures or label the mechanical check passed.

During final review the parent defined external `C686B3[10]` under its own
lease/write lock. The worker's second read-only check, `report_calls02.log`,
then checked the same eleven rows with two failures remaining, `BE5343`
and `BE539B`. The initial three-failure log is retained. The parent will
define the owned raw starts after worker completion and rerun integration.

## Frozen validation

MSVC Win32 Release build with `MSBUILDDISABLENODEREUSE=1` passed.
After `verify-seeds`, both existing CTests, `reconstructed_math` and
`native_math_differential`, passed. No permanent test suite was added.

The isolated `local/filestore_au/attempt01` fixture physically copied its
source, recursive headers, actual 49,910,008-byte `bsp_core.lib`, executable,
and all 12 linked library objects before first execution. Each object matched
its archive member. The fixture manifest covers 81 files; inputs and both
manifests are read-only. Postexecution hashes are unchanged. No failed fixture
attempt or seal was overwritten. Both probe executables embed manifests.

The six original bodies execute byte-for-byte at +`30000000`, with all
relative transfers checked and external bridges outside their ranges. They
use the original `0109DB68` publication, actual native profile table bytes,
real source allocation/free, the existing raw source manager getter and
registration, and real Win32 lock imports. The manager precondition is a
preexisting raw14h manager with a real256-pointer slot buffer and optional
real tracked section. Original manager construction and CRT internals are
therefore not validated by this composition fixture.

All 15 native/source states agree (406 checks): constructor layout, full
nondeleting lifetime stores, primary/adjusted-secondary/transient-base
deletion at flags0/1/2/101h, cache preservation, cold getter with and without
a section, single secondary registration, fast reuse without manager access,
and absence of unregister during deletion. A source-only test uses a genuine
CRT invalid-parameter handler that throws through registration, confirming
retained publication and released real captured section. Native FH3/SEH was
not executed. The stack/publication aliases and concurrent mutation schedule
are statically reconstructed, not differential runtime proofs.

Artifact hashes and paths are in `reports/native_filestore_factory.json`.
Preexecution seal SHA256:
`ae92765c852093bcc67eb756f4e756da7dd23b71e72f5a31267bd07075b30d94`.
The executable SHA256 is
`785b15ea4109acc19e13ac54a24fd8b492bb47a9b6ab2b838d743933aff7f54d`.
Build/fixture success does not establish canonical mixed-owner lifetime
dispatch for `D688B0`, provider shutdown closure, startup migration or game
execution. Those integrations remain explicit dependencies.

The reusable local driver is `local/filestore_au/run_fixture.py`; it accepts
`--repo <built-checkout> --attempt <new-absolute-directory>` and optional
`--build-log <current-build-log>`. Keep its `freeze.py`, `seal.py`, unchanged
`probe.cpp`, `launcher.cpp`, and `evidence01/` beside it. It verifies all eleven
native spans afresh through the fixed-target Ghidra wrapper and installed PE,
copies the specified checkout's actual library, freezes its exact linked
objects and source/header inputs, then runs and seals results in the new
directory. Its `driver_attempt02` validation passed the same 15 states and
406 checks with every preexecution hash unchanged. This enables independent
parent validation; worker object/library identity must not be carried over
to a promoted build from another worktree.
