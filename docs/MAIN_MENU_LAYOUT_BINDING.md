# Main-menu layout binding

Addresses: 005861B0 00541B80 00543710 00AA6BC0 00A9AC40 005861A0 00AA68F0 00AB7200 00AB2820

`bind_main_menu_layout_005861b0` implements the complete normal-path sequence of
the main-menu screen's +14h layout virtual: five page loads, 55 direct-child
lookups, visibility, listener and navigation-owner setup, two scroller attachments,
geometry/color capture, and the two embedded list/text initializers. All handles
refer to the existing `GuiLayoutWidget` graph. Descriptive names are hypotheses.
The projections are not binary replacements for the original 578h screen.

The checked Ghidra target is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Analysis used read-only `bsp.py` wrappers and original
PE bytes; no saved function names, comments, types, definitions or project state
were changed. Local name-ledger updates do not claim Ghidra annotation parity.

| Routine | Body, inclusive | Native ABI | Coverage |
| --- | --- | --- | --- |
| 005861B0 | 005861B0..0058827B | ECX screen, no stack arguments, RET | complete normal-path caller sequence |
| 00541B80 | 00541B80..00541CAC | ECX helper, RET30h | complete normal-path list initialization |
| 00543710 | 00543710..005437F8 | ECX helper, RET3Ch | complete normal-path text initialization |
| 00AA6BC0 | 00AA6BC0..00AA6BD3 | ECX widget, pointer+byte, RET8 | complete leaf stores, reused inline for this binder |
| 00A9AC40 | 00A9AC40..00A9AC4C | ECX Listbox, pointer, RET4 | complete analysis; required actual runtime field access |
| 005861A0 | 005861A0..005861AC | loads global screen, tail JMP005845D0 | complete raw-thunk analysis; callback target remains external |
| 00AB2820 | 00AB2820..00AB284B | ECX Icon, float2 pointer, RET4 | complete raw body analysis; actual derived scale dispatch required |
| 00AB7200 | 00AB7200..00AB7283 | ECX Text, state integer, RET4 | complete raw body analysis; actual Text dispatch required |
| 00AA68F0 | 00AA68F0..00AA6971 | ECX widget, output float4 pointer, RET4 | complete analysis; actual material getter required |

The raw thunk has no Ghidra function: instructions start at A0 (5 bytes), A5
(3 bytes), A8 (5 bytes); AD is padding. Its final instruction length is 5.
The three RET-immediate helper/leaf bodies end with three-byte instructions.
Raw Icon scale and Text state setters also have no Ghidra functions and end
with three-byte RET4 instructions (00AB2849 and 00AB7281).
No unread ranges remain within the reconstructed normal-path routines. Native
temporary-string allocation, failure behavior and SEH cleanup are represented by
C++ ownership; those exceptional paths are not ABI reconstructions.

## Corrections and ownership

The earlier `MAIN_MENU_MISSION_DETAIL.md` claim that **every** lookup uses +2F0h
is false. `00AA7E00` still searches direct children only. The binder changes ECX
to nested roots, including `FE_briefing/New_Group`, two separate clipboxes and
text groups, a slider group, and the listbox page. Recursive fallback would hide
incorrect ownership and can return the wrong identically named widget.

The +568h slot is first `historical_Group/bigtextbg_FrameBox` at 005869DE. That
root supplies the briefing clipbox and slider. After attachment finishes at
00586E3E, 00586F09 overwrites the same slot with
`mission_pic_Group/textbackground_FrameBox`. The reconstruction preserves this
single mutable field and the ordering.

The map companion +338h is `selector_Icon`, a direct child of
`FE_worldmap_historical`: ECX setup 00586F75, lookup 00586F8A, store 00586F8F.
+330h is explicitly cleared at 00587AC8 after backdrop capture. The map runtime
adapter can borrow `backdrop_328`, `selected_map_point_330`, and `selector_338`
directly from `MainMenuLayoutBindings` without a second set of handles.

`MainMenuLayoutBindings` stores offsets as descriptive suffixes, not a native
layout. Existing `GuiWidgetTransform`, `GuiWidgetOwner`, `FrontEndScreenScroller`,
`GuiTextColor` and `MovieWidgetState` declarations supply the field types. No
duplicate widget tree, material color store, or event dispatch registry is built.
`005861B0` is itself the producer of its screen handle/capture fields; helper
fields below are written by the complete initializer bodies.

