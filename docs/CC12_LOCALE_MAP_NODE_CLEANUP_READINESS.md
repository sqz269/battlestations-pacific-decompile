# CC12 locale map node cleanup readiness

The node's directly selected exception map has one cleanup state. Its action loads `ECX` from `[EBP-10h]` and jumps to `0041DD20`, the existing narrow-string destructor target. The saved node body places the original receiver at `entry_ESP-10h`. These observations support narrow-header cleanup from the captured original node **if the exception runtime supplies `EBP = entry_ESP`**. That runtime rebasing remains external; it was not proved by the action label or by reading the provider/runtime bodies.

## Exact fresh scope

| Retained span | Bytes | Result |
| --- | ---: | --- |
| `00C86268..00C86271` | 10 | Selector loads `EAX=00DB583C`, then jumps to `00BF6B43`. |
| `00DB583C..00DB585F` | 36 | FuncInfo DWORDs: `19930522, 1, 00DB5834, 0, 0, 0, 0, 0, 1`. |
| `00DB5834..00DB583B` | 8 | State 0: previous state -1, action `00C86260`. |
| `00C86260..00C86267` | 8 | `MOV ECX,[EBP-10h]`; `JMP 0041DD20`. |

All 62 fresh bytes match the installed PE. Each referenced metadata/action extent received a fresh lease before inspection. Every live batch verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`; the captures recorded modification 59. No target provider, CRT exception runtime, neighboring action, or additional data family was expanded.

The selector has two defined instructions but no stored function body. The action is a complete eight-byte, two-instruction function. Both entry MOV instructions have flow override NONE. At `00C8626D` and `00C86263`, the raw/default operation is an unconditional JMP while the current Ghidra `CALL_RETURN` override exposes effective `CALL_TERMINATOR` flow. Targets are unchanged and there is no fallthrough. Both representations are retained; no function creation, listing repair, annotation, or flow change was performed.

## Saved receiver and runtime frame boundary

Exactly 119 bytes of the prior node-body capture are reused as a hash-pinned dependency, separately from the 62 fresh bytes. They were not re-read from Ghidra in this packet. Let `S` be `ESP` at `00737CE0` entry, pointing at the caller's return address. The retained prologue pushes the initial state, handler and old exception link, placing the registration link at `S-0Ch`; it then pushes `ECX` and `ESI`. At `00737CF9`, `ESP=S-14h` and the original receiver is written to `[ESP+4]=[S-10h]`.

The action's literal load address is `[EBP-10h]`. It selects that saved receiver if `EBP=S`, equivalently if the runtime makes `EBP=registration_link+0Ch`. Neither the node body nor the two-instruction action establishes that rebasing. The action's own code adds no member offset: after the frame premise is supplied, the target receives the node's narrow header at offset zero, not its wide header at `+08h` and not an independently cached payload pointer.

The exact map establishes state 0's predecessor and target, not a full Native EH/SEH ABI implementation. The header flags are retained numerically without inventing runtime guarantees. Runtime rebasing, dispatch machinery, exception-object lifetime, and failure during cleanup remain outside this evidence.

## Expected cleanup schedule and field mutations

The prior node body arms state 0 after loading the wide pointer and before its wide getter/return stage. It reloads the narrow pointer after that stage, then writes state -1 before calling the normal narrow getter/return. Together with the selected map/action and the explicit frame premise, the expected Source contract is:

- While state 0 is active, an escaping wide-stage failure triggers the existing raw narrow-header helper on the **captured original node**. Its current narrow pointer and, if non-null, current length determine the cleanup return; these fields need not equal their values before the wide getter's side effects.
- After state -1 is written, a normal narrow-stage failure does not have this state-0 cleanup armed. Do not add a second narrow return or retry.
- The admitted action performs no wide return, node-slot recycle, map-head/count clear, publication repair, or receiver free. Those effects must not be invented as cleanup behavior.

The raw helper's pointer and `length+1` are captured before its own `00419CC0` getter, then returned through `00BD1510` using that getter's owner. A null narrow pointer skips both the length read and providers. The action-to-helper correspondence is an existing provider dependency: this packet does not reopen `0041DD20` Native code. There is no basis here for clearing dangling header values or treating a second cleanup exception as a normal recoverable result.

The map caller still holds its pre-node-call saved next link. It reaches the node's post-callback block index/table reads and inline recycling only after the node owner returns normally. An escaping failure can leave earlier nodes recycled and headers/count uncleared, as the accepted raw-map report records. This packet adds no transaction, retry, locking, or rollback guarantee.

## Existing Source context and smallest implementation candidate

The raw overload `destroy_native_string_header_0041dd20(void*, NativeStringRawPoolContext&)` already provides the narrow-header schedule with propagating getter failures. The context borrows the canonical `01090AA8` pool-publication cell, `01090AA4` return gate, and `01090AA0` raw manager-publication cell. `GameNativeStringProcess` retains those cells, and the existing VFS accessor exposes its raw context. A later cleanup scope must retain this borrowed context and the original node across the first failure; it must not cache a pool owner or create a private registration domain.

The existing wide-header helper and `00432050` range helper still accept `NativeStringStorage&` through `noexcept` interfaces. The actual storage bridge also makes release `noexcept`; lazy-getter failure therefore cannot propagate through those interfaces into the node's intended cleanup scope. Source needs an exception-compatible raw-context wide return before composing this node's normal wide-then-current-narrow path and state-0 cleanup. The range context remains a separate derived-locale dependency, and the map's `00E17698/00E176B4/00E176C0` node pool remains distinct from the payload pool.

Seven complete Source COFF sections, totaling 431 bytes, are independently reparsed and retained: raw narrow helper, storage-interface narrow helper, wide helper, storage getter, range helper, retained process factory, and VFS raw-context accessor. Their existing source/header contracts are checked against the same frozen inputs. Semantic `LocaleTables::Node`, `unique_ptr`, or standard-string ownership is not substituted.

The smallest later Source candidate is now explicit: preserve the original node, perform a propagating raw-context wide return while narrow cleanup is armed, disarm that cleanup at the observed boundary, then run the existing raw narrow helper. Root must choose the admitted semantic contract and cleanup-failure policy before implementation; this packet performs no C++ work. **Native EH/SEH ABI equivalence and application activation GO remain false.** Runtime frame rebasing and double-exception behavior remain unproved even if a bounded Source cleanup scope is later implemented.

## Retained qualification

The Source epoch remains `Source3352-particle-model-typed-views`. All 3,352 project inputs and four artifacts matched current Root bytes at capture. All actual Git blobs matched the 3,352 worker inputs; 3,345 also matched frozen compiler-input bytes exactly, while seven differed only by line endings. The byte cohorts remain separate. Whole Core indices cover 1,991 archive members/1,988 ordinary COFF objects; the 70 frozen application objects are the `bsp_game` subset, not Root's broader current 80-object inventory.

Five whole worker reports are pinned dependencies. Constructor/profile are accepted. Base and raw-map acceptances are separately retained as four complete Git-qualified review files at main `3129866ef`; their Git blobs were rechecked without borrowing Root's underlying proof claims or changing historical pending-status captures. Node owner `95d258abcd72c432e78f141bc5eae2470c0cff6a` is pending Root acceptance at this capture. The reused node bytes are tied to that immutable node report/bundle, not presented as fresh body evidence.

The adjacent report pins successful original-location and physically relocated ZIP-only replays, including deliberate external-read denial. Replay verifies saved material separately from actual Git/current-source qualification. No C++ change, build, CTest, Original/fixture execution, ABI replacement, startup, gameplay, SDK/OS interaction, or Ghidra mutation is claimed.
