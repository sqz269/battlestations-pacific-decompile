# GUI and media focus-owner lifetime (CC10)

Addresses: `00AA6340`, `00AA6540`, `00AA51D0`, `00AA5B20`, `00A4C510`,
`00A4C600`, `00A4C620`.

This packet supplies normal destruction for the native GUI/media owners already
constructed by `native_platform_focus_owners`. The media operations are complete.
The GUI manager's complete destruction schedule is represented, with populated
page virtual calls remaining required external providers. No production owner,
singleton publication, synthetic disposal, or application binding is created.

| Native entry | Source operation | Coverage and ABI |
| --- | --- | --- |
| A4C510, 72 bytes | destroy_native_media_list | Complete; ECX list header, RET |
| A4C600, 29 bytes | destroy_native_media_manager | Complete; ECX 18h owner, RET |
| A4C620, 49 bytes | scalar_delete_native_media_manager | Complete; ECX owner, DWORD flags, RET4, original receiver in EAX |
| AA51D0, 83 bytes | clear_native_gui_string_list | Complete; ECX list header, RET |
| AA5B20, 29 bytes | destroy_native_gui_string_list | Complete; ECX list header, RET |
| AA6340, 354 bytes | destroy_native_gui_manager_fragment | Partial domain: all manager instructions and source cleanup states represented; current page virtual20/04 implementations remain external |
| AA6540, 30 bytes | scalar_delete_native_gui_manager_fragment | Complete wrapper schedule, inheriting AA6340's partial page-provider domain; ECX owner, DWORD flags, RET4, original receiver in EAX |

Names describing GUI/media roles are hypotheses, not recovered symbols. The
existing compiler-generated scalar-destructor names remain intact. Both scalar
wrappers return the original receiver even after freeing it; misleading earlier
pseudocode attributed the free call's EAX instead. The native flags test is bit0
of the low byte, equivalent to the source DWORD bit0 test.

## Owner and cleanup contracts

The existing constructors AA5D70 and A4C5A0 establish owner profiles D5BFCC and
D24D94. Their actual slot0 values are AA6540 and A4C620. GUI +34 is the list
header with sentinel +38/count +3C; media +4 is the list header with sentinel
+8/count +C. The existing producer supplies GUI tree +8 and page vector +14.
No conflicting object layout or second lifetime domain is introduced.

A4C510 captures the first node, resets the current sentinel's next/previous
links and clears count before freeing any node. It captures each next pointer
before `free`, compares against the current sentinel after each free, then frees
the current sentinel and clears head. Node payloads receive no callback.
A4C600 performs that list cleanup, unconditionally clears the **current** media
publication F8AEF8, and resets the **original** receiver to CE3818. A4C620 has the
same inlined cleanup and conditionally frees the original allocation.

AA51D0 has the same detached-chain schedule but keeps its sentinel. Each node
captures data at +C, then next at +0, then length+1 at +8 with DWORD wrapping.
For nonnull data it calls the existing actual string-pool getter 419CC0, followed
by BD1510, then frees the node. The getter takes no arguments: three pushed
return arguments survive it and BD1510 consumes them with `RET 0Ch`. The same
raw pool/manager publications and live small-return gate are borrowed from
`NativeStringRawPoolContext`. AA5B20 then frees the current sentinel and clears
head. No rollback of an already detached chain is invented if the getter throws.

AA6340 sets D5BFCC before capturing its page cursor. Each loop captures end
before returning CRT validation, checks the current bounds before dereferencing,
calls the current page virtual20, reloads the vector slot, and calls a nonnull
current page's virtual04 with flag1. It validates again before advancing. The
`CMP ESI,ESI` branch at AA6392 is dead. The body neither caches the page between
calls nor snapshots the vector. A single combined host tree-retirement call is
therefore unsuitable. `NativeGuiPageLifetimeHost` has two pure virtual methods;
each must dispatch the current profile of the supplied actual page.

The actual D5BE38 page profile comes from AC6600 and selects AA8320 at virtual20
(recursive scene-node release) and AA38F0 at virtual04 (AC5480 teardown and
conditional AC4C40 pool return). Those bodies were inspected; their full
populated object producers and callbacks remain outside this packet. Existing
`GuiWidgetOwnerRuntime::retire_tree` combines both phases, so application wiring
must preserve the separate calls and the intervening reload.

After pages, the GUI manager releases captured +28 through the canonical atomic
resource domain, then clears the current slot. It destroys the +34 string list,
releases +2C, destroys the +14 vector, destroys the +8 tree, clears current
F8BC5C, and resets the original owner to CE3818. Existing actual resource,
vector, fixed-whole-tree, base and allocator providers are reused. There is no
unregister operation in either owner destructor.

Descriptor DEDB38 and its five entries at DEDB5C establish cleanup states:
4 list(+34) -> 3 reference(+2C) -> 2 vector(+14) -> 1 tree(+8) -> 0 base.
Normal code sets state4 before page validation, state3 before string-list
cleanup, state2 before +2C and keeps it through vector destruction, then state0
before tree cleanup. There is **no +28 unwind action** in that map. Source C++
unwinding follows those states; a second escaping C++ cleanup exception
terminates. This is not original FH3, hardware-fault or register ABI parity.

## Evidence and checks

The integrator repaired six truncated/gapped listings after read-only raw-byte
inspection established returning frees and the real final RETs. The previous
AA6340 body ended at AA640F, losing 146 bytes of cleanup. A4C510 ended at A4C54B,
and AA5B20 ended at AA5B30. Wrapper ADD ESP,4 instructions were also absent.
The primary repair receipt is `reports/platform_flow_repair_cc10.json`; this
worker performed no Ghidra writes. The repaired exports and containing-function
queries now agree. Every free call's caller cleanup is `ADD ESP,4`; the fixed
tree-range call consumes five pushed slots with `RET14h`.

Fresh verified BSP CLI reads match the installed PE for all seven bodies (646
bytes), the two existing tree helpers (254 bytes), and 138 bytes of FH3 data and
funclets: **1,038 bytes** total. Call-site rows and containing functions are in
`reports/native_gui_media_focus_lifetime_cc10.json`.

The strict Win32 build and all three existing CTests pass. An ignored MSVC /MD
fixture links the actual `bsp_core.lib` and compares 13 original/source pairs.
Six media and six GUI cases combine empty/populated lists with flags 0, 1 and
100h. GUI page callbacks replace the vector element between virtual calls;
large pooled strings go through the existing pool getter/return providers and
real CRT free. Resource reference counts decrement from two to one, so terminal
resource dispatch is not reached. The observed source allocator free import is
temporarily forwarded through a recording wrapper and restored. Free order,
flag timing, all final owner bytes, publication clears and return identities
match. Another pair repairs invalid vector bounds in two returning CRT
callbacks: the captured end correctly suppresses the newly visible page.

A source-only throwing-page case verifies state4 cleanup and the preserved +28
reference. It is not an original FH3 differential. Original media nondeleting
A4C600 and list destructor AA5B20 were byte-checked but are not separately
executed by the differential. Page callbacks are explicit fixture providers;
no actual page is destroyed and no application/gameplay result is claimed.
The pooled-string fixture uses a nonnull fast-path publication and large frees,
not a freshly constructed pool or lazy-registration failure. Concurrent invalid
storage, terminal resource callbacks and second cleanup failures remain unproved.
