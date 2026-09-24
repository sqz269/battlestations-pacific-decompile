# Actual page default constructor, CC10

Address: `00AA3840`.

| Routine | Coverage | Original ABI |
|---|---|---|
| `construct_native_gui_page_default_00aa3840` | complete normal body `[AA3840,AA38A9)`, 105 bytes | ECX fresh actual page storage, EAX same pointer, RET |

Last instruction `AA38A8 RET` is one byte. The descriptive name is a behavior
hypothesis. The sole native call is `AA3845 -> AA9390`, with literal type1 on
the stack and the same actual page in ECX; AA9390 returns with RET4. The source
uses `construct_native_gui_widget_base_00aa9390`, including its real allocated
self-linked child-list sentinel and its existing source failure cleanup.

After that call returns, load CURRENT D7A2F0 with MOVSS **before** stamping
D5BE38 at page+0. Zero +100/+104/+108 and store the captured bits at +10C.
Only then load CURRENT CE3804 and write +110; then CURRENT D7A24C and write
+114. Zero +118 and +11C. The original PE scalar words are respectively
0.1f, 1000.0f and 1.0f; the interface borrows actual current DWORD cells rather
than fixing those values. Assembly retains the original MOVSS/XORPS/store
schedule, so raw NaN/negative-zero bits and aliases are not converted.

The actual page payload is 124h bytes. This derived body leaves EC..FC and
120..123 untouched, along with the base's unwritten bytes. The physical page
pool's hidden DWORD at +124 also remains untouched. Returning the same actual
pointer does not allocate, publish or register the page. There is no own EH
frame and no local acquired resource after the base returns; base failure
propagates with the provider's existing cleanup. No extra slot return or
logical GuiLayerImage owner is invented.

This is distinct from the existing logical `construct_00aa3840` in gui_layer.cpp
and the partial script constructor AC6600. Neither substitutes for this raw
default constructor. The explicit C++ bindings change the native calling ABI;
native FH3/SEH, binary replacement and in-game behavior remain unclaimed.

Read-only type1 factory handoff: AA6560 tail-jumps AA3BD0 when type1 is chosen.
AA3BD0's AA3BEE call selects actual fixed pool AC51A0/AC4E50 at F8BF50; the
requested ECX=124h is discarded by that selector. This existing raw pool has
124h payload/128h physical stride and requires explicit completed CD73D0
startup (allocator-list registration, atexit CE0AD0). The factory wrapper does
not initialize the pool. Null allocation returns null; null source selects
this default body at AA3C2D. Nonnull source selects the genuinely missing raw
AC6040 copy constructor at AA3C0A. The wrapper and copy branch are not ported.
Its two native cleanup states both return the failed slot through AC4C40.

The evidence archive also retains `factory_readiness.json`, a bounded read-only
18-entry handoff with exact wrappers, requested sizes, allocation selectors,
actual pools, default/copy identities, known physical strides and missing
handler endpoints. AA6560 is only 148 bytes, with a separate 72-byte jump
table. Old decompilation inlined undefined Listbox wrapper AA64B0. Its actual
requested size is17Ch, and it too selects a fixed pool. All selectors discard
the requested ECX size. In particular Icon/Progbar share F8BDA0 and AB2930;
native AB2A06 multiplies by13Ch even though Progbar requests140h. Unknown
physical strides stay explicitly unknown. This table is readiness evidence,
not an implementation of the factory or its other constructor branches.

Validation: strict Win32 Release `scripts/build.ps1` and all three CTests
pass. The 105 body bytes and three scalar DWORDs match live Ghidra and the
installed PE; the exact call row is checked by `verify_report_calls.py`.
One ignored original/source probe runs three pairs: ordinary raw scalar bits,
first-cell alias to the base vtable plus later aliases to zeroed headers,
and one-cell alias to the freshly written +10C. Both paths use the genuine
raw AA9390 provider. It checks the real sentinel/self-links, the returned
pointer and every 128h storage byte, normalizing only the independently
allocated sentinel pointer before comparison. Sentinel resources are freed.
The original bytes change only one relative call and three scalar addresses.
This validates the concrete store/load alias risk, not the callee independently,
all allocation failure modes, the enclosing factory, native EH or gameplay.
Earlier packet archives remain byte-identical; no Ghidra mutation is made.
