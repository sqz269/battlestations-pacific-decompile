# Native pixel shader compilation and input usage

Addresses: `00B61280` (body `00B61280..00B61B06`, 2183 bytes, 687 instructions).

This packet reconstructs the complete normal path over actual native string,
renderer, shader, VFS and physical-stream owners. It reuses the existing physical
diagnostic writers `00BE4430` and `00BF4F50` unchanged. The descriptive names are
hypotheses, not recovered symbols. This is a new C++ interface; neither native
fastcall/FH3 ABI compatibility nor material-compiler/gameplay completion is claimed.

## Evidence and ABI

`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` was verified by the BSP
wrappers before the live queries. The original installed executable and live
Ghidra bytes match across the full body. The live flow query reports zero gaps.
The detailed report records every one of the 77 CALL instructions, their numeric
targets or explicit indirect classification, and the original body checksum.

Native arguments: ECX actual eight-byte engine name, EDX shader-profile C string;
stack source C string, `IDirect3DPixelShader9**`, TEXCOORD masks, COLOR masks. EAX
returns HRESULT; `00B61B04 RET 10h` consumes the four stack arguments. Callee-saved
EBX initially holds the captured device; EBP initially holds the name. Later EBX,
EBP and ESI are reused by the parser, so the pseudocode's inferred locals and
COM argument lists are not reliable ABI evidence.

Both call sites are inside `00B3B3C0`. At `00B3B6C1`, the caller supplies separately
zeroed TEXCOORD[10] and COLOR[2], a stack shader output, builder source at +50 and
descriptor profile at +40. At `00B3BE30`, both mask arguments point to the same
stack DWORD initialized to 500, and ECX is builder+9C. That alias is deliberate:
it selects `.psa2` and bypasses the usage parser. The implementation never assumes
that the two mask pointers differ or clears either array.

## Native ordering

1. Capture current renderer's device (`00B612A6 -> 00B1FEF0`) before searching the
   name and compiling. Compile flags are `1400h`, except a case-sensitive `shore`
   substring selects zero. Native `strstr` result-minus-current-name-data `-1`
   also means no match. D3DX receives ten stdcall arguments, entry `main`, null
   macros/include/constant-table outputs, real code and messages outputs.
2. A nonnull code buffer selects the rest of the routine regardless of compile
   HRESULT. Capture the device table before `GetBufferPointer`; reload table+1A8
   after it returns, then call actual `CreatePixelShader` (`00B61358`). If either
   mask is null, release code/messages and return the creation HRESULT.
3. With both masks, disassemble the code and replace the return HRESULT with the
   disassembly HRESULT, including after a failed creation result. Capture the
   current VFS manager before any path-string allocation. Read TEXCOORD[0] for
   `.psa1`/`.psa2`, construct suffix, `shaderfx/debug/`, stem and final path using
   the actual pooled-string helpers. Open the captured manager's current +04 with
   flags35; release path, stem, prefix, suffix in that order.
4. Construct diagnostic text from actual disassembly, call current stream+5C
   through the existing concrete BE4430/BF4F50 writers, release text, decrement
   actual stream+04, dispatch its current terminal when zero. Only after terminal
   returns, reread TEXCOORD[0]; 500 now bypasses parsing independently of the suffix.
5. Parser constructs a second full disassembly string, zero-initializes the
   persistent line header, processes TEXCOORD then COLOR, releases line then full,
   and releases disassembly, code, messages. The last color-loop full data pointer
   is captured before line cleanup; full cleanup combines that pointer with the
   current full length, matching EDI at `00B61A85..00B61AA3`.

## Parser storage and boundaries

Searches use current full string data/length, signed-negative cursor clamp to
zero and unsigned upper comparison. Declaration offsets subtract the captured
base; both newline searches subtract current full-header data reloaded after
`strstr` (`00B61714`, `00B6192E`). For `dcl_texcoord`/`dcl_color`, construct a
one-character actual substring at match+12/+9 and pass its data (or the native
empty fallback) to Win32 `atol`; release that index string before finding LF.
Construct the line substring at match+14/+11 with DWORD-wrapped `LF-start`.
Resize the persistent line with preserve=1, then copy its current length from
the current substring data. Capture line data before substring cleanup. That
capture is used for four `strstr` calls, with each direct mask OR occurring
before the next search; x/y/z/w select 1/2/4/8. No letters (including null line
data) OR15. Cursor becomes the LF position, with the same native miss behavior.

These are substring searches, not tokenized declarations, and the decimal index
is exactly one character. There is no arbitrary index clamp, mask reset or
synthetic parser callback. The valid domain requires readable, LF-terminated
disassembly and writable resulting mask locations. Native malformed input can
restart at cursor -1 forever or address outside a supplied mask buffer; the
source does not claim a sanitized replacement for those invalid domains.

Existing `00469840`, `0041DD40`, `0041E870`, `004261A0`, actual string pool and
VFS bindings supply concrete storage/lifetime work. The caller's `00BF7680`
copy is overlap-capable and uses `memmove` at the source boundary. CRT strstr,
atol, the overlap-capable copy and
installed D3DX/COM/Win32 imports remain library boundaries, not reimplemented
library code. The native caller inline line assignment is preserved explicitly
because its captured pointer must survive substring release.

## Retained source failure

The stable operation stores borrowed inputs, captured owners, actual COM outputs,
stream, nine actual string headers, current parser values and acquisition state.
No implicit source destructor releases native resources, unwinds private FH3 or
rolls back already-written output/masks. Destruction with unfinished state stops
the process; replay is rejected. Caller retains inputs/module/context and excludes
terminal admission. This is a caller obligation, not an installed retirement guard.

| Failure point | Preserved state |
| --- | --- |
| Compile returns no code | Compile HRESULT, unchanged shader output, messages retained exactly as the native missing cleanup |
| Diagnostic string allocation | Already-created shader output, code, disassembly, open actual physical stream, entered but unfinished text header |
| Parser helper | Current full/line/index/substring headers and mask writes; existing child substring/concat cleanup rules still apply |
| Stream terminal throws | Reference decrement already happened; `stream_reference_released` forbids treating it as an untouched acquisition |

Entered/returned active bits distinguish an unfinished helper from a completed
native header. A failed child header is an observable preimage, not proof that
the child's buffer is still owned; resolve the child's established exception
domain before explicit diagnostic acknowledgment. Missing disassembly output and
an unsupported numeric stream write profile raise an explicit source boundary
instead of deliberately reproducing a native invalid dereference.

## Validation and fixture limits

See `reports/native_pixel_shader_compilation.json` for finalized build, native
seed check, call verifier and the focused fixture results/artifact hashes.
The fixture links the default registered `bsp_core`; no permanent new tests are
added. It uses the installed D3DX9_40, an actual HAL device, shared actual pooled
strings, native VFS mount, physical stream pool and real diagnostic files under
`local/pixel-vfs`. Original native B61280, BE4430, BF4F50 and B1FEF0 instruction
bodies are copied from the pinned executable. B61280's direct CALL operands are
redirected to actual imports or existing reconstructed dependency services;
internal branches and other instructions remain unchanged. The fixture temporarily
makes four native VFS/stream table slots callable for the original leg and restores
their original numeric values before running source. This is fixture composition,
not fixed-address binary replacement or proof of private native unwinding.

Full native source generation, B3B3C0 continuation, compiled cache publication,
and installed material/gameplay execution remain separate integration work.
