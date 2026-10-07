# Native plane float readers

Two complete leaf bodies now have raw MSVC Win32 Source entries. They load
the live field on the same actual plane and return its value in x87 ST0.

| Native body | Field | Whole body | Source entry |
| --- | --- | --- | --- |
| `0074E260..0074E267` | plane `+C6C` | 7 bytes, 2 instructions | `native_plane_heading_0074e260` |
| `007B8E60..007B8E67` | plane `+B1C` | 7 bytes, 2 instructions | `native_plane_cached_speed_007b8e60` |

The end addresses are exclusive. Each body is exactly `FLD DWORD[ECX+offset]`
followed by `RET`, with no call, relocation, field store or stack argument.
The new `__fastcall` declarations carry the actual plane in ECX and an unused
EDX argument. Naked Source bodies preserve the load and ST0 return without
an intermediate C++ float store or conversion.

Callers must supply readable live binary32 storage at the relevant offset,
with its lifetime intact and at least one free x87 slot. This raw interface
supplies no copied field, profile, class admission, allocator or world service.
The ordinary component fixture uses masked x87 exceptions. Unmasked exceptions,
faults, original executable binding and gameplay remain unvalidated.

The installed PE and live `/battlestationspacific.exe` agree on both complete
bodies. The heading SHA256 is
`c7651d608c4934be6978d575b50c7e892e5d7265d9901a82e0c927e95a21e131`;
the speed SHA256 is
`480f5cb788ccdeb913e53e112d15ad29be9327fc15c54358d56ecc2295a4d833`.
Current Ghidra function membership covers all seven bytes of each body. The
older heading ledger text saying there was no function is historical.

The actual plane constructor stamps profile `D05F20` at `007CFD78`. Verified
data words `D05F70` and `D05F58` select these bodies at slots `+50` and `+38`.
The geometry caller loads these targets and uses `CALL EAX` at `009AFC7E` and
`009AFCB9`, with the fresh root `+4` plane as receiver. Profile data, constructor
stamp and caller fragments are structural witnesses; no whole plane constructor,
geometry body or original class dispatch is reconstructed by this packet.
The older nine-profile notes are retained; this new admission evidence concerns
`D05F20` only. Descriptive function names remain hypotheses. The existing speed
`trivial_body` tag is preserved.

One ignored connected fixture uses the same actual-shaped borrowed plane
storage for the unchanged original and Source readers. It updates the two live
fields through finite, signed-zero, denormal, quiet-NaN and signaling-NaN values.
These are five distinct FLD hazards within one fixture. The fixture compares
full 80-bit ST0 values before binary32 conversion, control/status words, TOP,
MXCSR, and every plane and guard byte. All 61 internal checks passed. Both
original bodies execute unchanged and unrelocated, without provider bridges.

The strict `/W4 /WX /fp:strict /O2` fixture freshly compiles two actual TUs.
Three Source/header/fixture inputs, one actual project include, 213 compiler
headers, seven searched host linker libraries and the compiler/linker binaries
are hashed. It links no BSP support library. Complete Source COFF code is
byte-for-byte equal to each original seven-byte body. The probe is manifested
PE32 I386 with `asInvoker`. No tracked test was added.

Primary integration at `563687e8f703c161ea7217cea71908e832e22cd2` passed the full MSVC Win32 build and all
three existing CTests. All component Source, compiler-header, linker-input,
toolchain, probe and installed-PE pins remain unchanged. Saved annotation
and refreshed export provenance is recorded in the report.
