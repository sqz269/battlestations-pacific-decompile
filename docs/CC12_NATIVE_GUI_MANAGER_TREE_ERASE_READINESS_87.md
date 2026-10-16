# Raw GUI manager tree erase readiness (packet 87)

Whole `AA46B0` is ready for a thin Source alias to the already recovered
`erase_native_int_pointer_tree18_iterator_0086e8a0`. A new RB-tree algorithm is
unnecessary. The actual raw tree layouts and the complete Original machine
bodies match after named call/EH operands. The historical
`STL_xlen_throw_00aa46b0` name and throw-site tag describe only its invalid end
iterator arm. There is no existing AA46B0 reconstruction record.

This packet is read-only. It merged published Root
`6647c210a528bea78d5a8e73ec023ba02749a801` with exit 0, producing worktree
baseline `a94acdb2524eff7a570c354c6204da23e3ad52f3`. Its lease covers AA46B0
and only this document/JSON report. It changes no Source, ledger, Ghidra name,
type, tag, flow or saved program. Export refresh only reads the analysis cache.
No build, test executable, game run or Native execution was needed or performed.

## Whole body, ABI and physical data

Verified BSP wrappers used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, before every live query/export batch. The complete
body is `[AA46B0,AA4957)`, **679 bytes / 231 instructions**, ending in RET0C at
AA4954. Its SHA-256 is
`12ca27deb70231c2ab747ca0f11ee511ecd83156ea4c7d24d21cd42f0509bf99`.
All live bytes match the unchanged installed PE. The live listing has no gaps;
the flow wrapper cannot read exact no-return properties, so no global flow
repair is inferred. Full assembly, including post-free AA4921..AA4956, was read.

Entry ECX is the actual tree; incoming EDX is unconsumed. Three stack DWORDs
are output address, by-value iterator owner, by-value iterator node. EAX returns
the output address; RET0C consumes those words. No owner==ECX check is present.
The operation initially reads node+15 before the checked increment routine.
The initial node must therefore be valid even for a diagnostic path.

Tree storage is 0Ch: opaque allocator word +0, head +4, unsigned count +8.
Head/node storage is 18h: left +0, parent +4, right +8, key bits +C, payload
+10, byte color +14, byte nil +15 and padding +16/+17. Head left/root/right
hold extrema/root. The erase algorithm does not compare or read key/payload
values; float-key GUI storage and the named int-pointer family share the same
physical link engine. A typed `std::map` or logical GUI owner is not involved.

## Normal schedule and ownership

1. Reject nil input through the owning exception arm. Otherwise save the
   original node separately and increment the iterator in its **actual by-value
   argument slots** at AA472E. The returned iterator is this successor.
2. Select left/right replacement for zero/one-child cases. For two children use
   the successor, transplant its links, and swap the original/successor color
   bytes. Preserve every parent/head/extrema update. AA4793 calls minimum;
   AA47B5 calls maximum only at the reached extrema branches.
3. If the original node's resulting color is black, run the full symmetric
   sibling/color/rotation fixup. Rotations are real whole raw helpers; nil/head
   fields and current root are reread at the shown sites. Color +14 and nil +15
   are byte accesses; links/count/output are DWORD accesses.
4. AA491C frees the **separately captured original node**, not the successor or
   its payload. Only afterward AA4921 reloads current tree count; unsigned
   nonzero decrements once, zero stays unchanged. There is no refcount gate.
5. AA4933/AA4937/AA493B capture current argument owner, output pointer and current
   argument node, before AA493F stores output.owner and AA4946 output.node.
   Restore nonvolatile state/FS frame and return the same output address.

The actual header, head, surviving links and output must remain valid/aligned
through all reached accesses in the same externally serialized, finite raw
tree. The original node must belong to the same allocation/free domain as the
accepted Source node allocator. Output may alias other **live** raw storage;
the exact capture/store schedule matters. It must not refer to freed node
backing. Private Native argument/EH spill aliases are outside the accepted
Source interface. No extra owner check, count repair, payload deletion, layout
retain, child detach or rollback may be added.

The accepted implementation uses actual `singleton_lifetime_free` (Source
`std::free`) and SDK `_invalid_parameter_noinfo`, including its returning-handler
continuations. Its node allocation domain matches existing raw GUI map creation
and node allocation. This is the owning Source CRT domain, not proof that a
different Original process heap/encoded handler can be substituted.

## Invalid iterator and EH schedule

