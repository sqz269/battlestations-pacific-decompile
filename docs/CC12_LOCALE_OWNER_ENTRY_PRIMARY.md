# Locale owner entry targets: primary acceptance

Root accepts worker `9064d4d49` as bounded instruction-window evidence.
The 22-byte prefix at `CC8D30` sets ECX to `E1768C`, calls `436230`, then pushes
`CD9410` before a `BF6FF5` call and normal return. The ten-byte prefix at
`CD9410` sets the same receiver and tail-jumps to `435A50`. Registration/atexit
semantics remain provisional; the two entry sites still have no function bodies.

Root copied and hash-checked the finite proof, replayed its ZIP-only guard,
verified 4,215 frozen inputs and ten dependency blobs against actual Git trees,
and independently decoded all eight prefix instructions. Both 64-byte windows
match the retained original PE. The prefix terminators do not establish routine
extents, neighbor ownership or target behavior.

The next bounded targets have noncontiguous stored bodies: `436230` owns 208
bytes across two ranges inside a 211-byte span; `435A50` owns 124 bytes across
three ranges inside a 138-byte span. Their bodies were not read in this packet.
A separate packet admits lifecycle-contract review while preserving those gaps
and the original listing, flow and prototypes.

The evidence is frozen at GPR modification67; later primary names/comments are
a separate saved epoch. All 4,215 current Root Source inputs match this freeze.
This packet changes no C++, runs no build/test, and proves no owner lifecycle,
Source binding, binary ABI or game behavior. Composition and activation remain off.

Evidence: `reports/cc12_locale_owner_entry_primary.json` and
`local/cc12_locale_owner_entry_primary/Root_guarded_replay.json`.
