# Native particle text file loader

## Scope and evidence

`load_native_particle_text_buffer_00af5850` reconstructs the complete 221-byte
`00AF5850..00AF592C` body. The original ABI is ECX = actual 1Ch text buffer,
one nullable C-string stack argument, EAX = actual read count or zero, RET4.
The descriptive name is a hypothesis. Prior Ghidra name was `FUN_00af5850`,
with no comments. The report preserves the prior complete documentation.

The installed PE and live `bsp.gpr` / `/battlestationspacific.exe` bytes agree
for the full body, the BF55BE allocation thunk, both supported manager profiles,
and all four supported stream profiles (eight spans total). Full image and span
SHA256 values, bytes and all ten body call sites are in
`reports/native_particle_text_loader_orch4.json`. The repaired listing reaches
both RET4 instructions. Full-byte disassembly counts 88 instructions; the
bridge's older function statistics still report 81. Source follows the complete
listing, including the short-read tail. No Ghidra writes belong to this packet.

## Concrete composition

The API borrows the actual current `0109CEEC` publication cell,
`NativeVfsRuntimeBindings`, `NativeVfsNameResolutionContext`, and
`NativeStringRawPoolContext`. The lookup and logging contexts share the existing
`ActualNativeStringPoolStorage`; it and the raw string context must use the same
`01090AA8` pool, `01090AA4` return gate, and `01090AA0` singleton manager cells.
No private pool, substitute container, invented global or result callback is
introduced. Native profile words remain readable at their original addresses.

The existing concrete dispatcher supplies manager slot4 `BDF310`, stream slot30
length (`BEF600`, `BF4F90`, `BF10A0`, `BBBDD0`), stream slot24 read (`BEF590`,
`BF5030`, `BF1000`, `BBC140`), and the current terminal. Physical terminal
`BF55A0` recycles without a second decrement. `BD30E0` follows its supported
current scalar slot4 through the existing dispatcher. Unknown profiles remain
explicit source boundaries in those providers.

The caller passes a fresh `NativeVfsNameResolutionAcquired` by reference. If
resolution fails, it must retain **both that frame and the actual text buffer,
including its name header and data**, while diagnosing the interrupted work.
The existing resolver terminates if a failed/incomplete frame is destroyed.
This loader neither creates a local frame nor changes that policy. Subsequent
provider exceptions propagate without adding cleanup absent from AF5850.

`GameNativeVfsRuntime::borrow_raw_services()` now exposes references to the
existing publication cell, runtime bindings and name-resolution context without
constructing another manager; see `NATIVE_PARTICLE_VFS_SERVICES_ORCH4.md`.
The subsequent raw AF4BA0 parser and 86BA60 resource loader compose this body
and retain its resolution invocation. The outer 00870DD0 cache acquisition
algorithm is now complete; see `NATIVE_PARTICLE_RESOURCE_ACQUISITION_RAW_ORCH4.md`.
Application context installation remains separate work. See
`NATIVE_PARTICLE_RESOURCE_LOADER_RAW_ORCH4.md` for the current caller contract.

## Ordering and lifetime

The raw eight-byte name header lives at text+C. Resize uses the nullable input
length and preserve=false; copy uses current destination data and length. BF7680
retains its correct `_memcpy` library name in Ghidra; the source uses `memmove`
to cover that native primitive's overlapping-range behavior.

Resolution captures the current VFS manager and ignores the returned boolean.
Open reloads the publication and current slot4, passing the current name and
flags32h. Length dispatch uses slot30 and only its low DWORD. The loader writes
text+0/+8 to that extent, text+4/+18 to zero, then allocates exactly that extent
through BF55BE's existing BF681B allocation provider. No terminator is added.
The current requested length is read before publishing the buffer at text+14.

There is one current slot24 read. Its output count starts with the incoming name
pointer bits, because native code reuses its argument word as the output cell.
The returned count is compared with the current text+0. On mismatch, the loader
frees the **current** text+14, clears it, and returns zero without releasing the
stream. On equality, it decrements the captured stream's refcount once, loads
the current terminal only if zero, and returns the count after that call.
It does not free an older text buffer before replacement, recover null streams,
validate lengths, retry reads, roll back state, or add an EH cleanup frame.

## Verification and limits

- Plain `scripts/build.ps1`: strict Win32 build and both existing CTests pass.
- Ignored focused probe: copied original AF5850 bytes and reconstructed source
  both load the installed `Scripts/fundamentals.lua` through the genuine raw
  manager, physical provider, resolver, pooled HANDLE stream and current string
  pool. All 657 data bytes, normalized name bytes and output fields agree.
  Successful terminal recycling and complete raw singleton/pool drain pass.
- The original probe redirects its five direct calls, two publication operands,
  one IAT operand and four physical dispatch entries to the same genuine source
  providers. Original table bytes are mapped read-only, not replaced by host
  tables. The fixture's BF0FB0 adapter calls the actual provider body and supplies
  no synthetic resolver result. No permanent test suite is added.
- All five direct body calls and the allocation thunk tail are checked by
  `verify_report_calls.py`; indirect sites are separately tied to listing bytes
  and the captured-slot contracts in the report.

This establishes complete loader source and a normal-path original/source
fixture comparison. Short reads, provider exceptions, callback mutations and
other stream profiles are listing-grounded, not exercised by this probe.
Original argument-slot alias identity, register ABI, native CRT identity,
hardware SEH and gameplay compatibility remain unvalidated.
