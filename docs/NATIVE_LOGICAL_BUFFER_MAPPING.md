# Actual logical vertex and index mapping

Addresses: `00B49980`, `00B49A80`, `00B49B60`, `00B49C70`. Names remain
descriptive hypotheses. Read-only wrappers verified project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe` for each batch.

| Routine | Inclusive body / final instruction | Original ABI | Coverage |
| --- | --- | --- | --- |
| Vertex lock | `00B49980..00B49A7B`; `RET 0Ch` at `00B49A79`, 3 bytes | ECX actual logical stream; count, offset, read-only stack DWORDs; EAX data | complete |
| Vertex unlock | `00B49A80..00B49B0A`; `RET` at `00B49B0A`, 1 byte | ECX actual logical stream; no stack args | complete |
| Index lock | `00B49B60..00B49C6B`; `RET 0Ch` at `00B49C69`, 3 bytes | ECX actual logical stream; count, offset, read-only stack DWORDs; EAX data | complete |
| Index unlock | `00B49C70..00B49CF5`; `RET` at `00B49CF5`, 1 byte | ECX actual logical stream; no stack args | complete |

New interfaces add the existing service context in EDX. Only the low byte of
read-only matters to the physical operation. These functions consume actual
native storage and invoke the already reconstructed physical mapping routines;
they do not use `LogicalVertexStream`, `LogicalIndexStream`, vectors or fake
COM uploads. No logical/physical allocation, ownership transfer, registry
publication or additional refcount operation is introduced.

Vertex lock first enters the optional renderer guard. If `(flags+60 & F000h)
== 1000h` and physical `+58` is nonnull, it captures declaration `+68`, writes
requested count to `+64`, reads declaration stride `+CC`, and calls physical
virtual `08` with `(stride*count, stride*offset, 1, logical+5C, false)`.
Otherwise it reloads physical `+58`; when present it computes `stride*count`,
substituting stored count `+64` when that product is zero, and uses
`(base_vertex+70 + offset)*stride`. This branch passes the incoming count stack
cell as the physical output offset, leaving logical `+5C` unchanged. The raw
result is published to logical `+08`. An absent physical owner returns the
existing `+08`, including its preimage, without clearing it.

Index lock uses format `+18`: `65h -> 2`, `66h -> 4`, every other value `0`.
Requested byte count zero substitutes stored count `+14` times that size.
It reloads physical `+08`, calls its virtual `08` with
`(bytes, (base_index+20 + offset)*size, 1, logical+0C, read_only)`, and returns
the raw result. An absent physical returns null. All additions and products
retain native DWORD wrap, including a wrapped-zero product selecting full count.

Vertex unlock reads the current physical `+58` after guard entry, calls its
virtual `0C` when present, then unconditionally clears logical `+08` after the
callback. Index unlock checks `+08`, reloads that physical pointer, and calls its
virtual `0C`; it has no cached mapping field to clear.

The four original immutable physical profiles are borrowed from the existing
`NativeLogicalBufferDeviceRestoreProfiles`. Their original numeric entries
select actual `00B4BA00/00B4B850` lock or `00B4B9D0/00B4B820` unlock code;
they are never treated as callable host addresses. The actual physical
implementations retain native failed-HRESULT, diagnostic, null-buffer sentinel,
cursor/depth and aliased output behavior. No extra HRESULT check or rollback is
added. The logical methods themselves do not relocate an array. Physical COM
Lock can return a different writable mapping; callers must use the published
current vertex `+08`, not a pointer cached before another callback.

Guard entry captures the current renderer only when current mode enables it.
Ordinary leave uses the captured renderer and saved AL, with a new mode test;
the leave routine again observes the current mode and renderer lock. Vertex
lock tests the exit mode before capturing `+08`, preserving the native order.
The EH state is disarmed before ordinary leave. C++ unwinding uses the existing
actual guard-destruction helper. No original SEH/stack-image ABI is claimed.
An entry-disabled/exit-enabled change reads an uninitialized native guard and
remains outside the valid execution domain; no invented initialized guard or
global mode repair is supplied.

Producer bodies were read before adopting offsets: `00B61E20` initializes
vertex writer mapped pointer `+08`, stride `+0C` and declaration-selected
attribute fields; `00B4BC00` establishes physical `+58`, flags `+60`, count
`+64`, declaration `+68`, shadow `+6C` and base vertex `+70`.
`00B4BF30` establishes index physical `+08`, output offset `+0C`, flags
`+10`, count `+14`, format `+18`, shadow `+1C` and base index `+20`.
Existing physical owner types supply the real storage and lifetime domains.

Live xrefs for the four methods are their table cells only. Bytes at
`00D61D7C/80` establish logical vertex virtual `10/14`; bytes at
`00D61DEC/F0` establish index virtual `0C/10`. Both Text consumers were read:
`00AB9FD0` maps vertex/index at `00ABA00E/00ABA02C`, then unmaps index/vertex
at `00ABA241/00ABA24A`; `00ABA270` uses `00ABA302/00ABA328` and
`00ABA8B2/00ABA8BB`. Each lock passes `(4 or 6 * current UTF16 length, 0, 0)`.
Every method retains the full count/offset/read-only contract, not only these
Text inputs. Complete-function register filters establish ECX physical at
dispatch, ESI logical storage, EDI/EBX captured renderer and temporary result,
and all stack count/offset/output argument provenance. Physical lock has five
stack words and `RET 14h`; physical unlock has no stack arguments.

Validation: strict MSVC Win32 compilation with `/W4 /WX /O2 /fp:strict` passed.
The report records live exact-call verification, including indirect rows that
were separately checked against table bytes and bodies. No new tests, native
differential result or game/visual result is claimed. Actual logical stream
creation and renderer bindings remain prerequisites; the integrator runs the
combined standard build.
