# Canonical GUI Text process ownership 37

`GameNativeGuiTextProcess` supplies one process-backed Text pool at the F8BDF0
role and the real Element/Text guard and descriptor cells. It reuses the existing
`NativeGuiTextPool`, canonical E188B4 allocator list, `GuiTypeCounterHost`, generic
GUI type initializer and `LightTypeBootstrap`. It constructs no widget, model,
scene, string pool, material, resource registry or renderer, and activates no
native Text drawing. Source baseline is Main
`e3f88d58e7fde8c8143ddf1c23d65c5f925777af`.

Implementation: `include/bsp/game_native_gui_text_process.hpp` and
`src/game_native_gui_text_process.cpp`. Receipt:
`reports/cc12_native_gui_text_process_37.json`.

## Whole Original bodies and order

Before each analysis/annotation/export batch the repository wrapper reported
the live `/battlestationspacific.exe`, configured in
`C:/Users/sqz269/bsp.gpr`. Full pseudocode and assembly were reviewed for all
three bodies and the AA8200 prerequisite. No Original instruction was changed.

| Entry and extent | Original behavior preserved by the Source composition |
| --- | --- |
| CD71F0..CD7205, 22 bytes | MOV ECX,F8BDF0; CALL AB77C0 at CD71F5; PUSH CE0A80; CALL BF6FF5 `atexit` at CD71FF; POP ECX; RET. The Source API returns/caches the real CRT registration result. Nonzero does not undo the initialized pool. |
| CE0A80..CE0A89, 10 bytes | MOV ECX,F8BDF0 then JMP AB7420 at CE0A85. It is the registered pool destructor, not a type initializer or Text-payload destructor. |
| CD7210..CD725E, 79 bytes | Test F8BDEC; set guard first, store F8BE34=D5C5DC (`cGuiText`), invoke AA8200 with ECX=F8BC88, copy current F8BC88/F8BC8C into F8BE2C/F8BE30, obtain 6FAC20 counter, capture counter+4. **CD7255 increments counter+4 before CD7258 publishes F8BE28.** Overlapping-global pseudocode incorrectly reverses these final stores. |
| AA8200..AA823D, existing prerequisite | Test/set F8BC80, target name D5C124, initialize canonical root0109DB84 through BEA780, copy its current own ID to target+4, obtain the same counter; AA8237 increments before AA823A publishes target own ID. This was already generically reconstructed; this packet supplies canonical backing/adaptation and does not claim another recovered body. |

Live table xrefs and bytes prove the adjacent CRT entries
`CE3470 -> CD71F0`, then `CE3474 -> CD7210`. Original guard bytes F8BDEC/F8BC80
read zero before execution. Source stores have loader-zero preimages, but no
accessor treats an uninitialized zero array as live lineage.
The current first main subset initializer CD7830 is referenced at CE34EC, and
material CD78D0 at CE34F8. Inserting the Text pool call before the current
particle-pool CD7830 call preserves this demonstrated relative table order.

The existing `gui_widgets_cc13::gui_type_initialize` helper is used for both
Element and Text. The Element adapter stages the one current root ID only after
the existing root bootstrap returns. No second root descriptor is created.
An actual `GuiTypeCounterHost` adapter forwards each counter request to the same
supplied `TypeIdCounterLifetime::get_006fac20`. A temporary
`LightTypeBootstrap` borrows the same counter and the caller's canonical common
type storage, avoiding a separately supplied bootstrap with an uninspectable
private counter. Host/native fields are not conflated.

## Startup API and required host wiring

`src/game_native_gui_text_process.cpp` is registered on `bsp_game` in shared
`cmake/startup.cmake`. This worker queued, claimed its authorized turn, added the
one source entry, then immediately released that file. The initial cold build
uses an ignored include overlay; the receipt distinguishes its subsequent
canonical-registration verification.

1. Obtain `auto& gui = game_native_gui_text_process();`. Construction binds the
   existing pool's trim dispatch on the canonical allocator list; it runs no
   native initializer or type initialization.
2. Call `gui.initialize_pool_once_00cd71f0()` explicitly in the native CRT
   startup sequence, before any Text allocation and before the Text type call.
   Preserve/log its real integer registration status. The Original table proves
   CD71F0 immediately precedes CD7210. Do not replace native pool teardown with
   a host application destructor.
3. Once the **existing** VFS canonical type-counter/root cells are available,
   call `gui.initialize_types_once_00cd7210(vfs.native_owners().types(),
   vfs.native_types().light_types())`. Pool startup must already have returned.
   This runs the established root bootstrap only as AA8200 requires. Call it
   before exposing lineage to any widget/Text lifetime. The current Source VFS
   already initializes other type families; wiring at that point supplies
   internally consistent IDs but does **not** prove full Original CRT-table
   order or absolute Original numeric type IDs. Recovering/reordering the whole
   application's type schedule is separate work; never insert fixed IDs here.
