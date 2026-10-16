# CC12 actual-storage Text constructor readiness 84

Read-only packet over reviewed Root baseline
`9bc71b4b10fdaa858c1352481c0fb1bee86ad1b7`, checked into the isolated worker
by merge `e1a4776116c6903ed6a64f8ba8edff3f306e5584`. Only this document and
its JSON report are deliverables. No Source, CMake, ledger, Ghidra analysis,
installed executable or Main change was made. No build or test was run.

**Verdict:** real Text allocation and the native eight-byte identity prefix
already exist. A complete raw `1F4h` Text default or copy constructor does not.
The preferred next Source packet is a genuine raw AB8530 normal implementation
over the existing actual Model/resource providers, followed by the complete
raw AB9650 default composition. The proven 610-byte derived-default fragment
is a fallback if that composition cannot close in one bounded Source packet.
It must not be presented as a whole constructor or factory.

## Current evidence and exact original boundaries

Each live batch used `bsp.py ghidra` wrappers for
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Pseudocode was read
before assembly; the register arguments, FH3 state and copy x87 operations
required assembly inspection. Installed Original SHA-256 was rechecked:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Entry | Live half-open extent | Bytes / instructions | Assembly-established original ABI |
| --- | --- | --- | --- |
| AA1380 | `[AA1380,AA1404)` | 132 / 40 | ECX optional actual source, EAX new actual slot or null, RET; preserves ESI |
| AB79E0 | `[AB79E0,AB79EA)` | 10 / 2 | Ignore incoming ECX; select F8BDF0 and tail-jump AB78A0 |
| AB9650 | `[AB9650,AB98EF)` | 671 / 123 | ECX fresh destination, EAX same destination, RET; preserves ESI |
| ABB2C0 | `[ABB2C0,ABB624)` | 868 / 195 | ECX destination, stack actual source, EAX destination, RET4; preserves EBX/EBP/ESI/EDI |
| AB8530 | `[AB8530,AB87CC)` | 668 / 208 | ECX same Text, no stack argument, RET; preserves EBX/EBP/ESI/EDI |

All five complete live byte ranges match the preserved installed Original.
Their live instruction-start lists, except the trivial separately read
two-instruction allocator, match independent Capstone decoding of those same
Original bytes with no gap. The JSON records per-body SHA-256, the ordered
field-store instructions, direct call/jump rows, indirect sites, EH tables and
25 exact helper/handler extents. These checks establish static evidence only.
All descriptive names remain hypotheses rather than recovered symbols.

AA6560's type-3 tail at AA6583 targets AA1380. The factory saves source ECX,
loads size `1F4h`, obtains the actual pool slot, and only then selects copy or
default. Allocation-null returns null without entering either constructor.
AB79E0 overwrites the size with the same F8BDF0 owner; AB78A0 performs the real
locked allocation. Its slot stride is `1F8h`; the final DWORD at `+1F4` is pool
metadata, outside the `1F4h` Text payload. No payload clearing follows allocation.

## Default and copy field contracts

AB9650 first completes AA9390 on the same raw destination with type 3. The
existing raw provider `construct_native_gui_widget_base_00aa9390` already
reproduces its actual list-sentinel/default schedule and preserves unwritten
bytes. AB9650 then stamps D5C6C8 and performs the following derived schedule.
The exact instruction order is retained in the report; this table groups values.

| Raw fields | Default behavior |
| --- | --- |
| EC/F0, F4/F8 | Empty eight-byte wide and narrow headers; four DWORD zero stores |
| FC, 100, 104, 108, 10C, 110, 114, 118 | Multiline byte 1; align, vertical align, font, distance/line-count/measurement and state-color byte zero |
| 11C..158 | Four color quartets: normal RGB from CE3E18 and alpha from D7A24C; focus/selected from that same captured one word; disabled RGB positive zero and late CE3800 alpha |
| 15C, 160, 164, 168..174 | Shadow byte 0, position 0, current D5C5C0 offset, four separately read current E12FD8..E4 shadow-color DWORDs |
| 178, 17C, 180, 184, 188, 18C, 190 | Positive-zero float, integer 2, three null pointer words, two positive-zero floats |
| 198, 19C, 1A0, 1A4, 1A8, 1AC, 1B0, 1B4 | Seven DWORD zeroes and one byte zero; includes empty vector backing and empty auxiliary wide header |
| 1C0/1C4, 1C8/1CC, 1D0, 1D8, 1EC, 1F0 | Two empty narrow headers, -1, captured one, null shader word, byte 1 |

