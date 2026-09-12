# Raw input cursor, focus reset and action retargeting

Addresses: `00BECB20`, `00BECA40`, `00BECE70`, `00A92840`.

These additive source variants consume actual raw input allocations and borrow the existing canonical source platform/online state. They do not reinterpret those source owners as native layouts. Prior typed implementations and descriptive Ghidra names remain intact. The names are hypotheses, not recovered symbols. The source interfaces are not binary replacements.

| Native range (inclusive) | Original ABI | Source coverage |
|---|---|---|
| BECB20–BECCC7 | ECX platform; loading byte in one stack DWORD; RET4 at BECCC5, length3 | Complete normal cursor policy with required raw providers |
| BECA40–BECB14 | No input; ECX unused; RET0 at BECB14, length1; alternate RET at BECAEC | Complete raw focus sequence and concrete lazy action lookup/retarget |
| BECE70–BECE87 | ECX platform; no stack argument; RET0 at BECE87, length1 | Current source application virtual10, then raw cursor service with false |
| A92840–A92905 | ECX actual24h action owner; replacement pointer on stack; RET4 at A92903, length3 | Complete raw pointer-field walk; no external calls |

All four complete byte spans match the installed executable. No missing Ghidra starts or excluded-tail calls were found in these owned routines. The report records exact ends, lengths and hashes. Original FH3/SEH, access-fault behavior, concurrent mutation and malformed allocation ranges are outside the source contract. Application/frame integration is still external.

## Canonical storage and producers

`NativeInputActionOwnerStorage`/A93DA0 and A93C10 establish the actual24h owner and30h action array at +4, with count+8. A93940 establishes each action's binding header at+10/count+14; A93500 supplies34h binding allocation/initialization. A93750 writes binding class+4 and cached device+C. A93100 preserves the full scalar words and deep-copies headers+18/+24. A92EE0 copies all five words of each14h modifier, while A91E80 resolves class+0 to cache+8. Existing concrete raw storage providers are reused; there is no second vector or listener owner.

A92840 visits every action, every positive signed binding count, and both positive signed modifier counts. It changes only class1 cached pointers (+C in bindings, +8 in modifiers), without checking enabled/resolved flags or adjusting references. It reloads loop counts/base fields at their native boundaries. The original outer end is pointer arithmetic from base+count*30h, not a new bounds policy. Valid addressable native ranges are required; negative nested counts skip as the native signed comparisons do. Fifth modifier words, padding and non-class1 pointers remain intact.

The focus path reads the actual first class1 active-vector element from backend+94/+98, whose10h header starts at+90. These are the existing active vectors; the fixed3x8 attachment table is not substituted. The repeated native diagnostic checks are retained with the real returning-capable `_invalid_parameter_noinfo` boundary. If the first diagnostic returns, removal reloads its backend ECX while retaining the old vector address; the replacement lookup retains its old vector address without that receiver reload. The adapter imposes no default success for an absent active mouse: native callees retain their actual validity requirements.

## Services and order

`NativeInputCursorContext` borrows the sole F8BBF4 raw publication, real `NativeInputActionOwnerContext`, three existing cursor bytes, live D7A2F0 float, ShowCursor binding and required calls. `PlatformManagerFlags` and `Win32PlatformState` are the existing canonical source owners, not fabricated raw online/platform structures. Accessors must be ordinary current-publication reads. Services and all objects they may publish must remain alive through the call.

BECA40 performs removal A90EE0; reload; class1 deletion BEBF30; reload/profile capture/slot0C enumeration; reload/class1 slot0 activation A91620; reload and capture the replacement; concrete lazy004BEC00; concrete raw A92840. In both getter arms the replacement is already pushed before004BEC00 and survives any getter-side publication changes. The getter takes no native argument despite misleading pseudocode; A92840 consumes the earlier stack word with RET4.

BECB20 guards current online publication, then raw backend, then the external first-mouse getter. Loading mode pumps the current online owner before sampling platform focus and the current owner's UI flag. Those policy samples survive later callbacks. Each load-time backend update reloads F8BBF4, explicitly executes x87 FLD of live D7A2F0, captures the current raw profile, executes FSTP to the float argument, then dispatches captured slot04. No copied canonical backend fields are maintained.

