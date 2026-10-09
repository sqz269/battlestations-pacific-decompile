# CC12 Mission Lua payload destructor unwind readiness

Baseline: published Main `7f38fc629dbbaf482b46df659ffad1d81ddce690`.
Evidence: [independent handler, record and actions](../reports/cc12_mission_lua_payload_destructor_EH_readiness.json).
This read-only audit admits no Source wrapper, Native ABI or gameplay credit.

## Encoded cleanup sequence

Handler `C97543` executes ten bytes: `MOV EAX,DC9AA4; JMP BF6B43`.
Its bounded 16-byte window also contains three padding bytes and three excluded
adjacent bytes. The shared handler body was not queried.

The absolute x86 FuncInfo at `DC9AA4` is 36 bytes: magic `19930522`, upper BBT
bits zero, maxState two, map pointer `DC9A94`, zero try/IP/type-list fields and
encoded EHFlags one. The 16-byte map contains exactly two entries.

| State | Next state | Action | Complete action |
| --- | --- | --- | --- |
| 0 | -1 | `C97530` | `MOV ECX,[EBP-10h]; JMP 41DD20` |
| 1 | 0 | `C97538` | `MOV ECX,[EBP-10h]; ADD ECX,8; JMP 41DD20` |

The actions are eight and eleven bytes. Each rereads the current mutable frame
slot independently. They tail-jump with the incoming return stack intact; the
second action's DWORD addition wraps and changes arithmetic flags. They contain
no local frame, header stores, callback adapter or direct free call.

Under the runtime/frame contract and normally returning cleanups, state one
selects the string header at current saved payload `+8`, then state zero selects
the string header at current saved payload `+0`, then state `-1`. State zero
alone selects only the latter. The map encodes no direct vector-buffer,
vector-element or outer queue-node free.

## Parent state and frame

The independently accepted full parent body is 148 bytes. With S as entry ESP,
its payload spill is `S-10h` and state is `S-04h`. The ordinary body never sets
EBP. **If helper EBP equals S**, both action offsets match the payload spill;
this corroborates the expected relation without establishing shared-helper ABI.

`8876D5` writes state one before the raw vector child and subsequent vector-data
free. `8876F2` writes only state low byte zero after loading/testing the first
buffer, before its conditional getter/return. `887714` writes full state `-1`
after loading/testing the second buffer, before its conditional getter/return.
Thus ordinary first-string release is in state zero; ordinary second-string
release is after state `-1`. No early disarm or rollback is added.

Child-vector unwind behavior and caller-node lifetime remain separate scopes.
State stores and map entries do not guarantee dispatcher recognition, state
publication timing, fault filtering or successful later actions after a failing
cleanup. Shared `BF6B43` and nested-failure behavior remain unexpanded.

## Concrete Source and verification

Existing `destroy_native_string_header_0041dd20(void*,NativeStringRawPoolContext&)`
captures current data, then current length plus one with DWORD wrap; null data
skips the getter, and the header is untouched. Its concrete raw-pool access gets
the current pool before every nonnull return, including large or disabled-small
returns. The overload permits getter exceptions to escape.

The distinct `NativeStringStorage&` overload and
`ActualNativeStringPoolStorage::release` use noexcept interfaces. They cannot
automatically supply the Native exceptional composition. Existing process-owned
Source publication cells do not establish a Lua payload/vector owner or EH entry.

Guarded queries verified existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe` and matching total count 64,729. All 87 captured
file-backed bytes match the installed PE: 81 semantic bytes, three padding bytes
and three excluded adjacent bytes. The 29 semantic code bytes decode to seven
instructions and three tail jumps. Eleven canonical repository pins, the full
parent PE span and the current SDK layout pin pass. The SDK corroborates the
record layout, not original compiler/runtime compatibility.

No native string-child body, shared runtime, other handler/caller, table or
bootstrap was expanded. No Source, ledger or Ghidra change, build, test, probe,
native execution or new Original credit occurred.