The nil arm initializes an actual no-init 1Ch legacy SBO string: capacity 15,
length zero and inline byte0 zero. AA46F4 assigns all **27 counted bytes** of
`invalid map/set<T> iterator` before AA4702 arms state 0. AA4706 constructs the
actual 28h logic-error storage; AA4715 publishes D6926C, then AA471D calls
`__CxxThrowException@8` with ThrowInfo D863A8. No normal tree mutation has begun.

Handler CB7108 loads FuncInfo DED838 and tails BF6B43. FuncInfo magic19930522,
maxState1, map DED830, no try blocks: state0 -> -1 runs CB7100, which is exactly
`LEA ECX,[EBP-50h]; JMP 004072D0`. This destroys the completed message temporary.
Assignment failure occurs while state is -1; that outer cleanup is not armed
early. Normal mutation stays in state -1 and adds no exception rollback.

The accepted 86E8A0 Source helper restores entry registers/stack on nil input,
then uses its established owning C++ throw path. It assigns 27 bytes before
arming temporary cleanup and uses `NativeHardwareLayoutInvalidIterator`: a
28h Source transport calling real 411700 construction, 441760 copy and 4412B0
destruction providers, with D6926C storage. Its name does not import a hardware
tree. Source RTTI/catch identity, private stack, Native FH3/SEH/hardware faults,
Original heap/handler domains and provider register/throw identity remain
explicit limits. An alias cannot expand that proof.

## Exact existing-body reuse audit

AA46B0 and Original 86E8A0 have the same instruction offsets, widths, registers,
branches and constants. Thirteen direct CALL rel32 fields plus the handler
immediate account for 56 named operand bytes. **All other 623 bytes are exactly
equal.** No instruction or constant was patched. The full operand/byte maps
and all Original hashes are in the JSON receipt.

| GUI target | Accepted whole target | Bytes | Equality outside named operands |
| --- | --- | ---: | --- |
| `4B6690` minimum | `8697D0` | 27 | All 27 equal |
| `AA1040` maximum | `8697B0` | 28 | All 28 equal |
| `AA1080` right rotate | `869810` | 82 | All 82 equal |
| `AA2180` left rotate | `86A2F0` | 78 | All 78 equal |
| `4BDA70` checked increment | `869A20` | 99 | Two CRT rel32 operands; other 91 equal |

Both EH handlers target BF6B43; FuncInfo fields match except each actual map
pointer. Each one-entry map is state0 -> -1, with identical cleanup instructions
targeting 4072D0. All twelve whole Original function bodies and both
handler/FuncInfo/map/action groups were checked against live Ghidra and installed
bytes. This is static equivalence/readiness, not a new Native differential.
The live report verifier checks fourteen declared direct CALL sites with zero
failures. Its call-site result does not imply execution or Native EH proof.

Existing GUI increment/right/left entries already delegate to these accepted
raw leaves in `native_gui_camera_store_map.cpp`. Existing scalar-tree 556DC0
also demonstrates a whole 679B alias to 86E8A0 with the same documented limits.
Missing standalone ledger records for GUI minimum do not create a missing raw
algorithm: the complete 27B accepted minimum is byte-identical.

## Ready next Source packet and remaining parent

The smallest next packet is a single whole raw entry in the existing
`include/bsp/native_gui_camera_store_map.hpp` and
`src/native_gui_camera_store_map.cpp`, adding the erase header only in the cpp:

```cpp
void* __fastcall erase_native_gui_camera_store_iterator_00aa46b0(
    void* actual_tree, void* unused_edx, void* actual_output,
    void* input_owner, void* input_node);
```

Delegate those five arguments to the accepted whole 86E8A0 body, as existing
family aliases do. No new tree engine, owner/context, callback, allocator,
exception substitute, CMake unit or defaults are needed. It borrows the raw
storage and inherits the fixed, explicit Source CRT/owning-exception domain
above. The new Source wrapper/private argument frame is not a Native binary
replacement. Root will assign/lease packet 89 before any Source/annotation edit.

AA4B30 remains a separate 237B/81-instruction raw parent gap: its current Source
entry is typed `GuiCameraStoreMap::remove_owned`. Its actual manager+8 tree
search frees current store payload, then **clears node+10 before AA4C0E erase**,
and removes only the first match. It passes the node and current iterator owner
by value, plus a real stack output, with ECX = tree. The current store+18 receiver
is the camera/node from AA5070 (scene is +1C); B6D800 supplies that same raw
name header, whose +4 data falls back to F8BC60 for the genuine 4254B0 RET logger.
These input evaluations/current reads remain required even though logging is
empty. That parent can follow the erase alias under separate authorization;
no raw parent, logger, factory, graph retirement or ancestor lifetime provider
was added here.
