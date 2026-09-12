# Raw singleton vector insertion and registration wrappers

These three complete entries operate on actual raw storage and call the completed general insertion provider. The owner fields are untouched DWORD+0, begin+4, end+8 and capacity-end+0Ch. Each slot is four bytes. Descriptive names are reconstruction hypotheses, not recovered symbols.

| Native body | Source entry | Original/source call shape |
| --- | --- | --- |
| BD08D0..BD0958,137 bytes | `insert_one_native_singleton_slots_checked_00bd08d0` | ECX owner, EDX unconsumed; stack result buffer, iterator owner, position, value-slot pointer; EAX result buffer, RET10h. |
| BD0BC0..BD0C28,105 bytes | `append_native_singleton_slot_00bd0bc0` | ECX owner, EDX unconsumed; stack value-slot pointer; RET4, no semantic EAX result. |
| BD0C30..BD0C56,39 bytes | `register_native_singleton_object_00bd0c30` | ECX owner, EDX unconsumed; stack object pointer; RET4, no semantic EAX result. |

All entries use naked MSVC Win32 fastcall declarations with an unused EDX parameter. Raw argument words, captured registers, modulo32 arithmetic, signed SAR2 distances and unsigned comparisons retain the native instruction schedule. An iterator result is eight-byte storage: owner at+0 and position at+4. Its position is written first, then its owner. Source CRT service effects and exception identities remain explicit boundaries; this is not an original-runtime binary compatibility claim.

## Checked one-element insertion

BD08D0 captures the iterator owner and the original container's begin. Null begin or zero signed-shift size selects index0 and skips iterator-owner validation. Otherwise it validates captured begin against captured end, then validates the iterator owner for nonnull/equality with the original container. Returning handlers continue with the captured values and original container; the foreign iterator owner is not substituted as the insertion destination. Index uses the current position argument minus captured begin, SAR2.

The function reloads position and value-slot arguments and calls completed BD0700 for one copy, retaining the original iterator-owner argument. After insertion it captures current begin and validates it against current end. It stores captured begin into the original position argument word, computes captured begin plus index*4, then validates the result against current end and current begin. A returning handler may mutate owner or argument words, but captured result position is retained. The current result-buffer argument is loaded only after validation, and result+4 is written before result+0. Aliases to result and invocation storage therefore follow the original order.

## Append and registration

BD0BC0 captures begin and computes size and capacity with raw subtraction/SAR2. A nonnull begin with unsigned size below capacity takes the direct path: capture current end, load the value-slot pointer argument and its DWORD value, write the value at captured end, then publish captured end+4. This schedule retains value-slot aliases to the owner or its storage.

The growth path captures end and validates captured begin against that end. After a returning validation it reloads the value-slot pointer and calls BD08D0 using the original owner, captured end, and an actual eight-byte stack result buffer. It does not recalculate captured end or use a typed container.

BD0C30 validates current begin/end before testing its current object argument for null. For a nonnull argument it passes the address of that actual stack word to BD0BC0. Null registration can therefore still invoke the validation handler. No lock, duplicate check, retain operation or cleanup frame exists in these bodies. Successful registration appends exactly one pointer value; it does not reconstruct the eventual registered-owner destruction dispatch.

## Providers and verification

All six original BF6713 validation edges bind to a fixed noinline cdecl adapter that tails actual `_invalid_parameter_noinfo`. The current SDK/UCRT owns the thread-local-then-global handler selection and fatal path. Original global encoded109DD64, pointer decoding, Watson internals, errno and service register/exception identity are not reconstructed by this adapter. Returning handlers remain possible; no noop or generic callback is substituted. BD0700 and its full copy/fill/allocation/length-throw/free providers are actual compiled implementations in the same archive.

Fresh guarded Ghidra queries verified the existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and all281 original wrapper bytes against the installed PE. Emission matches those bytes except nine named CALL operands. The additional adapter is exactly6 bytes with one actual invalid-parameter import relocation. All four owned CODE sections total287 bytes; no owner globals or exception/unwind metadata were emitted.

The combined insertion batch passed the strict Win32 main build, both existing CTests and all eight reference seeds. Primary froze29 unchanged prebuild inputs,7 exact archive members,7 compiler commands and195 read dependencies. All133 existing provider CODE sections and their data/EH metadata match the previously accepted main objects; all9 unique external symbols used by the two new objects resolve in actual archived providers/import libraries. The [audit](../reports/native_singleton_vector_registration_wrappers_audit.json) links the immutable `local/vector_insertion_primary/` evidence, saved Ghidra annotations and refreshed exports.

The worker supplied a reviewed source draft; primary completed its build and static proof. No worker seal, new runtime fixture, original CRT/FH3/SEH compatibility or game execution is claimed. These routines remain static-library entries pending full raw manager and caller integration.