## Lookup sequence

Every page call is `00AA5840(manager,name,1,0)`, RET0Ch. Every child call is
`00AA7E00(root,name,1)`, RET8; its second stack word is consumed but unread.
The containing function for every row is **005861B0**. Numeric roots and results
are screen offsets in hex. Temporary names identify register-held roots.

| Call site | Root | Name / type | Stored screen offset |
| --- | --- | --- | --- |
| `0058621C` | `GUI page registry` | `FE_main_listbox` / Screen | `470` |
| `00586297` | `GUI page registry` | `FE_worldmap_historical` / Screen | `2f0` |
| `0058631D` | `GUI page registry` | `FE_main` / Screen | `temporary` |
| `0058638B` | `FE_main` | `bg_01_anim_Movie` / Movie | `340` |
| `00586415` | `GUI page registry` | `FE_briefing_grid` / Screen | `244` |
| `0058649B` | `GUI page registry` | `FE_briefing` / Screen | `248` |
| `0058651F` | `248` | `New_Group` / Group | `temporary` |
| `0058658D` | `New_Group` | `Primary_Text` / Text | `24c` |
| `0058660D` | `New_Group` | `num_Text` / Text | `250` |
| `0058668D` | `New_Group` | `num_2_Text` / Text | `254` |
| `0058670D` | `New_Group` | `dest_Text` / Text | `258` |
| `005867C2` | `New_Group` | `dest_2_Text` / Text | `25c` |
| `00586877` | `New_Group` | `hl_bg_FrameBox` / FrameBox | `294` |
| `005868ED` | `2f0` | `historical_Group` / Group | `344` |
| `00586963` | `344` | `video_Movie` / Movie | `33c` |
| `005869D9` | `344` | `bigtextbg_FrameBox` / FrameBox | `568` |
| `00586A4F` | `568` | `test_Clipbox` / Clipbox | `3b0` |
| `00586AC5` | `3b0` | `textbox_Group` / Group | `350` |
| `00586B4F` | `350` | `content_main_Text` / Text | `3b8` |
| `00586BC9` | `568` | `slider_Group` / Group | `temporary` |
| `00586C3B` | `slider_Group` | `arrow_top_scroll_Icon` / Icon | `348` |
| `00586CC1` | `slider_Group` | `arrow_botton_scroll_Icon` / Icon | `34c` |
| `00586D47` | `slider_Group` | `textx_scroll_FrameBox` / FrameBox | `3b4` |
| `00586DCD` | `slider_Group` | `textx_scroll_bg_Icon` / Icon | `temporary` |
| `00586E8B` | `2f0` | `mission_pic_Group` / Group | `324` |
| `00586F04` | `324` | `textbackground_FrameBox` / FrameBox | `568` |
| `00586F8A` | `2f0` | `selector_Icon` / Icon | `338` |
| `00587000` | `2f0` | `mission_mappoint_template_Icon` / Icon | `114` |
| `00587084` | `2f0` | `mission_mapflag_template_Icon` / Icon | `118` |
| `00587108` | `324` | `missionpicture_Icon` / Icon | `2f4` |
| `0058717E` | `324` | `mission_name_Text` / Text | `304` |
| `005871F4` | `324` | `mission_name_backg_FrameBox` / FrameBox | `334` |
| `00587278` | `324` | `test_Clipbox` / Clipbox | `230` |
| `005872EE` | `230` | `textbox_Group` / Group | `1d0` |
| `00587378` | `324` | `arrow_top_scroll_Icon` / Icon | `1c8` |
| `005873EE` | `324` | `arrow_botton_scroll_Icon` / Icon | `1cc` |
| `00587464` | `324` | `textx_scroll_FrameBox` / FrameBox | `234` |
| `00587510` | `1d0` | `content_main_Text` / Text | `30c` |
| `00587586` | `1d0` | `mission_date_Text` / Text | `308` |
| `005875FC` | `324` | `checkpoint_Text` / Text | `300` |
| `00587672` | `324` | `checkpoint_FrameBox` / FrameBox | `2fc` |
| `005876E8` | `324` | `medal1_Icon` / Icon | `2f8` |
| `0058775E` | `2f0` | `missions_US_Group` / Group | `310` |
| `005877D4` | `2f0` | `missions_JP_Group` / Group | `314` |
| `0058784A` | `2f0` | `missions_US_DLC_Group` / Group | `318` |
| `005878C0` | `2f0` | `missions_JP_DLC_Group` / Group | `31c` |
| `00587936` | `2f0` | `training_Group` / Group | `320` |
| `005879AC` | `2f0` | `bg_01_Icon` / Icon | `328` |
| `00587A48` | `2f0` | `background_Icon` / Icon | `32c` |
| `00587B0A` | `470` | `ListBox_Group` / Group | `3bc` |
| `00587B80` | `470` | `SmallListBox_Group` / Group | `3c0` |
| `00587BF6` | `470` | `Sub_Listbox` / Listbox | `23c` |
| `00587C7B` | `23c` | `MainListbox_Text` / Text | `240` |
| `00587D3B` | `470` | `Main_Listbox` / Listbox | `1b8` |
| `00587DE7` | `1b8` | `MainListbox_Text` / Text | `238` |
| `00587E5D` | `324` | `arrow_top_Icon` / Icon | `1bc` |
| `00587EE3` | `324` | `arrow_botton_Icon` / Icon | `1c0` |
| `00587F68` | `324` | `arrow_botton_scroll_Icon` / Icon | `discarded` |
| `00587FE1` | `324` | `arrow_top_scroll_Icon` / Icon | `discarded` |
| `00588235` | `470` | `TextBox_Group` / Group | `argument to00543710` |