4. Borrow `gui.pool_00f8bdf0()`, `gui.text_lineage_00f8be28()` and, for the base
   widget lifetime, `gui.element_lineage_00f8bc88()`. Return references target the
   one persistent backing objects. No copies/reset/rebinding are provided.

Both initializations have explicit one-attempt **Source** admission metadata;
this is not a recovered Original once guard for the pool. A pool attempt is
latched before AB77C0, so an entered exception cannot replay it. A nonzero
`atexit` result still permits borrowing the initialized pool, matching native
retention without inventing a cleanup registration.

Type admission records the counter/root/guard identities, rejects reentry or
replay after an entered failure, and permits repeated successful calls only for
that same domain. Native guards, names and earlier descriptor/counter stores
remain sticky on failure. No rollback or zero-lineage fallback is performed.
Lineage getters require complete type startup, including both parent and own
counter consumption. External serialization is required across type calls,
shared-list mutations, Text operations and shutdown; no lock spans reentrant
root/counter callbacks. Keep the supplied canonical counter/root providers
through their callbacks and shared drain.
After the type call, the process retains counter/root/guard pointers only for
identity comparison on explicit repeat initialization. Lineage getters read
process-owned fields. The CE0A80 callback, pool APIs and process destructor do
not dereference those VFS/type-provider pointers after shared drain.

## Retirement

The GUI process and common allocator process are constructed before native
`atexit` registration. Reverse registration order therefore keeps pool storage,
trim dispatch and E188B4 list alive while CE0A80 executes. The callback runs the
existing AB7420 on the same pool; Source retiring/retired metadata blocks later
pool borrowing. A second callback or failed teardown cannot be replayed.
The C++ process destructor only destroys its companion/mutex; it does not run a
second AB7420.

`native_operation_interrupted() const noexcept` reads only Source metadata and
returns true for pool `threw`/`retiring` or type `threw`. It takes no lock, calls
no provider and reads no native bytes. The parent must inspect it before
unwinding a failed startup into manager/CRT cleanup and preserve an interrupted
graph with its existing process-exit policy. Prepared, returned and normally
retired states return false. Calls require the same external serialization as
startup/retirement. No new recovery or retry is authorized by this predicate.

All Text payloads, retained factory allocations, queued uses and failed native
operations must be settled before CRT teardown. AB7420 frees slabs/table,
destroys its critical section and unlinks the allocator element; it destroys no
Text payload and cannot authorize disposal of pending Text lifetimes. Retire
Texts, fonts and shared renderer resources using their existing owner order
before this callback. Nonzero `atexit` registration leaves the native pool live
without a callback; there is no replacement cleanup or registration retry.

## Validation and boundary

The strict focused MSVC Win32 `/O2 /W4 /WX` compile passes. Generated assembly
was inspected for the whole successful initializer continuation: sticky
guard/name stores precede parent callbacks, parent words are read after the
callback, and both Element/Text counter stores precede own-ID publication.
The helper therefore preserves the critical AA8237/AA823A and CD7255/CD7258
orders in the compiled Source.
The actual production object was also disassembled: its Element counter/own-ID
stores occur at method offsets `+14D/+150`; Text stores at `+19A/+19D`.
The interruption predicate contains only process-field comparisons and returns.

One ignored focused Source owner probe executes a real Text pool slot
allocation/return, then initializes GUI types after the existing node family
has consumed IDs. It confirms shared-counter lineage relationships and stable,
idempotent borrowing without asserting hardcoded IDs. A separately registered
late observer runs after CE0A80 and confirms that the retired pool accessor
rejects, before the process/list destructors. The existing probe also checks the
metadata predicate is false in normal prepared/ready/retired states. This is Source owner execution,
not an Original-vs-Source differential or a native Text/rendering test.

The canonical `./scripts/build.ps1` Release Win32 build passes, including the
new source through tracked CMake registration with the local overlay removed.
Both existing enabled CTests pass (`reconstructed_math`, `tool_tests`); this
worker build does not enable native differential tests. The initial cold attempt
built the core but encountered parallel CMake regeneration/FetchContent failures
after the source registry changed during compilation. Explicit canonical
configuration and the unchanged-input rerun passed with no Source compile
failure. Logs and artifact hashes are recorded in the receipt.

Ghidra names/evidence were
applied through the canonical write lock, old values/comments saved in
`local/ghidra-annotations-20261016T000712Z.json`, and all three affected exports
refreshed. Existing AA8200 annotations were unchanged. Ledger rows explicitly
stage the new process/source boundary.

Main/startup calls are left to the parent integrator. Mutable GUI scalars,
primary raw-model/group routing, actual Screen
services and Native Text drawing remain separate dependencies from readiness36.
These are new MSVC Win32 C++ interfaces and process backing; original fixed
global/register/FH3/CRT ABI, native Text drawing, visual parity and gameplay are
not established.
