# Raw invalid-parameter provider boundary at 00BF6713

The complete saved body is `00BF6713..00BF6722`: 16 bytes / nine instructions.
It consumes no explicit caller stack argument or receiver field. It zeroes
EAX, pushes that zero five times, calls `00BF66EF`, then executes
`ADD ESP,14h; RET` if the child returns. Its outgoing provider edge and local
return stack are now established; the unopened child's register, handler,
failure, throw and return policy remain unproved.

Current Source provides a typed `NativeInputDeviceRuntime` member that calls
`_invalid_parameter_noinfo()`. The selected UCRT import is a free zero-argument
`void __cdecl(void)` API and can support a later explicit Source borrower
without constructing that runtime object. Neither the typed member nor the
import supplies an established Original ABI bridge or Native failure policy.
This packet adds no Source implementation or admission credit.

## Scope and Native evidence

Packet `cc12_mission_lua_variant_invalid_parameter_provider_ABI_readiness` uses
published baseline `e1a33b79838f2b0c6778851a0c97411cb4cd8ac8`. Only this document
and its matching report are changed; the sole owned Native entry is `00BF6713`.
The one direct child's function metadata was read. Its body, subordinate CRT
bodies, handlers, Native callers, data windows, IAT contents and neighboring
bytes were not opened. No Source, CMake, ledger, Ghidra, build, probe, test,
runtime, ABI or gameplay changes or credit are claimed.

Each `bsp.py ghidra` batch verified project `bsp`, program
`/battlestationspacific.exe`, `x86:LE:32:default` and image base `00400000`.
The configured existing `C:/Users/sqz269/bsp.gpr` exists. This is not a stronger
server-side absolute GPR path claim. Live and saved counts are both 64,729.

- Full live/PE bytes: `33 C0 50 50 50 50 50 E8 D0 FF FF FF 83 C4 14 C3`.
- Body SHA-256: `cd349dae43dbf63016fe6ea9339dab15c59041e3224e48cf8a53004224b45e77`.
- RVA/file offset: `007F6713`; independently decoded coverage: all 16 bytes.
- PE SHA-256: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
- Saved metadata: nine instructions, one block, zero edges, one child call,
  zero parameters and `undefined LIBCRT_unmatched_00bf6713(void)`.

Every raw instruction has a saved listing entry. There is no internal gap,
branch, loop, indirect transfer, hidden continuation or owned IAT read. All
nine instructions are locally reachable when the child returns normally.
With usable stack, the seven-instruction / 12-byte prefix reaches the CALL;
the final four bytes / two instructions depend on that provider's return.
A RET in the body proves a conditional continuation, not that a Native handler
will return in a particular execution. No no-return property was changed.

## Complete local schedule

Let S be entry ESP and T the direct child's entry ESP.

| Address | Instruction | Physical effect |
| --- | --- | --- |
| `00BF6713` | `XOR EAX,EAX` | EAX becomes zero; incoming EAX is discarded. |
| `00BF6715` | `PUSH EAX` | Zero DWORD at S-4, outgoing argument 5. |
| `00BF6716` | `PUSH EAX` | Zero DWORD at S-8, outgoing argument 4. |
| `00BF6717` | `PUSH EAX` | Zero DWORD at S-12, outgoing argument 3. |
| `00BF6718` | `PUSH EAX` | Zero DWORD at S-16, outgoing argument 2. |
| `00BF6719` | `PUSH EAX` | Zero DWORD at S-20, outgoing argument 1. |
| `00BF671A` | `CALL 00BF66EF` | Writes return `00BF671F` at S-24; T=S-24. |
| `00BF671F` | `ADD ESP,14h` | Adds 20 to the actual provider-return ESP. |
| `00BF6722` | plain `RET` | Consumes the current return word; no immediate cleanup. |

