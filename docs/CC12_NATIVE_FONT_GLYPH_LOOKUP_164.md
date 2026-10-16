# Raw Font glyph iterator lookup AD4540 / AD4370

The two raw lookup APIs are now installed as a literal append in the existing registered Core translation unit. Production-path strict compilation and one required worker build with all three existing checks pass. The earlier local draft was compiled while production paths were queued; its evidence remains separately labeled. No lookup-body execution or new behavior test is claimed.

The change adds one actual raw unsigned-WORD lookup body, `find_native_font_glyph_iterator_00ad4540`, and a distinct `find_native_font_glyph_iterator_00ad4370` entry sharing it. Both use actual Font/tree BASE, raw output8 and a required current key WORD reference. The old typed `FontData` selectors, complete 146/155 Source prefixes and their existing services remain unchanged. This does not establish populated glyph production or the whole AD4C30 load.

Preparation used `715f8a78bbe97b1f2bad83630f55f5f293541446`, retaining accepted 155 and read-only 161, with 155 then still separate from 161's published `6ef581090021e4aac6b4df200e77a6561a1db675` snapshot. The worker waited in the fair Source queue until Root159 published/sealed/released. It then acquired the two Source leases, refreshed to published `f51ffe7eb4a2d3ea2f88fd5a38d567987a09c0ed` under an exact 39-file temporary lease, checked merge exit zero and released that refresh lease before editing. That published commit is the final Source/build baseline. The 146/151/155/161 evidence families are copied/read only and never regenerated.

## Complete Original and Source ABI boundary

Each whole body is 109 bytes/39 physical starts: AD4540 `[00AD4540,00AD45AD)`, AD4370 `[00AD4370,00AD43DD)`. Their complete bytes are identical, including all branches. The unchanged installed Original, current verified Ghidra BSP target and saved physical listing starts agree for both bodies. There are no direct/indirect calls, no local EH and no allocator, validation service or throw continuation in either function.

Original ECX is the actual Font/tree base; stack arguments are output8 and a key pointer. EAX returns SAME output8, with `RET 8`. The new interfaces explicitly use ordinary Source `__cdecl` with three arguments, including the required `const volatile std::uint16_t&`; they are not drop-in Original stack/register ABIs. The Ghidra library-role names `STL_Tree_LboundU16_00ad4540` and `STL_Tree_LboundU16_00ad4370` are retained. The final equality check follows a lower-bound scan; these are not unbounded semantic-container lookups.

AD4540 has the one Source implementation. Its `__declspec(noinline)` prevents the compiler from cloning that scan into the alias. AD4370 ends in a real `E9 rel32` to the actual decorated Source implementation symbol: five bytes in strict mode; nine in Core with its four-byte frame setup/teardown before that same tail jump. Original AD4370 remains a full 109-byte byte-identical body, not an Original thunk; the thin alias is a Source code-sharing choice. No fixed Native callable VA, fake callback, parallel tree, global or default provider is added.

## Exact current reads and outputs

The shared Source body preserves this full sequence:

1. Read CURRENT Font+04 as candidate head, then CURRENT head+04 as root; test root's nil BYTE+15.
2. Only if the root is nonnil, read the actual key WORD once. Keep that captured value through the traversal. At each node read unsigned WORD+0C; smaller follows CURRENT right DWORD+08, otherwise capture that candidate and follow CURRENT left DWORD+00. Test each reached next node's nil BYTE+15 before repeating.
3. After the traversal or empty arm, reread CURRENT Font+04 as the current head. If candidate equals that head, do not read the key or candidate-key fields.
4. Otherwise read the CURRENT key WORD again, then the candidate's unsigned WORD+0C, in separate statements. Only final key >= candidate key chooses the candidate; otherwise choose current head. This is the exact-hit decision after the lower-bound traversal.
5. After all input reads, store the SAME Font owner DWORD at output+00, then the chosen node DWORD at output+04. Return SAME output8.

On an ordinary genuine empty tree, neither key-value read occurs. No count+08, opaque Font+00, color BYTE+14, payload DWORD+10, descriptor/name field or unreached candidate-key field is read. A fabricated count/empty shortcut, unconditional key snapshot or returning-null default would change this body and is not present.

The first key capture and final key/candidate loads are explicit separate volatile operations. They do not rely on C++ binary-operand evaluation order or freeze the final key across traversal. The current head is also loaded separately after traversal. The output values are captured before the first output store, matching the original selected private pair's owner/node capture before its writes.

