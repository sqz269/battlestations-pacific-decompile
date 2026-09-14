# Native material constant metadata leaves (CE)

These five complete leaves use the installed Win32 PE with SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Live queries addressed `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; each current function has two instructions and
no direct callee. The JSON report pins function and source hashes and every
direct `00B42350` call site.

| Original entry | Inclusive end | Direct builder call | Native effect |
| --- | --- | --- | --- |
| `00B17390` | `00B17396` | `00B423D2` | `LEA EAX,[ECX+80h]`: borrowed parameter-table address, not the first slot's value. |
| `00B47900` | `00B47903` | `00B4296A` | Return declaration DWORD `+10h`. |
| `00B5B880` | `00B5B883` | `00B42442` | Return shader-constant RegisterCount DWORD `+04h`; Rows is a separate `+08h` field. |
| `00B75E50` | `00B75E55` | `00B42794` | Load current global DWORD `[010900FCh]`. |
| `00B7AAB0` | `00B7AAB6` | `00B4311A` | Return borrowed light shadow owner DWORD `+174h`. |

The four object getters preserve the original ECX input, EAX result, plain
RET, raw null/invalid-address fault behavior, and no ownership operation.
`NativeMaterialStorage::parameters_80` is the actual `+80h` array; the
parameter-table getter returns its address, so later caller reads still use
the original layout. The vertex declaration, shader metadata and light
interfaces accept borrowed actual-storage pointers, without typed-owner
projection, bounds checks or null repair.

`00B75E50` originally needs no argument and embeds the absolute `010900FC`
address in its instruction. The new source interface takes a
`const volatile uint32_t&` bound to that **same real cell**, passed in ECX, and reads the cell
on every call. A copied type ID or pointer-value snapshot would change this
contract. The added ECX argument also makes this source entry a new ABI, not
a drop-in replacement for the original no-argument entry.

The Release Win32 COFF object has separate zero-relocation COMDATs for all
five entries. The four object-getter COMDATs match their complete installed
PE spans (22 bytes total). The animator getter's three-byte `8b01c3` COMDAT
is intentionally different from the native six-byte absolute-cell load
`a1fc000901c3`; the borrowed ECX cell address is now required. `scripts/build.ps1`
passed, including its existing `reconstructed_math` CTest (1/1).

These getters are source and Win32 object reconstructions. They do not make
the typed `00B42350` builder an actual-storage builder, supply its other
missing providers or CRT exception routes, or establish game execution.