At child entry, `[T]` is `00BF671F`, `[T+4]..[T+20]` are five zero DWORDs,
and `[T+24]` is the wrapper's outer return word. EAX is zero. ECX, EDX, EBX,
ESI, EDI and EBP retain their incoming wrapper values at this edge. The wrapper
does not use those registers as explicit data inputs, but their forwarding
does not prove they are irrelevant to the unopened child.

XOR supplies CF=OF=SF=0, ZF=PF=1 and undefined AF to the child. PUSH and CALL
do not replace these arithmetic flags. No incoming arithmetic flag is used.
No wrapper instruction saves a register, reads a pointed object, or touches
x87/SSE state. Transitive child effects remain separate.

For a balanced normal return, the child resumes at `00BF671F` with ESP=S-20;
ADD restores S and RET leaves ESP=S+4. A callee-cleaning or otherwise altered
ESP is not checked or repaired. EAX returns the child's result/residue; it is
not forced back to zero and no semantic return type is recovered. All other
general registers likewise retain their child-return values. The wrapper
does not independently preserve nonvolatile registers across its child.

Returning CF/PF/AF/ZF/SF/OF come from the final **ADD ESP,14h**, not XOR or the
child's arithmetic flags. For actual provider-return ESP=P, its 32-bit result
is `(P+0x14) mod 2^32`; under balanced cleanup that result is S. RET leaves
these flags unchanged. Other flag state is not restored by owned code.

The wrapper requires 24 writable bytes below S, including the CALL return
word, and a usable current outer return word for ordinary return. Its six
stack writes remain observable through any aliases. Child mutation, faults,
handler reentrancy, unwinding, lifetime and stack corruption are not guarded
or rolled back. No allocation, local EH object or exception policy is added.

## Direct child and accepted caller composition

The sole physical child edge is a relative direct CALL to `00BF66EF`.
Metadata alone identifies `LIBCRT_unmatched_00bf66ef`, saved extent
`00BF66EF..00BF6712`, 36 bytes / 14 instructions, three blocks and three
recorded calls. Its saved prototype has zero parameters despite the five
outgoing words proved above. That saved prototype is not a delivery contract.
Its reported callee labels are `__invoke_watson`,
`BSP_CRT_DecodePointerCurrentState` and `BSP_Crt_ClearDebuggerHookState`.
These labels were retained as metadata; no subordinate behavior or exact
library identity was inferred from them. Existing names/comments are untouched.

The wrapper's shape and current Source routing are consistent with a
no-information invalid-parameter provider hypothesis. The five zero words
are not assigned expression/function/file/line/reserved meanings here, and
the Native wrapper or child is not renamed `_invalid_parameter` or
`_invalid_parameter_noinfo` on that basis.

Only the accepted 99-byte `006EDA20` and parent691 receipts supply caller
facts. No Native caller body was reopened. Let Q be `006EDA20` entry ESP:

- `006EDA28` calls this wrapper without arguments when `[B+0]` is zero.
  Wrapper S=Q-8, return=`006EDA2D`, ECX=ESI=B. Its zero stores occupy
  Q-12 through Q-28; the subordinate CALL enters at Q-32. The pair's saved
  ESI at Q-4 and original return at Q are outside these local stores. A usable
  balanced return and preserved ESI=B are still required before the pair
  reloads `[B+4]`, without rechecking `[B+0]`.
- `006EDA37` tail-jumps here after POP ESI, with wrapper S=Q and the original
  caller return at `[Q]`. The first PUSH overwrites the former saved-ESI word
  at Q-4 after it has been popped. There is no pair-local continuation.
  A compatible provider return reaches the pair's original caller directly.

Parent691 calls `006EDA20` at `006EE64E` with ECX pointing at actual adjacent
F/N stack words and no pushed arguments. It overwrites the returned EAX and
later reloads N. The accepted parent remains 637 saved bytes plus a 54-byte
continuation candidate, 691 bytes / 234 operations; 13 following INT3 bytes
remain separate. This audit closes the wrapper's local frame question while
leaving subordinate preservation and the pair/parent implementation open.

## Actual current Source and library boundary

