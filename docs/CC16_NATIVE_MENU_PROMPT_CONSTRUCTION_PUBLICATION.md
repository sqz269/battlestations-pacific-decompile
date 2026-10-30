# Genuine MenuPrompt construction and publication

Baseline is sealed `830efe569dc1cba491456eec75ad01bc8a5fdb8e` (parent Source `eb63c7c17ae1e20f0ad728e2d410a360e1ebc4c1`). This bounded packet adds four Source files and this document/report. Primary integration owns CMake, ledgers, compilation, caller admission and the shared application. **Whole frontend Source readiness remains false; no activation is authorized by these bodies.**

The two recovered ordinary Source bodies are:

| Native function | Whole body | Native interface | Physical bytes / instructions | Native body SHA256 |
| --- | --- | --- | --- | --- |
| `00533120` MenuPrompt constructor | `00533120..0053378D` | ECX actual `290h` receiver, EAX same receiver, no stacked input, bare RET | 1646 / 513 | `425041cf82a7d89f00522ec28e2dc6e906868108085cfb514c04b33dfa0b3d27` |
| `00425D10` MenuPrompt getter | `00425D10..00425DD9` | no consumed input, EAX result, bare RET | 202 / 56 | `1610480d457f399a02972ec049f824ca92f6a8d7920ccfa2f120890e6bb4a442` |

The bad live `undefined(void)` constructor prototype was resolved from its assembly, rather than used as an interface. No Ghidra prototype, listing, annotation, project or export was changed. The configured project is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; read-only program-targeted count/proto/disassembly/byte queries were made with autostart disabled. The live count remained 77632.

## Concrete Source composition

`construct_native_menu_prompt_screen_00533120` takes the actual raw pointer, `NativeMenuPromptConstructionContext&` and a fresh, retained `NativeMenuPromptConstructionOperation&`. `get_native_menu_prompt_screen_00425d10` takes that same context and a separate fresh retained `NativeMenuPromptPublicationOperation&`. Their headers explicitly describe the new ordinary C++ ABI. Neither accepts a semantic `FrontEndPromptScreen`, a supplied screen ID, a default bank or a callable cast from Native vtable DATA.

The construction context borrows the genuine actual string-pool context (and thus its SAME manager publication), the actual `E18E6C` publication reference, exactly 95 actual `E18B60` DWORD cells, the existing actual `0109EEA4` feature-word reference and fourteen authenticated borrowed literal references. `E1930C` is a different command-owner publication. No global, manager, Game cell, bank, selector, literal owner, allocator domain or canonical-owner plan is added.

`borrow_native_menu_prompt_literals` runs **before** native entry over the existing `GameNativeReadOnlyData` supplier. That supplier verifies the complete supported PE SHA256 before mapping original `.rdata`. The factory checks each original VA, length, trailing NUL and pinned content fingerprint, then retains pointer metadata referencing that same supplier. It cannot be default/null constructed. There is no copied active literal bank. The report records every literal's original bytes/SHA256; its FNV check supplements the supplier's full-image authentication.

| Destination vector | Exact literal order |
| --- | --- |
| actual `screen+40` | `globals.yes`, `globals.no`, `globals.accept`, `globals.restartmission`, `globals.exittomenu`, `globals.exitgame`, `globals.back` |
| actual `screen+50` | `globals.dialog_yes`, `globals.dialog_no`, `globals.dialog_accept`, `globals.dialog_restartmission`, `globals.dialog_exittomenu`, `globals.dialog_exitgame`, `globals.dialog_back` |

The constructor calls the already admitted genuine raw `004F7180`, then preserves native prefix order: `+8=CEB0FC`, `+C=CED2D8`, `+10=CEB110`; then derived `+0=CED3C4`, `+8=CED3B0`, `+C=CED3AC`, `+10=CED38C`, byte `+3C=1`. The two actual `10h` vector members at `+40` are initialized by the exact `4322C0` effects (`+4/+8/+C=0`, opaque header word untouched). Seven `48h` records at `+60` call existing genuine `pool_pool17_cc13::menu_prompt_record_construct_00530f40`. The raw vector at `+268` has only its three pointer words zeroed. The actual list at `+278` gets a genuine `50h` allocated self-linked head and size zero. Its opaque first word is untouched.

Each literal iteration zeroes the actual eight-byte temporary header at its native epoch, calls genuine raw resize `41DD40`, captures the resulting data pointer, copies the CURRENT length plus NUL through genuine `BF7680`, arms its temporary unwind state, then calls genuine `450540` on the actual destination vector. Normal temporary return uses the **captured** buffer and CURRENT length+1 captured before the fresh `419CC0` lookup, then real `BD1510`. It does not substitute a destructor which would reread a different buffer. The normal literal counts are at most 30; the real feature-word reference also supports the existing memcpy interface's reached live-read contract.

Final writes are DWORD `+284=FFFFFFFF` and byte `+288=0`. No Page, widget, `+258..264`, whole-object zeroing or raw95 registration is added. Original numeric profiles are identity DATA and are never invoked as Source function pointers.

