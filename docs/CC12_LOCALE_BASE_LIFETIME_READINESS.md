# CC12 locale base lifetime readiness

`007365E0` closes the normal-path base lifetime dependency of the native locale destructor. It unregisters the **current `00F8BC4C` publication after its second manager lookup**, which can differ from the captured receiver. After unregister returns, it clears that publication unconditionally, releases the first manager's captured section, and resets the captured receiver to the generic root profile. This is bounded reconstruction evidence; no C++ implementation or application activation is admitted by this report.

The retained Source epoch is `Source3352-particle-model-typed-views`. The accepted constructor/base packet is a pin dependency. Root independently accepted the preceding profile lifetime worker commit `11e8d43fdd4e8279dc2694aad85abf74a823e3c9` and integrated/pushed it on main `20a408581`. Neither prior proof tree nor Root's broader current application census is recursively retained or replayed here.

## Fresh Native scope and ABI

| Span | Bytes | Purpose |
| --- | ---: | --- |
| `007365E0..00736678` | 153 | Complete base destructor/unregister body, 40 instructions |
| `00C85F70..00C85F79` | 10 | Local handler selector |
| `00DB53BC..00DB53DF` | 36 | Selected FuncInfo |
| `00DB53AC..00DB53BB` | 16 | Exactly two unwind-map records |
| `00C85F60..00C85F6F` | 16 | Two admitted eight-byte cleanup actions |

All 231 bytes equal the installed PE spans. Every live query verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. Fresh typed metadata has a single complete 153-byte body, no no-return flag and no entry flow override. The stored listing covers all 40 decoded instructions, so this body has no listing holes. No Ghidra repair or mutation was performed.

The original call contract is a raw receiver in `ECX`, no stack arguments and plain `RET`. `EDI` captures the receiver and survives both manager lookups. The body preserves `ESI`/`EDI`; it establishes no meaningful `EAX` result. A descriptive unregister name must not be interpreted as proof that its removal argument is the captured receiver.

## Normal path

1. `007365FA..00736606` captures `ECX`, saves it for unwind, writes the locale singleton base profile `00CFEA5C`, then arms state 0.
2. `0073660E` calls the actual `00415350` manager getter. `00736613` captures that returned manager's `+10h` section. The local guard receives profile `00CE37FC` and this section. A non-null section enters through `00CE2218`, then increments its current `+18h` recursion word. A null section skips both operations.
3. `00736631` arms state 1. `00736636` performs a second manager lookup. Only after it returns does `0073663B` reload `00F8BC4C`. `00736644` calls `00BCFCA0` with the second manager in `ECX` and that reloaded publication as its sole stack argument. These are separate captures: first section, second manager, current publication, original receiver.
4. On unregister's normal return, `00736649` writes zero to `00F8BC4C` without checking its prior value or comparing it with the original receiver. Provider side effects or replacement publications are not rolled back or protected from this clear.
5. For a non-null captured first section, `00736657` decrements its current `+18h` word and `0073665C` leaves through `00CE2210`. The body never re-fetches the section for cleanup. `00736666` resets the original receiver's profile to `00CE3818`, restores the exception link/registers/stack and returns.

The fresh scope does not expand either manager or unregister provider. Their established Source interfaces are dependencies, not new Native proofs. In particular, this packet does not infer removal, allocation, bounds, or null-argument behavior from a callee name.

## Armed states and failure limits

The selector loads FuncInfo `00DB53BC`: magic `19930522`, maximum state 2, map `00DB53AC`. State 0 has previous state -1 and action `00C85F60`; state 1 has previous state 0 and action `00C85F68`.

The state-0 action loads the saved original receiver from `[EBP-18h]` into `ECX` and tail-jumps to `00412430`. The state-1 action takes the local guard at `[EBP-14h]` and tail-jumps to `00411EE0`. The accepted constructor packet supplies the generic root-profile reset and actual captured-section cleanup contracts for these targets; their bodies were not reopened here. Thus an ordinary C++ exception while state 1 is armed first cleans that captured guard and then resets the original receiver through state 0. No local action clears the locale publication, retries unregister, or frees the receiver.

Failure before the explicit publication-clear instruction retains the current publication and all provider side effects already performed; it does not promise the original pointer survives a throwing provider. Failure after that clear cannot restore it. The normal path does not explicitly disarm state 1 before its section release. Consequently this evidence does **not** establish safe recovery from a fault/throw during `LeaveCriticalSection`, a double exception, invalid raw storage, arbitrary SEH translation, or reentrant destruction. The profile write precedes state-0 arming. Ordinary-return Win32 calls, valid mutable storage, surviving captured section/receiver, and compatible exception behavior are required assumptions for later Source composition.

## Current Source and artifact qualification

All 3,352 frozen project inputs plus four build artifacts matched Root's current bytes at capture. Worker/Git bytes match each other for all 3,352 inputs; 3,345 also match the frozen compiler-input bytes exactly, while seven differ only in line endings. All cohorts and Git blob hashes are retained separately. Portable replay checks these saved bytes and capture receipts; it does not query current Git, compiler inputs, Ghidra, or the game.

The complete Core archive was reparsed (1,991 members, 1,988 ordinary COFF objects), together with 70 frozen **bsp_game-target** objects. This is not a census of every MSBuild target's output. The full symbol indices contain no exact address-named `007365E0`, `00736540`, `0073C9F0`, or `0073CAF0` implementation; that narrowly scoped absence is combined with explicit source and binding inspection.

Six complete Source COFF sections total 305 bytes: Sound manager getter (62), section view (28), unregister view (28), captured-section constructor (119), destructor (46), and `GameSingletonHost::sound_lifetime` (22). Their full instructions and relocations are retained. They confirm the existing view can borrow the application's actual manager-publication cell, read the actual manager's `+10h` section, dispatch to the raw unregister interface, and release the captured section. Its semantic-domain branch is a separate contract and cannot substitute for the raw application branch.

The exact `4040h` locale image and derived-fields constructor remain dormant. The eight source files pinned by the accepted constructor packet are unchanged. The application's semantic `GameLocaleHost`/`LocaleTables` ownership remains distinct from native raw locale destruction, and its typed singleton dispatcher still has no `00CFF140` profile binding. This packet implements none of those missing connections.

## Smallest next frontier

The base normal path can now inform a later dormant raw lifetime/context helper with separate original receiver, locale-publication cell, and borrowed actual Sound manager access. It must preserve two manager lookups, delayed publication evaluation, first-section capture, unconditional post-unregister clear, and failure side effects. Context and cell/section lifetimes must outlast all constructor, destructor, and drain paths; no private manager domain or publication repair may be silently introduced.

Before composing the full raw destructor/scalar wrapper, independently close `0073C240` raw-map cleanup, the derived destructor's directly referenced exception map/actions, and actual string-pool/range release context with compatible exception contracts. The constructor's failure publication behavior remains a separate accepted dependency. Only after those closures should a bounded dormant constructor/lifetime/context implementation be considered. Loader, text consumer, owning application lifecycle, and exact typed profile dispatch remain separate application gates. **Application activation GO: false. Complete lifetime implementation GO: false.**

The adjacent JSON report pins the finite bundle and both successful replay receipts. The physically relocated copy runs the same ZIP-only guard and deliberately denies an external source read. No C++ changes, build, CTest, Original execution, fixture execution, ABI replacement, startup, game, SDK/OS interaction, or live-runtime validation is claimed.
