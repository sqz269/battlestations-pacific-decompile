# Actual render batch preparation and queue getter

Addresses: `004C11F0`, `00B51DF0`, `00B1BF70`.

This packet reconstructs three complete normal native bodies, totaling 491 bytes,
using existing actual-storage providers. The public functions accept explicit
C++ contexts; they are not drop-in implementations of the original calling ABI.
The existing three descriptive Ghidra names are retained hypotheses, not recovered
symbols. No worker Ghidra mutations, shared registrations, ledger edits or new
repository tests were made.

| Entry and inclusive end | Coverage | Original ABI | Source |
| --- | --- | --- | --- |
| `004C11F0..004C12AC` | complete, 189 bytes | cdecl, no arguments, EAX queue, RET | `get_native_render_command_queue_004c11f0` |
| `00B51DF0..00B51F0B` | complete, 284 bytes | ECX actual 18h batch, one writable DWORD stack argument, RET4, no semantic result | `prepare_native_render_batch_actual_00b51df0` |
| `00B1BF70..00B1BF81` | complete, 18 bytes | original ECX ignored, one stack batch pointer, tail dispatch to RET4 | `execute_native_render_preparation_job_00b1bf70` |

The implementation is in `src/native_render_batch_preparation_actual.cpp` and
`include/bsp/native_render_batch_preparation_actual.hpp`. The accompanying JSON
report has numeric `address`/`native` call-site rows, original prototypes, body
ranges, byte hashes, fixture controls and exact source/library pins. Existing
key-only and typed fragment records at B51DF0 remain separate evidence.

## Queue publication and cleanup

The getter captures current F8D440 for its fast return. On a miss it calls the
existing raw 415350 manager getter, captures manager+10 as the native critical
section, enters it and increments its actual DWORD+18. Only then does guard state0
start. It rechecks publication under that captured guard.

On another miss it allocates exactly 34h bytes and saves that allocation before
arming state1. The C++17 implementation begins the existing trivial queue type's
lifetime with placement `new` without parentheses or braces. This performs no
value initialization: the full B1F280 constructor must observe the preexisting
control DWORD at +20, including its special value2 branch.

After the complete constructor returns, state0 is restored before publishing its
return value. The getter calls the raw manager getter again and only afterward
reads current F8D440 for actual BD0C30 registration. The saved allocation is not
freed on registration failure. Normal unlock decrements the current depth through
the originally captured section, leaves that section, then returns fresh F8D440.

Native EH metadata at D8D524 uses the map at D8D514: state1 frees the saved
allocation through C64F08 and continues to state0; state0 runs the actual guard
destructor through C64F00. The source scopes preserve this ordering and terminate
if an MSVC C++ exception escapes cleanup. Hardware SEH and native frame identity
are outside the proof. Root already defined the raw C64F13 handler; the worker
verified its current ten-byte body and made no Ghidra edits.

The separate C64F08 action has a truncated stored Ghidra body ending at C64F10
after `_free`. Fresh live/disk bytes include `POP ECX` at C64F11 and `RET` at
C64F12. Root's locked repair cleared the call-return override and decoded that
tail, but the stored function body still ends at C64F10. Its record is
`reports/native_renderer_ay_cleanup_flow_repair.json`; tail ownership metadata
remains incomplete. The complete eleven-byte action is included in the fixture;
`_free` is returning. This limitation is separate from the three complete normal
bodies reconstructed here.

## Original index, live fields and current dispatch

B51DF0 captures the incoming index before its unconditional queue getter and
initializes a local enabled byte to zero. Full B1CB30 writes the enabled byte and
then a current configuration DWORD through the original argument-slot address.
The captured index continues to select the path even if that DWORD is different.
The new source interface borrows a writable volatile slot to retain this behavior.

Disabled configuration returns without touching entries. Enabled index0 uses the
existing full raw key fragment, including its ordered material/effect/texture
reads and x87 conversion. It subsequently reads entry base before current count
and chooses the unsigned-key comparator B51B00. Enabled nonzero index skips key
generation, reads count then base to form the end, reloads the first base and
chooses material/depth comparator B51AB0. Both paths compute the signed32 wrapped
byte difference shifted arithmetically by two and call the complete actual
pointer-slot introsort B1DCE0. No new bounds, finite-value or entry guards exist.

B1BF70 reads the batch argument, captures its profile, reads mode+08, then reads
the captured profile+0C and overwrites the argument slot with mode before tail
dispatch. Verified current profiles D5E5AC and D62064 both contain B51DF0 at+0C.
The source requires those current tables. `NativeRenderPreparationJobDispatch`
composes this path with the existing actual scheduler for D5E160's B1BF70 slot.
It checks the current owner and primary table on every execution. Other job
owners use a required remaining dispatcher; unknown preparation profiles fail.

