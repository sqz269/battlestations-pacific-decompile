# Raw GUI type factory readiness — packet 78

Read-only review of Main `008099d7881c8d0e6b157d8c9ec2ac54ade02d71`, merged into worker `38f9ec231d48d2b1707a28ec7cc7f1236935e5a2`. Whole raw factory composition remains **unavailable**. Existing real shared-domain pools must be reused; their constructor bindings and the complete raw Text body remain separate requirements. No Source, Ghidra, CMake or normal startup changes were made.

Live target was the existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE32. Each live BSP query verified that target. The unchanged installed executable has SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. The [JSON receipt](../reports/cc12_native_gui_raw_factory_readiness_78.json) freezes 29 live/disk byte ranges and 25 Source inputs.

`AA6560` is exactly 148 bytes, exclusive end `AA65F4`, 42 instructions. Native inputs are ECX type ID and EDX optional actual source; result is EAX, RET. Unsigned `(type-1)>17` returns null without reading the source. The 18-DWORD table at `AA65F4` selects a `MOV ECX,EDX; JMP factory` pair. There is no allocation, cleanup, scene-node construction, publication or virtual hook in this dispatcher.

Every selected factory has the same 132-byte instruction schedule after masking only the handler immediate, requested-size immediate and three CALL operands: capture source, allocate once, return zero on null allocation, otherwise select default or copy construction. The copy receives actual destination in ECX and source on the stack; its RET4 consumes that argument. The default receives destination in ECX. Successful wrappers return the constructor's EAX unchanged. The following table is in actual type-ID order; values are hexadecimal addresses and requested byte counts.

| ID/type | Factory | Allocate | Default | Copy | Request | Current Source boundary |
|---|---|---|---|---|---|---|
| 1 Screen | AA3BD0 | AC51A0 | AA3840 | AC6040 | 124 | Real raw default; copy schedule needs actual strings/scalars/scene bindings; factory uncomposed |
| 2 Group | AA12F0 | AC76D0 | AC6F50 | AC6FA0 | EC | Complete raw factory in its documented actual Model26/3E, parent0 copy domain |
| 3 Text | AA1380 | AB79E0 | AB9650 | ABB2C0 | 1F4 | Real pool and canonical companion; only raw eight-byte identity prefix constructed |
| 4 Line | AA1530 | AC2380 | AC2D00 | AC2D60 | 104 | Raw-storage schedules; base/vector host uncomposed |
| 5 Progbar | AA14A0 | AB2B60 | AC77C0 | AC7840 | 140 | Raw schedules require genuine Icon construction/copy |
| 6 Icon | AA1410 | AB2B60 | AB5C60 | AB5D30 | 138 | Retained default projection; raw copy requires state/base/rebuild services |
| 7 Scrollbar | AA15C0 | AC84F0 | AC7C20 | AC7CB0 | FC | Raw schedules require actual base bindings |
| 8 Grid | AA1650 | AC9EA0 | ACB0A0 | ACB3F0 | 124 | Raw schedules require base/column-copy/child-resolution services |
| 9 Model | AA16E0 | ACC490 | ACC740 | ACCF70 | 11C | Raw schedules require base/raw strings/LoadModel services |
| 10 Movie | AA1770 | AAD990 | AAFE20 | AAFFF0 | 130 | Existing field projection uses C++ string/vector; not actual raw storage |
| 11 Listbox | AA64B0 | A9B7E0 | A9DF40 | AA5B40 | 17C | Raw pool engine exists; process publication owner/default raw body unprovided; copy hosts required |
| 12 AnimIcon | AA2340 | AD3960 | AD2D50 | AA1C60 | 144 | Raw copy requires Icon copy; no closed raw default found |
| 13 Curve | AA1800 | ABC790 | ABE030 | ABE100 | 140 | Raw schedules require actual base/member bindings |
| 14 Sound | AA1890 | ACD7F0 | ACDB30 | ACDBA0 | 108 | Raw schedules require base/ref/string services; copy default-constructs its base |
| 15 Button | AA4440 | AAC260 | AAC6F0 | AA3C60 | FC | Raw schedules require actual default registry F8BD38, base/string services |
| 16 ClipBox | AA1920 | ACEA10 | ACE0A0 | ACE0F0 | 10C | Tiny raw adapters can reuse genuine AA9390/AA9520; present APIs use retained companions |
| 17 Section | AA19B0 | ABF080 | ABF420 | ABF5B0 | 12C | Derived companion/own-part copy projection; complete actual raw constructors uncomposed |
| 18 FrameBox | AA1A40 | AD0280 | AD2600 | AD27A0 | 11C | Retained state/copy projection; complete actual raw constructors uncomposed |

