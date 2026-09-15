# Camera viewport access and the post-effect submission dependency

The source reconstructs only B6FDE0..B6FDE6: read the current borrowed viewport
pointer at actual camera +180h into EAX and RET. ECX is the actual camera, with
no stack arguments. No retention, copy, registration or null substitution is
added. The one-input naked fastcall leaf preserves the native physical register
behavior. Its seven native bytes are the packet's only proposed body credit.

B4DEF0..B4DFA0 is **not implemented**. Its complete 177-byte normal schedule is
known, but the final draw provider has not been reconstructed over actual native
pass and entry storage. No preparation-only adapter or callback substitutes for
that missing dependency. B529A0's update composition also remains open.

The native submission receiver uses the existing 24h post-effect layout. It
saves EBX/ESI/EDI, has no stack arguments and ends in RET. Its call schedule is:

1. Read receiver +08 and call B1F6D0 with color slot0; capture that surface.
   Reread +08 and call B1F6D0 again, capturing a second surface.
2. Read the first surface's current virtual +20 for height, then the second
   surface's current virtual +1C for width. D619A0 selects B3CD20/B3CD10.
3. Capture current receiver +0C camera and call B6FDE0. Pass the captured width
   and height pair to B1F940 on the returned viewport.
4. Capture current F8D394 renderer/profile, then current +0C camera, and call
   renderer virtual +A0. D5F0A8 selects the real B285A0 camera preparation.
5. Capture F8D394 again and its profile **before** calling B6FDE0 on current
   +0C. Then call +A4 from the captured profile on the captured renderer with
   the returned viewport. The concrete slot is B26770.
6. Capture F8D394/profile again, read current receiver +08, and call virtual
   +98 with that frame target. The concrete slot is B24E70.
7. Read current receiver +14 material, material +7C effect, and effect +9C
   pass. Read its current virtual +08, then receiver +1C draw entry, and call
   the pass. D61BE8 selects B454D0.

B454D0 is a separate unlisted 15-byte wrapper after B45360's body ends at
B454C6. It takes ECX pass and one stacked entry pointer, pushes null override
and that entry, calls B44750, then RET4. Its raw span and absolute call target
are preserved as evidence; it is not defined, renamed or credited in this
packet. No existing caller/callee graph edge proves the missing entry boundary.

B44750..B44B03 (948 bytes) is the next substantial dependency. The current
`material_entry_dispatch` interfaces project `MaterialEntryGeometry`, shared
`CompiledMaterialPass` objects and bool/error channels. They are recorded as
semantic fragments, not a complete actual-pass/raw28h-entry provider. Current
main was checked read-only as well as this worktree; no replacement full
provider was established. B44750 in turn reaches incomplete B43410..B43667
(600 bytes), plus named-but-incomplete vector/stream/section helpers. Existing
matrix, renderer stream-frequency, clip-plane and camera providers must retain
their concrete actual-storage and calling-convention contracts.

A follow-up packet must recover B44750 and B43410 control flow/ABI and close
their actual material constants, program bindings, geometry/index queries and
draw contracts before composing B454D0 and B4DEF0. Candidate source files are
`native_material_pass_geometry.hpp/.cpp` and `native_material_pass_draw.hpp/.cpp`;
addresses and files must be lease-checked before ownership is assigned. Whole
partition segments are not independent work packets.

The worker stopped at a usage limit after writing the getter and completing a
build log with one passing existing CTest. It left no commit, final input stamp,
report or documentation. The primary independently reviewed the source and
native leaf and preserves the partial worker evidence; final validation is tied
to a separate primary committed build. No new test is needed for this raw load.

See `reports/native_post_effect_submission_bv.json` for live/PE spans, concrete
slot addresses, the exact completed getter build and saved annotations. Only the
getter is reconstructed. No submission execution, final pass/entry composition,
new native fixture, broader binary ABI, exception or gameplay claim is made.
