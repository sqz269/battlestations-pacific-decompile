# Actual raw-DEFLATE stream methods

Addresses: 00bbbdc0, 00bbbdd0, 00bbbe10, 00bbbe50, 00bbbf00, 00bbc060, 00bbc140, 00bbc1c0, 00bbc320, 00bbc3e0

`src/native_raw_inflate_stream.cpp` implements ten complete bodies against the actual
34h-byte owner constructed by BBC1D0. Its numeric profile is D64400. The owner keeps
the source at +0C, descriptor offset/compressed/decoded lengths at +10/+14/+18,
input/output header pointers at +1C/+20, position at +24, decoder at +28, and
remaining compressed/decoded bytes at +2C/+30. Each 10h-byte buffer header stores
begin, buffered end, capacity end, and current cursor at +0/+4/+8/+0C. The decoder
is the fetched stock zlib 1.2.1 Win32 38h-byte `z_stream`; no zlib code is ported.

| Native routine | Original ABI | Recovered behavior |
| --- | --- | --- |
| BBBDC0 | ECX owner; RET; AL | Return raw open byte +9. |
| BBBDD0 | ECX owner; RET; EDX:EAX | Return decoded length +18 with high DWORD zero. |
| BBBE50 | ECX owner; RET; EDX:EAX | Return position +24 with high DWORD zero. |
| BBBE10 | ECX owner; RET; incidental EAX | Reset decoder, seek current source to descriptor offset, restore three counters; preserve buffer cursors. |
| BBBF00 | ECX owner; RET; no semantic result | Refill compressed input, inflate into output, update counters and publish buffered output. |
| BBC060 | ECX owner; low/high/origin; RET0Ch | Seek by wrapping low DWORD, within buffered output or through refill/reset. High DWORD is ignored. |
| BBC140 | ECX owner; destination/count/optional actual; RET0Ch | Consume output, refill as needed, write advanced destination count; EAX is the optional count pointer. |
| BBC1C0 | ECX owner; three unused stack arguments; RET0Ch | Literal return, including leaving the optional count untouched. |
| BBC320 | ECX owner; RET | Release source, end/free decoder and buffers, restore stream/base profiles. |
| BBC3E0 | ECX owner; flags; RET4; EAX owner | Destroy and free the owner when bit0 is set. Existing scalar deleting destructor name is retained. |

The reset intentionally retains stale buffer cursors. Refill starts its local read
count with the owner pointer bits (the first PUSH ECX), selects flush 2 or 4 from
current counters, exits on any nonzero inflate status, and adds no progress guard.
An input capacity of zero takes the original early return before publishing output.
Seek preserves its incidental EAX paths because source dispatch forwards the return.
Copying uses the overlap-capable CRT contract of BF7680 via `std::memmove`.

The destructor uses captured source/header owners and the assembly's current field
reloads after callbacks. Its sole FH3 state is FuncInfo DFE930 -> unwind map DFE928
-> action CC4B00 -> BB86E0 base reset. An exception performs only that base reset;
later buffers are not additionally reclaimed. The C++ catch expresses the cleanup
contract; it does not reproduce the original FH3 stack frame.

Ghidra originally ended BBC320 at BBC380 after a free call. The AZ repair verifies
all 186 bytes, clears the erroneous call-site overrides, recreates the body through
RET BBC3D9, and preserves the prior name/comment. Readback reports 59 instructions
and no gaps. Evidence and old values are in `reports/native_az_flow_repairs.json`
and `reports/native_az_function_definitions.json`.

These are new C++ interfaces over actual storage, not binary ABI replacements.
The earlier `src/inflate_stream.cpp` remains a separate bounded semantic projection.
Validation status and per-body hashes are recorded in
`reports/native_raw_inflate_stream.json`; independent native/source fixture evidence
is recorded separately. Original stack-spill aliasing, original FH3, installed
archive loading, and gameplay require separate evidence.