Constructor API presence above is a readiness classification, not a new whole-body audit of every constructor. `gui_widgets_cc13.cpp` already supplies fourteen raw-pool factory wrappers over required `GuiWidgetConstructionHost` methods; no production implementation of that host exists in current `src/`. Its member/lifetime/copy hosts likewise require real providers. They must not be filled with no-op or semantic owner substitutions.

The fourteen process pools in `native_gui_widget_pools_cc13.hpp/.cpp` own actual headers and borrow the SAME `E188B4` allocator-list domain. They require explicit completed initialization. Page F8BF50, Group F8BFA0 and Text F8BDF0 have their existing distinct owners. Listbox's real F8BC10 engine has 17C payload/180 stride, but no process publication owner was found in game sources. The misleading existing name `CG_static_dtor_stub_00a9b7e0` is actually `MOV ECX,F8BC10; JMP A9B580`; this packet preserves names. Progbar requests 140 but uses Icon's F8BDA0 pool with 13C stride/trailer at138. No larger substitute allocation or pool correction is justified here.

Listbox target `AA64B0`, exclusive end `AA6534`, has no Ghidra function start. Its exact 132 bytes equal disk bytes and the normalized wrapper schedule; live instruction-context confirms its allocation/copy/default calls and final RET. No boundary repair was attempted. Its transfers are recorded separately from function-backed call-verifier rows. Its CB7350 handler maps both states to captured-slot return through A9B330; this does not supply the missing raw constructor.

`NativeGuiTextIdentityPrefix` explicitly leaves raw `+08..+1F3` unconstructed. `GuiTextRuntimeFactory` owns the body through the same canonical C++ companion. That supported identity transport cannot receive raw `AAA710` field accesses. The retained seven-type `construct_child_00aa6560` route also creates a model and retires its tree on failure; it would duplicate the subsequent model step in the missing `AAA710` tail. No all-type dispatcher or normal native Text activation is proposed.

The smallest next Source packet is ClipBox's 20-byte `ACE0A0` and 25-byte `ACE0F0`: call genuine raw base default(type16) or raw base copy, then stamp D5D058; preserve every derived EC..108 and unwritten base byte. Optional whole `AA1920` can use the SAME initialized `GameNativeGuiWidgetPoolProcess<GuiClipBoxPoolTraits>`, F8C13C, payload10C/stride110. Its CB6F50 handler selects info DED5A8, two-state map DED598; actions CB6F40/CB6F48 load captured slot `[EBP-10]` and tail ACE610. This packet only proposes that work; Root reviews and leases it separately.

The factory failure seam is narrower than whole-graph cleanup. `native_gui_widget_base_storage.cpp:50–58` finishes ref-base cleanup before rethrowing sentinel-allocation failure. `native_gui_widget_copy_storage.cpp:44–50,112–115` finishes noexcept timed/list/ref cleanup by state, resets its EH state, then rethrows; a cleanup exception terminates. Model clone failures after completed construction can retain an independent actual Model/creators in the caller's persistent acquisition frame. Copy adopts the model into widget+4C only after clone success; no throwing callback follows that transfer. A future factory must preserve those diagnostics and services even after returning the failed widget slot, and must not infer widget liveness from a saved destination address. Arbitrary callbacks outside the existing transactional contracts are not admitted. No private FH3/double-exception equivalence, full acquired-graph retirement, binary replacement ABI, runtime or game proof is claimed.
