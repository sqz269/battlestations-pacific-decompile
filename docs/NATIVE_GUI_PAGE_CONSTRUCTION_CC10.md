# Raw GUI page construction prefix

Addresses: `00AC6600`, `00AA6720`; read-only continuation/FH3 evidence
`00AC6714..00AC6878`, `00CB8A40..00CB8A9C`, `00DEFA98`, `00DEFABC`.

| Source entry | Native coverage | Status |
| --- | --- | --- |
| `initialize_native_gui_page_00ac6600_prefix` | normal prefix `AC6600..AC6713`, 276 bytes | **partial projection**; remaining `AC6714..AC6878` is not implemented |
| `bind_native_gui_widget_node_00aa6720` | `AA6720..AA6734`, 21 bytes | complete source body, new C++ interface |

This is an actual-storage initialization stage, not a page factory or a finished
constructor. It borrows the canonical `AA9390` raw widget base, actual string
pool and original constant cells, initializes the same raw `124h` page slot,
and stores the actual scene node. It never casts `GuiLayoutPage`,
`GuiWidgetOwner`, `GuiPageLuaEvaluation` or a companion to native storage.
It returns no page pointer or success boolean. The retained frame reports
`awaiting_lua_continuation`, native instruction `AC6714`, and EH state 1.

## ABI correction and producers

The full original constructor is ECX raw page, three DWORD stack slots
`(NativeString* name, actual_node*, share_scene_low_byte)`, EAX original page,
RET0Ch. At `AC66AB` the **second** argument becomes EAX and is pushed into
`AA6720`; at `AC66A4` the **third** argument's low byte becomes DL and is stored
to page+120 at `AC66EE`. Higher bits are ignored; the byte is not normalized
to bool. Caller `AA5944` pushes the third slot, ESI node, then EDI name.
The initial ledger text's `(name,int share_scene,char unused)` signature is
wrong; this confirms its later appended correction rather than replacing that
evidence. Its old callsite `AA5931` is interior to a MOV; the call is `AA5944`.

`NativeGuiLayerPool` already defines object size124h, slot size128h and the
allocator ID at+124. Existing `AA9390(type1)` produces the actual base and its
12-byte list sentinel at+68. It leaves +E4/+E8 and padding untouched; the prefix
preserves those bytes and the pool ID. `NativeNodeStorage` and its producer
`B6F5A0` already define the node's `auxiliary_flags_138`. The new binder uses
that exact type/offset, writes page+4C first, then masks the current node flags
with FFFFFFFC when the node is nonnull. There is no AddRef or release.
Its other callers bind a newly constructed model at `AA6695` and a returned
clone at `AB9D8C`; no additional bind argument or ownership operation appears.

## Prefix schedule

After the actual base returns, capture destination name address+100 and compare
it to the original name before further page writes. Publish D5BE38, clear
EC/F0/F5/F8/FC, enter native state0, then zero the eight-byte page name header.
Unless the headers are identical, resize from current source length and copy
current bytes through the established overlap-safe BF7680 semantics. Native
copy site `AC6691` is followed by ADD ESP,0Ch at `AC6696`.

Read D7A2F0 before storing +108=0 and +10C; read CE3804 for+110; read D7A24C
once for+114 and retain the same word for the later +20/+24 writes. The original
constant bits are 3DCCCCCD (0.1f), 447A0000 (1000.0f), and 3F800000 (1.0f), but
source borrows their current cells. Zero +118/+11C, write low-byte argument3
at+120, zero+121, enter native state1, store the two size words, and call the
complete raw node binder. No Lua state has yet been created at this boundary.

## Outstanding lifetime and continuation

The caller retains the original raw pool slot, page name allocation, base list
sentinel and bound node obligation. The frame owns none of them and has no
cleanup destructor. A prefix object must not enter the page registry, a logical
page vector, normal rendering, or an ordinary scalar-deleting path as though
the full constructor had returned. Replaying the frame is rejected.

