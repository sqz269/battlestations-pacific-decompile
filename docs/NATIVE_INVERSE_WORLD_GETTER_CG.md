# Raw inverse-world node getter (CG)

`00B6E0D0..00B6E109` is a 58-byte getter in the installed Win32 PE, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Read-only live analysis used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. `00B42350` calls it directly at `00B4283A`
and `00B42F13`. The existing typed full-function source
`get_transform_inverse_world_00b6e0d0(CameraTransform&)` remains in
`material_transform_constants.cpp`; this packet supplies a distinct raw
actual-storage interface and claims zero new native body/byte credit.

The original and new raw entry take ECX as the actual node, return EAX as
that same node's `+60h` cache address, and use plain RET. A 40h stack scratch
and saved ESI surround the miss path. The entry captures one flags DWORD
from `+5Ch`, tests bit 8 for a cache hit, then tests bit 2 from that same
capture. When bit 2 is clear, it calls the real raw world refresh
`00B6DB70` before inversion. It then inverts current node world `+F0h`
into the stack scratch using raw `00B63B30`, copies that scratch into the
actual shared cache `+60h` using raw `004134F0`, and ORs bit 8 into the
**current** `+5Ch` DWORD only after both calls return. A cache hit returns
the existing `+60h` address without touching the flag or matrix.

The native call instructions are at `00B6E0E1`, `00B6E0F0`, and `00B6E0F9`;
the PE displacements resolve respectively to `00B6DB70`, `00B63B30`, and
`004134F0`. The adjacent raw `00B6FCB0` camera getter has the same schedule
and is source evidence, but its entry is not substituted for `00B6E0D0`.
The new entry neither wraps typed `CameraTransform` nor copies owner storage.
The Release Win32 object contains the full 58-byte COMDAT, byte-identical to
the installed PE after masking only three call displacements. Its three
`REL32` relocations bind the matching raw provider symbols at offsets `+12h`,
`+21h`, and `+2Ah`. `scripts/build.ps1` passed, including its existing
`reconstructed_math` CTest (1/1).
It has no null/hierarchy/singularity/alias repair, no synthetic matrix math,
and no added native exception frame. Raw providers retain their documented
x87 and memory-access effects. This source/object result does not establish
a complete actual-storage `00B42350` builder or game execution.

## Primary integration at7da09ce9

The combined library passes strict MSVC Win32 compilation, eight native seeds, both CTests and32 numeric call rows. Fresh Ghidra and installed-PE captures match52 spans/12295 bytes. Independent review of all1125 native body lines resolved descriptor count/base capture, world-flag versus width ordering, and the live-one read between decoded component1 FLD/FSTP. Seventeen names/comments were saved and read back, including four new descriptive names; previous comments were preserved.

The existing full original/source fixture compares both4096-byte banks in fallback and optimized skinning variants. It executes27 original bodies/6704 bytes, with shared source B48CE0 and host memmove/invalid-parameter boundaries. Actual texture-source providers and source retained-failure/replay checks also pass. Nonempty renderer/shadow binding, full compiler/pass/debug parents, private ABI/FH3/SEH and game execution remain unvalidated. Worker source6336f486 remains historical; final corrected source is7da09ce9.

Immutable checkpoint: local/checkpoints/7da09ce9/native-material-constant-build-wave/validation.json; SHA256 40e2d9986cf8c3e53cf84d715a1d9aa87081c7df36d8dde6639c4359e8e69b76; 3516 artifacts and7 actual mapped runtime modules. It retains source/build/probe/native-byte/annotation/review inputs and outputs, compiler dependencies, searched libraries and tool/runtime closure.
