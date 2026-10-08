# Raw physical file-stream handle predicate

Address: 00BF5020.

`raw_valid_native_physical_stream_00bf5020` adds one explicit original-register
entry beside the unchanged ordinary `valid_native_physical_stream_00bf5020`
helper in the existing `native_physical_stream_open` module. This is an
additional interface for an already reconstructed Original function, not new
function coverage. The descriptive name is a reconstruction hypothesis.

| Coverage | Complete Original span | Bytes | Original and raw Source ABI |
| --- | --- | --- | --- |
| complete | BF5020..BF5029 inclusive; end BF502A | 10 | ECX actual backing; unused incoming EDX; full EAX 0/1; no stack arguments; plain RET |

The Source declaration is `std::uint32_t __fastcall` with the actual backing
pointer first and an unused DWORD second. Its definition is MSVC Win32 naked
assembly. Those two parameters occupy ECX and EDX; the return type preserves
the complete EAX value. The whole body is:

```asm
xor eax, eax
cmp dword ptr [ecx + 8], -1
setnz al
ret
```

The ten bytes are `33 c0 83 79 08 ff 0f 95 c0 c3`. No call, global,
relocation, facade delegation, check or additional branch occurs. Clearing
EAX before SETNZ defines the full result as 0 or 1. Only FFFFFFFF returns 0;
zero and every other DWORD return 1. This is a stored-word predicate, not an
OS handle-validity query.

Admission is limited to stable actual backing with a genuinely live aligned
HANDLE DWORD at +8h. The leaf reads that word without writing the backing.
It establishes no constructed physical source class, vtable or numeric profile,
tail-dispatch composition, source producer, ownership or lifetime. Concurrent
mutation, faults, whole-class compatibility and game/world behavior remain
outside this packet. In particular, this addition alone does not admit the
BF1090 source tail-dispatch or BF10E0 seek path described in
`docs/CC12_SUBSTREAM_SEEK_OPEN_READINESS.md`.

## Static byte proof and normal build

Fresh read-only Ghidra queries verified the configured `bsp` project/program
(`/battlestationspacific.exe`, configuration pointing to
`C:/Users/sqz269/bsp.gpr`), the complete function boundary and all four
instructions. All ten live bytes match the installed PE. The normal production
Win32 COFF object contains exactly one complete ten-byte raw function, with
zero relocations, and its full bytes match that Original span literally.
The decorated Source symbol also confirms the explicit fastcall declaration.
The current `bsp_core.lib` contains that exact complete production object once.
This proves production object/archive membership; it does not claim that an
executable called the entry.

The current Source/header, 202 compiler-consumed inputs, recipe and build
configuration inputs, and installed Original image were sealed before and
after the normal build. The compiler's consumed-input set was unchanged.
The resulting production object, archive, compiler tracking logs, build
project, normal build log and executable artifacts are hash-pinned in the
ignored static proof under `local/cc12_file_open_raw_entry_20261008a/`.
No standalone component executable or native fixture family was created.

`./scripts/build.ps1` passed for Release Win32, including the three existing
CTests: `reconstructed_math`, `native_math_differential`, and `tool_tests`.
No new tests or historical fixture replay were added. These normal tests are
not runtime validation of BF5020. The new raw Source entry and the Original
BF5020 body were not executed for this packet: the result is a complete
**byte-proved raw entry**, with native runtime qualification still unperformed.

`reports/cc12_file_open_raw_entry.json` records the complete bytes, COFF
boundaries and symbol, input/artifact hashes, build/test results and limits.
No CMake registration, shared ledger or Ghidra mutation belongs to this change.