The actual current default constant words were read: D5C5C0=`3D4CCCCD`,
E12FD8..E4=`{0,0,0,3F400000}`, CE3E18=`3F333333`,
D7A24C=`3F800000`, CE3800=`3F000000`. A future implementation must borrow
the established live constant/global cells and preserve the load schedule,
including the sole mutable `GameNativeGuiTextState` shadow quartet. These
snapshots are not permission to replace them with per-widget constants.

Exactly 217 of 264 bytes in `[EC,1F4)` are written. The 47 untouched bytes are
`[FD,100)`, `[119,11C)`, `[15D,160)`, `[194,198)`, `[1B5,1C0)`,
`[1D4,1D8)`, `[1DC,1EC)`, `[1F1,1F4)`. AA9390 also leaves `[E4,EC)`
untouched. In particular, no zero can be inferred for `+194`, glyph offsets
`+1B8/+1BC`, font-dependent `+1D4`, or material payload `+1DC..1EB`.
Do not memset the payload, clone its padding, or touch the pool trailer.

ABB2C0 completes AA9520 first. It constructs the EC wide header with 4C8DD0,
then independently clears/copies the narrow headers F4, 1C0 and 1C8 using
41DD40 and length-byte memcpy. The source-header identity checks occur after
the destination-header zero stores. It copies scalar/byte fields, clears the
cursor/shadow/glyph/auxiliary/cache fields, and copies the sixteen state-color
DWORDs last. The same 47 derived bytes remain unwritten. Seven pairs at
10C,114,164,178,18C,190,1D8 use x87 FLD/FSTP; bitwise copies would lose the
original signaling-NaN/conversion behavior. Finally it enters cursor AB8910,
sections AB8530 and current-text rebuild ABB1D0 in that order.

## Native exception evidence and ownership

Default handler CB809C selects FuncInfo DEEE88/map DEEEAC. Copy handler
CB820C selects DEF040/map DEF064. Both contain seven cleanup states:

| State | Next | Default / copy action | Effect |
| --- | --- | --- | --- |
| 0 | -1 | CB8040 / CB81B0 | AA9730 base destruction |
| 1 | 0 | CB8048 / CB81B8 | 436430 wide EC cleanup |
| 2 | 1 | CB8056 / CB81C6 | 41DD20 narrow F4 cleanup |
| 3 | 2 | CB8064 / CB81D4 | 5D6CF0 vector at **194**, not 198 |
| 4 | 3 | CB8072 / CB81E2 | 436430 wide 1A4 cleanup |
| 5 | 4 | CB8080 / CB81F0 | 41DD20 narrow 1C0 cleanup |
| 6 | 5 | CB808E / CB81FE | 41DD20 narrow 1C8 cleanup |

The default helper owner is saved at FH3 frame EBP-20h; copy uses EBP-10h.
Default stores states 0 then 6, and copy stores 0,1,4,5,6. The optimized
absence of intermediate state stores does not remove those cleanup actions.
Their handlers tail-jump BF6B43 (`__CxxFrameHandler3` hypothesis/library name).

Factory handler CB6E10 selects DED3A0/map DED390: states 0 and 1 both
transition directly to -1 and return the saved failed raw slot through AB76F0,
via CB6E00 and CB6E08. This happens after the constructor's own unwind. It
does not call AB8250, run a successful Text destructor, decrement +04, or
invent a successful-payload cleanup. The existing raw pool return is real.

These are actual native EH table/control-flow facts. A Source exception
projection must distinguish native construction failure from later metadata
admission failure, retain reached creators/publications, and never replay
consumed effects. They do not establish native FH3/SEH or binary entry ABI parity.

## AB8530's actual fresh-default branch

The zero primary node at `+4C` does **not** make draw-section creation empty.
At AB8553 the first test is current `+188`. Fresh AB9650 sets this to zero,
so it necessarily enters the following normal path before testing `+4C`:

1. Allocate the actual Model slot through B74EB0, construct temporary `Shadow`,
   execute raw B75030, and publish the result to the same raw Text `+188`
   **before** returning the temporary name. Clear current shadow `+138` bits 0/1.
2. Allocate/construct a real mesh. FLD current D7A260, capture current Text+188
   between that load and argument stores, FST scalar17C, disarm raw-mesh
   cleanup, then FSTP scalar178. B75170 receives `(0,mesh,scalar178,scalar17C)`.
   D7A260 currently holds `BF800000` (-1.0), and its x87 handling matters.
3. Create the actual 533FA0 section, append it with B73C60, construct the
   actual `guidefault.mshd` name header, enter genuine numeric535320, return
   its name and assign the material with B864C0.
