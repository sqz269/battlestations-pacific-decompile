# Embedded native input configuration lifetime

Addresses: `00698680`, `004DCEB0`.

This packet reconstructs the actual constructor and destructor used for the
game's embedded configuration. Game constructor4DDB90 passes `game+3C` to698680
at4DDBE2. Its next member begins at `game+560`, establishing a524h embedded span.
The source borrows that original span; it allocates no separate configuration,
copies no settings owner, and does not cast `InputScriptStartup` storage.

| Routine | Coverage | Original ABI and body |
| --- | --- | --- |
| 698680 constructor | Complete actual-storage initialization | ECX configuration; no stack arguments; EAX same address. Plain RET at698729, length1. Body698680..698729, exclusive69872A,170 bytes,36 instructions,0 gaps. |
| 4DCEB0 destructor | Complete actual-storage lifetime schedule | ECX configuration; no stack arguments or semantic result. Tail JMP B669A0 at4DCF84, length5. Body4DCEB0..4DCF88, exclusive4DCF89,217 bytes,64 instructions,0 gaps after primary repair. |

The important layout correction is that the Lua owner starts at configuration
**offset zero**, not+4CC. At698697 the constructor saves ECX in ESI; its call to
B66BD0 at698699 uses that same ECX without adjustment. The existing concrete
`NativeLuaStateStorage` occupies4C8h and provides the exact sparse initialization.

| Relative field | Producer evidence / treatment |
| --- | --- |
| +0..+4C7 | Actual4C8h Lua owner. B66BD0 clears owns byte0, state4, wordsC/10, fifty count words28+18*i and high-water4C4. Padding, opaque8 and tracked reference pointers preserve their preimage. |
| +4C8,+4C9,+4CA,+4CB | Constructor clears bytes in order4C8,4CB,4CA,4C9 after all vector triplets. Loader698A10 tests4C8, calls Lua open on the same base, and later sets4C8. |
| +4CC | DWORD untouched by constructor/destructor; no Lua-owner meaning is assigned. |
| +4D0,+4E0,+4F0,+500,+510 | Five actual10h checked-vector headers. Each retains opaque word+0 and initializes pointer begin/end/capacity at+4/+8/+C. Outer4D0 owns actual10h DWORD-vector rows; other four own DWORD buffers. |
| +520 | Constructor clears this single byte last; destructor preserves it. |
| +521..+523 | Padding before the next game member; retained. |

The constructor uses the already reconstructed native Lua constructor and then
performs the fifteen pointer-word and five byte stores. It does not create a
Lua interpreter, execute configuration scripts or initialize the opaque words.
It has a native FH3 registration with initial state-1 and handlerC7ECC0.
FuncInfoDABB40 contains five unwind entries, but the emitted constructor never
advances its state from-1. Its only callee B66BD0 is a nonthrowing field-store
leaf; the remaining operations are stores. The source introduces no speculative
cleanup for unreachable higher constructor states.

The recorded unwind map is state0→C7EC80/Lua close at the same base; state1→
C7EC88/outer4D0 destruction; state2→C7EC96/flat4E0; state3→C7ECA4/flat4F0;
state4→C7ECB2/flat500, each chaining to the previous state. HandlerC7ECC0 has
no Ghidra function start: MOV EAX,DABB40 then JMP BF6B43 atC7ECC5, ten bytes,
exclusiveC7ECCA. These are compiler EH contracts, not new implemented routines.

Destruction proceeds independently of698730's action reset:

1. Capture and free each nonnull flat buffer in order510,500,4F0,4E0; clear its
   begin/end/capacity triplet after the free returns.
2. Capture the outer4D0 begin. If nonnull, capture its end and destroy actual10h
   rows forward to that fixed endpoint. For each row, free its captured nonnull
   begin then clear the triplet, leaving opaque+0 untouched.
3. Reload outer begin after row destruction, free it, and clear the outer
   triplet. No capacity/range check or alternate malformed-range termination is
   added.
4. Tail-close the actual Lua owner at configuration offset zero through the
   existing B669A0 implementation. It closes a nonnull state only when owns byte
   is nonzero, then clears state4. Other metadata and configuration flags remain.

The nested row walk consumes the small4D49B0 library contract. Its original
ECX/EDX are captured begin/end, and RET8 consumes two unused stack words. All
four callers were inspected. The new private source container adapter uses the
existing singleton CRT free service; it does not claim a recovered STL symbol
or add a reconstruction/name record for4D49B0. All buffers must belong to that
same compatible allocation domain. The embedded configuration itself is never
freed here. The source neither unregisters actions nor calls698730; its separate
cleanup packet owns those operations.

Game destructor4DCF90 calls4DCEB0 at4DD577 with ECX=`game+3C`. Game destructor
EH mapD8F978 state1 uses C66A5B→C66A61 tail4DCEB0; game constructor EH mapD8FB54
state1 uses C66D0B→C66D11 tail4DCEB0. Each adds3C to its saved game receiver.
The source therefore provides one reusable embedded destructor and leaves its
enclosing game lifetime and registration to the caller.

Primary repaired five CALL_RETURN-induced gaps in4DCEB0 and one in4D49B0; each
was the existing three bytes83C404 (`ADD ESP,4`). Fresh listings have0 gaps.
The saved receipt is `reports/native_input_configuration_owner_flow_repair.json`.
No native bytes changed, no global no-return annotation was changed, and this
worker performed no Ghidra writes.

Win32 Release, both existing CTests and eight native seed checks passed. One
ignored archive-only probe compared two complete524h constructor preimages to
copied original bytes. It then supplied four nonempty flat buffers, two nonempty
nested rows and a real owned Lua state. Copied native destruction freed seven
exact identities in the expected order, retained nested opaque words, and ran
Lua finalization after all five outer triplets were zero. The new source matched
the final whole524h storage; both real Lua allocation counters reached zero.
Repeated empty-member destruction did not close Lua again.

The native free trace is directly observed in the copied-native run; the source
uses the concrete CRT free path and is checked through its executed body and
final storage. Original FH3 exception paths were not exercised. No configuration
loading, scripts, action reset, SDK, input polling, force, cursor or game activity
was performed. Original callable/FH3/SEH ABI, hardware faults and asynchronous
mutation remain outside this new C++ interface.
