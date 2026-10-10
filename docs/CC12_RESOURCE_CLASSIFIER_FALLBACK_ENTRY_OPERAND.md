# Fallback type entry: immediate operand and original transfer

The complete original ten-byte entry at `00CD82C0` loads the **address**
`0109021C` into ECX and jumps to `00B86A00`. It performs no fallback-cell
write. This identifies the next delegated code address; it does not establish
that target's initialization behavior or admit a reconstructed provider.

## Scope and identity

Root approved only `[00CD82C0,00CD82CA)` (10 bytes). The worker claimed the
inclusive lease `00CD82C0..00CD82C9` and the two packet outputs, on isolated
branch `agent/cc12_fallback_entry_operand`, baseline
`5a2027ce606ec678de0b6a31401bea87e0c3aac9`. No target bytes, target listing,
target decompile, data cells, tables, handlers or additional Native windows
were opened.

Configured and observed identity is project `bsp`, marker
`C:\Users\sqz269\bsp.gpr`, program `/battlestationspacific.exe`, language
`x86:LE:32:default`, image base `00400000`, loopback endpoint port 8089.
Named current-program/instance responses and complete typed response identity
envelopes are retained. The two typed-flow batches bracket the byte/listing
reads; every response and both batch validations accepted modification **5**.
No Ghidra mutation, analysis, repair, save, script or restart was performed.

## Original instructions

| Address | Original bytes | Decode | Bounded effect |
|---|---|---|---|
| `00CD82C0` | `B9 1C 02 09 01` | `MOV ECX,0x0109021C` | Places an immediate address in ECX; does not dereference it. |
| `00CD82C5` | `E9 36 E7 EA FF` | `JMP 0x00B86A00` | Direct relative jump; does not push a call return address. |

The signed displacement is `-1382602`: `00CD82CA + (-1382602) = 00B86A00`.
The ten bytes have SHA-256
`6284e94b5f4795635e0b946e9af5919309f9d54f0986121b0354577f890ed5e3`.
Original PE extraction, live Ghidra memory, disk Capstone decode and the
two-line Ghidra listing agree. The original file is
`I:\SteamLibrary\steamapps\common\Battlestations Pacific\battlestationspacific.exe`,
size 12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The window lies in `.text`, RVA/file offset `008D82C0`; the retained binaries
contain exactly ten bytes each. Whole-file hashing and PE header/section
mapping are provenance work, not additional instruction/data inspection.

These instructions only establish ECX address preparation and a tail transfer.
They perform no memory operand access, stack adjustment or flag change within
the approved window. The target's behavior, return convention, preserved
registers, initialization order and actual execution remain outside this packet.

## Listing ownership and flow override

Both queried addresses are exact five-byte instruction starts. The complete
single-range containing function is `00CD82C0..00CD82C9`, address count 10,
recorded provisionally as `BSP_UnknownResourceTypeTokens_StartupEntry`, with
`no_return=false`, `is_thunk=false`, `direct_thunk_target=null`.

At `00CD82C0`, `function_at_query=00CD82C0`; default/effective flow are
`FALL_THROUGH`, override `NONE`, fallthrough `00CD82C5`, and target arrays
are empty. At `00CD82C5`, `function_at_query=null` and its containing entry
is `00CD82C0`. Its original/default flow is `UNCONDITIONAL_JUMP`, but the
saved listing has **`flow_override=CALL_RETURN`**, making effective flow
`CALL_TERMINATOR`. Both target arrays contain `00B86A00`; both fallthrough
values are null and `fallthrough_overridden=false`.

The typed endpoint also includes `00B86A00` in `direct_call_targets` with
`default_target=true`, `effective_target=true`, and the exact-target function
metadata `FUN_00b86a00`, non-thunk and not no-return. That bundled metadata is
retained as returned; no separate target query was made. This collection must
not be mistaken for evidence of an original CALL opcode. The raw opcode and
default flow show a JMP. No override was removed or changed.

Four complete raw HTTP metadata responses, Base64 and SHA-256, all listing
nulls, body ranges, flow flags and identity/modification envelopes remain in
the ignored evidence directory. Flattened leaf-path indexes retain nulls,
empty arrays and each indexed array value. The tracked report includes the
complete parsed response payloads and pins the raw captures and indexes.

## Retained evidence and limits

The previous tracked `native_resource_item_frontier_bf.json` record for this
same ten-byte span has the identical live/PE SHA-256. Its historical
`no_ghidra_function=true` is not current: fresh typed metadata reports an exact
function at `00CD82C0`. Its referenced ignored decoded-text artifact is absent
from the main checkout; that missing artifact is not claimed as replayed.

The earlier metadata-only discovery report's DATA reference from `00CD82C0`
to `0109021C` is explained by the immediate operand. A DATA reference and the
provisional startup name do not make this entry a memory writer. No new xref
queries were made. Prior absence of recorded writes is not proof that no
runtime writer exists.

Ten whole context/config/tool files are pinned against baseline Git blobs,
with exact working-file hashes and retained baseline copies; only the
comparison normalizes CRLF. Complete CLI stdout/stderr/exit receipts and the
disk comparison are pinned in
`reports/cc12_resource_classifier_fallback_entry_operand.json`. The ignored
receipt directory is `local/cc12_resource_classifier_fallback_entry_operand/`
in the worker worktree. No reconstructed C++ source was needed for this
instruction-only finding.

## Follow-on gate

The packet-local next proposal is a separate lease and metadata gate for
exact target `00B86A00`, to establish its actual owned extent, instruction
start, flow flags and identity before approving any target bytes or body.
Any byte approval must then name its exact range and preserve original/live
comparison and target-specific uncertainty. Root reports a concurrent,
separately owned delegate audit; that audit supplies its own receipts and is
not counted as this packet's evidence or scope.

Fallback cell values/layout at `0109021C/01090220/01090224`, guard semantics,
actual writer/publication, provider admission, CRT registration/order, ABI
equivalence and consumer attachment remain held in this packet. Address
preparation and delegation alone close none of those gates. No copied IDs,
substitute storage or C++/CMake/ledger/config changes were made. There were
zero builds, tests, probes, game runs or runtime/gameplay validation.

## Primary review

Root retained 119 pin occurrences and verified ten baseline/current files, all four complete typed raw responses at modification 5, and the exact ten original bytes/two instructions. The original JMP and separate saved CALL_RETURN override remain distinct. No live query, Ghidra mutation, Source/build/test or runtime credit was added. The separately owned delegate readiness audit supplies its own larger scope.
