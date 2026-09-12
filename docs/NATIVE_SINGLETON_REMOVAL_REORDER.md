# Raw singleton unregister and reorder

This packet reconstructs complete BCFCA0..BCFD12 (115 bytes) and BD0D70..BD0F49 (474 bytes). Their raw owner has begin+4, end+8 and capacity-end+0Ch; each slot is four bytes. DWORD+0 and the section pointer+10h are untouched. Descriptive names are hypotheses, not recovered symbols.

| Entry | Source | Original/source ABI |
| --- | --- | --- |
| BCFCA0 | `unregister_native_singleton_object_00bcfca0` | ECX owner, EDX unconsumed; one stack object pointer; RET4, no semantic return value. |
| BD0D70 | `move_native_singleton_object_after_00bd0d70` | ECX owner, EDX unconsumed; stack object then anchor; RET8, no semantic return value. |

Both use naked MSVC Win32 fastcall declarations with an unused EDX parameter. Raw32 arithmetic, register captures, invocation arguments and current owner reloads retain the original instruction order. Neither function installs an exception frame, synchronization, or replacement-storage rollback.

## Unregister

BCFCA0 captures the object argument before testing it. A null object returns without accessing owner fields. Otherwise index starts at zero and the completed BCF910 provider reads current count. For each candidate, null begin or unsigned index beyond the raw SAR2 size invokes the actual invalid-parameter service. After a returning handler, current begin is reloaded before comparing the slot to the captured object.

A nonmatch increments index and calls BCF910 again, so the next loop limit is current count. A match receives another bounds validation, followed by another current-begin reload before writing zero at that index. A returning handler can therefore move the storage between comparison and clearing. Only the first match is cleared: end/capacity stay unchanged and later duplicates remain. This entry does not call the object's destructor.

## Reorder

BD0D70 initially captures begin, validates it against current end, then captures the requested object argument. Each search iteration captures current end, validates current bounds and iterator ownership, compares the retained iterator with that captured end, then validates and dereferences the iterator. Its explicit null-owner checks occur after raw owner reads; they are not a safe null-manager entry gate. Returning handlers retain the native captures and subsequent reloads. Even redundant ownership comparisons and the six-byte alignment LEA remain in the source instruction schedule.

At the first match, or the native end-of-search continuation, it validates again and copies the current slot value into the invocation's object argument word. It computes `(current_end - (iterator+4)) SAR2`; only a signed-positive distance calls actual SDK `memmove_s`, with equal destination size and count. It then decrements the current owner end by four and validates the resulting bounds. This is a physical erase, unlike unregister's zero slot.

The second search starts from newly captured current begin and uses the then-current anchor argument. It searches the remaining entries, so duplicate and object-equals-anchor behavior follows that order. Both searches require valid matches for ordinary successful use. Missing matches reach the original validation/dereference or out-of-range insertion continuation; the reconstruction does not add a no-op return, fallback anchor, or rollback. If a source service throws after erasure, the erased state remains.

After the anchor search, current end is saved into the original anchor argument word. Anchor iterator+4 is retained and checked against current bounds. Equality with the saved end selects an inline append path: current unsigned size/capacity decides whether to write the current saved-object argument at current end or call the completed BD08D0 insertion provider. The interior path repeats its native bounds validation before calling BD08D0. Both calls pass the address of the actual saved-object argument and an actual eight-byte stack iterator result buffer. The intermediate stack iterator stores, captured anchor position, argument reloads and all three RET8 paths remain observable in their original order.

## Concrete providers

The two bodies have28 direct CALL sites: two to completed raw BCF910,23 to the fixed invalid-parameter adapter, one to actual SDK `memmove_s`, and two to completed raw BD08D0. BD08D0 reaches the complete general BD0700 insertion and its real allocation, owning length-error, copy/fill and free providers. All are actual members of the successful main archive.

The six-byte adapter tails actual `_invalid_parameter_noinfo`; returning handlers remain possible. The current UCRT chooses a thread-local handler before its global handler, unlike original encoded global109DD64. Current CRT handler, errno, heap and exception ownership are explicit source-service boundaries. The SDK `memmove_s` body is94 bytes and matches the completed leaf provider's full code and relocations. Original service-internal volatile registers/flags, hardware-fault locations and FH3/SEH identity are not claimed.

## Evidence and integration limit

Fresh guarded Ghidra queries verified existing project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. All589 native bytes/225 instructions match the installed executable SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Emitted bodies preserve every byte except28 named four-byte CALL operands. All four emitted CODE sections total689 bytes, including the two actual CRT providers. There is no owned global storage or local exception metadata.

The strict Win32 main build passed both existing CTests and all eight reference seeds. Primary captured31 unchanged inputs before the build and froze all eight exact archive members and their actual compiler dependencies. The seven existing provider objects are entirely byte-identical to the previously accepted main build, including their code, relocations and exception metadata. Every external symbol used by the new object resolves in the actual archived providers or observed CRT import libraries. No new test or runtime probe was added. The [audit](../reports/native_singleton_removal_reorder_audit.json) links the immutable `local/singleton_removal_primary/` evidence, saved annotations and refreshed exports.

These are raw source entries in `bsp_core.lib`; the existing game host's execution of them is not established. Raw manager destruction remains a separate integration dependency: BD0400 calls each current profile's slot0 with ECX object and flag1, whereas some completed source destructors require additional owned bindings in EDX and retain original profile identities as data. A complete mapping must cover every owner admitted to the canonical manager. These two vector operations do not close that dispatch, original runtime ABI, or gameplay validation.
