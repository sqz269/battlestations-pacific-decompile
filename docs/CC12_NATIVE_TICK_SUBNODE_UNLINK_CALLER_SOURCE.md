# CC12 native tick subnode unlink caller Source candidate

This unregistered, unbuilt candidate composes the accepted `00875960[69]`
schedule with the current Source registry getter, admitted raw unlink leaf,
and current Win32 section imports. Its explicit borrowed-cell API is a
qualified Source interface. Source, Native, and Original-ABI credit remain
false until the applicable independent gates are completed.

- Header: [`native_tick_subnode_unlink_caller.hpp`](../include/bsp/native_tick_subnode_unlink_caller.hpp).
- Implementation: [`native_tick_subnode_unlink_caller.cpp`](../src/native_tick_subnode_unlink_caller.cpp).
- Candidate report: [`cc12_native_tick_subnode_unlink_caller_source.json`](../reports/cc12_native_tick_subnode_unlink_caller_source.json).
- Accepted audit: [`CC12_PENDING_REGISTRY_TICK_SUBNODE_UNLINK_CALLER_ABI_READINESS.md`](CC12_PENDING_REGISTRY_TICK_SUBNODE_UNLINK_CALLER_ABI_READINESS.md).

The baseline is `63f6de61cabb88766a98effb8f3377cd960ae0b3`. The accepted
Original body has 25 operations, four physical calls, and SHA-256
`c0ecd0fa7e59f3a5e2077de82b52378ca3a06fc5565f4847176556edba5e2323`.
This packet reuses that pinned evidence and does not reread Native bodies.

## Explicit Source entry and stack

`void __fastcall unlink_native_tick_subnode_00875960` receives the actual
receiver in ECX and an explicit unused DWORD in EDX. Three stack arguments
are addresses of stable, borrowed `void* volatile&` cells: the actual node
input word, registry publication, and manager publication, in that order.
All three actual cells must remain valid at stable addresses for the entire
call. Private publications or copied pointer values do not provide the same
identity or late-read contract.

The body retains the EBX/ESI/EDI saves and captures the receiver in EBX.
Let S be Source entry ESP. After those saves ESP=S-0C. The first
`PUSH DWORD[ESP+18h]` reads the manager-cell address from S+0C; after that push,
the second identical operand reads the registry-cell address from S+08.
The direct current cdecl getter receives registry then manager references.
`ADD ESP,8` removes these Source arguments after its normal return, before
capturing the current section from getter EAX+4 and testing it.

After optional Enter/depth increment, `MOV EDI,[ESP+10h]` obtains the actual
node-input-cell address at S+04. Only the following `MOV EDI,[EDI]` reads its
current node value. This preserves a late borrowed-cell read across getter,
Enter, and depth effects. The new reference-address formal and extra getter
arguments are Source adaptation; they do not reproduce Original stack identity.

The epilogue restores the three registers and uses `RET 0Ch` for the three
Source references, leaving normal ESP=S+10h. Original instead uses `RET 4`
and normal ESP=S+8. The void declaration makes no common semantic EAX promise:
the null-section path can retain the leaf node while Leave can replace EAX.

## Retained effects and qualified providers

The Source body has 29 assembly operations: the 25-position Original schedule
with current provider symbols and a changed return immediate, plus two getter
argument pushes, their caller stack cleanup, and the late node-cell dereference.
Both conditional branches retain their corresponding Original control-flow
destinations. There is no C++ runtime body outside the naked inline assembly.

The current registry section is captured once in ESI and used for both
imports and depth updates. Depth `ADD DWORD[ESI+18h],1` follows Enter.
The node is then pushed, ECX is formed with LEA receiver+1C, and the current
raw unlink symbol is called directly. EDX is left as its current volatile
residual because the leaf's explicit placement argument is unused.

After the leaf, TEST ESI precedes `MOV DWORD[EDI+4],0`; the MOV preserves the
flags used by the following JZ. This clear occurs on both section paths and
uses current EDI without reloading the input cell or substituting EAX. On the
nonnull path actual depth `ADD -1` follows the clear and precedes Leave.
The imports use the repository's inline-assembly DLL-import spelling; their
concrete emitted relocation/target form remains a primary build-review gate.

Raw receiver backing extends through +27h for the embedded list; the node
requires 0x10 bytes and selected writable neighbors under the leaf contract.
The captured section needs valid current Win32 section backing and its raw
depth DWORD at +18h. No null/membership guard, clamp, lock substitute, RAII,
catch, helper, static owner, copied list, producer, profile, or slot binding is
added. In particular, no child node or publication owner is synthesized.

Current-EDI, spill, control-stack, and partial-fault limits remain explicit.
The leaf's spill can alias its writes and change the caller's subsequent
EDI+4 target; the new Source argument cells/frame add their own raw overlap
possibilities. Normal return requires valid selected backing and intact save
slots. There is no owned exception cleanup to release a section after a later
failure. Current Source getter/import exception policy, register residuals,
fault/concurrency behavior, and Original runtime ABI remain separately qualified.

## Static validation and primary gates

The static checker maps all 29 assembly operations to the 25 accepted positions
or four explicit Source additions. It verifies the two branch targets, both
moving-ESP reference pushes, late cell dereference, unchanged EDX handling,
TEST/MOV/JZ order, actual ADD depth operations, and conditional stack balance
with the Source `RET 0Ch`. This is source-text/stack arithmetic review only.

All 12 direct pins from the accepted caller audit still match. Latest leaf
primary inputs remain 53/53 current; getter primary inputs remain 37/38 with
only the already-qualified CMake snapshot drift. Their 48 and 30 Source inputs
respectively match, as do their current document pins. Candidate/current pins
and historical compiled receipts have separate domains in the compact report.

Only the assigned four files change. No CMake/ledger/Ghidra mutation, caller
migration, Native child or import inspection, build, probe, test, or runtime
run occurs. Primary review owns registration and the normal MSVC Win32 build,
whole emitted-body/relocation/stack/EH review, concrete current providers,
unique Core membership, and the current application-map result. Application
retention is not forced. Actual producers, node/list lifetime, profiles,
virtual-slot targets, Original ABI, and gameplay remain open.
