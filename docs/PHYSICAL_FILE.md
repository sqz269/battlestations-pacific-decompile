# Physical file stream read-only reconstruction

`PhysicalFile` in `include/bsp/physical_file.hpp` and `src/physical_file.cpp`
implements the established Win32 physical-stream path needed by native texture
and script reads. It owns a handle and preserves the native cached position and
size behavior. It is a typed C++ interface, not the original 20h-byte polymorphic
object, and it does not implement the VFS, archives, allocator pool, reference
counting, write modes, or native error callbacks.

See [TEXTURE_VFS_HANDOFF.md](TEXTURE_VFS_HANDOFF.md) for the preceding manager,
mount and provider dispatch. The installed game files are read-only inputs.

## Behavior and ABI evidence

Native constructor `00bf50d0` has ECX destination, no stack arguments, plain RET.
It installs table `00d691b0`, reference count `+4 = 1`, handle `+8 = -1`, cached
position `+10h/+14h = 0`, and size `+18h/+1Ch = 0`. Native `+0Ch` is not set.
The host default constructor projects the handle and two cached values only.

`open_read_only_00bf52a0_fragment` implements only original flags value 2 in
`00bf52a0` (ECX stream, stack native path-string reference and flags, RET 8).
It calls `CreateFileA` with GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, null
security/template pointers and attributes 0. A null native string data pointer
uses the empty string; the host accepts a null ANSI path with the same fallback.
On success it queries size using `GetFileSizeEx`. It always zeros cached
position after the open attempt, and does not otherwise clear cached size.
An open failure after a prior close therefore leaves the previous cached size,
matching the fields written by the native body.

Native open ignores the size-query result and logs failed opens. The host
returns Win32 errors explicitly. If opening succeeds but size querying fails,
it returns false while still owning the valid handle, which will be closed by
destruction or explicit close. It rejects opening an already-open host owner
with ERROR_ALREADY_EXISTS and leaves it unchanged; native would overwrite its
handle without first closing. This is an explicit ownership guard, not a
reconstructed native branch. Other flag modes, directory creation and logging
in the full 589-byte native routine remain unported.

`valid_00bf5020` (native ECX stream, AL result, plain RET) checks only handle !=
INVALID_HANDLE_VALUE. `size_00bf4f90` (ECX stream, EDX:EAX result, plain RET)
returns the cached 64-bit value; it does not re-query the file.

`seek_00bf4f20` (ECX stream, stack 64-bit distance followed by DWORD origin,
RET 0Ch) calls `SetFilePointerEx` with the cached-position destination. The host
uses the same arguments and preserves the previous cached bits unless the API
writes a new result. It does not clamp distance or update size when seeking
beyond end. Win32's success/error result is exposed.

`read_00bf5030` (ECX stream, stack destination, DWORD byte count, optional actual
count pointer, EAX actual count, RET 0Ch) performs one synchronous `ReadFile`
with actual count initialized to zero. It adds the actual count to cached
position with unsigned 64-bit wrap matching the native ADD/ADC, then supplies
the actual count to the caller. EOF is a successful zero-byte read; a short
read is not retried. The host interface returns bool plus actual count and
Win32 error, instead of invoking `00bd9e30` on failure using manager `+18h`.
The native callback's behavior is not established; the host continuation on
failure must not be described as complete native error-path parity. No new
test suite is introduced for this adapter.

## Handle destruction versus storage recycling

The destruction dependency is explicit in the assembly:

- `00bf5090`: ECX stream, stack destruction flags, EAX owner on the non-free
  path, RET 4. It installs table `00d691b0`, calls `CloseHandle` on `+8`, sets
  `+8 = -1`, installs base table `00d5c104`, calls base destructor `00bd30f0`,
  and frees storage only if flags bit zero is set. It does not zero size or
  position. It calls CloseHandle even for an invalid handle and ignores errors.
- `00bf55a0`: ECX stream, no stack arguments, plain RET. This final-reference
  virtual slot calls stream virtual `+4` with flag zero, then acquires the pool
  using `00bf42a0` and appends the destroyed storage via `00bf5190`.
- `00bf5190`: ECX pool subobject, stack storage pointer, RET 4. It enters the
  optional lock obtained via `00b1cd90`, grows pointer capacity to twice the
  previous capacity (at least one) when full, appends the pointer and increments
  count, then leaves the lock. This recycles object storage; it does not keep
  the file handle open.

`close_00bf5090_fragment` and the host destructor implement only handle ownership
and invalidation. Explicit close returns any Win32 failure and invalidates the
stored handle regardless of that result, as the native destructor does. It
leaves cached size and position intact. An already closed host owner is a no-op,
avoiding the native redundant CloseHandle(-1). Copying and moving are disabled
to prevent duplicated ownership. Native vtable transitions, interlocked counts,
optional pool locks and allocation reuse are not reproduced or stubbed.

## Verification boundary

Parent integration: the existing D3D9 probe now opens its installed DDS atlas
through this class, seeks to the beginning, reads once, checks the actual count
and cached position, and explicitly closes the owner. The 524416-byte input
passed the existing managed DXT1 texture byte comparison, atlas checks and the
remaining renderer probes. The Win32 build and both existing CTests passed.
The probe retains its own size/short-read rejection; it does not claim the native
VFS or memory-wrapper conversion has been implemented. No new test suite was added.

The reconstruction is based on assembly and pseudocode from
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; project and program were
verified before each read batch. The implementation agent did not run a build
or modify the probe, CMake, shared metadata or Ghidra. Integration should use
the existing probe for an installed-asset open/seek/read/size/close check, plus
the normal Win32 build. Such a check establishes this physical-read path only;
it does not prove native mount selection or a runnable game.

The complete native body ranges below matched the saved image and installed PE
byte-for-byte. End addresses are exclusive; ignored detailed evidence is in
`exports/bsp/owner_textures/vfs/physical_file_evidence.json`.

| Start | End | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `00bf5090` | `00bf50cd` | 61 | `0de76ba2958c0356eed853c963f3ffcf835038045adc7aa6b63c7f54da2933da` |
| `00bf55a0` | `00bf55be` | 30 | `cace622e7a8516c941d2ed3e39d4c264cdf16957dd618e952d74cc7198183572` |
| `00bf5190` | `00bf522b` | 155 | `c6736ed0cd8d3e6090c6190089abd6b7f43443bea15a3c42ba529a2956a53efd` |
| `00bf52a0` | `00bf54ed` | 589 | `c861932ca72ee94f3f4895b4d540754537c94c9716e97ef0ec130d6f5093687c` |
| `00bf4f20` | `00bf4f40` | 32 | `956d15aa55fd38f9f349ef3c873b984bd3fae3e02c78e78e949743f929dbb27c` |
| `00bf5030` | `00bf5084` | 84 | `d0b1d2fd5e3863f9e82af8f94b038bb84c2fc189c683fd5f3a31fd92397ad48d` |
| `00bf4f90` | `00bf4f97` | 7 | `72499fbb5b630d058b85fc2bb7b6d50c4febbb4fc23621cf4bf05b073999a9dd` |
| `00bf5020` | `00bf502a` | 10 | `0a032cb89a32866d6ebcd61d725e5a5cf50356630344759f0d564fd74cd57636` |
| `00bf50d0` | `00bf50fb` | 43 | `9f0a1e587ebcc5e8f5a5447de34adc30be4a0d7bc3ffdcbb605769ed4eb28716` |
