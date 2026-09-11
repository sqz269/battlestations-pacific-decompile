# Native wide-string storage

`native_wide_string.hpp` exposes the complete constructor, resize and destructor
bodies against a borrowed, valid native header. Its Win32 layout is a DWORD
length at `+0` and a pointer to 16-bit code units at `+4`. There is no capacity,
embedded allocator, implicit destruction or value-string copy. Callers must
retain the actual header and its matching storage owner for the whole lifetime.

The three APIs take the existing `NativeStringStorage` contract. In production,
`PooledStringStorage` supplies the existing `SizedStoragePool` owner, following
the recovered `00419CC0` / `00BD1120` / `00BD1510` route. This module does not
create a second pool, choose a CRT substitute or model a `std::wstring` as a
native allocation. The existing `widen_update_path_004c5e60` value projection in
`xlive_updates.cpp` remains unchanged; these APIs recover the missing header and
storage operations for subsequent integration.

## Native ABI and coverage

| Address | Native ABI | Exact covered end | C++ entry point |
| --- | --- | --- | --- |
| `004C5E60` | ECX header, stack C-string; EAX returns header; `RET 4` | `004C5EB6`, 3-byte `C2 04 00`; exclusive `004C5EB9` | `construct_native_wide_string_header_004c5e60` |
| `004C53E0` | ECX header, stack DWORD length and preserve byte; `RET 8` | `004C548E`, 3-byte `C2 08 00`; exclusive `004C5491` | `resize_native_wide_string_header_004c53e0` |
| `00436430` | ECX header (`__fastcall` inference), no stack parameters; `RET` | `0043644D`, 1-byte `C3`; exclusive `0043644E` | `destroy_native_wide_string_header_00436430` |

Resize also returns at `004C5420` (`RET 8`, length 3) on its zero-length path.
All three listings are contiguous through their returns. No missing-start body
or unresolved instruction gap was found. Current Ghidra signatures still show
`undefined name(void)`; the calling conventions above come from the assembly,
not those incomplete prototypes. Descriptive function names remain hypotheses,
not recovered symbols. No Ghidra names, comments or program state were changed.

The batch verified the configured `bsp` project and
`/battlestationspacific.exe`, x86 image base `00400000`, through `bsp.py ghidra`;
`config/target.json` points to `C:/Users/sqz269/bsp.gpr`. Pseudocode and assembly
were read through bounded `bsp.py show` calls. Live prototype/body information
and terminal instruction bytes were checked separately.

## Allocation, copy and release order

Constructor `004C5E6A` first writes length zero, then `004C5E70` writes pointer
zero. It never releases a prior buffer. Its byte scan begins only afterward, so
a source alias of the header observes those clears. It measures bytes through
NUL, calls resize with preserve set, then captures the destination pointer once
at `004C5E95`. A null pointer skips widening. Otherwise `004C5EA0..004C5EB0`
loads one unsigned source byte, writes one WORD, advances the two pointers, and
tests the byte just written. The loop includes the terminator and is not bounded
by the saved length. Bytes `80..FF` become code units `0080..00FF`; neither
`MultiByteToWideChar` nor any Unicode/locale decoder is called. Streaming
read/store order is retained when source, destination or header alias.

Resize first reads the old length. Equal lengths return before reading the data
pointer. In particular a fresh `{0, null}` header stays null when resized to zero,
while an unusual `{0, nonnull}` header is also left unchanged by resize-to-zero.
A different length of zero captures the old pointer and uses the initially read
length for `2*old_length+2`. After release it writes pointer zero, then length zero,
overwriting any header changes made during release.

A nonzero changed length follows this sequence:

1. Allocate `2*new_length+2` bytes at `004C5436`, before releasing old storage.
2. If preserve is set, reread the actual header length and pointer; choose the
   unsigned minimum of current and requested lengths, then double it for memcpy.
3. After memcpy, reread the actual pointer at `004C5462`. If nonnull, reread the
   length and release that pointer with `2*current_length+2` bytes. This matters
   when the copy destination aliases the header or allocation changes it.
4. Publish the new pointer (`004C547F`), then length (`004C5482`), then the WORD
   zero at `block + 2*new_length` (`004C5484`). A release callback's header
   changes are overwritten; a terminator alias of the header observes that order.

The destructor captures the data pointer first and skips even the length read
when it is null. For a nonnull pointer it captures `2*length+2` before the pool
calls. It performs no header writes at all, so the released pointer normally
remains in the header, and changes made by a release callback remain visible.
Calling destruction again on an unchanged nonnull header repeats the release.

## Boundaries and verification

All byte counts and pointer offsets use Win32 unsigned arithmetic, including
wrap. Valid readable/writable storage is required; no overflow, invalid-pointer,
null-source or allocation-failure recovery has been added. The existing storage
contract requires a nonnull allocation and retains its documented pool behavior
and failure-policy differences. Passing an arbitrary storage implementation is
an explicit host boundary, not proof of native process-global pool binding.

The native preserve call uses CRT `_memcpy` (`00BF7680`); overlapping copy
ranges are not promised memmove behavior. As with the canonical narrow resize,
the zero-byte memcpy call is omitted because standard C++ does not define a null
source as valid even for zero bytes. No CRT, synchronization or singleton ABI was
reimplemented by this packet. These are source interfaces, not drop-in binary
replacements. No startup consumer or installed game file was changed.

`scripts/build.ps1` built the registered source with MSVC Win32 `/W4 /WX`
and `/fp:strict`; both existing CTests passed. All eight existing native math
seed byte ranges subsequently matched the disk image via `verify-seeds`.
The ignored `local/native_wide_string_probe.cpp` passed after compilation with
the same strict options and `/MANIFEST:EMBED`. Its focused lifetime/alias sequence
checks unsigned-byte widening, post-allocation header reads, preservation before
release, a copy into the actual header affecting release size, publication after
release, and destructor callback changes remaining visible. This is a host
fixture, not execution of the original wide-string machine code. No permanent
test suite was added, and no gameplay or binary ABI validation was performed.
