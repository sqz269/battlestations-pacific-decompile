# Native renderer DrawPrimitive

`draw_native_renderer_primitive_00b21b40` reconstructs the complete 175-byte
`00B21B40..00B21BEE` body using the application's raw renderer and its actual
synchronization storage. The previous `D3D9StateCache::draw_primitive_00b21b40`
remains a diagnostic interface that returns HRESULT/S_FALSE. This raw provider
has no result policy: native DrawPrimitive ignores the device HRESULT.

## Native behavior and source mapping

| Native instructions | Reconstructed behavior |
| --- | --- |
| B21B40..B21B59 | Original FS/FH3 frame and renderer capture; source uses a new C++ frame. |
| B21B5B..B21B6B | Test actual DWORD renderer+1D90, then byte+1D8A; return on either nonzero. |
| B21B6D..B21B74 | Read current renderer+1904 and call complete existing B1F740. Its LEA/RET computes pointer+10 without dereferencing it; its result is unused. |
| B21B79..B21B7F | Return if primitive count is zero, after the viewport field read/helper. |
| B21B81..B21B95 | Read actual mode0108D6DC. When enabled, save renderer before calling actual B33AD0, then save returned AL. |
| B21B99..B21BB2 | Obtain arguments and current renderer+1A10 device, its current table and slot+144; push count/start/type/device. Source parameter slots belong to its new C++ interface. |
| B21BB3..B21BBB | Arm cleanup after device/table lookup and call DrawPrimitive. No enum or range validation; ignore HRESULT. |
| B21BBD..B21BD7 | Reread current mode, disarm cleanup, and only for nonzero mode read the complete saved guard DWORD and saved renderer, then call actual B33B00. |
| B21BDC..B21BEE | Restore original frame/registers and RET0C; source uses its ordinary C++ epilogue. |

There is no renderer counter update, query of the viewport dimensions, or
additional owner retention. The device and table are read after optional guard
entry. Skipping entry leaves the native guard record uninitialized; a later
mode transition that requires this uninitialized record remains outside the
valid source domain. Entered mode changing to zero before return skips leave,
including the nesting decrement, as the original does.

## Dependencies and exception boundary

The viewport provider is `native_viewport_size_address_00b1f740` from
`native_renderer_indexed_draw.cpp`. Entry, normal leave, and unwind use the
complete B33AD0, B33B00, and B21110 providers from
`native_renderer_synchronization_actual.cpp`. No replacement callback domain,
copied renderer view, or synthetic global storage is introduced in the provider.

The original handler at CBCDF8 selects FH3 metadata DF53A0 and jumps to BF6B43.
Its single unwind action at CBCDF0 addresses EBP-14 and tails B21110. Source C++
cleanup invokes that existing guard destructor when the device call throws and
terminates on a second C++ exception during cleanup. Normal leave disarms first.
Original FH3 runtime/private frame, hardware-fault and SEH handling, and aliasing
through the original caller's stack are not reproduced. The new public C++ entry
is not a drop-in ECX/RET0C binary replacement.

## Validation

Five fresh saved-Ghidra versus installed-PE spans cover 241 bytes, including the
complete body, viewport helper and the original FH3 action/metadata. The isolated
strict Win32 build passed both existing CTests after verifying all eight native
math seeds. Its 1,738 tracked source/build input hashes stayed unchanged during
the successful build. The MSVC listing confirms actual provider calls, ordered
raw loads, cleanup arming after table lookup, and current-mode-before-disarm.

One local COM ABI capture fixture compared the captured complete native body
against source for seven normal scenarios: disabled mode, both renderer skip
flags, zero count, optional entry with a null lock, mode changing from one to
zero, and a real Win32 tracked critical section. The two mode operands and three
direct provider calls were relocated; native FH3 paths were not traversed. All
calls, raw argument words, synchronization fields and renderer bytes agreed.
One source-only C++ device exception exercised actual guard cleanup. No permanent
test suite was added. No real graphics device or gameplay claim is made.

The existing Ghidra name `BSP_D3D9Renderer_DrawPrimitive` should be retained.
Primary integration owns the evidence comment, ledger update, forced export,
and permanent CMake registration. This worker changes only its four owned files.