The unimplemented normal continuation must construct the actual4C8h Lua state
with B66BD0; open with mask65h through B6A020; compose/run
`interface/_Common.lua` and `interface/<name>.lua`; create globals and the raw
14h CE44FC reader; enter `GuiScreen`; invoke the page's **current** slot18
(known D5BE38 target AC4C50); leave the table; propagate current page+4C root to
null through B6D890; tidy the reader; and close Lua. All call rows are recorded,
including the omitted continuation, without presenting those calls as source
implementation. Existing Lua state/bootstrap/file providers are concrete;
the raw reader/vector and AC4C50/AAA710 property/tree composition are separate
frontiers. The primary is examining the reader independently.

Descriptor DEFA98 has seven states and map DEFABC. State0 cleans the widget
through `CB8A40 -> AA9730`; state1 first destroys page+100 via
`CB8A4B -> 41DD20`, then state0. State2 closes Lua and transitions to1;
states3/4 release the current temporary path and transition to2; state5 destroys
the reader through441A20 and transitions to2; state6 uses410600 and transitions
to2. The report gives all action ranges and targets.

The raw full widget destructor is **AA9730..AA99B8**; AA9750 is an interior
address. Current logical widget/text cleanup covers projections rather than
that complete raw destructor. This prefix therefore retains acquired state on
source C++ failure. It does not silently imitate native unwind with a logical
destructor, release only the name, free a bound node through a guessed owner,
or return the pool slot. A failure before the base returns still uses AA9390's
existing internal allocation-failure cleanup, with its documented boundary.
After base return, `native_eh_state` identifies the pending original cleanup;
external resolution must discharge it before reuse or release. This is an
explicit source failure boundary, not native exception equivalence.

The raw handler `CB8A93..CB8A9C` is ten bytes (last instruction CB8A98, length5)
and is absent as a Ghidra function. It loads DEFA98 and jumps to BF6B43. It is
listed for primary definition; worker access was read-only.

## Validation

Current results and source/byte hashes are recorded in
`reports/native_gui_page_construction_cc10.json`. The MSVC Win32 Release build
and both existing CTests pass. The production translation unit and comparison
probe also compile with `/W4 /WX /MD`; the probe embeds its manifest.
The call checker reports 32 checked rows and zero failures, with symbolic
indirect and raw-handler limitations recorded. All633 constructor bytes and
21 binder bytes match between live Ghidra and the installed PE.

Two normal prefix comparisons pass against the original 276-byte stage and
21-byte binder. The fixture redirects the original frontier to its normal
epilogue, supplies actual constant cells, and bridges AA9390/resize/CRT to the
current production providers. It uses the actual canonical page pool, a
constructed actual string pool, and the actual node producer. It compares all
slot bytes after normalizing independently allocated name/sentinel pointers,
checks those referents, preserves pool IDs and unwritten E4/E8, and checks the
node flags and source continuation/state. Cases cover populated name/nonnull
node/non-Boolean flag byte and destination-name alias/null node/zero low byte,
with changed constant preimages in the latter. Fixture-only allocation cleanup
does not execute or substitute for native page destruction.

Build, static calls and bounded normal-prefix evidence do not establish full
page construction, Lua/UI behavior, native destructor/FH3/SEH parity,
application binding or gameplay.

## Primary integration and handler definition

Published worker bf070f370 in main 35a065629 after the strict Win32 integration build and all three existing CTests passed. The primary defined the exact ten-byte CB8A93 handler under the Ghidra write lock, saved the program, recorded the prior function state, applied the provisional name EHHandler_GuiPageConstruction_00cb8a93, and refreshed its export. The formerly unresolved CB8A98 call row now records the verified tail jump. This does not extend the AC6600 constructor prefix or establish exception-unwind parity.

Primary evidence archives and the separate finite application startup result are recorded in reports/cc10_lua_gui_renderer_integration.json. That application run does not exercise these still-unbound raw stages.
