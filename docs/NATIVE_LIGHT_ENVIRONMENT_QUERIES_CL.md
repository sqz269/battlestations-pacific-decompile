# Raw light-environment address queries (CL)

The installed PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Read-only live analysis used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Each complete leaf has one live caller,
`B46A70`: mode-3 ambient at `B46F0B`, ordinary ambient at `B46F66`, and
cube face at `B47055`. The caller passes its actual lighting environment
in ECX. It copies four DWORDs from each returned address; for cube faces
it pushes indices 0 through 5. The JSON report pins complete PE spans,
source/provider hashes and generated Win32 COFF evidence.

`B7AA20..B7AA23` returns `ECX+18h`, and `B7AA30..B7AA33` returns
`ECX+28h`. Both use LEA: they produce borrowed interior addresses even
when ECX is zero, without reading a color or an owner field. Each has a
plain RET. `B7AA40..B7AA4D` reads the caller's single stack DWORD,
shifts it left by four bits, forms `ECX+38h+(index<<4)` modulo 32 bits,
and uses RET4. It does not validate the index or dereference the resulting
address. The new fastcall declaration reserves EDX with an explicitly unused
parameter, leaving the index in that original stack slot. The naked Win32
implementation keeps the original ECX/stack/EAX/return sequence.

The Win32 Release object has separate 4-, 14-, and 4-byte function COMDATs.
Each equals its entire installed-PE counterpart byte for byte and has zero
relocations. The full Win32 build linked `bsp_game`; the existing
`reconstructed_math` CTest passed (1/1). This verifies source object bytes,
not integration into the game's call graph or runtime storage ownership.

Existing `system_lighting_constants` getters remain as typed projections:
they access reference fields on `SystemLightEnvironment` and have a new
C++ ABI. CL exposes the original actual-storage entry points separately;
it does not substitute that typed owner or change its full-function ledger
entries. These raw address leaves do not reconstruct B46A70's light-list
traversal, indirect calls, stack prefix, upload behavior, native EH, or
game execution. A returned address is borrowed and may be invalid to read
unless the original caller's storage preconditions hold.

## Primary integration at1bbc1662

The combined library includes the raw gather and three consumed light-environment address getters. Primary review corrected five private bridges to use the profile captured by the original instruction; public context/API is unchanged. The strict MSVC Win32 build, eight native seeds, both existing CTests and47 numeric call rows passed. Fresh Ghidra and installed-PE captures matched24 spans/5157 bytes including parent/profile context. Fifteen existing names/comments were preserved, augmented, saved and read back; affected exports were refreshed. BEE070 was already present as a saved function.

Two real Direct3D9 HAL source cases passed with the same308-word prefix at VS/PS upload, original preimage preservation, current renderer/count changes after ignored VS failure, and raw fog/lighting/shadow with unsigned dimension/x87 reciprocal behavior. Original/source execution covered only the four-byte width/height leaves. The full original gather, uncached axes, private ABI/FH3/SEH and game behavior remain unexecuted. Worker frozen evidence describes80027137; the primary checkpoint describes corrected1bbc1662.

Immutable checkpoint: `local/checkpoints/1bbc1662/native-system-constant-gather-wave/validation.json`; SHA256 `78ee9018977843340db954b871be19c91a27e56c175f83a216053571a02b2f66`; 3377 artifacts and58 actual mapped runtime modules. The checkpoint retains exact sources, build/probe inputs and outputs, compiler dependencies, searched libraries, tools, native bytes and annotation before/after state.
