# FrameBox length-error transport 133

The whole AD1810 Source entry now directly delegates to the genuine owning
`native_singleton_length_error_00bd0590` service, at the boundary already accepted
for 431F70. It introduces no temporary, catch, exception class, profile, handler,
global, generic `std::length_error`, or fixed Native call. AD2230 failure ownership
and rollback remain unready.

The worker merged published Main `7937c5647cf81d95b7a891ccdaf12752a436d7a2`
with exit 0, producing baseline `9bf3e10e0ab0bbf5e27395e99a385a9e184e5be9`.
Only AD1810, the two existing allocation files, this receipt/report, and the owned
address-shard rows were changed. The existing TU is already registered to
`bsp_core` at `cmake/startup.cmake:1074`; no CMake or normal-host activation changes
were needed. The sealed 130/125/123 evidence families were preserved.

## Exact Native basis

AD1810 is the complete `[00AD1810,00AD1879)` body: 105 bytes and 23 instruction
starts, no consumed incoming register/stack arguments and no normal return/RET.
Its unchanged Original SHA256 is
`d6e4873cdfb4510a0e9cc2cba51f34fd7beddbf39e39ab6760c0b62d869f6408`.
The fresh post-annotation live bytes match the sealed 130 bytes and installed
Original. All live batches verify `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` through the BSP wrappers.

The captured 130 comparison is reused without rerunning its scripts. All 89
nonrelocation bytes match whole BD0590. The only excluded operands are the
four-byte handler immediate and three CALL displacements; every displacement
resolves to the same actual service:

| AD1810 site | Actual target | Observed role |
| --- | --- | --- |
| AD1848 | 408720 | Counted assignment |
| AD185D | 411700 | Legacy logic-error payload construction |
| AD1874 | BF6885 | `__CxxThrowException@8` |

Both bodies initialize only capacity 15, length 0 and the first buffer BYTE of a
no-init 1Ch legacy SBO temporary. They assign the SAME CE37E0 message, exactly 18
non-NUL bytes (`vector<T> too long`); the 19th stored byte is its terminator and
is excluded from count 12h. After assignment returns, state 0 is armed before
411700 constructs the 28h payload. The actual D69260 profile and D83F98 ThrowInfo
operands are shared identities, not inferred class names.

The full 36-byte FuncInfo records at DF051C/DFF4BC differ only in their actual
unwind-map pointer at +08. Both have magic 19930522, maxState 1, zero local-catch,
IP-map and exception-specification fields, and flags 1. Each state-0 map consumes
to -1 and reaches the same effective action, `LEA ECX,[EBP-50h]; JMP 4072D0`.
Both handler tails resolve to BF6B43. Assignment failure is before the outer
completed-temporary cleanup arm; constructor/throw escape cleans the completed
temporary, with the constructor's own partial-base cleanup retained. This static
agreement does not establish modern Source catch/RTTI or Native FH3/SEH execution.

## Actual Source transport and preserved allocator

`[[noreturn]] void __cdecl
throw_native_gui_frame_box_vector_length_error_00ad1810()` is a new Source ABI.
Its sole statement calls the actual BD0590 provider. That existing provider
performs the exact no-init SBO writes and real counted assignment, creates its
`CompletedMessage` guard only after assignment succeeds, and throws the owning
`NativeSingletonVectorLengthError`. The class owns actual 28h legacy storage,
using real 411700 construction, D69260 address data, 411940 copy and 411780
destruction. Guard cleanup uses 4072D0. The profile DWORD is never a callable host
vtable. The ten genuine provider Source files remain byte-identical to 130.

The current allocation family is the existing Source malloc / `_callnewh` retry /
matching free domain. Source compiler exception type, RTTI and C++ EH intentionally
replace Native BF6885/D83F98/FH3. Native catch identity, SEH, private stack/register
aliases, original heap/new-handler state and fixed-VA binary compatibility remain
outside this contract. This is not a `std::length_error` catch-compatible provider.

The two entire pre-existing 124 file byte prefixes are unchanged; new content is
appended in reopened namespaces. The optimized allocator remains exactly 91
bytes, SHA256 `6c8c238a389bd55fbcb92b48e91e24a1e4f0e70a808a2bc67303361996b4594e`,
with all six offset/type/target-symbol relocations identical to sealed 124.
Its count, zero request, allocation/failure domain and output behavior are not
changed. The new optimized adapter is five bytes, one `JMP rel32`, with one
`IMAGE_REL_I386_REL32` relocation at +1 to the real BD0590 Source symbol. It has
no frame, temporary, catch, profile store or normal RET. This is unlinked Source
object evidence, not a reproduction of the Original 105-byte private EH body.

## Verification and annotation

Strict MSVC Win32 `/MD /EHsc /std:c++17 /O2 /Gy /W4 /WX /fp:strict /permissive-`
compilation passed. `strict.asm:183-253`, the COFF operand audit and both old/new
allocator byte captures are sealed. Root reviewed the complete two-file diff and
optimized slice with no correction. One canonical `./scripts/build.ps1` run
compiled the registered TU and passed all three existing checks in 7.02 seconds.
The canonical Core archive, new unit object, actual BD0590 object and check log
are frozen. No new test, behavior probe, adapter execution, Native differential,
game validation or production activation is claimed.

Only AD1810 was annotated through the locked tool. Its established
`STL_xlen_throw_00ad1810` library-role name is unchanged. History preserves the old
name/comment state, the project was saved, and the owned exports were refreshed.
The name shard previously had no reviewed mapping; one owned mapping was added,
along with one owned reconstruction row. All unowned rows and Ghidra signature,
locals and labels are unchanged. The first preservation audit assumed a prior
reviewed-name row; that verification assumption was corrected without repeating
any ledger or Ghidra mutation. Final preservation checks pass 20/0.

The report records the three verified Original call rows, full 130 normal/EH
provenance, exact Source/object/relocation comparisons and unchanged provider
hashes. Private artifacts are sealed in
`local/cc12_native_gui_frame_box_length_error_133/artifact_manifest.json` before
commit and held until Root capture. No parent lifetime or populated FrameBox
production-authority claim follows from this small service adapter.
