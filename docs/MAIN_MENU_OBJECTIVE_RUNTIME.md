# Main-menu objective page and vector producers

Addresses: `00519DC0`, `0051CC90`, `0051DDA0`, `0058F5B0`, `005C5B40`.

The main-menu vectors at `+2CC/+2DC` contain borrowed widgets from the briefing
screen's resource-loaded page map. They are populated by `0058F5B0`, called at
`00594C91` inside the objectives page builder `00594BF0`. The vectors contain
authored Group pairs, primary before secondary. They are not the Text clones
created later in `00594BF0`, and no replacement widget tree or node map is used.

The C++ interface composes the existing page registry/loader and canonical
`GuiWidgetOwnerRuntime`. It models native map/vector ownership with standard
C++ containers; those containers must be the sole corresponding screen fields.
The providers resolve only the actual selected mission and briefing-screen map.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| `00519DC0..00519DEA` | ECX vector; unsigned index stack; RET4 at519DE8; EAX pointer to element | complete normal checked accessor; BF6713 becomes out_of_range |
| `0051CC90..0051CD7D` | ECX map; native-string key stack; RET4 at51CD7B; EAX node+14 value slot | complete normal map indexing semantics; native tree/allocator/SEH replaced |
| `0051DDA0..0051DFC4` | ECX briefing screen; RET | complete normal loading sequence; native string/container allocator mechanics replaced |
| `0058F5B0..0058FED0` | ECX main-menu screen; RET | complete normal objective binding sequence; native vector allocator/debug iterators replaced |
| `005C5B40..005C5CCC` | ECX mission tree; output vector stack; RET4 at5C5CCA | complete normal duplicate-preserving append sequence; native string/vector allocations replaced |

`STL_inst_00519dc0` is the correct existing library name and remains unchanged.
No Ghidra mutations are made in this worker packet. Proposed descriptive names
in the report are hypotheses for integrator review, with old names preserved.

The briefing map is constructed empty by `0051E4D0`: node pointer `+130` and
count `+134`, with map object `+12C`. `0051E280` calls `0051DDA0` during briefing
screen registration. The loader only runs when that map is empty. It collects
layer names from `005C5B40`, inserts/finds the original key, calls
`load_gui_page_00aa5840` with `briefings/<key>,1,false`, then publishes the actual
loaded root. A null loader result remains a null entry. An exception retains
any entries already inserted, so a later invocation skips the nonempty map.

`005C5B40` visits groups and missions in stored order. For each grouped mission
it picks side0 when its enabled byte is nonzero, else side1, and appends that
side's nonempty `briefingGuiLayer`. It then visits all multiplayer records and
appends both sides' nonempty keys. `00450540` is vector push_back; duplicate keys
are retained. `0051DDA0` therefore still visits duplicates, invokes the page
loader and reports progress for every occurrence. Existing registry reuse
returns the same page for repeated names. Global byte `00E18D91`, reread each
iteration, forces resource name `briefings/IJN05` while preserving each original
map key.

At `0051DF36..0051DF54`, FILD signed index, FMUL qword `00CEC8F8`, FADD qword
`00CEC8F0`, FSTP float, FLD float and FSTP argument feed `0057BEC0`. The new helper
preserves that x87 schedule and composes the existing concrete loading-progress
implementation over its actual current singleton and conversion services.

`0058F5B0` first clears both borrowed vectors without deleting their widget
payloads. It captures the selected mission through `005806A0`, computes the
side with `005C27E0`, reads the current briefing-screen map `[E198AC]+64+12C`, and
indexes by record `B8 + side*154 + 4`. The selected root is published at `+2C8`
and receives current virtual34(true). Direct `BG_01_Group` lookup publishes
`+2EC`; this function does not require that lookup to succeed or show it.