`NativeInputEnumerationCalls` declares virtual
`void invalid_parameter_00bf6713()`. `NativeInputDeviceRuntime` overrides it;
the current definition at `src/native_input_device_runtime.cpp:97` is:

```cpp
void NativeInputDeviceRuntime::invalid_parameter_00bf6713() {
    _invalid_parameter_noinfo();
}
```

The body does not inspect receiver fields or translate a returned status.
The member API nevertheless requires an actual Source runtime object and
its polymorphic context. Its constructor borrows application services;
fabricating an object or casting the raw F/N pair to that runtime is unjustified.
Neither declaration nor body promises `noexcept` or `[[noreturn]]`. A typed
member with no explicit arguments is not an explicit free raw-entry adapter.

The existing Win32 project selects toolset v145 and SDK 10.0.26100.0.
That SDK's `ucrt/corecrt.h:371` declares:

```cpp
_ACRTIMP_ALT void __cdecl _invalid_parameter_noinfo(void);
```

The current application map selects both the runtime member from
`bsp_core:native_input_device_runtime.obj` and
`__imp___invalid_parameter_noinfo` from
`ucrt:api-ms-win-crt-runtime-l1-1-0.dll`. The current **Source executable's**
import metadata independently confirms `_invalid_parameter_noinfo` from that
DLL at Source IAT address `103CC620`, image base `10000000`. This is Source
library availability, not a Native data-window read. The compiled member's
specific operand and provider body were not reopened or executed.

The smallest later Source borrower can expose a free zero-argument cdecl
declaration, for example `void __cdecl invoke_native_invalid_parameter_00bf6713();`,
and explicitly use that existing Source CRT import. It needs no dummy ECX/EDX,
receiver object, captured context or stack arguments. It must borrow the
current provider/handler state and its lifetime, reentrancy and failure policy;
it supplies no default handler installation, new owner, throw type or return
guarantee. No `noexcept` or `noreturn` promise follows from this audit.

Such a Source provider substitution would not by itself reproduce the Native
five-zero child delivery, final ADD flags or `00BF66EF` identity. Exact Native
schedule emission would require a separately qualified subordinate edge.
No adapter is implemented, linked, build-reviewed or admitted here.

## Receipt versions and current evidence

The accepted99 report at `94fa080a0ad57fa94c418f805240dbb827b61f64` and parent691
report at `034e0fd4f9aaccd6b63977cad2885b723e9d6b30` are unchanged at this baseline,
as are their document pins. Their older whole input sets remain statements
about their recorded baselines. Those sets and older reports were not broadly
reread or promoted to current validation.

The three actual input-provider Source files are unchanged since accepted99
and are pinned separately: they are **not** among the latest build's 65 input
pins. The accepted99 historical runtime receipt and its returning-handler
fixture remain historical. Neither its aggregate Source-tree hash nor its
old whole-file snapshots is treated as current fixture evidence.

Both latest primary reports replay the same 65 current canonical input pins,
including qualified historical CRLF forms, and both current document pins.
All four recorded Core/executable/map/test-log artifacts match their hashes
and stayed stable during reads. Hashing did not rerun the build/checks or
reparse the compiled object graphs.

The inherited registered build ran `2026-10-09T14:48:26Z..14:48:44Z` and passed
three existing checks. Its context covers 11 complete objects and 13 positive
Core roots, with nine prior objects byte-identical and the prior 20 typed-owner
code/relocation and eight EH payload/relocation contracts unchanged. Source55
has exact 28+27 emitted bytes. The pending-group fragment is qualified Source
155 bytes / 32 operations for Native153 / 22, retaining its relocated twenty
stores. These are current admitted Source contexts, not evidence that this
16-byte provider or the 99/691 callers has been implemented or executed.

The `00BF66EF` provider, Native handler/return/throw/type/EH policy, an explicit
raw Source adapter, actual caller backing/lifetime and production storage
remain open. No outside Native body expansion is needed to finish this audit.
