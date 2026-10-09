# Complete World entity-chain receiver entry

`retire_native_world_entities_00904390(void*)` implements the complete
`00904390..00904397` body as guarded MSVC Win32 naked fastcall Source:

```asm
MOV ECX,[ECX+4]
JMP 009041A0
```

Fresh live bytes and the installed PE are exactly `8B 49 04 E9 08 FE FF FF`.
The whole emitted entry is eight bytes/two instructions. Its sole external
REL32 operand4 binds to the concrete `retire_native_world_entity_chain_009041a0`
symbol; rebinding the operand to -504 (`08 FE FF FF`) makes every byte equal.
The current provider's complete 100-byte/43-instruction body also matches the
PE without relocations. Two exact complete core members and two unique
positive public definitions establish that physical tail binding.

The normal MSVC Win32 build and all three existing checks passed. Whole objects,
physical indexed graphs, complete body bytes and archive definitions are in
`local/cc12_world_chain_receiver_primary/whole_objects_and_physical_tail_binding.json`.
The build log/receipt, pinned source copies, whole objects/library, game
map/executable and existing test log are retained in the same directory.

ECX is the actual World. The single current pointer read at `+4` supplies the
actual chain header. EDX and explicit stack arguments are not consumed. The
tail transfer inserts no return address, cleanup, guard, owner lookup or typed
Host. The provider inherits the original stack and returns directly through
its own plain RET. The saved placeholder `void(void)` decompiler prototype
does not erase the physical ECX input.

The original documented two-instruction function record is preserved. Its
older header end-address comment is not the extent proof; fresh complete bytes
establish the end at `00904397`. Admission adds one actual machine-level
fragment and zero new Original functions or bytes. The provisional Ghidra name
and prior comments are preserved with appended evidence and a refreshed export.

The current provider retains that header throughout its current-head unlink,
late virtual slot-zero CALL and post-return count/head reload. Its signed
membership comparison, wrapping count update, current field-store ordering,
callee cleanup4, header/entity/table lifetime and coherent mutation/progress
qualifications remain in force. Numeric Original profiles do not supply Source
callable tables. This entry does not establish a complete World or method table.

The root and provider are absent from the game map. Whole emitted/static/build
evidence does not establish Native ABI entry, fault/SEH/EH/runtime equivalence,
production wiring, startup or gameplay. No new test or probe was added.
Descriptive names remain hypotheses. Primary report:
[cc12_native_world_entity_chain_receiver_primary_review.json](../reports/cc12_native_world_entity_chain_receiver_primary_review.json).
