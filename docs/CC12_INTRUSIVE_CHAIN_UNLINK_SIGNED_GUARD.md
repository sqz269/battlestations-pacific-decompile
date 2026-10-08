# Intrusive chain unlink signed-count correction

The existing `chain_unlink_00903f30` handle/vector Source helper used the
opposite count guard. A sole live node has both links zero and count one;
the old helper returned without removing it. The corrected branch returns
only when both links are zero and the signed count is greater than one.
One regression case in the existing semantic check verifies that removing
the sole node clears first/last and makes count zero.

Complete Native `00903F30..00903F83` and `00924710..00924763` extents are
83 bytes/29 instructions each. The fresh verified Ghidra listings and memory
agree with the installed PE. Both use `CMP DWORD[ECX+8],1` followed by signed
`JG` to the return. They do not skip count one or add a count-zero guard.
Original ABI is ECX=actual `{first,last,count}` header, one stacked actual
entity pointer, `RET 4`; the link offsets are `34/38` and `40/44` respectively.
The descriptive names remain hypotheses.

The normal MSVC Win32 build and all three existing checks passed. All 469
actual selected compiler inputs match 476 physical preimages frozen before
that build. Exactly one of the complete object's 180 extents changed; the
other 179 bodies and ordered relocations are unchanged. The current complete
113-byte helper has no relocations and emits `CMP [EDX+8],1` / signed `JG`
at Source offsets `2E/32`. Its whole object is a unique Core archive member.

Receipt: `local/cc12_intrusive_chain_unlink_signed_guard/receipt.json`,
SHA-256 `ccb4043472c2810973c7cfd39352996b5d69df4ae1d5aa5b1dbd4ca31b5d5400`.
The normal build log and complete old/new objects, compiled fixture and
Native listings/bytes are retained in that same ignored evidence family.

This corrects the already-counted semantic Source interface. Its vector
handles and bounds admission remain; it is not a raw entity/world producer
or an Original pointer-layout/class ABI replacement. No new reconstructed
function credit, selected Native unlink execution, startup or gameplay proof
is added. The installed executable remains unchanged.
