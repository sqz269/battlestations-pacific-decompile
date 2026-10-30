# MenuPrompt current-secondary scalar lifetime composition

Baseline is sealed `16991d87c0f5b77b0a0284b44a77b65bd67ca7b8`. This packet adds only `native_menu_prompt_lifetime.hpp/.cpp` and this document/[report](../reports/cc16_native_menu_prompt_scalar_lifetime.json). The existing constructor/getter, full destructor, singleton deletion map, application, global view, CMake and ledgers are unchanged. **Whole frontend Source readiness and activation remain false.**

The native profile word `CED3AC/+0` is `00532340`. Whole `00532340..00532347` is exactly `SUB ECX,0Ch; JMP00532A00` (8 bytes, 2 instructions). It consumes the actual `instance+0C` singleton subobject registered by the genuine `00425D10` getter. Its adjustment is DWORD address arithmetic, not a semantic object cast or lookup of a newer publication.

Whole scalar `00532A00..00532A1D` is 30 bytes/11 instructions:

1. Capture the actual allocation identity from ECX in ESI.
2. Call the complete existing raw `menu_prompt_destroy_005321f0` at `532A03`.
3. Only on normal return, test bit 0 of the **current byte** at the full DWORD flags home (`TEST byte ptr [ESP+8],1` at `532A08`).
4. When set, call `BF65AC` at `532A10` on the captured original allocation.
5. Return that original identity in EAX, even after free, then `RET4`.

The Source functions are `delete_native_menu_prompt_screen_00532a00(void*, volatile uint32_t&, Seg15Host&)` and `delete_native_menu_prompt_secondary_00532340(void*, volatile uint32_t&, Seg15Host&)`. Both use ordinary new C++ interfaces. The adjustor forwards the SAME full flags **reference** without loading, truncating, normalizing or rewriting its DWORD. The scalar reads a `volatile unsigned char` only after the existing full raw destructor returns. It never substitutes an eagerly captured flag value or bool. Destruction callbacks may change the home before that late read; free callbacks cannot cause a second flag read.

The scalar directly calls the existing `bsp::frontend_seg15_22_cc13::menu_prompt_destroy_005321f0` over the actual raw receiver and SAME genuine required `Seg15Host`. It then uses that host's required `free_00bf65ac` method when reached. There is no replacement member destructor, new host adapter, shadow owner, allocator fallback, retained reference count or successful default. The old raw member body is not re-recovered or counted.

The address is captured as integer identity before destruction. After possible free, the routine performs no receiver-field access, publication reload, profile read or new liveness check; the result is only the captured address word. A throwing member provider leaves prior effects and skips flag read/free/return. A throwing reached free propagates and skips the normal return. No `noexcept`, catch, termination, replay, retry, rollback or implicit extra cleanup is added.

The caller must supply a genuine retained raw allocation, a valid live DWORD flags home through the late byte read, and the actual existing host/member providers through the whole call. Reference transport does not freeze the home. Original private-stack alias identity, physical tail jump/RET4/thiscall ABI, FH3/SEH, hardware faults, runtime and gameplay remain unproved.

The future actual lifetime binding must resolve the captured subobject's CURRENT primary word at the native invocation epoch. For current `CED3AC`, its slot 0 selects `532340`, then this fixed adjustor/scalar chain reaches the existing full `5321F0`. A changed base/current profile must use its own real route; the wrapper must not force derived deletion merely because an older registration described a prompt. Root must bind this route into the existing actual singleton deletion service with the SAME application/manager/publication ownership and real Page/list/record/string providers. That shared binding is not installed here.

The complete existing destructor reaches actual Page removal, record/list/vector/string release, `E18E6C` clearing, callback-base restoration and raw screen unregister. Those reached required `Seg15Host` operations must all have genuine providers before activation. A supplied `Seg15Host` type alone is not proof that the real Page graph or application route is bound. Separate InitOnce current-`+10` registration still uses existing `5311C0`; this scalar does not perform it or complete broader raw95/Frame/press-start/manager gates.

Read-only analysis targeted configured `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; live count/proto queries with autostart disabled reported 77632 functions. Assembly established the whole scalar's actual late BYTE test and absence of wrapper-local EH. Installed PE SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. The report pins both whole native body hashes, profile DATA, transfer/call bytes, Source/provider hashes and the unchanged full member-body receipt.

Validation is static byte/target/hash/Source/whitespace inspection only. No build, compiler, test, probe, Native/game process, process control, Ghidra write or activation occurred. Root owns shared integration and build. Both native wrappers remain `compiler_generated_out_of_scope`: this required Source composition adds **zero Original, vendor, CG, Native-byte or duplicate-destructor credit**.
