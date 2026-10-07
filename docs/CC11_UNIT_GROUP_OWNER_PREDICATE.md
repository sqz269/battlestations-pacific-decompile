# Complete raw unit/group ownership predicate at 00778890

This packet reconstructs the full 24-byte, ten-instruction routine at
`00778890..007788A7`. Native ECX is the actual unit, there are no stack
arguments, and both exits use plain `RET`. **Only AL is the boolean result.**
The native nonnull path ends with `MOV AL,DL`; upper EAX retains the captured
group pointer's bits. The old name-ledger description of seventeen bytes and
`MOV EAX,EDX` is incorrect. Shared ledger correction belongs to the primary.

The raw source captures the pointer at actual unit+284h exactly once. Null
returns false. Otherwise it reads the captured group's +14h pointer and compares
it with the same actual unit identity. There is no group reload, callback,
default object, validation, initialization, or ownership change. Its `bool`
return is a new C++ interface and does not promise the original class ABI or
full EAX residue.

This is a direct dispatcher prerequisite. In `00780120`'s 97h arm,
`0078059E` places the actual unit in ECX and `007805A0` calls `00778890`.
`007805A5` tests AL; success plus positive message+28h leads to the formation
operation `0070EFD0` on that unit's group. The dispatcher and formation provider
remain open. The existing semantic function
`ship_ai_unit_group_leads_00778890(entity, group, group_leader)` consumes copied
values instead of loading actual fields and remains unchanged.

Native structural witnesses establish the borrowed storage identities:

- `0077EED0` clears EBX at `0077EF15` and initializes unit+284h to zero at
  `0077EF79`.
- `0070DAB0` initializes group+14h to zero at `0070DB0B`.
- `0070D7B0` stores the leader into group+14h at `0070D7B8`, then stores the
  group into that leader's +284h at `0070D7BB`.

These witnesses are structural evidence only. No constructor or publisher runs
in this fixture or becomes closed by this packet. In particular, `0070D7B0`
still performs genuine observer registration and a unit virtual +5Ch query;
that publisher is not replaced or bypassed.

One focused fixture uses valid borrowed unit/group prefixes and follows absent,
other-owner, and self-owner states. Six result comparisons check native AL and
source `bool` against explicit expected values. All input bytes remain unchanged.
The native whole 24-byte body executes unrelocated, with no provider bridge,
callback, or patch, and matches live Ghidra and the installed PE. Its bytes are
checked before and after the sequence. Logged nonnull EAX values retain the
group-pointer upper bits; only AL participates in the result comparison.

The strict MSVC Win32 build passed `/O2 /W4 /WX /fp:strict` and embeds an
`asInvoker` manifest. Four actual source/header/fixture inputs are frozen and
verified through the compiler include trace, with 173 host-header hashes. No
BSP support libraries are linked; six searched CRT/Win32 libraries are pinned.
The complete source COFF body is 24 bytes/nine instructions with no calls or
relocations. Its single +284h load and captured +14h comparison are verified.
The native invoker is 20 bytes/ten instructions and supplies actual ECX without
changing the original code; full EAX is captured for diagnostic logging only.

Validation passed 18 checks, zero failures. Six earlier reports and all 772
recorded artifacts retain their hashes. Detailed evidence is recorded under
`local/cc11_unit_group_owner_predicate_20261007_a`. Primary registration and
the full project build remain separate. No complete unit/group construction,
publisher, formation, dispatcher, class-ABI, or game-validation claim is made.

Primary integration at `488abdcb06ae1d841e9c1447727237c6e5cdc668` passed the full MSVC Win32 build and all three existing CTests. Independent current primary validation freshly compiled2 actual TUs;3 Source/header/fixture pins and2 actual project compiler includes were checked, along with current linked BSP libraries (none for the two predicates), the original PE and814 unchanged historical worker inputs. Complete Source COFF bytes match the worker receipts. Complete24B10inst00778890..7788A8: capture actualunit+284 once, NULL false; capturedgroup+14 equals SAMEactualunit. Native plainRET/AL-only boolean, nonnull upperEAX retains capturedgroup-pointer residue; actualMOV AL,DL at7788A5. Corrects old historical17B/MOV EAX,EDX description; preserves old source projection/history. One connected absent/other/self validborrowed unit/group-prefix family,18internalchecks/6 lowbyte Source-original comparisons; all inputbytesunchanged. Exactoriginal24B unrelocated/no bridges/patches/BSP libs. Actual7805A0 CALL/7805A5 TESTAL and77EED0/70DAB0/70D7B0 producer witnesses structuralonly; realpublisher virtual5C/formation70EFD0/completeunits/dispatcher/nativeABI/game unbound.