The complete strict scan is 94 bytes/37 physical starts with no relocation or call, both in the local draft and the final production-path compile. Its initial nil test/branch are +11/+15; the first key read is +17 only on the nonnil arm. Current head is reloaded at +3B. The candidate==head branch at +40 skips final key +42 and candidate key +45. Owner store +55 precedes node store +58. The compiler places a saved-register pop between those stores; that is not a raw graph read. The strict alias has one REL32 relocation at +01 naming the actual Source lookup.

Actual Core emits a different 94-byte/40-start scan using an EBP frame and a nine-byte/four-start alias (`PUSH EBP; MOV EBP,ESP; POP EBP; E9 rel32`). Its alias relocation is at +05. Its complete operand sequence is independently frozen; equal scan sizes do not imply strict-versus-Core byte identity. Neither variant reads graph/key data after the first output write. Private frame/epilogue differences remain outside the Native ABI/fault boundary.

## Raw storage, lifetime and overlap

The required tree header is the actual Font BASE with head+04. The sentinel and reached nodes have actual link DWORDs +00/+04/+08, unsigned WORD key+0C, payload pointer DWORD+10, color BYTE+14 and nil BYTE+15. The head/nodes are genuine initialized storage in the already admitted MSVC Win32 implementation/SDK graph domain; a semantic `FontDescriptor`, `FontData`, `std::map` node or copied header is not substituted.

The caller independently retains the receiver, sentinel/nodes, key object and writable aligned output8 until their reached accesses finish. This function neither establishes nor ends that storage lifetime. Existing SAME SDK allocator/process, partial-failure and caller-retained graph obligations from 146/155 remain unchanged. It adds no owner, initializer/completion counter, payload retain/free, cleanup action or retry policy.

The new Source key reference has a valid-object/type/alignment precondition even on the empty arm. A Native invalid/unread pointer that happens not to be touched on a nil root is not thereby a valid Source reference. Every reached raw read/store must be valid; no invented input validation converts malformed or retired graphs into a success result. Externally serialized access is required; volatile is not concurrency synchronization.

Valid public overlap follows the actual schedule: all input operands are consumed before output owner then node stores. For example, this body does not add later tree/key reads after the first output write. The caller still supplies valid storage and accepts any graph bytes overwritten by its chosen output region. Aliasing into EITHER the Native or new Source callee's private stack, argument slots or scratch storage is excluded, along with unowned spill slots, invalid object lifetimes or concurrent mutation. Ordinary caller-owned key/output objects are the public input/output domain.

Original private pair storage, registers, flags, fault timing and stack layout differ from the ordinary Source frame. No local cleanup is synthesized. The absence of calls/EH in this leaf does not prove a Native hardware-fault/caller-unwind ABI, populated-tree producer, complete Font load/retirement, Text/font association or game runtime/activation behavior.

## Preservation and validation

Before any production edit, the complete Source155 header/CPP prefixes and immutable 155 strict/Core objects were copied after checking their selected manifest entries. Root159's immutable manifest `82d9c80d92e26bedded6e4b8150b01a75d185481b33a4963479dfa755d7b12af` and actual Core object entry were also verified and copied after release. All four old executable COMDATs are tracked: allocator (strict108/Core107 bytes), constructor356 including its catches, generated constructor handler39 and `raw_context` accessor4; their ordered relocation counts are respectively 3/8/4/0. Comparison is within each corresponding compiler mode. No strict108-versus-Core107 allocator equality is asserted.

The local draft was literally the pinned full 155 files plus the two approved append blocks. After installation, Root's requested appended-header comment clarification excludes both callees' private storage; declarations and CPP algorithm did not change. The final strict Win32 C++17 `/MD /EHsc /O2 /Gy /W4 /WX /fp:strict /permissive-` compile succeeds. All four old code sections and every ordered offset/type/symbol relocation remain unchanged for strict155→strict164, canonical155→Core164 and actual canonical159→Core164. Both complete 146/155 Source file prefixes remain byte-identical. The old constructor's ordinary Source cleanup/exception and compiler FH3-placement limits are neither repaired nor relabeled by this lookup packet.

`scripts/build.ps1` completed once after the production C++ edits, compiling this actual registered Core TU. All three existing checks passed in 7.69 seconds. No new test, behavior fixture, probe or lookup-body execution was run. Whole Original/static operand review supplies the branch/read evidence; a report-call verifier checks zero rows because both Original functions genuinely contain zero calls. Zero-row verifier success is not itself branch/call-order proof.

Only the two owned library-role-name naming/function records are added, with old comments and unrelated shard rows preserved. Annotation uses the Ghidra write lock, prior-value history, save and forced refreshed exports for these two entries only. The final machine receipt pins final Source, actual objects/commands, optimized full scan/alias and all old bodies, Original/listings, build/check logs and owned annotation/history before the immutable seal and exact owned commit. Canonical evidence is taken from the actual worker build, not inferred from the local draft.
