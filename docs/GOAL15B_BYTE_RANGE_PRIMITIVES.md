# Native byte-range primitives

Owned addresses: 0041F450, 0041C630, 0041C650, 0041E630.
Read dependencies/caller: 00BF67A7 `_memmove_s`, 00420DD0 byte-vector insert.
The four existing STL helper names are retained; they are descriptive template
names, not recovered C++ declarations.

| Native routine | Inclusive body | Bytes | Coverage | Original/source argument shape |
|---|---|---|---|---|
| STL_ByteRange_FillN_0041F450 | 0041F450..0041F47D | 46 | complete | stdcall `(first,count,value)`, RET 0Ch; EAX endpoint |
| STL_ByteRange_Fill_0041C630 | 0041C630..0041C64D | 30 | complete | cdecl `(first,last,value)`, plain RET; source promises no result |
| STL_ByteRange_CopyBackwardMemmove_0041C650 | 0041C650..0041C674 | 37 | complete | cdecl `(first,last,destination_end)`, plain RET; EAX endpoint |
| STL_ByteRange_CopyMemmove_0041E630 | 0041E630..0041E655 | 38 | complete | stdcall `(first,last,destination)`, RET 0Ch; EAX endpoint |

Public entries are in `include/bsp/native_byte_range_primitives.hpp`; the source
uses actual byte addresses, not an invented iterator/owner class. Each pointer
is converted to a Win32 DWORD before addition, subtraction, increment or endpoint
comparison. Arithmetic wraps modulo 2^32. No signed pointer difference, C++
out-of-object pointer increment, signed-overflow assumption, or range guard is
introduced. Any byte span that the native schedule accesses must be valid when
reached. These source entries retain the observed argument/return shapes, not
original binary bytes, CRT heap identity, private frames, or SEH equivalence.

## Fill contracts

`fill_n_native_byte_range_0041f450` returns the DWORD sum `first+count` on every
return. Native TEST/JBE at 0041F45A/0041F460 skips the loop only for zero count;
bit 31 is not a signed-count guard. Count zero avoids dereferencing `value`.
Each iteration loads one byte from `value` and then stores it at the current
destination (0041F467/0041F469); it does not cache the original value.

`fill_native_byte_range_0041c630` skips all byte accesses when the endpoints are
equal. Otherwise it reloads `*value`, stores one byte, increments the current
DWORD address and repeats until equal (0041C641..0041C64A). It has no source
return-value promise. The source uses volatile byte accesses to retain every
load/store, including when the value pointer aliases destination storage; there
is no `memset`, snapshot or substituted fill provider.

The strict optimized MSVC listing confirms this behavior: FillN tests count
before loading/dereferencing `value`, then has a repeated `MOV AL,[EDI]` followed
by the byte store and loop branch, returning with `RET 0Ch`. Fill compares
endpoints before loading/dereferencing `value`, repeats `MOV CL,[ESI]` followed
by a byte store, and ends in plain RET. The local listing is
`local/output/native_byte_range_primitives.asm`.

## Copy contracts and CRT boundary

Both copies compute `n = uint32(last)-uint32(first)` before any copy call.
Backward computes and returns `destination_end-n` regardless of the branch.
Its native TEST/JLE at 0041C661/0041C663 calls the service only when
`0 < n < 80000000h`; zero and sign-bit lengths skip the service. Forward computes
and returns `destination+n` regardless of the branch. Its native JZ at 0041E642
skips only `n==0`; sign-bit lengths still reach the service.

| Site | Native service | Exact arguments | Caller cleanup |
|---|---|---|---|
| 0041C669 | 00BF67A7 `_memmove_s` | `(destination_end-n,n,first,n)` | ADD ESP,10h at 0041C66E |
| 0041E648 | 00BF67A7 `_memmove_s` | `(destination,n,first,n)` | ADD ESP,10h at 0041E64D |

The full native `_memmove_s` body was read before adopting the contract: zero
count returns success; nonzero null destination/source or insufficient size
sets errno, reports an invalid parameter and may return; the valid path calls
overlap-safe memmove. Both callers ignore its result and retain their captured
endpoint even after a returning invalid handler.

The source calls the genuine current SDK `::memmove_s` with exactly those four
arguments and ignores the result. It adds no pointer checks, retry, exception
translation, handler installation or fake provider. Current Source CRT errno
and invalid-handler policy are an explicit external boundary; the old CRT's
handler selection, fault locations and other binary details are not recreated.
In the optimized object the SDK inlines its genuine secure wrapper and calls
the CRT memmove/errno/invalid-parameter services; a standalone `_memmove_s` CALL
instruction is not promised. The current source guards and returned endpoints
remain outside that wrapper, including the backward sign-bit skip.

## Caller and target evidence

The configured live client verified `bsp` at `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 image base 00400000 before analysis batches.
All four complete native assembly listings and their pseudocode were read.
The two service call sites were checked against the live containing bodies.

Live xrefs and a fresh physical census agree on seven direct calls, all within
00420DD0; no literal-address occurrences were found in the census sections:

| Helper | Native consumer sites in 00420DD0 |
|---|---|
| FillN | 00420E99, 00420F18 |
| Fill | 00420F2C, 00420F64 |
| CopyBackward | 00420F55 |
| Copy | 00420F01, 00420F46 |

All seven argument setups were inspected. FillN's three pushed words are
callee-cleaned by RET 0Ch; Fill's caller cleanup is ADD ESP,0Ch at 00420F31.
The later CopyBackward/Fill pair is jointly caller-cleaned by ADD ESP,18h at
00420F69. Forward Copy is callee-cleaned by RET 0Ch. Incoming ECX values at
these sites do not add a receiver to the helpers: each complete body takes its
three inputs from the stack.

## Verification and integration

The owned source compiled under MSVC x86 with
`/std:c++17 /EHsc /W4 /WX /permissive- /O2 /c /FAcs`. Optimized fill loops and
the copy guards/endpoints were inspected in the emitted listing. Call-report
and diff checks are recorded in `reports/goal15b_byte_range_primitives.json`.
Object compilation does not prove library linkage or runtime equivalence.

No new tests are introduced. The parallel byte-vector packet reuses these
public entries and owns the requested combined ignored fixture. That fixture
and the full registered project build are not claimed by this worker's static
checks. Primary registers `src/native_byte_range_primitives.cpp` in `bsp_core`
and runs the required `scripts/build.ps1` build and existing CTests. No Ghidra,
shared CMake, packet-registry or external-workstream edits are made here. Leases
remain held through primary integration.

Primary integration: registered MSVC Win32 build and all three existing
CTest suites pass. Ghidra annotations are saved and exports refreshed.
See [the batch receipt](../reports/goal15b_reconstruction_batch.json) for
the precise Source fixture/static-proof scope and remaining Native limits.
