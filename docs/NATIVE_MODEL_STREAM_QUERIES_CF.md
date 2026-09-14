# Native model and stream query leaves (CF)

The three complete getters are sourced from the installed Win32 PE (SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`)
and read-only live `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The JSON report pins the full spans, bytes,
source/provider hashes and every direct `00B42350` call site.

| Entry | End | Builder call | Actual-storage behavior |
| --- | --- | --- | --- |
| `00B61E10` | `00B61E1C` | `00B429CF` | Shift stacked index left 5, then add current stream backing DWORD `+50h`; return address, `RET4`. |
| `00B8FF00` | `00B8FF06` | `00B426A8` | Return signed model count DWORD `+188h`, plain RET. |
| `00B90620` | `00B9062F` | `00B426D5` | Load current table DWORD `+184h`, then stacked index, then table slot; borrowed node, `RET4`. |

`00B61E10` performs 32-bit wrapped pointer arithmetic. It does not inspect
the record, backing length, or element count. A null backing pointer is still
added to the shifted index; a null stream faults when `+50h` is read.
`00B90620` performs the native unchecked scaled slot read. A null model or
invalid table/index keeps the native memory-fault domain. Neither getter
retains ownership or substitutes a typed container. `00B8FF00` returns the
unaltered DWORD bit pattern as `int32_t`, matching the builder's signed `JLE`.

The indexed functions are C++ free functions with an **ignored EDX DWORD**.
MSVC Win32 `__fastcall` then places the actual owner in ECX and index at
`[ESP+4]`, preserving the original register/stack inputs and `RET4` body.
Callers must supply that extra EDX argument; these are new source interfaces,
not drop-in original two-argument symbols. The existing `LogicalVertexStream`,
`MeshVertexStreamPayload` and `MaterialSkinModel` types are semantic/checked
projections, not actual native storage for these raw getters.

The Release Win32 object contains separate zero-relocation COMDATs whose
entire 13-, 7- and 16-byte bodies match the installed PE. `scripts/build.ps1`
passed, including its existing `reconstructed_math` CTest (1/1).

This is source and object reconstruction of three leaves only. It does not
establish a complete actual-storage `00B42350` builder, its native exceptional
routes, or game execution.