| Screen field | Producer evidence and interpretation |
| --- | --- |
| `+64` | starts1 at58F781; increment58F890; decrement58FAAC: contiguous primary Group count |
| `+68` | starts1 at58FB0D; increment58FC1A; decrement58FE3B: contiguous secondary Group count |
| `+6C` | zero58FE40, count side hiddenObjectives begin+38/end+3C at58FE60..58FE78 |
| `+2C8` | 58F678: actual page-map value selected by canonical mission side briefing_key |
| `+2CC` | begin2D0/end2D4/capacity2D8; stores58F7E1 and58FB7B or vector insertion |
| `+2DC` | begin2E0/end2E4/capacity2E8; stores58F846 and58FBD7 or vector insertion |
| `+2EC` | 58F6D6: actual direct BG_01_Group lookup result |

For each primary index starting at1, direct lookup of
`<index>_pri_objective_Group` must exist to continue. Its companion is
`<index>_pri_Group`. Both pointers are published before hiding objective then
companion through their actual current34. Secondary pairs use the analogous
`_sec_` names and append to the same two vectors. The first missing numbered
objective stops that category; later numbered children are not visited. A
missing companion is published and then reaches the native invalid dereference
domain after the objective hide; the C++ interface throws there.

The lookup's second argument0/1 is preserved even though the full `00AA7E00`
body ignores it. Lookup is direct-child only and reads actual scene node name
headers through the existing canonical-owner overload. It reloads current
screen+2C8 for each lookup, including after visibility callbacks. The captured
objective and companion survive those callbacks. Temporary generated names
use existing native string operations and storage; suffix construction precedes
the current count read, and temporaries retire before generating the other name.
Finally the originally captured mission is used again with a fresh side index
to count canonical `side_extras[side].hidden_objectives`.

Every CALL instruction in the five owned bodies is represented in the report,
including native allocator/STL/error mechanics replaced by C++ ownership.
Direct call addresses are checked by `verify_report_calls.py`; indirect34 calls
also carry original table evidence. Group `D5CB80+34=D5CBB4` contains
`30 85 AA 00` (`AA8530`); page/Screen `D5BE38+34=D5BE6C` contains `50 44 AC 00`
(`AC4450`). Runtime type dispatch remains required: the loader and visibility
paths cannot be completed with dummy Text, Group, Layer, material or node owners.

Three listing gaps were read as original bytes and are included separately in
the report. `0051DFAF..0051DFB1` is `83 C4 04` (ADD ESP,4), the cleanup after
the free at51DFAA, and falls through into51DFB2 despite having no Ghidra function
body membership. `005C5BAA..005C5BAF` is `8D 9B 00 00 00 00` (LEA EBX,[EBX]),
alignment bypassed by the unconditional5C5BA8 jump to5C5BB0; also no function
membership. `0058FB29..0058FB2F` is `8D A4 24 00 00 00 00` (LEA ESP,[ESP]),
alignment bypassed by58FB27's jump to58FB34. None of these gaps contains a CALL.

This packet does not implement the later text/Listbox row-building body
`00594BF0`, the briefing screen's complete registration/entry/destruction, or
native binary layouts, allocator callback schedules for standard containers,
corrupt iterator termination, and SEH. Compilation and local fixtures are
reported separately from runtime rendering/gameplay; no game equivalence is
claimed.

Validation: strict MSVC Win32 `/W4 /WX /fp:strict` compilation passed. The final
`scripts/build.ps1` build passed, including the existing reconstructed_math
test (1/1). The focused ignored `local/objective_container_probe.cpp` fixture
passed against the newly built library: grouped side selection reads only the
enabled low byte, empty keys are omitted, duplicates/order and prior output
are retained, case-insensitive map aliases share one stable value slot, and
the accessor returns the actual vector element reference including null
payloads while rejecting unsigned out-of-range indices. This fixture does
not execute resource loading or canonical-owner Group visibility traversal.
The report's 139 direct calls passed live verification; five virtual34 sites
have separate table-byte evidence. No new permanent test suite was added.
