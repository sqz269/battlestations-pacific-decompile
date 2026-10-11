# Root retirement action candidate review

Root accepts the complete eight-byte candidate at `00C86200`, with physical
`MOV ECX, SS:[current EBP-10h]` and `JMP 00BEA990`. The E9 pushes no return
address. Saved CALL_RETURN metadata does not change its physical operation.
The candidate's Original FH3 role and invocation remain conditional.

Root independently decoded the ModRM and signed E9 displacement and ran actual
detailed Capstone 5.0.7. Root also decoded the immediate positive parent report's
93 bytes and checked its 28 instructions. The receiver spill starts at
`007379BA`, encoding `89 74 24 08`. Six pushes place storeESP at parent entry
ESP minus 18h. Under the same SS, matching the candidate's load address requires
`actionEBP = storeESP+18h = parentEntryESP` modulo 2^32. Obtaining receiver A
additionally requires preserved or reestablished cell contents. This establishes
an address condition, not the Original dispatcher's frame binding or ABI.

Root compared its original exact-entry and whole-body gate bytes before copying
archive helpers, verified all 109 archive entries, compared 21 retained actual
Git objects with Git, and checked nine finite inputs and exactly two final
outputs against the worker commit/index/files. The original and worker copy are
byte-identical with distinct physical file IDs. Root executed the fully reviewed
pinned pure replay under an audit allowing exactly one archive read, one helper
execution and one receipt write; it passed without denials. One corrected offline
prefix assertion failure and its producer preimage remain retained. No Native
query failure is claimed. Worker commit: `c63bf02239a008197710b3713c3dd0c16f3d3b20`.

Private evidence is under
`local/cc12_application_vector_retirement_action_candidate_entry_primary/`.
The public report pins Root gates, decode, actual Git review and strict replay.
Archive SHA-256:
`971024e1ecc93fa49ea5828e495f71a8b35440eb3f54d4c9d9b56c7a60cba2e5`.
Native body SHA-256:
`2c08320829fdef733253cdfa9bbf17d2f4a0f6a6e69eab756c032554f9f5d910`.

The next Source parent may use explicit actual A/context and qualified C++
cleanup through the accepted BEA990 implementation. It must preserve the actual
signed live row-resize behavior, reload data before genuine array free, and
keep cleanup armed through that free. This admission adds no authenticated
Original EH states, destination behavior, Source implementation, build, fixture,
ABI, startup or gameplay credit. No fresh build was needed for these documents.
