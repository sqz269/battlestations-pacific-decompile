# Shader binary cache source review

Addresses: 00B3A600, 00B38A70, 00B35340, 00B34C70, 00B34C80,
00BE4300, 00BE45F0, 00BE4620.

The unfinished worker's two production translation units compile with strict
MSVC Win32 flags. No normal-path source defect was identified in the eight
reviewed routines. The completed comparison fixture runs the original cache
loader and stream/row bodies alongside the reconstructed loader. Both reproduce
all 1,628 records and all 2,250,700 bytes of the installed shader cache, with SHA256
`dcc65686e26e0b7b4f4179b45b153aaa353696b0b1b08c32251fbc7c97c0ecf8`.

Both sides use real read-only physical handles, actual reconstructed ReadFile
and raw string/pool providers. Array cookies, cursor publication, stream positions,
untouched stream bytes, EOF argument-pointer preimages and normal row cleanup
also agree. The original functions total 846 input bytes. The comparison uses
17 explicit direct-call relocations, a physical-read entry bridge, two constructor
PUSH relocations, two data operands, four original-side table entries and one
original-side stream-table pointer. The source-side numeric profile is unchanged.
Original CRT allocation ABI and EH-vector unwind are explicit external boundaries.

Fresh Ghidra captures match the installed executable and worker inputs across
eight complete bodies (1,153 bytes) and seven profile prefixes (284 bytes).
Listing review checks count*16+4 saturation, prefix-only row initialization,
publication and current-slot order, the shared parent temporary, captured string
length/data, space filling, output clearing, and stale headers after release.
Constructor/acquisition retain cursor and previous-publication behavior in source,
but are only statically reviewed and compiled. The linker removes these unused
entrypoints; the fixture's printed claim that they are linked is too broad.

Only a local copy of the prepared fixture was corrected. It had two wrong CALL
addresses, a missing context include, invalid Toolhelp names and pointer casts.
Its preparation script attributed five valid table captures to wrong addresses;
the reviewed report records their actual identities. Its unused cache context
also binds an open-visitor table where the VFS-manager table belongs. A future
constructor fixture must supply actual D685B4/BDF310 and a real VFS manager.
Two probes refused occupied preferred ranges before execution. The final fixture
uses fresh allocations with the explicit relocations above; earlier failures remain.

Worker production files are unchanged and uncommitted. They are not registered
in CMake, and no full VFS-to-cache construction, variants write, original FH3/SEH,
failure/overflow/short-payload behavior, canonical effect admission, drawing or
gameplay is validated. The B34C80 saved-listing gap and annotations remain deferred
under address ownership. See `reports/native_shader_cache_parent_review.json`
and the retained closure under `local/native_shader_cache_parent_review/`.

The live call verifier passed 23 direct calls; ten indirect calls are recorded
separately with their current profile targets. The immutable local checkpoint is
`local/checkpoints/b8776857/shader-cache-review/validation.json`, SHA256
`778e470b4f5dfe359714b51b014a483fdf47b53cb410af48c4cf7e56db28febd` (691 artifacts, eight physical x86 modules).
