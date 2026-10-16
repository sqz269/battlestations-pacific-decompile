# CC12 raw FrameBox default constructor 96

`construct_native_gui_frame_box_default_00ad2600(void*, NativeGuiFrameBoxDefaultBindings&)` reconstructs the whole 152-byte, 31-instruction Original default constructor `[00ad2600,00ad2698)`. Original ABI: ECX is fresh actual destination storage, EAX returns that same identity, RET has no stack arguments. The new C++ binding API is not a binary replacement ABI. The established Ghidra name `BSP_GuiFrameBox_Construct` and earlier semantic fragment remain intact.

The required bindings borrow three live `const volatile uint32_t&` cells: actual one `00d7a24c`, X `00d5d11c`, and Y `00d5d10c`. They provide no implicit globals, copied default profile, or ownership. Caller supplies fresh DWORD-aligned storage covering at least `00..11b`, the same genuine base/scalar domain, and live scalar cells disjoint from destination. Bindings metadata remains live and stable, disjoint from destination and callee scratch; access is externally serialized.

The native instruction `00ad261b CALL 00aa9390` invokes the real raw base with type 18. Source uses that existing reconstructed base directly. After it returns, the exact ordered derived operations are:

| Operation | Width and destination |
| --- | --- |
| Capture current X bits once, before derived vptr | DWORD from borrowed `00d5d11c` |
| Stamp FrameBox vptr | DWORD `00d5d130` at `+00` |
| Set frame ID | WORD `ffff` at `+ec` |
| Empty vector begin/end/capacity | DWORD zero at `+f4`, `+f8`, `+fc` |
| Set dirty flag | BYTE one at `+100` |
| Empty actual shader-name header | DWORD zero at `+104`, `+108` |
| Copy the same captured X bits twice | DWORD at `+10c`, `+110` |
| Only then capture current Y bits once | DWORD from borrowed `00d5d10c` |
| Copy the same captured Y bits twice | DWORD at `+114`, `+118` |

All stores are volatile with their actual widths. Integer DWORD loads/stores preserve the bit pattern of native MOVSS, including signaling-NaN encodings, without arithmetic or FP conversion. Observed Original preimages are one `3f800000`, X `3d6eee95`, Y `3d9f498c`; these are evidence, not supplied defaults. The adapter preserves `+ee..+f3` including the unwritten vector allocator/proxy word at `+f0`, padding `+101..+103`, all base-unwritten bytes, and a caller's hidden pool word `+11c..+11f`. It does not allocate or bind a FrameBox pool.

The Original prologue initializes EH state to -1 and never arms state 0. Its handler `00cb92a8` points to info `00df084c`, whose one-entry map `00df0844` describes state-0 cleanup through `00cb92a0` to real base destruction. The only potentially throwing C++ callee in the valid-storage body is base construction; its existing failure contract applies. Source adds no catch, speculative base destruction, state arming, slot return, or replay policy. Private Native FH3, asynchronous faults, and cleanup equivalence are not established.

`./scripts/build.ps1` passed the existing three CTests in 9.06 seconds. The new translation unit remains unregistered in CMake; it was independently compiled fresh with MSVC Win32 `/MD /EHsc /std:c++20 /O2 /Gy /W4 /WX /fp:strict /permissive-`. The inspected 137-byte Source object calls only real base construction, loads X at object `+1a` before the vptr store `+21`, uses WORD/BYTE/DWORD stores, places both X stores `+69/+6f` before the Y bit load `+78`, then stores Y at `+7a/+80`. It contains no floating-point instructions. The final header includes the independently reviewed bindings-metadata lifetime sentence; the final strict compile used that header.

One manifested `/MD` Win32 `frame_box_probe.exe` compared the whole Original 152-byte normal body against the new Source function, both using the same real reconstructed raw base and borrowed scalar views initialized from actual Original bits. Only three normal operands were relocated: the displacement of the audited base instruction to the real-base ABI bridge, and absolute X/Y address operands at `00ad2620`/`00ad2672` to those same borrowed cells. Every other body byte, including the opaque native handler literal `00cb92a8`, remained unchanged; that private handler was never entered. Both normal executions returned their actual destination with balanced native ESP, type 18, creator count one, `00d5d130`, genuine distinct self-linked base sentinels, equal remaining `0x120` buffer bytes after normalizing only sentinel allocation identity, exact X/Y bits, and unchanged derived padding/allocator/hidden-word preimages. The fixture disposes only its genuine empty base allocations; it is not FrameBox retirement proof.

That comparison establishes the narrow normal adapter/base boundary. It does not independently validate the native base or Native CRT, a signaling-NaN runtime case, failure/FH3/SEH behavior, raw copy/factory/rebuild hooks, complete graph retirement, production activation, visual behavior, or gameplay. No copy, pool/factory, hook, loader, CMake, game, or Main Source changes are included.

Eight live Ghidra byte spans matched the installed Original, whose SHA256 remains `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Every live query verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. Only owned `00ad2600` was annotated, under the tool's write lock, with old-name/comment history at `local/ghidra-annotations-20261016T050358Z.json`; the project was saved, exports refreshed, and index rebuilt. The new whole-function ledger row supplements the retained semantic fragment.

The exact source/build/probe/export/history artifacts and hashes are sealed in `local/cc12_native_gui_frame_box_raw_default_96/manifest.json`; detailed call and provenance evidence is in `reports/cc12_native_gui_frame_box_raw_default_96.json`. No further local build may replace those files before the integrator captures them.
