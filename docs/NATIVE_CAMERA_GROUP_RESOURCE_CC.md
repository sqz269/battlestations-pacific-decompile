# Camera and GroupParams resource items

Packet `orch4_native_camera_group_resource_cc` reconstructs twelve complete ordinary bodies (894 bytes) from the existing BSP program into `native_camera_group_resource.cpp`. Addresses and original ABIs are recorded in `reports/native_camera_group_resource_cc.json`. These are new source interfaces with explicit raw stream/string contexts; they are not binary replacements.

| Native address | Complete operation |
| --- | --- |
| B868B0 / B86890 | Item base construction / destruction |
| B8E5A0 / B8E580 | GroupParams construction / float read and payload skip |
| B8EB50 / B8EBC0 / B8E5C0 | GroupParams parser / scalar deletion / type predicate |
| B8B0B0 / B8B240 | Camera reader / parser allocation and construction |
| B8AA60 / B8B2C0 / B8AA30 | Camera destruction / scalar deletion / type predicate |

B868B0 writes CEB130, reference count 1 at +4, then D631A0. B86890 writes D5C104 and tail-calls the existing BD30F0 base destructor, which writes CEB130. GroupParams construction calls the base and stamps D634B0; its float at +8 remains untouched until the reader stores ST0 directly there. The reader then calls BE9C40 to skip and detach the remaining node payload. B8EB50 allocates 0Ch, constructs if nonnull, captures the current profile/current slot20, and disarms allocation cleanup before the virtual reader call. A failed allocation still leads to a null dereference; no fallback object is introduced.

B8B240 allocates 18h, constructs the base, stamps D632DC, and clears only the name header at +10/+14. It disarms allocation cleanup before directly calling B8B0B0. Neither parser destroys a completed item when its reader fails. The allocation EH states cover construction; their bodies perform only nonthrowing stores/base construction in the source C++ exception domain. Hardware-fault cleanup remains outside this interface.

The camera reader calls BE99D0 twice, storing each result to a 32-bit local. It loads the first float, computes its reciprocal with FLD1/FDIVRP, stores and reloads that reciprocal, calls the x87 arctangent library entry BF8490, and performs both original result store/load round trips before doubling ST0 and storing item+8. It loads the second float and stores item+C. The source naked helper preserves every FSTP32/FLD32, including the repeated intermediate round trip; ordinary C++ arithmetic would introduce an unsupported rounding schedule. The shared host `_CIatan` supplies the established atan boundary already used elsewhere in this repository. Its identity with the original CRT CPU dispatch, control-word/MXCSR branches, special values and math error handling is not claimed.

Remaining camera children are read through BEA680. A nonnull tag matching `TargetName` through case-insensitive BF7FBF reads an actual temporary 8h string header with BEA010. The returned header pointer controls assignment: if distinct from the item name header, resize from its captured length, recheck its current length, then capture current destination length, destination data and returned data in that order before the overlap-capable BF7680 copy. Cleanup captures the local output header's data before disarming its state, then reads its current length+1 and resolves the current raw pool before returning that captured data. It leaves the stale local header unchanged. Unknown or null tags explicitly skip their payload. Recognized chunks receive no extra seek.

The camera reader arms child state0 only after child creation returns and name state1 only after the string read returns. Unwind destroys a completed local name, then releases the child; a second cleanup exception terminates. State becomes -1 before normal child release, so a throwing release is not retried. The original private stack slots alias float scratch, child handle and temporary string storage; those private spill aliases are outside the new source ABI.

B8AA60 captures item name data before arming base cleanup. It returns that data using current length+1/current raw pool without clearing the name header or stamping a derived profile, then disarms cleanup and destroys the base. A name-return exception still destroys the base. Both scalar deletion bodies finish destruction before testing flags bit0, optionally free the captured item, and return its original pointer.

The two 40-byte predicates are instruction-identical to B86950 after substituting only local addresses and the three-cell token ranges (GroupParams 010902E4..EC and camera 01090288..90). They reread current cells in order and stop at the first match; token startup values are borrowed, not inferred. Their executed LEA ESP,[ESP] instructions are included in complete body ownership.

`NativeCameraGroupResourceCalls` composes the two actual parse-slot8 targets and type predicates with existing root/game-resource dispatch and forwards other targets. `NativeCameraGroupResourceReferences` composes actual slot release with BD30E0 and rereads current slot4 before binding either scalar deleter. Numeric profiles are identities, not callable host tables. Complete resource-container destruction and the remaining concrete parsers are separate unresolved work.

## Ghidra evidence

Four returning-free tails were repaired (B8EBC0, B8B2C0, CC2980, CC2AA0), both predicates were defined, and four missing EH dispatch handlers were defined. Instruction bytes and callee no-return flags are unchanged. The four complete EH groups are DFBEDC/DFBEE4 (camera destruction), DFBFA4/DFBFB4 (camera reader), DFBFD8/DFBFE0 (camera parser), and DFC150/DFC158 (GroupParams parser). Nine support bodies have complete instruction ownership. Prior names/comments and mutation receipts are preserved; descriptive names remain hypotheses.

## Validation and limits

Strict standalone MSVC Win32 compilation (`/O2 /fp:strict /W4 /WX /EHsc`) and one controlled-child fixture pass. Project registration/build is pending release of the shared `cmake/startup.cmake` lease; the integration receipt will record the final build revision and results.

The fixture executes all twelve copied ordinary bodies with 30 direct and 9 absolute relocations. It compares constructors, current three-cell predicates, GroupParams payload skipping, camera scalar bit patterns, repeated/case-insensitive TargetName assignment, unknown children and scalar flags2 destruction. Eleven paired payloads include finite positive/negative values, signed zero/infinities, the smallest positive subnormal and a quiet NaN. Both paths use the same host `_CIatan` and existing concrete read/string/pool dependencies. Only the copied GroupParams parser's profile immediate is rebound to a fixture table with copied B8E580 at slot20, then restored for normalized comparison. Original EH paths are guarded out.

Two source failure checks cover a string-length read throwing before local name completion and a normal child release throwing after disarming cleanup. They verify the resulting child/path/reference state and retained item contents before explicit fixture cleanup. They do not prove native FH3/SEH handling.

The integration case constructs the actual raw28h manager with all six registered parser singletons, constructs an actual44h default resource, and feeds GroupParams and camera chunks through B7F430/B7E970. It checks ordered item storage, parsed fields and node/path budgets, then releases actual item slots through their reference/deleting targets and drains the actual manager/parser singleton registrations. Resource-container cleanup itself is explicit fixture cleanup. The pools are preconstructed. Native token startup, original library/ABI identity, lazy pool recreation, allocation failure, full resource-container destruction, executable admission and gameplay remain unverified.