4. Consume section, material and mesh creator references in that order through
   actual +04/current virtual0 behavior. Do not add a protective retain.
5. At AB86CB reload **current** Text+4C. If still zero, AB86D0 jumps AB87B9
   and returns, skipping all main-geometry, parenting and root work. If a
   callback changed it, preserve the genuine main Model/geometry/attachment
   path. The earlier constructor zero cannot replace this late read.

No text-content, font or `+15C` read occurs on the late-null main path.
`+15C` is relevant only when the late main node is nonnull. Native AB8530
does not create the missing main drawable.

AB8530 handler CB7F59 selects DEED34/map DEED58: state0 returns only raw
Model through B748C0; state1 conditionally clears temporary mask bit0 before
41DD20 name cleanup then transitions to state0; state2 performs that conditional
name cleanup directly to -1; state3 returns raw mesh through B72F70; states4/5
destroy the temporary effect-name header. Completed Model/mesh/section/material
creators and Text+188 publication do not gain destructor rollback on exception.
The future Source caller must retain native header/mask backing, unfinished raw
slots, acquired references, registration status and release-consumed progress.

## Actual providers, remaining gaps, and next Source packet

The current `GuiTextRuntimeFactory` uses AB79E0's real slot, but it initializes
only `NativeGuiTextIdentityPrefix` in that storage. Its `GuiTextLifetime`,
strings, colors, cursor and shadow fields are separate C++ members. Default
semantic base construction even precedes its pool call. Neither its pointer
nor its typed member layout can serve raw AAA710 or a native Text address.

The fourteen `kGuiWidgetFactories` rows use real shared-list process pools but
retain `GuiWidgetConstructionHost` callbacks; Text is not one of those rows.
Group's newer real raw factory does not supply Text construction.

Existing concrete prerequisites for the preferred raw AB8530 packet are:

- Actual raw B75030 and `NativeInstanceGeometryAccess`'s genuine same-slot
  prepare/retire-failed/bind-completed Model lifecycle. Raw Node name-pool mode,
  the existing Model pool, actual constants and canonical actual-owner registry
  must be the same throughout; no new Text or Model hierarchy is needed.
- `GuiNativeGeometryOwners::create_native_mesh_00b73b60` and
  `create_native_section_00533fa0`, with their stable acquired frames. Prefer
  these tracked native providers to older convenience factories which add
  stronger null/host-failure cleanup policies.
- Raw B75170, B73C60 and B864C0 plus current actual-owner releases. For a late
  nonnull main node, existing actual Model geometry access and raw node
  parenting/root providers close the reached normal branch.
- Genuine actual-header numeric535320/cache/loader/material admission, either
  its real persistent context or `GameNativeGuiEffectApplication`'s already
  published material-operation interface. The existing renderer, string pool,
  mesh/section/material pools, profiles and actual owner registry remain alive.
- Actual temporary narrow-header construction/return and raw-pool getter context;
  no noexcept semantic-release adapter may silently change exception behavior.

Implementing that genuine raw receiver schedule is preferable to accepting a
default/no-op callback or treating the late-null path as empty. It can then
close full raw AB9650 using existing raw AA9390, the exact derived stores and
outer raw AA9730/container cleanup. A fresh-shadow/null-main-only overload
would need an explicit late-null supported domain and must not advertise
complete AB8530. The proposed new files are
`native_gui_text_storage_construct.hpp/.cpp`; a new Source lease and Root review
remain necessary. This packet has not made that Source change.

Copy remains a separate larger packet. Existing raw AA9520 admits actual
nonnull source+4C Model clones under its flags26/3E,parent0 contract. Raw wide
resize/destruction exist, while the current 4C8DD0 implementation exposes a
typed header plus resize callback. Cursor and rebuild implementations still
take canonical typed Text lifetimes. They must be converted through actual
raw header/field access and the same genuine resource providers before raw
ABB2C0 or whole AA1380 can be claimed.

The real FontHost producer/export/bind protocol, borrowed existing registry,
mutable Text state and explicit locale/comparison profiles already exist.
Normal native Text remains unactivated. These providers are useful dependencies;
they do not establish raw layout, default callbacks, fixed global ownership,
full CRT order, native ABI or gameplay.

Receipt checks: the supplied Source79 report contains zero call rows, which is
not coverage evidence; Source76 verifies eight rows with zero failures. This
report's seventy direct call/tail-jump rows pass live validation with zero failures. No new
build, fixture, runtime, rendering or game result is claimed.
