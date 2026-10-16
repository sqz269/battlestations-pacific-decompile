# B3 tagged leaf packet 03

Twelve previously tagged, unreconstructed whole word-reader entries now have
genuine C++ Source in `src/native_tagged_leaf_b3_03.cpp`: **12 routines, 45
Native body bytes**. Every entire executable COMDAT is byte-identical to the
corresponding live Ghidra and installed PE body under strict MSVC Win32 `/MD
/O2 /W4 /WX`. No prefix, omitted tail, helper, or frame is accepted. CMake
registration and Ghidra annotation remain with the primary integrator.

## Discovery and scope

The worker began with `bsp.py brief`, merged current Main without conflicts as
`1edaf69c76731a661b9effb92e17541f097490fc`, and refreshed the index before
qualification and a new explicit lease. The first two packets' eight published
files remain unchanged from `b7f850253d537f3001ae77cdc9acd491f986516e`.
No shared CMake, packet-registry, or Ghidra changes were authored.

Twenty new whole pseudocode and assembly bodies were inspected from the 52
uninspected candidates. Twelve related word readers passed index and typed/raw
Source coverage checks; none had an existing complete Source entry. Eight were
deferred without a new reconstruction claim:

- `00B3C4F0`, `00B3C500`, `00B3C510`: plain `RET` bodies, outside this reader
  family and packet capacity.
- `00B3CCF0`: ECX+10h address arithmetic, outside the selected twelve readers.
- `00B3D110`: the same eleven-byte zero-three-word pattern deferred in packet
  02; no new Source contortion or repeated compiler experiment was performed.
- `00B3D4C0`, `00B3D4D0`: `RET 4` with no consumed input; their exact Source
  cleanup boundary is deferred.
- `00B3D4E0`: `XOR AL,AL; RET` retains upper EAX; a full-EAX zero is not an
  accepted substitute.

Across the original 79 candidates, three packets have inspected 47 actual
bodies, completed 29 entries/120 Native bytes, deferred 18, and left 32
uninspected. Index/tag holes remain screening hints rather than proof of a
missing complete Source body. Existing interfaces and library names remain
unchanged.

## Complete contracts and anchors

Each selected Original is `MOV EAX,dword ptr [ECX+offset]; RET`. It consumes
only ECX, returns all 32 EAX bits, and consumes/cleans no stack arguments. There
is no consumed EDX, global, call, branch, x87, or hidden dependency. The Source
uses a one-argument `__fastcall` boundary over `const void*`; the Original
language-level convention and symbols are not recovered. Exactly four backing
bytes at the stated offset must be readable. `memcpy` avoids imposing alignment,
alias type, field meaning, or a speculative class. No validation/default/guard
is introduced.

| Original | Offset | Whole bytes | Live DATA reference |
| --- | --- | --- | --- |
| `00B3CD00` | 18h | `8b4118c3` | `00D619B8` |
| `00B3CE90` | 30h | `8b4130c3` | `00D6198C` |
| `00B3D090` | 24h | `8b4124c3` | `00D618E8` |
| `00B3D0A0` | 28h | `8b4128c3` | `00D618EC` |
| `00B3D0B0` | 2Ch | `8b412cc3` | `00D618F0` |
| `00B3D100` | 04h | `8b4104c3` | none found |
| `00B3D380` | 04h | `8b4104c3` | none found |
| `00B3D390` | 00h | `8b01c3` | none found |
| `00B3D3C0` | 04h | `8b4104c3` | none found |
| `00B3D3D0` | 00h | `8b01c3` | none found |
| `00B3D400` | 04h | `8b4104c3` | none found |
| `00B3D410` | 00h | `8b01c3` | none found |

Each DATA slot was checked in the installed PE and contains its entry address.
`docs/CC13_SHADER_VTABLES.md` associates `00D619B0` with the reconstructed D3D9
surface wrapper, the `00D6197C` region with a texture-2D wrapper, and `00D618D0`
with volume-texture locking. These are existing class-region hypotheses, not
recovered Original names or proof of these fields' meanings. Source names
therefore describe only observed operations. Exact declarations are in
`include/bsp/native_tagged_leaf_b3_03.hpp`.

No direct call references/setups were identified for the twelve entries. Five
have the recorded DATA refs; seven have no live xrefs. This does not establish
unused code or closure over indirect/virtual/generated callers.

## Verification and reproduction

All live `bsp.py` analysis uses its verified client for project `bsp`, configured
saved project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
x86 language and image base. The strengthened ignored audit requires each
symbol offset to be zero, its section to be executable code and COMDAT, and
`SizeOfRawData` to equal the whole Native size. Every nonempty executable
section is accounted for: twelve sections totaling 45 bytes. Entire live/PE/
COFF bytes and instruction operands match, with zero section/body relocations
and zero undefined external symbols. Native extents are also checked against
fresh brief live prototypes. No executable or runtime fixture was added.

The required warm-cache `./scripts/build.ps1` and both existing CTest suites
passed (2/2). The new Source is unregistered; compiled-core inclusion is not
established by that build. Its evidence is the actual strict COFF artifact.
The report records the build duration and full elapsed qualification, wait,
review, and handoff preparation from the initial UTC clock reading.

Ignored artifacts remain under `local/goal15o_tagged_leaf_b3_03/`: `compile.cmd`,
`audit.py`, `leaf.obj`, `coff_dump.txt`, whole-body proof JSON, fresh live
proto/individual-body bytes/xrefs, and `build.log`. Reproduce with `cmd.exe /d
/c local\goal15o_tagged_leaf_b3_03\compile.cmd`, then `python
local/goal15o_tagged_leaf_b3_03/audit.py`.

This is complete Source and fixed-compiler machine-emission evidence, including
the observed register/result/stack contract. Runtime use, full class semantics,
installation, binary deployment, and game validation remain unproved.
