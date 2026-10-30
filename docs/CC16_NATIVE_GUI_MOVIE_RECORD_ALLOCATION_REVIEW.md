# Raw Movie record allocation review

Independent **qualified pass** of candidate
`a10686893116585ca6b7e1d1f848fd9d44781712`, based on published
`60c8453a1b661dfa62625945986e38d71bee0216`. No source blocker was found.
The candidate changes exactly its four declared new source/header/doc/report
files. All four were read; the actual allocation/free service and existing
raw FrameBox allocation pattern remain unchanged from that base.

This accepts a Source allocation-boundary contract, not original heap/exception
identity, final link, caller/owner or runtime admission. The separate range
candidate is not an ancestor of this review tree and supplies none of this
proof. Future common-build results are not inherited.

## Complete native input partition

The configured existing `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`, x86/image400000, were verified by read-only BSP
wrappers; live/snapshot count77631. Pseudocode was checked before complete
assembly/live bytes. Each native body is85 bytes/27 instruction starts:
AACD30..AACD84 and AACD90..AACDE4. ECX is an unsigned count, incoming EDX is
dead, and normal return leaves allocation bits in EAX with plain RET/no stack
inputs. Overflow terminates in the existing native throw call.

TEST ECX/JA sends every nonzero DWORD, including high-bit values, to unsigned
FFFFFFFF/count. Zero skips division but still forms size0 and calls BF681B;
it is not a null-result branch. For positive stride s and nonzero uint32 n,
`floor(FFFFFFFF/n) >= s` is equivalent to `n <= floor(FFFFFFFF/s)`.

| Stride | Last accepted count | Accepted product | First rejected product |
| --- | --- | --- | --- |
| 14h | 0CCCCCCC | FFFFFFF0 | 100000004 |
| 24h | 071C71C7 | FFFFFFFC | 100000020 |

The Source unsigned comparisons implement exactly this partition. Accepted
multiplication fits the32-bit extent; no signed/high-bit acceptance or latent
overflow mismatch was found. Zero takes the same Source allocation arm with
native_bytes=host_bytes=0, so the actual service decides its result/failure.

Both installed PE spans independently match the candidate's raw bytes and
retained live-byte receipts. Native call sites are BF681B allocation,
BF6340 exception construction and BF6885 throw in each body; the existing
read-only checker independently passes all six. Those vendor bodies were not
inspected or reconstructed. Native D6923C/E03CC0 object/RTTI identities and
private-stack exception construction are not promoted into host equivalence.

## Real boundary and matching return

The new fastcall declarations reserve unused EDX explicitly. Both functions
pass object kind and equal native/host byte extents to the real
`singleton_lifetime_allocate`. Its existing source calls actual host malloc,
then `_callnewh(request.host_bytes)` on failure, retries for nonzero handler
result and throws Source `std::bad_alloc` otherwise. It has no zero-size
shortcut or synthetic callback. Its matching `singleton_lifetime_free` calls
the actual host free in the same Source CRT domain.

The independent service object contains those real definitions/import
relocations: malloc, `_callnewh`, and free. This is not proof of an Original
CRT/private heap or arbitrary operator-new/new[] compatibility. The existing
raw FrameBox allocator uses the same explicit count gate, Source bad_alloc and
object/native=host extent policy; its source/header and both service files
are byte/blob-identical to the candidate base. Existing semantic allocation
helpers are not substituted or authenticated by that analogy.

The functions return raw backing without initializing it, storing a pointer
into an owner, constructing records, cleaning partial records, freeing backing,
publishing a container or supplying rollback. The caller retains actual
backing and new-handler/provider/partial-graph custody. Foreign/private heaps,
string-pool buffers and widget-pool slots remain outside the contract.

## Independent strict objects and emission

Fresh MSVC Win32 objects passed the candidate flags:
`/std:c++17 /permissive- /MD /O2 /Gy /fp:strict /EHsc /W4 /WX /c /FAs`,
with default /GS, for the exact candidate allocation source and the unchanged
actual singleton service. No executable was linked or run.

Both public candidate records independently reproduce94 bytes, including one
terminal INT3 byte. The public code bytes and all relocation tuples equal
the retained primary object. Inspection confirms:

- unsigned count compare at+0E and JA at+14;
- count scaling at+16/+20, object kind3 at+19;
- equal request extents stored at+23/+27;
- the real allocation-service relocation/call at+30/+2F, even for zero;
- normal RET at+45; host bad_alloc constructor at+49 and throw at+58;
- no incoming EDX reads or returned-backing dereference/stores.

Security-cookie/global/check, host SDK bad_alloc/RTTI and C++ throw support
are actual Source emission. They are essential qualifications, not native
byte/stack/flags/register-scratch/FH3/SEH/fault or cross-CRT exception proof.
The service's matching definition/import checks establish object-level
availability; primary common build/link and admission remain separate.

All six author Source pins and eleven retained artifact hashes match. The
review report also pins the exact candidate files, unchanged dependencies,
fresh objects, compiler/static-audit receipts and six-call receipt. No source,
CMake/shared metadata/ledger/Ghidra/guard/peer file changed. No new tests,
probe, allocation execution, game, DLL/module load or common build occurred.
Zero new/duplicate/vendor-body, original ABI and activation credit. This
qualified pass does not close vector tidy, AAFEF0/flags1 ownership, records,
factory/Page/decoder consumer or any lifetime/runtime gate.
