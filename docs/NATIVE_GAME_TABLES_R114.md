# Native game static tables (R114)

Addresses:008D9150,00727BD0,004DDB90;008D97A0 read-only consumer evidence.

Two required game providers now have concrete defaults: the complete23-row unit-conversion builder and a raw12-by97 target-rank builder. Four game-service providers remain. Both borrow the actual table storage through `NativeGameTablesContext`; canonical literals,factor bits and1164 authored preference DWORDs are compiled into C++ and require no original PE at runtime.

## Unit rows and opaque frame bytes

008D9150..008D9747 is1528 bytes. It appends23 rows of two literal pointers,one exact factor DWORD andone flag DWORD. It reloads/increments the global count after each row without resetting it or checking capacity. Native storage F88A50..F88BBF holds exactly23 rows and its count isF88BC0; startup therefore needs fresh count0.

The flag's low byte is0 except for `rpm`/`1/sec`,whose flag is1 and factor bits42700000. Native code writes only the low byte of its local DWORD,then copies all four bytes to every row. The upper24 bits are stack preimage. Source exposes `unit_frame_word_preimage`,masks only its low byte and preserves the other three. It does not fabricate zero padding. The original comparison wrapper seeds precisely this DWORD at native entryESP-4; this is an explicit diagnostic input,not proof of natural native caller-stack or ABI equivalence.

Consumer008D97A0 reads the low flag byte at8D9851 and8D989A; adjacent paths multiply/divide with x87. Its full306-byte span and seven calls are archived/audited only. Its conversion behavior is not reconstructed by this packet,so its generic function name is retained.

Canonical C++ strings have this executable's pointer identities. Exact-byte fixtures bind the equivalent original-image literal pointers; a separate check compares all23 canonical strings,factor DWORDs and flags against native data.

## Raw ranks

00727BD0..00727C23 is84 bytes; the olderC24 endpoint included the next byte. It clears each97-word output row before reading its97 current preference DWORDs. Zero entries leave holes; nonzero IDs receive ranks1,2,...; duplicate IDs overwrite previous ranks. Writes use `row*97+id` without clamping,including authored row0's ID97 that touches row1 slot0 before row1 is later cleared. The new raw adapter preserves that order and DWORD address arithmetic. Existing typed projection helpers remain separate and unchanged.

## Validation

- Strict MSVC Win32 build and all3 existing CTests passed.
- 23821 live Ghidra/PE bytes agree;434 CALL rows cover382 composed fixture calls,9 separate Dyn calls,36 read-only score-producer calls and7 read-only conversion-consumer calls.
- 85 copied envelopes16265B and33 original/source comparisons matched **1657 ordered observations /121405968 bytes**. Parent snapshots now include complete unit rows/count and rank output.
- Two standalone unit cases use005566FF andDEADBE00 frame words. Rank cases use all1164 authored words and a bounded holes/duplicate/cross-row input. All canonical C++ data matches native data.
- The first harness attempt lacked theCFDEB0 constant page and exitedC0000005 during setup. Adding that page fixed the harness without changing source code; its initial receipt is retained.
- The existing Dyn fixture was rebuilt for the virtual signature changes:280 paired buffers/9380996 bytes,204 handles closed,0 tracked allocations and exact dispatch CRT-exit cleanup passed. It still uses zero workers and does not execute collision/solver methods.
- Application executable equal except timestamps:True; changed application objects:[]. No application runtime rerun.

One new1528-byte function plus three reuse/composition/read-only fragments. No flow repair or function creation was needed. Prior comments and correct names are preserved; the unit builder receives a provisional behavior name. All names remain hypotheses rather than recovered symbols.

## Follow-up packets

Remaining providers:`call_00432650, call_0087d7b0, call_00717e80, call_0070bd70`. Global-config ownership and the complete Lua loader must share the same actual2E8h owner. Resource registration needs real parser ownership. The84h grid constructor70BD70 calls70B330,which allocates six vector buffers,computes normals and creates renderer-backed format/vertex/index objects; its renderer dependencies must remain explicit. Native ABI/FH3,application startup/lifetime and gameplay remain open.

Evidence:`reports/native_game_tables_r114.json`,`reports/native_game_tables_flow_r114.json`; exact native/input/source/fixture artifacts are sealed before integration.