The two concrete constructor-iterator uses have fixed nonthrowing Source element bodies: empty checked-vector initialization and the admitted raw record constructor. The Source loops execute those exact element bodies over the actual two/seven members and retain completion metadata. They add no generic STL host or successful helper fallback. Original CRT iterator ABI, hardware-fault unwinding and private iterator frames remain qualified; no vendor-library reconstruction credit is claimed.

## Exception schedule and custody

Native constructor handler `C6C216` names FuncInfo `D95C44`, whose 21-state map starts at `D95C68`. The report pins its bytes and every cleanup parent/action. Source catches advance to that same parent before invoking the real action:

| State | Parent | Reached cleanup |
| --- | --- | --- |
| 0 | -1 | raw `4F71A0`: first store `CEAE54`, then clear every matching cell across all 95 words |
| 1 | 0 | real `4F8F10`: restore `CEB0FC` at receiver `+8`; this is a store, not a no-op |
| 2 | 1 | real `530CC0`: clear CURRENT `E18E6C`, then store `CE3818` at receiver `+C` |
| 3 | 2 | destroy two vectors at `+40` in reverse through genuine `4324A0` |
| 4 | 3 | destroy seven raw records at `+60` in reverse; strings `+38`, `+10`, `+0` retain the record's own two-state cleanup schedule |
| 5 | 4 | genuine `4324A0` on actual `+268` vector |
| 6 | 5 | exact `532190`/`531F80` list clear, current head free and `head=0` |
| 7..20 | 6 | genuine `41DD20` on the CURRENT temporary header |

Armed constructor states are 0, 2, 3, 5, 6 and 7..20. State 6 remains active during resize/copy and normal temporary return. The temporary destructor is armed only immediately before append, and disarmed back to 6 before normal return. Thus resize failure or normal-return getter failure must not gain a new temporary cleanup. Completed native stores and any resulting native leaks are retained; no stronger rollback is invented.

The list helper preserves its native head reloads and captures `next` before record cleanup. Fixed operation metadata records the actual node/next, record cleanup state, array suffix and vector receiver before reached callbacks, without allocating metadata or adding native reads/stores. No per-callback owner construction is deferred until after a potentially reentrant native effect. Failed operations are not replayable. The application retains their diagnostic frames and still-live providers; getter-freed allocation addresses are diagnostic only. A secondary cleanup failure remains explicitly qualified rather than triggering a fabricated recovery traversal.

Private helper composition is leased and documented for `4F71A0`, `4F8F10`, `530CC0`, `530E30`, `4322C0`, `532190`, `531F80` and raw `530F90`. Existing genuine `4324A0`, `530F40`, actual raw strings/pool and allocator/free are called directly. These private composition effects receive **zero additional helper-function/Native-byte credit**. Existing `5311C0`, `5321F0` and `532310` are not re-recovered or counted here.

The getter captures its first publication for the fast return. On the slow path it obtains the SAME genuine manager, captures its actual section at `+10`, prepares the eight-byte `CE37FC` guard, enters and increments the section's physical `+18` counter, then arms state 0. It rechecks the publication, allocates exact raw `290h`, saves that allocation before state 1/constructor, and frees only that saved allocation on a constructor exception after constructor cleanup. It publishes the constructor result after leaving state 1; it then captures CURRENT publication+`0C`/null **before** a second genuine manager lookup and real `BD0C30` registration.

Native getter FuncInfo `D84CC4` uses map `D84CB4`: state 1 frees the saved allocation through `C5E7D8`, then state 0 invokes full guard destruction through `C5E7D0 -> 411EE0`. Registration failure retains publication/allocation and releases the captured guard. The normal path decrements/leaves the captured original section, then returns CURRENT publication. No publication rollback, alternate manager, raw95 register call or InitOnce completion is added.

## Remaining exact provider and validation boundary

Before this getter may be activated, the SAME retained application must bind actual current `CED3AC/+0 = 00532340`, whole adjust-minus-`C` -> `00532A00`, and full existing `005321F0` over the real Page/list/record/string graph. Both `532340` and `532A00` are currently exports/compiler-generated metadata rather than genuine callable Source providers, and the current finite singleton deletion map has no `CED3AC` branch. A descriptor alone does not retire a registered `instance+0C`. This is the smallest remaining actual retirement provider; it is outside this packet's implementation scope. Separate InitOnce current-`+10` registration must then use existing raw `5311C0`, real raw95/current dispatch and authentic GUI/Page providers.

The complete frontend also retains the producer, Frame/Page/Text, main/HUD manager and press-start gates in the [producer closure](CC16_FRONTEND_RAW95_PRODUCER_CLOSURE.md). The canonical borrowed-view contract must expose the genuine `E18E6C` reference from the already authenticated page; mapping that page does not authorize another independent prompt cell. There is no new frontend owner or active adapter here.

Static validation authenticated the installed PE SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`, all 14 literal contents and all 70 per-label native direct call sites. Whole constructor assembly, getter assembly and exact EH maps/cleanup instructions were reviewed. Source/report hash, range/count, path and whitespace checks were performed. **No compilation, test, probe, Native/game process, process control, Ghidra write, Native ABI comparison, runtime comparison or activation was performed.** Root must compile/integrate before Source admission. Original FH3/SEH, hardware faults, secondary-unwind transport, CRT/provider identity, unrestricted private-frame aliases, concurrency and gameplay remain unproved.
