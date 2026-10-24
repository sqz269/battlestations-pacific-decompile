# Raw Scrollbar source prerequisite

`native_gui_scrollbar_storage` supplies actual Scrollbar allocation, construction,
copy, destruction, scalar deletion, min/max/value setters, loaded handling and the
verified property-finish fragment. `native_gui_scrollbar_identity` registers one
companion over the same object's existing atomic `+04`. This packet does not admit
Scrollbar to the retained factory or construct a page, manager, alternate layout,
Model, native count or transform.

Evidence is the saved listings and narrowly queried live bytes from
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, function count77629.
Names remain descriptive hypotheses. These are new C++ interfaces, not drop-in
x86 entry points; native FH3/SEH/fault transport and runtime delivery are unproved.
Existing Scrollbar names were preserved; evidence comments were appended through
the Ghidra write lock and saved, with prior values recorded in
`local/ghidra-annotations-20261024T113654Z.json`. Affected exports were refreshed.

| Original entry | Implemented contract |
| --- | --- |
| `00AA15C0` | ECX optional source, EAX same actual slot or null, RET. Calls existing `AC84F0` with requestFCh; default `AC7C20` or nonnull copy `AC7CB0`. Split interfaces require only reached inputs. Constructor failure returns the captured failed slot after its constructor cleanup. |
| `00AC7C20` | ECX actual, EAX same, RET. Existing raw `AA9390(type7)`, then `D5CCF8`, positive-zero float DWORDs EC/F0/F4, DWORD F8=1. |
| `00AC7CB0` | ECX destination, stack source, EAX same, RET4. Existing raw `AA9520`, profile stamp, FLD/FSTP separately for EC/F0/F4, then DWORD copy F8. No derived allocation or additional cleanup state. |
| `00AC7CA0` | ECX actual, RET. Stamp `D5CCF8`, tail-jump genuine `AA9730`. Tail fields have no destructor work. |
| `00AC8210` | ECX actual, stack flags, EAX saved identity, RET4. Stamp profile, genuine `AA9730`, inspect CURRENT flags low byte afterward; return same slot to F8C004 through `AC80A0` iff bit0. Explicit scalar neither decrements nor tests the count. |
| `00AC7D00` | ECX actual, float stack DWORD, RET4. Native x87 comparisons/spills choose min, max or the original argument bits. Capture current profile, chosen bits and current7C target before publishing EC, then call that target once. |
| `00AC7D80` | Same argument/return ABI. Store original argument bits at F0; native ordered comparisons may raise F4 and clamp value through SetValue. Always call current7C afterward; when clamping, this is a second call and recaptures the profile after the first callback. |
| `00AC7DE0` | Same argument/return ABI. Store original argument bits at F4; native ordered comparisons may lower F0 and clamp value through SetValue. Same one-or-two current7C call schedule. |
| `00AC8890` | ECX actual, RET. Four genuine `AAA2B0` lookups with actual pooled8h temporary strings; discard each result. Spill CURRENT F0 to a float argument, SetValue, then genuine `AA7170`. |
| `00AC85A0`, only `AC86DE..AC872A` | After successful SAME base/property reads: spill current F0, SetMinimum; then capture/spill late current F4, possibly lower F0 and clamp current EC through SetValue, then current7C. The full reader and its Alignment temporary cleanup are not supplied by this fragment. |

The existing `GuiScrollbarPoolTraits` bind process F8C004, pool profile D5CD78,
slot100h, 16 slots/slab, allocator stub AC84F0 and failed-slot thunk AC8200.
The payload is FCh. Its FC DWORD is the pool's slab index, not another widget
field. The same existing `GameNativeGuiWidgetPoolProcess<GuiScrollbarPoolTraits>`
must be explicitly initialized first. Factory handler CB6E90 selects FuncInfo
DED470; both unwind-map entries at DED460 target CB6E80/CB6E88, which load the
captured slot and jump to AC8200. No larger or substitute allocation is used.

The actual derived tail is exactly four DWORDs: value EC, minimum F0, maximum
F4 and orientation F8. F8 is **not a C++ bool or float**; default1 is horizontal.
Setters operate directly on these fields, never through `GuiScrollbarState`.
MSVC Win32 inline assembly retains the original FLD/FSTP, FCOMI/FCOMIP and JBE
unordered paths. This also preserves native float spills and signaling-NaN
conversion rather than substituting `std::clamp` or C++ comparisons. A zero-valued
argument is not a reset operation: loaded resets by calling SetValue(current
minimum). No separate reset entry was inferred or invented.

`NativeGuiScrollbarCalls` requires genuine current raw-table lookup, current7C
child layout (normally AC8A80), raw-name child lookup AAA2B0 and base loaded
AA7170. Lookup must be free of side effects, and returned tables must survive the
call. Numeric native addresses identify implementations; they are never invoked
as host code. There is no successful empty binding. A missing table or target is
rejected; interrupted real callbacks leave the original partial mutations in
place and are not retried or rolled back.

Loaded uses the existing `NativeStringRawPoolContext` and actual-header
0041DD40/0041DD20 bodies. Source literal bytes D5CDF0/D5CDE0/D5CDCC/D5CDBC are
`_BackArrow_Icon`, `_ForwArrow_Icon`, `_Background_Icon`, `_Slider_Icon` (lengths
15/15/16/12). Each normal release occurs after clearing its native EH state;
only an entered lookup failure consumes that temporary's cleanup. No cached
child pointer or string owner is retained by the provider.

The full AC85A0 raw visitor remains unclosed: its actual by-value8h key/output/
default wrappers and pooled Alignment result must be supplied by the real reader.
It first calls AAA710, maps case-insensitive Vertical to raw F8=0 (otherwise1),
then writes raw F0/F4 from MinValue/MaxValue with float-zero defaults. Only after
those operations succeed may the new explicitly named finish fragment run. The
existing projected typed helper is not treated as that raw visitor.

`NativeGuiScrollbarReference` borrows the same completed positive atomic at+04,
validates current D5CCF8 cells0/1 as BD30E0/AC8210 (live bytes confirmed), and
registers that exact companion in the same actual-owner registry. It performs no
native retain or count initialization. Zero dispatch follows BD30E0 to current04
flags1. Explicit scalar deletion remains independent of the count. Both retire
metadata through an opaque-key unbind with no post-pool payload read. Slot reuse
is externally serialized until unbind finishes. Destroying still-live companion
metadata terminates. Failed registration leaves the actual allocation with its
caller; entered terminal failure retires metadata but preserves failed storage
and provider frames for diagnosis, with no retry/reuse.

Future activation still needs one authoritative raw widget/layout association,
same canonical Model4C binding, real current7C/base/child/timed dispatch and sole
page authority. The shared raw AA9390/AA9520/AA9730 and existing child-link services
remain the owners of their state. This packet copies no base state and supplies
no borrowed/no-op page token. Legacy pages and all manager chains are unchanged.

Validation: `scripts/build.ps1` completed Win32 Release, including both new
sources, and existing CTests passed2/2 (10.51s). The temporary private CMake
registration was restored byte-for-byte (SHA256 equality checked). No test case
or game run was added. This proves source/build compatibility, not native ABI or
runtime Scrollbar/page delivery.
