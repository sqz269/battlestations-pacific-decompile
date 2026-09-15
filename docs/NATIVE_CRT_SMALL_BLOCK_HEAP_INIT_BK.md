# Native CRT small-block heap initializer BK

Address `00C11CF5`, complete 72-byte body. The current Ghidra
`/battlestationspacific.exe` bytes equal the installed PE bytes, SHA-256
`b40f758cf3227398a781e6c18edbc7e2366130a0a96949531ca7b7ac5846b9ec`.
The accepted discovery is `docs/native_crt_heap_frontier_bk.md` and
`reports/native_crt_heap_frontier_bk.json` at
`c7a9ae6877b711c7a7e196b4039b5ae4b71373ee`; the report SHA-256 is
`e5c621c0ab5dee303af97ab4fb176b67cb1762ad1a3d9607db1b7396ea0c5c8e`.
Its 114 local evidence files are retained unchanged.

The original function is cdecl with one stacked DWORD threshold, EAX 0 or 1,
plain RET and caller cleanup of four bytes. Its only call is the indirect
`KERNEL32.dll!HeapAlloc` import at `00C11D02` through IAT `00CE20F8`.
It reads the current owning private CRT heap word `0109E1BC` and requests
`140h` bytes with flags zero. The result is published to `0109ED68` even
when null. Null returns zero immediately and leaves the other five output
words untouched. The allocation is not zeroed here.

On success, native `AND mem,0` reads and clears `0109E310`, then
`0109ED64`. It publishes the saved allocation to `0109ED70`, writes the
incoming threshold to `0109ED6C`, writes `10h` to `0109ED74`, and
returns one. The C++ interface uses borrowed volatile references to these
actual words and calls the real Win32 `HeapAlloc` service with the current
heap handle. It introduces no state, allocator callback, host `malloc`,
allocation clearing, mode selection or free operation.

The C++ call frame and IAT belong to its build, so this source is not an
original binary ABI thunk. It assumes the references bind to the current
owning CRT state and the heap handle is valid in the calling process.
Initialization of that heap, mode selection, SBH allocation/free, native
exception identity and gameplay remain separate. Compiler command/read/write,
exact object and unique archive-member evidence, both existing CTests and
double-hash inventories are in `reports/native_crt_small_block_heap_init_bk.json`.