## Initialization and host-call evidence

The JSON report records all 60 page/child call sites and 45 further native call
sites, including the helper calls. The following table groups repeated calls by
contract; it does not replace the report's exact per-site rows.

| Call site(s) | Native target | Containing function | Recovered operation |
| --- | --- | --- | --- |
| 005862D1, 0058644F, 005864D5 | current widget+34h | 005861B0 | Hide world map and the two briefing roots. Canonical owner dispatch retains derived screen behavior. |
| 005865C7..005867FC | current widget+34h | 005861B0 | Hide the five objective text widgets immediately after each lookup. |
| 005863C9 | Movie+7Ch / 00AAFEE0 | 005861B0 | Set +F8h callback to raw005861A0, before loading briefing pages. |
| 00586759, 0058680E | Text+54h / 00AA68F0 | 005861B0 | Read material diffuse if geometry exists, otherwise widget color; capture at +270h and +260h. |
| 00586B02, 0058732B | 00683790 | 005861B0 | Attach content to existing scrollers +354h and +1D4h. This also resets scroll state and publishes positions. |
| 00586E3E, 005874C3 | 00683380 | 005861B0 | Attach thumb, track length, top arrow, bottom arrow; four stack words, RET10h. |
| 00586C78, 00586CFE, 00586D84, 00586F3D, 0058749D, 00587E96, 00587F19, 00587F6F, 00587FE8 | 00AA6BC0 | 005861B0 | Write widget+DC=actual screen+40h, then byte+79=0. The final two lookups repeat the mission scroll arrows. |
| 0058703A, 005870BE, 0058722E | current widget+34h | 005861B0 | Hide point/flag templates and mission-name background. |
| 005879FE | Icon+48h / raw00AB2820 | 005861B0 | Set both scale lanes to bits3F800054; base recompose,00AB2600 predicate, compareIcon+134h, conditionally dispatch+8Ch. This is not exactly1.0. |
| 00587A7C, 00587A9D | 00AA6740, 00AA6750 | 005861B0 | Capture original backdrop size and resolved position after the scale call. |
| 00587C2E, 00587D70 | 00A9AC40 | 005861B0 | Store actual screen+8h into each Listbox+114h. No inferred navigation operation occurs here. |
| 00587CF1 | Text+50h / 00AB6B50 | 005861B0 | Set RGBA=(bits3F4DCDD5,3F15959A,3EA8A8B1,3F800000), including shadow-material alpha. |
| 00587D80 | 00AA6750 | 005861B0 | Capture main listbox resolved position at +558h..+560h. |
| 0058801F | Text+80h / raw00AB7200 | 005861B0 | State argument0: choose disabled color if widget+77, otherwise normal color; call current+50h. |
| 005880F5 | 00541B80 | 005861B0 | Initialize helper+3C4h from ListBox_Group/ListBox_Text, count24, spacing20, gray/white RGBA. |
| 00588241 | 00543710 | 005861B0 | Initialize helper+414h from TextBox_Group/TextBox_Text, value32, gray/white/dark RGBA. |
| 00541B95, 00541BD7, 00541BFD, 00541C3B, 00541C4D | 00AA7E00, 00AAB4C0, 00AA67F0, 00AA7DC0, current+34h | 00541B80 | Find template, clone with null parent override, read template LOCAL position, set clone local position/bounds, show clone. |
| 00543759 | 00AA7E00 | 00543710 | Bind text before copying three color quartets and initializing flag/counter. |

