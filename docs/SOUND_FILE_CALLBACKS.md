# FMOD file callbacks

The four pointers installed by `00a88770` at `00a88961` now have callable MSVC
Win32 implementations in `src/sound_file_callbacks.cpp`. Their stack signatures
come from the complete instruction bodies, not Ghidra's zero-argument prototypes.
The names below are descriptive hypotheses, not recovered symbols.

| Address, body end | Native ABI and behavior |
| --- | --- |
| `00a7d410..00a7d555` | `__stdcall(const char* name, int unicode, uint32* size, void** handle, void** userdata)`, `RET 14h`, EAX result. Construct a pooled native string, call VFS resolve `00bdf4c0` and ignore AL, copy the resulting name, open through manager `+4` with flags `2`, publish the handle, and release the copy. Null handle returns `17h` without writing size. Otherwise call stream `+2Ch(NULL)`, store EAX as size, release the first string, return zero. Unicode and userdata are never read or written. |
| `00a7b750..00a7b774` | `__stdcall(void* handle, void* userdata)`, `RET 8`. Null handle is accepted. Otherwise `InterlockedDecrement(handle+4)` and call the current stream virtual `+0` only at zero. Return zero. No extra retain or explicit physical close occurs here. |
| `00a79930..00a79963` | `__stdcall(void* handle, void* destination, uint32 requested, uint32* actual, void* userdata)`, `RET 14h`. Invoke stream `+24h(destination, requested, &local)`, optionally publish actual, return `16h` iff the unsigned actual count is below requested; otherwise zero. The incoming handle stack slot is the local, so its initial value is the handle bits. Stream return registers and userdata are ignored. |
| `00a79970..00a79988` | `__stdcall(void* handle, uint32 position, void* userdata)`, `RET Ch`. Call stream `+1Ch(position low, 0 high, 0 origin)`. Always return zero after it returns, including a seek reported as failed by a typed host adapter. |
| `00be41a0..00be41b3` | Direct bounded dependency: `__thiscall(ECX stream, uint32* optional_high)`, `RET 4`. Invoke the current `+30h` virtual returning EDX:EAX size, optionally store EDX through the argument, preserve low EAX as return. The public reconstruction has a new C++ entrypoint and an adapter bridge. |

The short-read and failed-open numbers are preserved as literal `16h` and `17h`.
The callbacks do not translate the underlying stream's error into another FMOD
result. A seek that returns failure still yields zero; a read whose count output
was not written retains the incoming handle bits. No error recovery is invented.

The original open has two distinct pooled-string lifetimes. The implementation
uses the canonical eight-byte `NativeString` and `NativeStringStorage`; the copied
name is released before testing the newly published handle. The resolved name is
released after the size query or on the null-handle return. C++ cleanup also runs
when supplied VFS operations throw; this is not a reproduction of the executable's
SEH frame layout. FMOD hosts must arrange a nonthrowing operational path.

## Native stream and VFS contracts

Live memory-stream table `00d642c0` contains `+0=00bd30e0`, `+1Ch=00bef540`,
`+24h=00bef590`, `+2Ch=00be41a0`, and `+30h=00bef600`. The final-reference
invoker calls the current deleting destructor; the callback itself only performs
the interlocked decrement and virtual dispatch. It does not invoke a deleting
destructor with an invented flag or rebuild a retained owner.

`PhysicalFile::read_00bf5030` is the established synchronous physical read and
updates its count and cached cursor even on failure. `MemoryStream::read_00bef590`
clamps to remaining bytes and updates the cursor and count on a successful read;
its host guards leave those outputs untouched on failure. The existing physical
and memory seek and size methods supply the other operations. Details remain in
`docs/PHYSICAL_FILE.md` and `docs/MEMORY_STREAM_HANDOFF.md`.

The canonical native retained-owner reconstruction stores original image profile
words and explicitly dispatches lifetime operations using its supplied context.
Those words are not callable host read/seek tables. The new factories therefore
make an explicitly partial native-callable adapter around an existing
`shared_ptr<PhysicalFile>` or `shared_ptr<MemoryStream>`. The adapter has a real
interlocked count at `+4` and only callable slots `+0`, `+1Ch`, `+24h`, `+2Ch`,
and `+30h`. Other slots are outside its domain, with no no-op virtuals supplied.
One returned reference is transferred to FMOD; the final close destroys the
adapter and releases its retained typed owner. The same cursor is retained without
cloning, resetting, or silently buffering it. These adapters are not complete
native stream objects, native pool owners, or whole-stream ABI replacements.

`sound_file_stream_adapter_status` exposes the actual typed operation success and
physical DWORD error while the adapter is alive. It never changes the callback's
result or repairs a failed seek/read. A memory guard failure has no physical error
code and is reported as failure with zero system error. Each stream cursor must be
used serially, as required by the existing stream classes.

`NativeSoundFileContext` makes global VFS `0109ceec` explicit. Its resolve binding
accepts the mutable canonical string and must preserve failed-resolve mutations;
its open binding receives a distinct copy and flags `2`, transferring one retained
stream reference. Existing VFS projections can compose
`resolve_existing_resource_00bdf4c0_fragment` with
`open_resource_memory_00bdf310_fragment` and then the memory adapter; that preserves
their documented provider selection, buffering, and error boundaries. This packet
does not duplicate or replace VFS loading, archive decoding, mounts, or candidates.

Bind the context before installing `SoundFileCallbackBundle` in FMOD. Keep the
context, storage, VFS state, and binding fixed until FMOD has stopped callbacks and
released every file handle, then restore the previous binding. The original open
has no usable userdata input and never initializes userdata output; this is why
the binding is process-wide. Unlike native code that reloads the VFS global after
resolve, the supported host domain requires the binding to remain fixed.

## Evidence and validation boundary

Every live query/export verified project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base `00400000` through
`bsp.py`'s verified client. The parent repaired the missing read, seek, and size
helper function definitions under the Ghidra write lock after inspecting their
bounded raw bytes. No function was imported into another project.

Read and seek contain only local control flow and indirect calls through the
supplied stream. Their full bytes were compared with the installed PE and then
executed in an isolated local probe against the same real memory and physical
adapters as the reconstructed callbacks. The probe covers full and short reads,
EOF, zero-length reads, optional count output, absolute seeking, and the unchanged
count behavior of a typed memory guard failure. Open mutation/copy/order and
failure outputs, ignored userdata, seek-error result preservation, null close,
and retained/final close were checked in the same focused probe.

The ignored local probe and fixture are under `local/sound_file_*`; no file was
written into the game installation. `reports/sound_file_callbacks.json` records
commands, hashes, and outcomes. Native differential claims are limited to read
and seek on these adapters. Open and close are fixture-tested. The callback
signatures use the recovered stdcall ABI, but neither complete stream ABI parity
nor game validation is claimed. The parent's actual FMOD integration and runtime
probe are separate evidence.
