# Native allocator base-message constructor Source candidate

This packet implements the complete `00BF638E..00BF63A5` leaf as
`bsp::construct_native_allocator_base_message_00bf638e`. The accepted Original
body is 24 bytes / seven instructions. Its saved library name, `exception`, is
preserved; the Source descriptive name is provisional.

The worker provides a static candidate only. The new file is not registered in
the worker's CMake base. The primary integrator owns registration, the required
normal MSVC Win32 build, whole emitted-body review, Core validation and final
ledger/Ghidra review. No compilation, fixture execution, ABI admission or game
validation is claimed by this packet. No Original credit is applied here.

## Raw interface and ABI

The public header requires MSVC Win32 and declares a naked `__fastcall`
implementation with four Source arguments:

| Source argument | Entry location | Contract |
| --- | --- | --- |
| `void* actual_receiver` | ECX | Actual live receiver backing for 12 bytes |
| `uint32_t unused_edx` | EDX | Unread and unchanged by the body |
| `const void* actual_pointer_slot_address` | `[ESP+4]` | Actual readable address of one four-byte slot |
| `uint32_t ignored_numeric` | `[ESP+8]` | Unread numeric word consumed by `RET 8` |

The explicit EDX argument places the two Native stack words at the correct
offsets under the Source fastcall convention. It is a Source binding device;
the recovered Native leaf does not consume an EDX parameter. The body leaves
EDX unchanged, while the ordinary C++ fastcall declaration still treats EDX as
a volatile register. No extra calling-convention guarantee is inferred.

EAX returns the original ECX receiver. ECX ends with the loaded slot value.
There is no local stack frame, helper call, branch, allocation, cleanup or
exception-handler wrapper. EBX, ESI, EDI and EBP remain untouched. The body
retains the exact arithmetic-flag effects of `AND DWORD [EAX+8],0`; the later
store and return do not replace that instruction. No `noexcept` promise or
portable C++ exception-class ABI is added.

## Instruction and access order

| Original address | Instruction | Effect |
| --- | --- | --- |
| `00BF638E` | `MOV EAX,ECX` | Save the receiver for return and memory accesses |
| `00BF6390` | `MOV ECX,[ESP+4]` | Capture the slot address before any receiver store |
| `00BF6394` | `MOV DWORD [EAX],00D69370h` | Publish the raw profile word at receiver+0 |
| `00BF639A` | `MOV ECX,[ECX]` | Read the slot value after publishing the profile |
| `00BF639C` | `AND DWORD [EAX+8],0` | Read-modify-write the final receiver DWORD |
| `00BF63A0` | `MOV [EAX+4],ECX` | Store the captured value in the middle DWORD |
| `00BF63A3` | `RET 8` | Return the receiver and consume both stack words |

The expected whole Original encoding is:

```text
8bc18b4c2404c7007093d6008b0983600800894804c20800
```

The three DWORD writes cover receiver bytes 0 through 11 without holes. The
last DWORD must be readable as well as writable because its operation is an
actual read-modify-write, even though the normal resulting value is zero.
Earlier stores can remain visible if a subsequent access faults.

The implementation captures the slot address before the profile store but
loads the slot contents afterward. This distinction preserves overlaps: a
slot at receiver+0 observes the newly published profile; a slot at receiver+8
is read before that DWORD is cleared. Other overlaps retain the same literal
instruction schedule. There is no receiver or slot null check. The copied
word itself may be zero; its pointee is never read, copied or validated.

The literal `00D69370h` is raw numerical data. This packet adds no Source global,
callable vtable, RTTI, string ownership, shared slot or larger typed owner.
Actual backing, slot/pointee lifetime, ownership and usable runtime identity
remain separate caller contracts. In particular, this 12-byte footprint is
not cast to the separate existing 0x28-byte owner representation.

## Evidence and validation boundary

The report pins the accepted leaf-readiness report, its documentation, the
existing 27-byte naked initializer pattern and the relevant build inputs at
published base `34ad122c6cb3894c962821550711613a021c1966`. It replays all 58
inherited canonical hash references across 26 unique paths. Physical and
canonical-LF Source hashes distinguish working-tree newlines from Git content.

Static validation compares the entire 24-byte Original span with the current
configured PE and independently decodes all seven instructions. It checks the
candidate inline assembly, the explicit argument positions, target guards and
the absence of new helpers. These checks do not establish compiler emission.
The current packet performs no Ghidra query or mutation and relies on the
accepted report for saved project/function metadata.

No ad hoc compiler probe or new test is added. The worker intentionally does
not build an unregistered file. After registration, the primary must verify
the complete emitted function, including `RET 8`, the real DWORD RMW, register
effects and literal profile word, and complete the normal build/Core review.
Only then may the primary consider one Original function / 24 bytes for
admission. Wider allocator/exception/runtime, startup and game behavior remain
unclaimed.
