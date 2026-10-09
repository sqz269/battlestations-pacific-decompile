# Application manager registration: bounded readiness

The Application's BD0C30 registration prerequisite already has a matching,
genuine **raw-storage Source entry**:
`register_native_singleton_object_00bd0c30(manager, unused_edx, object)`.
Use the existing canonical raw manager and its current Source services. No new
registration wrapper, typed-manager cast, default manager cell or parallel
lifetime domain is needed. This closes that operation's reuse question, not
actual Application construction, tables, retirement or startup wiring.

This evidence-only packet owns this document and its JSON report. It preserves
the earlier Application producer audit and synchronizes published main
4b8c9780ad27ad881ff99528c79d1168d712086e. It adds no Source or Original credit.

## Exact native entry

Guarded queries accepted project `bsp`, program `/battlestationspacific.exe`,
x86 little-endian 32-bit at image base 00400000; configuration identifies
`C:/Users/sqz269/bsp.gpr`. The current function body is BD0C30..BD0C56,
**39 bytes / 14 instructions**, so it is below the 300-byte inspection bound.
Ghidra's current prototype has no recovered parameters; the full assembly,
rather than that placeholder prototype, establishes the physical contract.

| Step | Exact behavior |
| --- | --- |
| C30..C36 | Preserve ESI, capture ECX manager in ESI, capture `[manager+8]`, compare unsigned `[manager+4] <= captured end`. |
| C39..C3B | Valid order skips BF6713; invalid order calls it and continues if it returns. |
| C40..C45 | Only now test the **current object argument word** at `[ESP+8]` for zero. Null skips append, but cannot bypass the earlier manager validation. |
| C47..C4E | Form the address of that actual stack word, push it, restore ECX from captured manager ESI, call BD0BC0. |
| C53..C56 | Restore ESI and `RET 4`. No semantic EAX result is established. |

ECX is the actual manager/container; EDX is unconsumed; one object DWORD is on
the stack. The manager must be live and have readable raw fields even for a null
object argument. Its raw vector prefix is begin+4, end+8, capacity-end+C, with
four-byte slots. Append and its existing growth providers require their genuine
current storage/allocation domain; a detached header or typed projection is not
an acceptable receiver. No owner-field or method-table read occurs in this body.

The body installs no lock or local exception frame, performs no deduplication or
AddRef, and writes no non-stack storage itself. Registration can retain the
object pointer through append; it does not certify that object's later scalar
dispatch or lifetime. Returning validation may affect current owner/argument
state. The source must preserve the actual argument word and post-validation
test; replacing it with an early null guard or copied temporary changes this
boundary. Original private CRT handler/exception behavior remains separate.

## Application caller ordering, reused evidence

The previous BEA810 capture is reused; no new parent decompile, listing or live
byte query was made. Its selected slices establish:

1. BEA83E calls the first manager getter; BEA843 captures that manager's raw
   section at +10. The surrounding caller manages this captured lock.
2. BEA866 publishes the actual Application receiver to E1AE90.
3. BEA86C calls the manager getter **again**. BEA871 then reloads **current
   E1AE90**, pushes that value at BEA877, sets ECX to this second getter's result,
   and calls BD0C30 at BEA87A.
4. The caller's BEA87F..BEA888 tail releases the original captured section when
   nonnull, including its recursion adjustment.

The first and second manager results must not be conflated. Likewise, replacing
the post-getter E1AE90 reload with the initially captured Application receiver is
not justified if a service changes publication. The registration helper does
not acquire, release, replace or validate the caller's captured critical section.
The already-published Application remains observable to current services and
returning handlers; an exception is not proof that publication or graph effects
can be rolled back.

## Current genuine Source composition

The raw entry in `native_singleton_vector_registration_wrappers.cpp` is a naked
MSVC Win32 fastcall implementation with an explicit unused EDX parameter. Its
14-instruction schedule matches the native body, with the two named call targets
bound to the current `_invalid_parameter_noinfo` adapter and the existing raw
`append_native_singleton_slot_00bd0bc0`. The SDK handler is real and may return;
no no-op, always-fatal or invented callback is supplied. UCRT handler selection
and Original encoded-global/Watson/EH identity remain different domains.

`get_native_singleton_manager_00415350` borrows the actual mutable manager
publication. On absence it allocates raw 14h storage and calls the existing
BD0960 Source constructor; that initializes +4/+8/+C, reserves real slots and
publishes an actual tracked section at +10. It preserves unobserved +0 rather
than inventing a manager table. This Source getter/constructor evidence is
reused as a provider contract; their native descendants are not re-audited here.

`GameSingletonHost::manager_publication_01090aa0()` refers to the same stable
process cell returned by `game_native_string_process().manager_01090aa0()`.
That process owner retains the cell and string bindings through later callbacks.
Existing B1B730 Source already demonstrates the exact relevant composition:
publish owner, get the current manager again, reload current owner publication,
then invoke the raw registration entry. It is a precedent for the service
contract, not an implementation of Application BEA810.

`ConcreteSingletonLifetimeManager::register_object` is a separate typed
implementation over `callbacks_` and `slots_`. It has similar high-level
validation/append behavior but is not raw 14h manager storage, and its callback
boundary differs. Do not reinterpret its C++ address as the raw receiver.

Current raw-manager shutdown consumes registered owners through actual deletion
bindings. The inspected dispatcher contains no cases for native Application
profiles D68BC4/D68BC8/CFEAB0 and rejects unrecovered profiles/bindings. The
registration operation itself does not fix that obligation. Application owner,
callable tables, publication lifetime and qualified removal/destruction remain
required before wiring the real caller. No new combined Game-frame factory is
made ready by this registration result.

## One supported next check

The next bounded **read-only** check is **00BEA990**, reached by the actual
Application direct destructor at 007379E7 in the retained producer capture.
That existing call is relevant to retirement of the registered Application.
No address-labelled Source entry was found in the current `.cpp`/`.hpp` search.
Start with its ledger and size; inspect only if it is within the same small-body
bound, and otherwise record its exact boundary. Its behavior, Source readiness
and descendants are not inferred here. Reimplementing the already available
39-byte registration body is not useful next work.

## Verification boundary

The current 39 native bytes match both the installed PE and the earlier
owner-escape audit. Three reused BEA810 slices total 48 bytes / 13 instructions
and still match the installed PE. No parent native query or descendant body
sweep was performed. The wrapper Source hash is unchanged from the earlier
Source snapshot; the existing primary report records complete 39-byte emission
with only the two call operands rebound. Its frozen manifest hash was checked.
That historical archive/build proof is not a new executable or runtime test.

The report pins current Source, bounded excerpts, canonical publication bindings,
the recorded registration proof and the prior caller evidence. This packet runs
no build, test, probe or native body and changes no Source, CMake, ledger or GPR.
Original CRT/FH3/SEH compatibility, actual Application integration and gameplay
remain outside its result.
