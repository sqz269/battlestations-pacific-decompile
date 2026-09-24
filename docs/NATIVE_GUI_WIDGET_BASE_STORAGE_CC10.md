# Raw GUI widget base construction (cc10)

`construct_native_gui_widget_base_00aa9390` reconstructs the complete normal
body of `AA9390..AA9513` inclusive (388 bytes), retaining the existing
`BSP_UIContext_Construct` hypothesis. It operates on the actual aligned raw
widget allocation. Earlier `GuiWidgetOwner` code is a logical projection and
the earlier native identity helper covers only the first eight bytes.
Neither earlier coverage is replaced or promoted to full raw construction.

The native ABI is ECX destination, one stack DWORD type, EAX destination,
RET4; EBX/ESI/EDI are preserved. The source API takes an explicit borrowed
volatile binding for the word at `D7A24C`. This API is not a drop-in ABI.

## Field and allocation schedule

The constructor publishes CEB130 and a fresh atomic reference count of one,
reads D7A24C, then publishes D5C130. It writes the exact zero and captured-one
words through +5C, writes the caller's type at +60, and allocates a 0Ch child
list sentinel through A9B720. The second D7A24C read occurs after allocation
and before the +68 sentinel publication. The remaining flag, pointer and
color words use their original store order; +C4 is the final field store,
after +D4, +D8, +DC and +E0. No floating-point arithmetic is substituted for
the original XORPS/MOVSS copies: even NaN and signed-zero bit patterns in the
borrowed word survive unchanged.

The caller must supply fresh aligned storage through at least +E3. Derived
producers reserve an EC-byte base region. Bytes the constructor does not
write remain unchanged, including +08, +64, +7A..7B, +85..87, +98..A3,
+C8..D3, +D5..D7 and +E4 onward. In particular, +E8 is not initialized.
The first eight bytes use the existing `NativeGuiTextIdentityPrefix`, with
the same live atomic at +04; there is no shadow count or companion map.

A9B720 is a complete 26-byte helper. It requests 0Ch from BF681B, conditionally
self-links DWORDs +00 and +04 using the native pointer tests, and leaves +08
untouched. Its receiver is unused. Its bytes match existing A4C4A0 except for
the four-byte displacement of the CALL to the same BF681B. The source reuses
that established helper and its real CRT/new-handler allocation provider.
It does not port another allocator or fabricate a null-result success path.

## Allocation failure

AA9390's state-zero unwind map at DEDC20 is `{-1, CB73B0}`. Handler CB73B8
names DEDC28. CB73B0 loads the saved receiver then jumps to AA6E10.
AA6E10's complete 11-byte body writes D5C104 and tail-jumps to BD30F0, which
writes CEB130. It releases neither the count nor the allocation. Its former
`CG_adjustor_thunk_00aa6e10` name was incorrect: the body does not adjust ECX.
`BSP_GuiRefCountedBase_Destroy` is a descriptive hypothesis, not a recovered
class name. The source retains the exact assembly leaf and invokes it when
the source allocation throws. Full native FH3/SEH and CRT exception identity
are not reconstructed by this C++ catch.

## Validation and remaining production work

The strict MSVC Win32 build and all three existing checks passed. All 425
bytes across AA9390, A9B720 and AA6E10 match the live saved Ghidra program and
the installed PE. Three focused original/source normal-path comparisons
match the entire poisoned 124h caller buffer and the complete pre-allocation
buffer. The probe executes the original constructor and original list helper,
relocating their two data operands and two calls to the same real host CRT
allocator used by source. The allocation hook changes the borrowed word and
poisons the fresh list. Both paths retain unwritten bytes, the separate first
and second values, sentinel self-links and the untouched sentinel payload.
Only independent sentinel pointer addresses are normalized for comparison.

A separate source-only forced allocation failure verifies CEB130 cleanup,
unchanged reference count, preservation of all pre-allocation writes, and no
later stores. No exception is raised through the original native FH3 frame.
The ignored probe is linked with /MD and /MANIFEST:EMBED.

This constructor does not bind scene nodes, construct AC6600 GUI pages,
publish pages into the manager, implement complete widget destruction, or
wire application activation. The caller must eventually pass the successful
list through the real widget lifetime; the probe frees it explicitly after
observation. Application and gameplay validation remain outstanding.
