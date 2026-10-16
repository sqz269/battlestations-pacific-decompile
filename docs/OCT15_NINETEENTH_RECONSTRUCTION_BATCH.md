# Nineteenth reconstruction batch

Two complete ordinary routines, one lexical catch and one logical cleanup action
cover 1,061 distinct native bytes and 312 instruction starts. The ordinary bodies
account for 1,036 bytes and 305 starts; reference bodies, producers, handlers and
compiled Source code have separate counts.

The constructor preserves count capture before destination clearing, allocation
outside the protected scope, returning diagnostics and current-header cleanup
followed by rethrow. It uses the accepted allocation, diagnostic and copy/tidy
services without a new allocator, callback, rollback or count bridge.

The tree eraser advances its private iterator before topology changes, preserves
the ordered transplant and six rotation sites, and cleans the original node's
payload before releasing the node. Pair failure propagates before free, count
and output commits. The current count is read after free; every nonzero DWORD
is decremented. Output owner and node are captured before either output store.
Only the completed error message has an outer cleanup action, armed after its
assignment. Existing narrower actual interfaces and full-selection prefix remain.

Primary review passed 29 checks with no findings. All 33 component report rows
and 33 Markdown transfer claims pass. Combined Source at dcfadb3589ba9cf107ea5bb6e41b540130efc185
passed MSVC Win32 and all 3 existing suites in 7.51s. Worker builds
passed 3/3 in 7.94s and 7.32s, with focused COFF checks. Four saved annotations,
a forced snapshot and four refreshed exports completed. No new tests were added.

The eraser adds RawCtx and explicitly changes Source RET10h versus native
RET0Ch. CurrentCpp/currentCRT, private frames, fault behavior, original
FH3/SEH/RTTI, binary ABI and game equivalence remain unproved. Existing native
math differential checks cover older seeds, not these new entries. Whole range
erasure, backward swaps, Tidy helpers and remaining range cores are separate work.
