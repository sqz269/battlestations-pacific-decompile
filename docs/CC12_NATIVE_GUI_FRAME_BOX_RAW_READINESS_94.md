# Raw FrameBox construction readiness — packet 94

Read-only review on worker `020024ef2d363f4174bd31e55d97192a657a0aa4`, preserving90 and the already-reviewed published Main `1c4fc481b4b2bfce78b68cf4158c84ada2d71df8`. Only this document and its JSON receipt were created. No Source, Ghidra, ledger, CMake or Main changes, builds/tests, or sealed90 artifact writes occurred. Live BSP wrappers verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE32; installed Original remains SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The smallest ready complete new raw function is **default AD2600, 152 bytes**. Whole raw copy and factory composition remain unclosed; they must not be credited through the retained FrameBox state projection or a null-only factory substitute.

| Entry | Complete bytes / exclusive end | Verified original interface |
|---|---|---|
| AA1A40 factory | 132 / AA1AC4 | ECX optional actual source; EAX actual result/null; RET |
| AD0280 allocator adapter | 10 / AD028A | Ignores incoming size, MOV ECX,F8C188; tail AD0050 |
| AD2600 default | 152 / AD2698 | ECX actual destination; EAX same destination; RET |
| AD27A0 copy | 214 / AD2876 | ECX destination; stack actual source; EAX destination; RET4 at AD2873 |

The actual pool is **F8C188**, not F8C18C. Both allocator AD0280 and return thunk ACF8E0 select that exact publication. Native ACF610 reads hidden slab ID+11C and divides the slot offset by120; its free stack starts1200, count1220. AD0050 allocates1224-byte slabs. The existing genuine `GameNativeGuiWidgetPoolProcess<GuiFrameBoxPoolTraits>` has 11C payload/120 stride,16 slots, and uses the SAME E188B4 allocator domain. Explicit completed startup and outstanding same-pool slot ownership remain requirements; no second pool is needed.

Default invokes genuine raw AA9390(type18), then loads actual DWORD bits D5D11C **after base success and before the vptr stamp**. It stamps D5D130 and performs the following ordered stores, then loads D5D10C only after both X stores. MOVSS performs bit transfers, not arithmetic or x87 conversion.

| Derived fields | Native stores/preimage |
|---|---|
| EC..ED | WORD FFFF |
| EE..EF, F0..F3 | Untouched padding/vector allocator-proxy preimage |
| F4,F8,FC | DWORD0: native vector begin/end/capacity |
| 100 | BYTE1;101..103 untouched |
| 104,108 | DWORD0: empty raw ShaderName header |
| 10C,110 | SAME first captured D5D11C bits, currently3D6EEE95 |
| 114,118 | SAME later captured D5D10C bits, currently3D9F498C |
| Hidden11C..11F | Outside payload; untouched |

Every base-unwritten byte is preserved. The nearby D5D110 `FrameSizesX` text is not a constructor literal input; neither audited constructor body has a direct string-literal operand. Transitive rebuild/string paths are separate dependencies. Current C++ decimal defaults in `gui_framebox.hpp` are semantic constants, not a supplied live-cell identity. A raw implementation should borrow the actual scalar views explicitly, with no invented global/data owner or `GuiFrameBoxWidget` cast.

Default handler CB92A8 selects info DF084C, one-state map DF0844:0→(-1,CB92A0→AA9730). **The actual152-byte body never arms state0**: state remains-1, its only throwing callee is AA9390, and all post-base work is valid-storage scalar loads/stores. The existing raw base catches sentinel-allocation failure and finishes its own ref-base cleanup. A new default adapter therefore adds no speculative base cleanup, slot return or catch. Its allocation belongs to its caller/outer factory.

Copy is substantially different: genuine raw AA9520; D5D130 stamp; current source WORD EC; real AD2230 copying the actual F0 vector of2Ch records; byte100 copied; empty104/108; four x87 FLD/FSTP transfers10C..118; current destination table+80 invoked with a **zero-extended WORD** argument; byte74 cleared; then ACF8F0. Current D5D130+80 cell D5D1B0 resolves AD0D80. The call remains current/dynamic, not a permanently cached direct target. x87 copies can change signaling NaN representation; C++ field assignment/bit-copy must not be assumed equivalent.

Copy handler CB92E4 selects info DF0888/map DF0870:0→(-1,CB92C0→AA9730),1→(0,CB92C8→AD2320(this+F0)),2→(1,CB92D6→41DD20(this+104)). State0 is armed before AD2230; state2 at AD2819 after the first x87 transfer. AD2320 is an undefined five-byte tail to actual vector destructor AD21B0; live instruction context and bytes verify it without creating a function. AD21B0 iterates2Ch records through AD0860, frees backing and clears vector begin/end/capacity. Its raw Source provider, AD2230's allocator/uninitialized-element-copy cleanup chain, and actual element ownership are absent. Their established STL names are preserved.

Existing `gui_framebox_copy_state_00ad27a0` copies C++ string/vector/shared ownership and clears ShaderName; it is explicitly a derived-state projection. AD0D80/ACF8F0 have retained geometry/services implementations plus actual geometry-publication fragments, not complete actual11C FrameBox bodies. ACF8F0 includes mesh construction/association and actual AA7DC0, while AD0D80 has its own material/string/stream paths. Those bodies and their genuine raw services cannot be replaced with semantic casts or no-op/default callbacks.

Before AA9520 returns, its existing cleanup can retain independent actual Model acquisitions in the caller frame; a future factory must not discard those merely because a failed widget slot is returned. After successful base copy, its Model creator transfers into widget+4C. Later FrameBox failures require the real derived string/vector unwind and AA9730, which may destroy that adopted Model; saved clone diagnostics are then not proof of Model/widget liveness. Until those raw cleanup providers close, a Source catch that blindly returns/reuses the widget slot would be unjustified.

Outer factory AA1A40 captures source before AD0280, requests11C, then chooses AD27A0(nonnull source) or AD2600(null source); allocation null returns0. CB6F90→info DED610/map DED600 has both states0/1→-1 via CB6F80/CB6F88, returning the captured slot through ACF8E0. This Native map alone does not establish equivalent Source cleanup of missing copy callees. Existing generic raw-pool factory still requires a genuine constructor host.

Recommended Source96, **not implemented here**: new `native_gui_frame_box_factory.hpp/.cpp`, only `construct_native_gui_frame_box_default_00ad2600(void* actual_destination, NativeGuiFrameBoxDefaultBindings&)`. The bindings borrow three `const volatile std::uint32_t&` views: actual D7A24C one, actual D5D11C X, actual D5D10C Y. Reuse whole raw AA9390, preserve the two scalar capture points and all byte widths/preimages, then return the same pointer. Fresh aligned destination covers11C bytes; borrowed views/services stay alive; caller owns allocation. No Source pool allocation/registration, copy/factory subset, typed state, new failure frame, private FH3/SEH equivalence, binary replacement ABI or production/game proof is proposed. Copy/factory should wait for concrete raw2Ch vector/element lifetime and actual current+80/ACF8F0 closure.

The [JSON receipt](../reports/cc12_native_gui_frame_box_raw_readiness_94.json) freezes Original/Source bytes, ABI/call/unwind evidence, unresolved providers and this exact bounded plan. No Source96 lease or edit was taken.
