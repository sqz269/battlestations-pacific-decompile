# Current-process pipe preimage capture

`CurrentProcessXLivePipePreimageHost` supplies actual allocation and temporary
storage to the recovered protocol/framing bodies. It removes the startup need
for an external heap/stack trace. It observes **this process's** bytes; it does
not reproduce an unrecorded original-game allocator, stack layout or execution
history. The interfaces and capture leaf are new MSVC Win32 bindings, not native
game ABI replacements.

The audit in `XLIVE_PIPE_PREIMAGE_AUDIT.md` established that encoded preimage low
two bits can change frame bytes. Upper six bits are discarded by native masks.
This binding captures all eight actual bits into fully written C++ byte storage,
without choosing zero low bits. Raw CryptGenRandom output uses all64 bits and
does not follow the encoded-byte low-two-bit rule.

## Actual allocation and temporary addresses

The protocol provider now receives `const XLivePipeProtocolNativeState&` naming
the exact allocation made by `Win32XLivePipeProtocolSystemHost::allocate_context`.
That function uses `new T`, not `new T{}` or `new T()`. The trivially default
constructible type leaves its payload bytes unwritten. A `unique_ptr` owns the
new context until capture and the representation copy finish. If a custom
provider throws, the allocation is freed before the original exception propagates.
No protocol locks have been created at this point.

The frame provider now receives `const XLivePipeEncodedWideValue&` naming the
actual stack temporary in the false-key encode branch. Its declaration has no
initializer. The body creates and stores its lock first, captures its72 payload
bytes, writes the observed representation back, then starts the native value
transforms. If the capture provider throws, the temporary's actual lock receives
its deleting destructor with flags1 before the exception propagates. This new
capture-boundary cleanup does not add unwind semantics to the native locked
transforms, whose required host operations still must not throw.

This ordering supersedes the earlier framing packet's explicit pre-lock capture
adaptation. The sequence is now allocation/store-lock, capture actual payload,
native transforms, header copy, lock destruction. No shadow temporary supplies
the claimed preimage. Production system-host constructors still take their same
preimage-host references; only the provider methods gain actual-source arguments.
Recording providers can continue returning deliberate fixture inputs after
inspecting the actual object identity.

## Capture leaf and language boundary

`capture_current_process_xlive_pipe_bytes` is noinline and restricted to audited
MSVC x86. Its compiled body is:

```asm
push esi
push edi
mov esi, source_argument
mov edi, destination_argument
mov ecx, count_argument
rep movsb
pop edi
pop esi
ret
```

It requires valid, distinct, owned ranges and the normal Win32 clear-direction-
flag ABI. It makes no calls, accesses no TLS, and performs no allocation, API,
lock or callback operation. Consequently it preserves LastError without adding
GetLastError/SetLastError calls. The MSVC assembly extension observes machine
storage and writes the whole destination. Subsequent C++ copies use that fully
written representation; ordinary C++ expressions do not read or mask the
indeterminate source bytes. This is not a portable ISO C++ implementation of
indeterminate-value access, and other compilers/architectures deliberately fail
the build instead of receiving an invented fallback.

## Real random API output and failures

The Win32 frame host now passes an actual uninitialized8-byte local array to
CryptGenRandom. Immediately after the API returns, the same leaf copies that
exact region to the typed output and marks all bytes known. It preserves the
actual BOOL result and API LastError, including on failure. Bytes written by the
API and any untouched remainder are all the observed current-process values.

`XLivePipeFramePreimageHost` no longer has an unrelated random preimage method.
The system host's existing `random_output_preimage()` remains for explicit
recording inputs; the Win32 implementation returns an unknown carrier that is
never passed to the actual API. Its zero-initialized backing storage therefore
cannot become a fabricated native failure preimage. Recording system hosts may
still leave bytes unknown and exercise the decoder's existing guard.

Native decode ordering is unchanged: seed writes precede SetLastError0 and the
random call; a false result triggers GetLastError, and only a signed-negative
error returns early. On a signed-nonnegative failure the production host now
has actual observed bytes for the original continuation. No success status is
invented, no failure writes are discarded, and no previous state is rolled back.

## Verification

The strict Win32 Release build and both existing CTests passed. Optimized
production-source assembly confirms the capture leaf above, no context payload
initialization between operator new and capture, and the same uninitialized
random-buffer address used by the API and capture. The allocation has a genuine
RAII unwind path. The temporary captures after its lock is stored.

The existing focused framing/native-loop fixture passed with the source-aware
provider interface and an added throwing-provider cleanup check. One additional
ignored fixture checks actual source identity, default construction over known
fixture patterns, heap deallocation on provider exception, and LastError.
It invokes the real compiled Win32 random-host method with a recording import
inside the fixture process: failure writes two of eight bytes, leaves six bytes
untouched, and sets a sentinel error. All eight observed bytes and the exact
failure/error survive; the recorded success path also passes. The import is
restored afterward. No real CryptGenRandom call, endpoint, thread, SDK account
operation or game launch is performed by that fixture.

Original function evidence and ABI remain in the protocol/framing reports. This
packet changes only their typed preimage boundaries and ownership around added
capture calls; it performs no Ghidra mutation or new native-address reconstruction.
