# cc13-bigB2: large leftovers, whole bodies

Packet cc13-bigB2 (2026-10-17): four large leftovers from docs/CC13_SWEEPB10.md, each taken as a
whole body. Code: `src/cc13_bigB2.cpp`, `include/bsp/cc13_bigB2.hpp` (`Cc13BigB2Host` extends
`Cc13SweepB9Host`, so the sweepB..B9 hooks are reused). Nothing is wired. Names are hypotheses.
Status words: reconstructed and build-tested only; not ABI-compatible, not game-validated.

## 00670020 support-manager screen refresh (reconstructed)

`cc13_bigB2_hud_support_refresh_view_00670020`, 2147 instructions, `__fastcall(ECX = screen)`, RET.
EH frame 00C7BF30 (states 0-10h) noted, not modelled. Main thread: the two x87 steps (`1 - h`,
`DE E1` = FSUBRP st(1), st, and the FMUL by the double 0.5 at 00D7A280) are modelled as float
operations. Callers: 00671D60, 006723F0, 00674B80, 00675C40 (cc9's per-frame update). Read from the
listing, ESP anchored on the EH-state stores at [esp+0FCh]; every call argument was taken from the
pushes, not from the decompiler (which loses the thiscall arguments and swaps the 45h/46h flags in
the gauge section).

Order of work:

1. 0065A2B0 (cursor sync); every slot widget +8Ch[i]: +88h(3, 0, 1.0), +34h(1); slot text
   +9Ch[i] and page mark +0CCh[i]: +34h(0). +1F4h set: 0065A550 (slot strip).
2. With entries (+34Ch): 0066FB00; a changed count while +2B1h is clear resets the cursor
   {+350h, +354h} to begin() of the list +344h, +308h = 0, +288h = 1, +28Ch = 1.0f; +2B1h = 0,
   +45Ch = count; 0065A2B0; the cursor steps back +30Ch times (checked --, begin() wraps to end()).
3. Slot labels. More than four entries: slots 0..3 get +88h(0, 0, 1.0) and a label. Four or fewer:
   every slot +34h(0); the first `count` slots +88h(0, 0, 1.0), +34h(1) and a label; the remaining
   slots +88h(3, 0, 1.0) and text +34h(0). A label is the name of the cursor value's object
   (+14h), else of the first element of its list (the element's +3Ch target unless that target is
   kind 9, else the element itself); the text after the last '|' (00CF100C) is shown on the slot
   text through 00ABAED0 when it is at most three bytes, otherwise the text is hidden. The
   object source of the four-or-fewer path inlines the reverse search (006709C5); the other five
   call 004BCB80. The cursor advances (checked ++, end() wraps to begin()) once per slot.
4. 0065A2B0 again. With +1F4h: the cursor unit (+300h/+304h) of kind 46h sets the icon +1CCh to
   state 0, kind 9 to its cached state (map +42Ch keyed by (unit+538h)+70h; a miss adds a state
   with texture (unit+538h)+0D4h and uv {0, 0, 1, 1} through 00AB64C0), kind 45h to state 2. Every
   marker of the map +3FCh gets +88h(0 for the cursor unit else 1, 0, 1.0), +34h(1), and the map
   position (00658580) of its unit's cached world translation (+0FCh..+104h, refreshed inline
   from +74h and the parent +3Ch through 00414DB0 / 00413920) through 00AA7D00.
5. With entries: the cursor value's unit list is copied twice (005AAAF0) and the first copy's list
   destroyed inline. Per unit, a fresh list (004E79F0) is assigned from unit+328h (local player
   +28h == 0) or unit+31Ch through an inline clear and 00664AF0, and released by 004E8150. An empty
   result sets +204h. Otherwise the unit's +3Ch target: kind 46h or 45h queries
   +198h("InferiorFailure", 0, &flag) and with the flag stores its health (00923BE0) as h46 / h45;
   kind 9 on the local player's side (+54h == player +28h) takes the cached icon state. The gauges
   +12Ch/+130h/+134h are hidden, then: both 45h and 46h seen -> icon state 3, +134h quad
   ((1 - h46) * 0.5, 3F49096Ch (0.78530), 0, 1) and +130h quad((1 - h45) * 0.5, 407B50B1h (3.92680), 0, 1) when their
   flags were set, each followed by +4Ch(0.4f); only 46h -> icon state 0, +12Ch quad(1 - h46, 0,
   0, 1); else kind 9 -> icon state (cached state, or 1 when it is 0); else 45h -> icon state 2,
   +12Ch quad(1 - h45, 0, 0, 1). The list copy is tidied (005A0B80).
6. Still with entries: each +338h value widget +34h(0); each +344h value widget +88h(1, 0, 1.0)
   then +34h(1) (each through a value copy and 005A0B80); the cursor widget +88h(0, 0, 1.0) and its
   resolved position (00AA6750) set back with z = +2D0h (00AA8240).
7. The page mark +0CCh[+30Ch] +34h(1).

Constants from the image: 00D7A24C 1.0f, 00D7A280 0.5 (double), 00CF6B0C 3F49096Ch, 00CF6B08
407B50B1h, 00CE7804 3ECCCCCDh (0.4f), 00CF0B74 "InferiorFailure", 00CF100C "|".

Uncertain or simplified: the tag dword pushed to 00664AF0 is a stale stack slot in the image
(passed as 0; the empty iterator-category tag is not read); the 16 matrix floats are copied through
FLD/FSTP in the image (an SNaN would be quieted there, not here); the list copies whose object and
widget fields are never read are still built, as the image builds them. No 00BD2F10 draws.

## 00627960 mission objective lists rebuild (reconstructed)