The first scroller track is `background.size.height - thumb.size.height`:
00586DFD reads background; 00586E0B spills to double; 00586E1D reads thumb;
00586E25 subtracts; 00586E32 spills float. The second track is the literal
`0.10277777910232544f` (bits3DD27D28), not a recomputed geometry difference.

`00541B80` has exactly one caller, 005880F5. Its twelve stack words are group,
NativeString pointer, count, spacing, and two float4 colors; `00541CAA RET30h`
confirms grouping. Each clone is attached under the prototype's existing parent
by the already reconstructed `00AAB4C0`. The loop computes a signed32 product,
divides by the **double768.0** at 00CE42B0, spills float, adds that to the
prototype's local Y and publishes position before showing it. The helper keeps
borrowed clone pointers; their destruction belongs to the canonical GUI host.

`00543710` has exactly one caller, 00588241. Its fifteen stack words are group,
char pointer, integer, and three float4 colors; `005437F6 RET3Ch` confirms the
count. The three colors are `(0.5,0.5,0.5,1)`, `(1,1,1,1)`, `(0.2,0.2,0.2,1)`.
Field +04=32 is deliberately not given an unproven width/line-count meaning.

Complete listing filtering establishes ESI=screen at 005861D4 and EDI=zero at
005861D6 with no subsequent writes. EBX holds FE_main at 00586322, New_Group at
00586524, slider_Group at 00586BCE, and its background at 00586DD2. EBP becomes
screen+354h at 00586AF9, then returns to the SEH sentinel at 00586E9A; later
EBX is screen+1D4h, screen+8h and screen+40h. These changes explain why the
decompiler's losing ECX and stack arguments produced the prior root mistake.

## Verification and remaining integration

Strict compile-only MSVC Win32 C++17 `/W4 /WX /EHsc /permissive-` passed using
the map-runtime worker's updated Icon header. The binder depends on its concrete
`GuiIconRuntime::set_scale48_00ab2820` addition: it resolves the retained
`GuiIconTypeImplementation`, rejects a different implementation, and invokes
that actual runtime directly. The primary integrator merges that dependency,
registers the source and runs the combined build/checks.
The stronger primary `verify_report_calls.py` checked **87 direct call rows,
zero failures**; 18 indirect virtual rows remain explicitly marked and their
register/vtable evidence was checked separately. No new tests were added.

This packet does not connect the binder to `bsp_game.exe` or claim runtime,
render, gameplay or ABI compatibility. The existing loader and canonical owner
must supply actual Movie, Listbox, Text and copied-widget implementations. The
required native services have no fallback: Movie state resolution, Listbox+114h
storage, Text material-color reads, Text color/shadow publication and Text state
dispatch. The callback binding must implement raw005861A0's global-manager
selection and actual005845D0 behavior. A null callback is rejected.

Missing direct children, wrong authored types, or missing retained owners fail
explicitly. Such diagnostics differ from native null-dereference/allocation
failure behavior. Native SEH and allocator parity are not claimed. Rebinding
does not add cleanup of the native helper's old clones; this packet does not
invent a lifetime/reinitialization policy.

`00582F30` is the register virtual+10h which invokes this binder at+14h; it is
not the constructor (`005902E0`). Register, enter/exit, event routing, callback
target and game entry wiring remain separate dependencies. The existing older
partial-analysis documents retain historical claims; this page and the report
explicitly supersede their root and +338/+568 interpretations.
