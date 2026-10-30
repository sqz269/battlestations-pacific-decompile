# Singleton cleanup raw DWORD access

This packet changes raw accesses in `00411EE0` and `00412430` only. It adds no
entry point, caller, singleton, provider, allocation, or startup activation.
`source_ready` remains false for the unbound raw locale-manager route.

The source baseline is `b7f934b42056325c4d79a5f4b868847cd358b963`.
Saved evidence is `exports/bsp/functions/00411ee0/assembly.txt` and
`exports/bsp/functions/00412430/assembly.txt`, read without Ghidra mutation.

## Exact access order

`00411EE0` is the ECX eight-byte guard cleanup with plain RET. Native
`411EE0` captures the DWORD at guard+4 before `411EE5` stamps `00CE37FC` at
guard+0. A null captured pointer skips the decrement and leave. Otherwise
`411EED` subtracts one from the CURRENT DWORD at captured-section+18h using
32-bit wrapping arithmetic, then `411EF2` calls the actual
`LeaveCriticalSection` import through `00CE2210`.

The Source entry now copies these DWORDs through byte storage into local
`uint32_t` values, with `_ReadWriteBarrier` around each copy. It decodes the
captured 32-bit pointer only for the nonnull branch. It still captures before
the profile store and reads the current counter after that store; it neither
clears guard+4 nor calls a provider on the null branch. Its existing signature
and lack of `noexcept` are unchanged.

`00412430` is the seven-byte ECX receiver cleanup with plain RET. Native
`412430` writes `00CE3818` at receiver+0. Source retains its existing `noexcept`
API and uses one fenced four-byte copy. It performs no pointer/header read.

These entries no longer require C++ DWORD field objects at the raw accessed
addresses. Local typed variables and existing actual eight-byte local guards
remain ordinary C++ objects. The shared `tracked_counter` and `depth` helpers,
all other functions, native calls, cleanup states, signatures and policies are
unchanged. Header edits document only these two APIs.

## Caller and provider limits

The caller must provide valid readable guard bytes [0,8), writable profile
bytes [0,4), and, for a captured nonnull section, its actual initialized Win32
critical-section lifetime plus readable/writable counter bytes [18h,1Ch).
`00412430` requires writable receiver bytes [0,4). Backing and captured
identities must remain valid throughout the accesses and real leave call.
Serialized access, correct entry/exit balance, current SDK provider validity,
and failure retention remain caller obligations. Compiler barriers are not
hardware fences, atomics, locks, or a concurrent-writer contract.

This closes the specific transitive typed-field prerequisites identified in
the dormant raw locale constructor revision `d5c116833d55eeb91ccd84756792dd8f473bbeb3`.
It does not close that constructor's publication, canonical manager, allocator,
provider, lifetime or startup obligations. It does not reconstruct Original
register ABI, FH3/SEH/fault behavior, a Windows/vendor body, or the whole
dependency closure. Existing armed-through-leave cleanup behavior is preserved;
no rollback, counter repair or exception suppression is added.

## Validation

Both affected translation units compiled once in a fresh CMake OBJECT target
with MSVC Win32, C++17, `/O2 /W4 /WX`; both objects are COFF I386 (`014C`).
The complete old CPP files are recovered byte-for-byte after projecting only
the two replacement bodies and their added includes back to baseline. The old
headers are recovered after removing only the four new comment lines.

Preserved source/object baselines, command/log receipts, saved assembly hashes,
and projection results are in ignored
`local/native_singleton_raw_cleanup_validation_20261030/`.
No test, probe, game, executable target, full build or runtime validation was
performed by this packet. Root owns the coupled Win32 build and metadata.