UI entry resets focus, reloads the mouse through the external getter, invokes the existing raw A9A140 with flags6, and optionally updates. Focus loss can defer a reset until focus returns. Cursor show/hide captures the ShowCursor binding once before writing DB8E, then repeats the same captured function while its signed result is respectively negative/nonnegative. Previous-UI stores the captured flag after successful completion. Callback exceptions propagate with only already-completed writes/calls visible; no invented rollback runs.

BECE70 captures the current canonical source application identity, calls required virtual10, and then updates the same platform with loading=false. BECCD0 is the other direct BECB20 caller, passing1 at BECD2A inside verified body BECCD0–BECD36. BECE70 is published in the platform table at D68CE4; there are no direct CALL xrefs to that entry. All owned calls and the external loading caller are included in the report.

| Required boundary | Actual contract / integration |
|---|---|
| 004BA6D0 | Borrowed raw backend, signed class, unsigned index, RET8; owned by another orchestrator. No implementation or naming claim here. |
| A409F0 | Pump the supplied current canonical online owner; existing real XLive services remain required. |
| A90EE0 / BEBF30 / A91620 | Exact raw backend/device/class/slot arguments; other worker supplies concrete removal, scalar deletion and filtered activation. |
| Backend slot0C | Select from captured profile; actual enumeration provider belongs to the bindings packet. |
| Backend slot04 | Select from captured profile; the actual update body performs its own current-profile prepass. Startup/update packet supplies the finite dispatcher. |
| A9A140 | Reuse `set_native_mouse_cooperative_level_00a9a140`; no duplicate port. Native captures old COM table, obtains current window, reloads COM receiver and loads slot34 from the old table, calls stdcall(this,HWND,flags), then writes byte234 even if HRESULT fails. |
| Application virtual10 | Existing `PlatformApplicationServiceHost`; substantive application implementation remains required. |
| ShowCursor | Borrowed stdcall(int) function cell; actual application supplies Windows export. Fixture uses explicit counters without changing OS cursor state. |

All providers remain required. There is no fallback online state, fake listener, private lifetime domain, projected InputTickState or synthetic active-device set. The primary must compose these services and the external raw getter before claiming application attachment. Live settings/action producers and frame consumption remain separate work.

Consumer review found one shared-provider correction for primary integration: the existing `native_keyboard_mouse.cpp` cooperative helper loads the captured table's slot before evaluating the reloaded receiver. Native A9A15C reloads the receiver before A9A163 loads that slot. The required contract above retains the native order; this worker does not edit that separately owned implementation. Ordinary immutable COM tables are unaffected, and this packet's policy fixture does not execute that SDK body.

## Verification

Strict MSVC Win32 build (`/W4 /WX`, strict floating point) and both existing CTests passed. All eight existing seed routines match analysis/disk bytes. The CALL audit checks18 direct sites with0 failures; seven indirect sites are explicitly reported and manually checked against the listing, argument cleanup and captured receiver/profile/import provenance.

One ignored manifested probe links only this checkout's headers and built archives. It compares relocated, byte-verified A92840 against source over actual concrete storage:2 actions,4 bindings,16 modifiers,12 complete buffers; every byte matches. Its policy callbacks deliberately replace backend publications between stages. A real returning CRT handler reached through actual raw-manager registration changes the published action owner and active mouse during lazy004BEC00; the retargeted fields retain the earlier captured replacement. This controlled fault injection is fixture evidence, not an application provider.

The same bounded sequence verifies2 focus resets,4 external-getter observations,1 pump,1 load update,1 cooperative boundary,2 show calls,1 hide call,1 lazy-handler mutation and1 application service. It checks captured ShowCursor identity, deferred reset, load-step reload, UI capture, and stopping after an injected removal exception. Actual BD0400 plus concrete nonempty action teardown clears action/manager publications and frees their storage. The fixture's listener provider is required and unreachable because its records own no listeners.

Logs: ignored `local/native_input_cursor_final_build.log`, `local/native_input_cursor_final_probe.log`, and `local/native_input_cursor_call_audit.log`. The report records artifact hashes. No game was launched; no real ShowCursor, device poll/force, SDK enumeration/cooperative call, foreground activation or input injection ran in this fixture. Its known one-element getter result only observes the external contract and is not a production getter implementation.
