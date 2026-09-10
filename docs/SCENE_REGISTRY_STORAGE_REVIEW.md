# Canonical scene registry storage review

Reviewed 2026-09-10. The latest primary checkout storage rewrite has no blocking
discrepancy in the bounded behavior reviewed below. This is an independent source
and native-byte review; it does not certify an ABI replacement or game behavior.
The review changed only this document and its JSON report.

## Evidence and reviewed source

Every native read used the repository Ghidra CLI, which verifies project `bsp`
(`C:/Users/sqz269/bsp.gpr`) and program `/battlestationspacific.exe`. The installed
PE is `I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All 16 spans below, totaling 3,444 bytes, matched the saved Ghidra bytes. Per-span
disk offsets and both SHA-256 values are in
[`scene_registry_storage_review.json`](../reports/scene_registry_storage_review.json).
Addresses are inclusive starts and exclusive ends; padding is excluded.

| Start | End exclusive | Bytes | Behavior inspected |
| --- | --- | ---: | --- |
| `00b81b90` | `00b81be5` | 85 | iterator allocation helper |
| `00b82390` | `00b823aa` | 26 | 12-byte list sentinel allocation |
| `00b82570` | `00b8259c` | 44 | 8-byte iterator fill |
| `00b827c0` | `00b828f4` | 308 | single-entry unlink/free/count decrement |
| `00b829d0` | `00b82a18` | 72 | list destructor including omitted native return tail |
| `00b82bf0` | `00b82c5a` | 106 | iterator vector erase |
| `00b82cf0` | `00b82d21` | 49 | registry destructor, bucket storage before list destructor |
| `00b82fd0` | `00b83211` | 577 | iterator insertion and capacity growth |
| `00b83220` | `00b832b6` | 150 | initial iterator vector allocation |
| `00b83490` | `00b8353a` | 170 | bucket boundary resize |
| `00b83560` | `00b835cc` | 108 | bucket boundary assign |
| `00b83600` | `00b83678` | 120 | registry constructor |
| `00b83700` | `00b83bdf` | 1247 | insertion and linear hash split |
| `00b83be0` | `00b83c42` | 98 | clear retaining iterator capacity |
| `00b83d90` | `00b83e42` | 178 | range erase routing whole-list erase through clear |
| `00b83e50` | `00b83eba` | 106 | key erase entry point |

The reviewed primary working tree was based on `79e5cd871880ee65a3587b77985345eeab61fb5e`.
Hashes identify the actual uncommitted implementation read, not that base commit:

- `include/bsp/scene_attachment.hpp`: `d90a5c89d446e1bf020924388fff4cba9d3c62d6c4b74982f88d1af9f3a59997`
- `src/scene_attachment.cpp`: `65abebe502aca5e4d32a8dc33df3086d2336866a5abd56d36967583a2a8d1f8b`

## Layout, ownership and constructor order

`00b82390` requests 12 bytes and writes only the sentinel's next and previous
pointers. Native payload `+8` retains its allocation preimage. `00b83600` places
the sentinel pointer at registry `+8`, size at `+0c`, and then creates nine
8-byte iterators through `00b83220`; the first allocation precedes the second.
`00b82570` copies both iterator words. Each iterator is
`{registry + 4, node}`: its first word is the list owner, whose head is at `+4`
and count at `+8`.

The primary rewrite has one raw `Entry{next, previous, key}` list and one raw
iterator buffer. `ListOwner` preserves the native head/count relationship, with
Win32 static assertions, and iterators contain `{&list_, node}`. An initial
review concern about using the head-pointer slot as owner was resolved before
the source hashes above were taken. The host association map borrows attachment
bindings, contains no ordering links, and does not supply the sentinel payload.
Copying valid iterator pairs or changing only their node retains the required
owner invariant. Native corruption/validation behavior is outside this interface.

## Growth and clear

`00b83700` splits before duplicate lookup. Boundary logical lengths grow
`9 -> 17 -> 33 -> 65 -> ...`. The general vector helper `00b82fd0` computes
`max(required, old_capacity + floor(old_capacity / 2))`, unless the geometric
candidate exceeds `0x1fffffff`; it first rejects a required length beyond that
limit. For this registry's reachable capacities, `required = 2 * capacity - 1`
always exceeds the geometric candidate. The private exact-count allocation is
therefore correct for this registry. It is not a general implementation of that
vector helper. Existing pairs are copied and new pairs filled before the old
allocation is freed; replacement pointer/count/capacity are published afterward.

`00b83be0` makes the sentinel self-linked and sets size zero before freeing list
entries. It then assigns nine sentinel iterators through `00b83560`, leaving the
vector allocation and capacity intact, and finally resets mask and active-bucket
count to one. Assignment uses vector erase/insert, not allocation replacement.
For example, clearing capacity 65 produces logical length 9 with capacity 65;
regrowth to 17, 33 and 65 must reuse that allocation. The source preserves this.
`00b83d90` routes a whole-list erased range through clear, which is reachable
when deleting the last unique key. `00b827c0` handles a partial erase by unlinking,
freeing the entry, then decrementing the count. The source preserves that order.

## Destruction and misleading no-return analysis

`00b82cf0` frees the iterator allocation first, zeros its three vector pointers,
then tail-calls `00b829d0` with the list owner. The list destructor resets head
links and size before freeing entries in forward order, frees the sentinel last,
then sets the head pointer to null. The source has that same logical sequence.

Assembly is necessary because the saved analysis had treated `_free` as
non-returning in these paths. The verified disk/Ghidra bytes show:

- `00b829f8..00b82a02`: stack cleanup, compare the saved next pointer with the
  sentinel, loop, and restore `EDI`.
- `00b82a0c..00b82a17`: stack cleanup, zero `[ESI+4]`, restore `ESI`, and `RET`.
  Thus the native destructor ends at `00b82a18` exclusive, beyond the old
  Ghidra body ending at `00b82a0b`.
- `00b82d00`: `ADD ESP,4` after iterator free; execution proceeds into pointer
  zeroing and the list-destructor tail call.
- `00b8310f`: `ADD ESP,4` after freeing the old iterator block; execution
  proceeds into replacement capacity/end/begin publication.
- `00b828dd`: `ADD ESP,4` followed by the partial-erase size decrement.

The JSON report records raw bytes for the missing continuations. This worker
made no Ghidra mutations; annotation/body repair remains an integration action.

## Verification boundary

This review compared native bytes and inspected native assembly and primary
source. It did not run a build, native differential fixture, or game session,
and added no tests. No existing focused registry fixture was found in the
checkout. The primary integrator owns build and focused runtime verification.

The source is a new C++ interface with explicit native allocation sizes, not a
native object overlay. Extra host association allocations, exception behavior,
and individual store timing are not covered by this review. In particular,
native sentinel `+8` is never written; the source preserves those bytes through
a read-and-write, so equivalent payload values do not claim identical write
traces. The bounded storage and lifetime conclusions above do not establish
whole-process allocation identity, ABI compatibility, or visual/game validation.