Queue, batch and pointer-slot types are reused from the existing queue access,
queue constructor, batch lifetime and raw sorter modules. The context borrows
the queue constructor's sole F8D440 publication reference and the raw manager
cell. It introduces no alternate publication, semantic entry overlay or numeric
code-address call. Native storage lifetime, valid spans and common allocator,
string, manager, renderer and current-profile domains remain preconditions.

## Verification

Fresh live queries verified project `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe`. Sixteen live/installed byte spans matched, totaling
968 bytes, including all 491 owned body bytes, complete EH data/actions, current
tables and copied helper bodies. The original installation was unchanged.

Current owned source passed MSVC Win32 `/MD /EHsc /std:c++17 /W4 /WX /O2
/fp:strict`. After verifying all eight native seed ranges, `scripts/build.ps1`
passed `reconstructed_math` and `native_math_differential`, two of two. The shared
CMake registration remains root-owned, so that build does not yet include this
new source. The owned module was compiled explicitly for the external fixture.
The live report call verifier passed fourteen numeric rows with zero failures;
two imported OS calls and the current-table tail dispatch are explicit indirect
rows and are separately grounded by listings and verified table bytes.

`C:/Users/sqz269/bsp-ay-batch-preparation/probe_resumed.log` records eleven
original/source pairs and 775 compared DWORDs (3,100 normalized bytes):

- Five getter pairs cover fast publication, lazy/existing actual manager,
  actual registration failure throwing through copied native FH3, and returning
  validation that mutates publication before the getter's fresh return.
- Four preparation pairs cover enabled/disabled configurations and both original
  indices over forty actual entries, with the configuration output deliberately
  opposing the captured index. Key DWORDs and final pointer order agree.
- Two pairs run both batch profiles through actual existing scheduler enqueue
  and dispatch, checking scope, pending count and cleared job slots.

The getter's native copied body has only three documented operand relocations:
its EH immediate and two manager-call relative operands. The complete EH span has
one relocated guard tail jump. B51DF0 and B1BF70 remain byte-identical. Final
checks verify every expected copied span and only those relocation exceptions.
Both sides share full source manager, allocator/free, constructor, registration,
guard, sorter and scheduler providers. The comparison establishes parent
composition; independent original execution of every shared provider is not
claimed. Copied original config, comparators and CRT key helpers remain native.

Source, all included BSP headers, fixture inputs and three current own libraries
were hashed before and after the passing replay and remained unchanged. Full
pins are in `fixture_report.json`; `source_snapshot` and `library_snapshot`
preserve the inputs. The worker capture archive and manifest must be retained
before any root replay replaces logs or binaries.

The fixture links `/MANIFEST:EMBED`, fixes its own image at 00400000 and appends
the fixture-owned `.ayprep` section through 0109FFFF before process startup.
It verifies the region belongs to its own committed MEM_IMAGE allocation before
changing protection or copying native bytes. The generator verifies all existing
section bytes and unrelated headers remain unchanged. An inherited suspended
launcher is historical and is not used by the current run recipe.

After source registration, root replays with:

```powershell
& C:/Users/sqz269/bsp-ay-batch-preparation/run.ps1 -Root <integrated-root> -LibraryRoot <integrated-root>
```

The default compiles only external `probe.cpp` and links that root's current
`bsp_core.lib`, `bsp_lua511.lib` and `bsp_zlib121.lib`. `-WorkerSource` was used
only for this unregistered worker validation. Hash `.inc` and `.inl` source inputs
as well as `.cpp`, `.hpp`, `.h`, `.py`, `.ps1` and byte inputs during replay.

The fixture does not force allocation failure, constructor failure or an allocated
old control20 value2. It does not establish secondary cleanup exceptions, general
SEH, native argument/EH-spill aliasing, invalid storage, huge wrapped ranges,
arbitrary concurrency, complete queue execution/destruction or gameplay.

## Follow-up packets

Root integrates the three complete functions without removing existing fragment
records, preserves names/comments, annotates original signatures, refreshes
exports and validates the exact combined tree with current-library-only replay.
Full B1EBE0/B1D950 queue execution still requires complete actual B46A70 system
constants and B55550's material/resource/compiler chain. Named or typed fragments
do not prove those dependencies complete.

## AY integration analysis refresh

The integrator saved all sixteen AY original signatures and reviewed names,
verified full stored bodies and refreshed exports. CBBC8E, CBD436 and C64F13
are ten-byte analysis-only EH handlers defined under leases and the write lock.
Earlier missing-function observations are retained as worker capture history.
Two existing raw string bodies were extended separately; neither those
extensions nor the EH definitions add to the sixteen normal-body count.
Exact combined validation follows separately from worker fixture evidence.
