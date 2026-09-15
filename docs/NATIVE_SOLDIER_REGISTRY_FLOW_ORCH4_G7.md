# Soldier registry, float-map and base-reader listing repairs

Addresses: 0048F670, 004B0330, 004B0600, 00444490, 00444252, 004B0312.

The existing bsp project and `/battlestationspacific.exe` were verified before
each batch. Complete live/disk byte comparisons preceded all mutations. Repairs
ran through the owning address leases and shared Ghidra write lock; old function
metadata and comments were captured. Game bytes were not changed.

| Entry | Complete span | Recovered behavior |
| --- | --- | --- |
| 0048F670 | 0048F670..0048F86C, 509B | Three-byte stack adjustment after free at 0048F7F2; control continues into point-group result handling and final name cleanup. |
| 004B0330 | 004B0330..004B05FB, 716B | 58-byte tail after free at 004B05BD: decrement nonzero current count, publish output iterator, restore frame and RET0Ch. |
| 004B0600 | 004B0600..004B0651, 82B | Eleven bytes after free at 004B063C restore the leftward subtree-drain loop. |
| 00444490 | 00444490..0044475B, 716B | Same missing 58-byte erase tail in the float-map specialization. |
| 00444252 | 00444252..00444266, 21B | Float-node allocation catch: free captured raw node, adjust stack, rethrow. |
| 004B0312 | 004B0312..004B0326, 21B | Registry-node allocation catch: the corresponding captured-node free and rethrow. |

All six final listings have zero gaps. Truncated saved bodies were recreated only
after independent byte preflight; gap decoding alone was not treated as repaired
function ownership. The project was saved and affected exports refreshed.

The original reader 0048F670 loads a class resource, conditionally acquires
FakeFireEfx and replaces the current handle with retain/release ordering, then
queries a named point group and transfers three coordinates with ordered x87
FLD/FSTP instructions. The old decompiler return after free incorrectly skipped
the point handling and name cleanup. The source reader remains absent pending
genuine effect-component and reader dependency closure.

## Detached allocation catches and tool support

The main allocation bodies end before their separately defined catch handlers.
For example, 004441E0 has handler C5FBF1, FuncInfo D8687C, unwind map D86864 and
try map D86850. The try covers states0..1; catch state2 invokes 00444252. State1
unwind invokes C5FBE0 -> 00401130, the one-byte no-op placement-delete body.
The explicit catch then frees the captured raw node and calls BF6885 with two
zero arguments. It does not invent an additional key cleanup. The registry
specialization uses the corresponding C64920 placement action and 004B0312 catch.

`tools/ghidra_flow_repair.py --tail-rethrow` now accepts this explicit catch-tail
form. It requires the exact decoded `PUSH 0; PUSH 0; CALL 00BF6885` sequence,
an exact instruction boundary and matching whole live/disk bytes. Ordinary
explicit tails still require RET. It does not change callee no-return flags or
accept an arbitrary terminating call. One focused guard test checks accepted and
rejected exit sequences. The actual two repaired catches exercised the new path.

Mutation records and byte preflight:

- `reports/native_soldier_reader_flow_repairs_orch4_g7.json`
- `reports/native_soldier_registry_flow_repairs_orch4_g7.json`
- `reports/native_soldier_registry_body_repairs_orch4_g7.json`

Source closure and full Win32 validation are recorded by the corresponding
registry/LoopLengths packets. These listing repairs alone do not establish
original FH3/SEH execution, binary compatibility or gameplay.
