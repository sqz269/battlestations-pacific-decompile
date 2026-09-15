# Native shadow pointer-vector disposal

This module reconstructs **00AE1C60..00AE1C76 (23 bytes, 10 instructions)** using
the full existing EP resize and shared source CRT free providers. It is an
unbuilt source candidate; generated-byte identity and runtime/game behavior are
not claimed. The descriptive name is a hypothesis. No parent or standalone EH
action receives credit.

| Entry | Original ABI | Coverage |
|---|---|---|
| AE1C60 | ECX actual three-DWORD header; no public arguments; RET; preserves ESI | Complete normal body |

The receiver is the array header itself, not its enclosing entry. Its data,
signed count and signed capacity are at +0/+4/+8. The source naked fastcall
interface adds an explicitly unused EDX slot and retains the noargs RET.

The exact schedule is PUSH ESI / PUSH 0 / MOV ESI,ECX / CALL AE19B0 / fresh
MOV EAX,[ESI] / PUSH EAX / CALL BF6989 / ADD ESP,4 / POP ESI / RET. Resize consumes
the pushed zero with RET4. The free call is unconditional, including a null data
pointer. The source calls `resize_native_shadow_pointer_vector_00ae19b0` and
`singleton_lifetime_free(void*) noexcept`, which uses the same source CRT
`std::free` domain as EP reserve. The final RET is an explicit single-byte C3.
The expected generated body is 23 bytes after its two CALL rel32 bindings are
normalized; that remains a build-time check.

No pointer/capacity clear, element release, header free, null-free skip or
failure cleanup is added. Requested zero still runs the **full** resize body;
unusual negative capacity can reach its reserve branch. Header/backing validity,
actual source allocator pairing and existing provider failure limits remain
caller obligations. Source allocator/OOM, arbitrary alias, native fault/SEH/FH3
and exceptions across naked frames are not established.

| Site | Target | Preparation / boundary |
|---|---|---|
| AE1C65 | AE19B0 | ECX actual header, pushed zero; callee RET4 |
| AE1C6D | BF6989 | Fresh current [ESI] data; caller ADD ESP,4 |
| CB9E1C | AE1C60 | Actual JMP after ECX=[EBP-10] and ADD ECX,44 |
| CB9E6C | AE1C60 | Actual JMP after ECX=[EBP-18] and ADD ECX,44 |

The two unwind callers prove a +44 array subobject only. They do not reconstruct
their enclosing constructors/destructors, whole unwind maps or actual producer
ownership. Ghidra xrefs describe them as UNCONDITIONAL_CALL, but their original
bytes/listings are JMP and the report records `tail_jump`.

Primary repaired the actual AE1C6D flow override from CALL_RETURN to NONE, then
separately recreated the stored body through AE1C76 after repairing its listing
tail. The five-byte tail is 83 C4 04 5E C3. Original name/plate were preserved,
the program saved and exports refreshed. The worker made no Ghidra changes.
Fresh guarded queries now show all ten instructions and 23 live bytes equal the
installed PE. Full native bytes are
`566a008bf1e846fdffff8b0650e8174d110083c4045ec3`.

The report pins the native spans, source/provider files and primary repair
evidence. EP dependency source at 4d0a0cbd is accepted; its first combined build
failed only a CMake timestamp operation before tests. The coordinated retry and
this candidate's later build are separate acceptance steps. No standalone build,
new test, fixture, source-provider change, ledger edit or gameplay claim is made.