`cc13_bigB2_mission_objectives_rebuild_00627960`, 2287 instructions, no register or stack arguments,
RET. EH frame 00C78FAD (states up to 48h, plus a flag word at [esp+10h] for the conditional
LuaObject temporaries of the `a && b` tests) noted, not modelled; each temporary is destroyed at
the image's point. Callers: 008BDEC0 (Objectives_ClientRefresh) and 00920A20 (cc9 band). Ghidra's
decompilation drops about 240 blocks as unreachable, so the body was read from the listing only.
No x87.

1. game+1FE4h == 2: return.
2. The twelve image vectors are emptied: name strings 00E196CC/DC/EC and ids 00E19768/78/88
   through 004954F0, player ints 00E19738/48/58 through 006226F0 (each after two checked
   begin()/end() tests), success bits 00E196FC/E19710/E19724 through 005F6190.
3. `Mission` and then `Mission.Objectives` (globals of the Lua state at game+1A0Ch, 00B67980) must
   not be nil (00B65FB0); the temporaries are destroyed newest first.
4. For "primary", "secondary" and "hidden": the table (primary fetched straight into the holder,
   the others through a temporary and 00B67690); when not nil, its keys are collected
   (00B67080/00B66420/00B66290/00B67190) into a stack vector<int> and sorted (00623170). Each entry
   (00B67720) with HideFromRewards nil and Active true is emitted: by `PlayerIndex` when set;
   else by `Party`, once for every player slot 0..7 (game+18CCh + 4i) whose +28h equals it, with
   the slot as letter and value; else for the local player's +20h. A single-player entry uses its
   position in the sorted keys as letter and is skipped when the player value is negative.
   Emitting appends `char('@' + letter) + Text` (00531030, 004261A0, 00450540), `Success` (false
   when nil), the player value (006257D0) and `ID` to the category's four vectors. The key vector
   is then emptied inline (the memmove_s branch never runs).
5. The table, the key buffer (00BF65AC) and Objectives are released; with game+1FE4h non-zero,
   00773860 on game+1EF0h.

Code-shape differences kept per block: the primary block calls 00442190 (key push), 0049F2D0 (bit
push) and 0041E870 (both strings on the party path); the secondary and hidden blocks inline the
key push (00441F30 on the slow path), the bit push (end() iterator, 0048D3B0, 0049E150) and every
string construction. Strings at 00CF5148 "Mission", 00CF513C "Objectives", 00CF5134 "primary",
00CF50FC "secondary", 00CF50F4 "hidden", 00CF5124 "HideFromRewards", 00CF511C "Active", 00CF5110
"PlayerIndex", 00CE5804 "Party", 00CEC778 "Text", 00CF5108 "Success", 00CE59B4 "ID".

Uncertain: the bool pushed to 0049F2D0 / 0049E150 is AL inside a dword whose upper bytes the image
leaves unspecified (passed as 0/1). No 00BD2F10 draws.

## 005DB050 multiplayer lobby widget binding (reconstructed)

`cc13_bigB2_lobby_bind_widgets_005db050`, 4159 instructions (0C3Ch bytes), vtable 00CF23B0 slot
18, `__thiscall(ECX = screen)`, RET. EH frame 00C74967 noted, not modelled. Read from the listing
with a local idiom compressor (`local/bigB2_compress*.py`: inline NativeString constructions,
pool releases and FindChild calls collapsed to one line each), and the raw listing wherever the
pushes interleave (clone parent pushed before FindChild's arguments, the resolved-position output
pushed before FindChild, the 7FFFFFFFh pushed for 00467CF0 left on the stack as the count of the
following 00469840, the PUSH 1 / PUSH 0Ch of the first map loop shared by its body and exit).

Order of work: the widgets of page +14h (players, settings, unlock, noseart, chat and event groups;
names and stores in the function comment), listener 00AA6BC0 on the screen+0Ch sub-object with
flag 1 for the frame boxes players/settings/unlock/noseart and 0 for arrows, sliders, the chat mode
icons and the settings list; the settings scroller +30h (00683790 text box, 00683380 with
back - slider heights, the back height held as a double and subtracted by FSUBR, rounded to float
under 24-bit PC), the chat scroller +158h and the event scroller +0CCh (00683790, 00683380,
006834A0 with textbox - clipbox heights). Then page +18h: Settings_Listbox +228h (byte +141h = 1,
+12Ch = 3C088889h, +130h = 3B360B61h, +134h = 0), setting_name/value texts hidden, the left/right
arrows; selected_description_Text +22Ch last.

The two map-icon lists (+1F4h under the settings group, after the settings scroller; +240h under
page +18h, before selected_description_Text) clone "map_001_Icon" once per map record (434h bytes,
vector at (*00E198AC)+5Ch+24h, checked iterator), name the clone "map_Icon" with the 1-based index
(004260B0, 004261A0, vtable +2Ch), copy the template's resolved position (00AA6750, 00AA8240), set
the shader "guidefault_noalpha.mshd" (00AB4630) and the eight state textures 0..7
"gui/radarshots/" + base.substr(0, 14h) + one of _C, _D, _E, _ICH, _ICL, _ICM, _ICS, _S ".tga"
(00469840, 0043C130, 004261A0, 00AB44B0 with uv {0, 0, 1, 1}), where base is the record path +20h
after the last '/' (00467CF0, 00469840) cut before ".scn" (00BF9440). After the last record the
template and every clone are hidden. The first list inlines its string constructions and its
push_back (slow path: 0043DB60) and tests the record dereference twice in sequence. The second
list builds its strings through 0041E870, appends through 0043DCA0 and nests the two tests.

Constants: 00CF2318 3E088889h (the New_5_Text ellipsis width, 00ABB000(.., 1) after 00ABBE50(" ",
1)), 00CEF0B8, 00CEE600 as above. No 00BD2F10 draws.
