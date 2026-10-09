# Native mission Lua raw variant initializer, 008849B0

This packet supplies a guarded MSVC Win32 Source leaf for the complete
27-byte leaf at `008849B0..008849CA`. It stores five DWORDs into the incoming
ECX receiver and returns that receiver in EAX. It makes no calls and has no
branch, hidden input register, local exception frame or x87 operation.

**Primary admission: normal build and whole object/Core review passed.** Primary CMake registration, normal build, complete COFF comparison and Core
review are documented in the admission section below. This worker claims no build, fixture, ABI compatibility, original-call,
runtime or game validation. No harness, test, probe or Ghidra write was made.

## Scope and evidence

- Published baseline: `c9c6808db971c603e57a469500ad3f7c24b69be5`.
- Worktree owner: `agent/cc12_settings_live_fov_cell`; lease:
  `cc12_lua_variant_initializer_source`.
- Only Native body opened: `008849B0..008849CA`, 27 bytes, 8 instructions,
  one block, no edges or calls. This is below the 300-byte packet gate.
- CLI target verification precedes live queries: project `bsp`, existing
  `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language
  `x86:LE:32:default`, image base `00400000`. Live/snapshot count: 64,729.
- Full live disassembly and bytes agree with the original PE's complete span
  and independent Capstone decode. Exact byte and PE hashes are in the report.
- No profile table, child, caller, handler or adjacent Native body was opened.
  No re-import, analysis repair, export refresh, annotation or ledger edit.

The accepted parent vector audit pins this leaf's growth use: its current
element address is `data + 20 * index`, passed in ECX without stack arguments
at `006B8938`, after a computed-address null check. That relation comes from the
accepted report; the caller was not reopened. The older deferred-payload report
is a 58-byte prefix audit, not full parent-body evidence. Its prior limits remain
historical; this packet neither republishes nor broadens them.

## Exact schedule and footprint

Let `P` be entry ECX. All offsets below are byte offsets. Normal completion
requires writable backing for every store and a valid return stack.

| Native site | Bytes | Operation |
| --- | --- | --- |
| `008849B0` | `8b c1` | Copy full receiver ECX to EAX. |
| `008849B2` | `33 c9` | Zero full ECX with XOR. |
| `008849B4` | `c7 00 f4 e6 d0 00` | Store DWORD `00D0E6F4h` at `P+0`. |
| `008849BA` | `c7 40 04 ff ff ff ff` | Store DWORD `FFFFFFFFh` at `P+4`. |
| `008849C1` | `89 48 08` | Store zero DWORD at `P+8`. |
| `008849C4` | `89 48 0c` | Store zero DWORD at `P+Ch`. |
| `008849C7` | `89 48 10` | Store zero DWORD at `P+10h`. |
| `008849CA` | `c3` | Plain RET; no stack argument cleanup. |

The five non-overlapping stores cover all bytes `P+0..P+13h`: 20 bytes with no
internal holes or preserved old fields. There are no explicit data reads.
The machine body does not initialize bytes outside that footprint. The parent
stride agrees with it; neither fact proves a complete class layout, allocation
size, profile extent, recursive value representation or ownership policy.

`FFFFFFFFh` is preserved as a raw full-width bit pattern. Calling it a signed
type tag, invalid enum, count or ownership sentinel would require further
evidence. The three zero words likewise receive no invented semantic types.
The first word is the exact immediate `00D0E6F4h`; the body does not dereference
that address. Its Ghidra profile label is a naming hint, not a callable Source
table or proof of its entries. No profile contents were queried.

There is no null, bounds, alignment, prior-initialization or alias guard. With
ordinary unmapped null storage, `P == 0` faults at the first store after EAX has
become zero and ECX has been cleared. Failed later stores can leave the earlier
prefix written; no local cleanup runs. Passing an existing object overwrites
the words without releasing anything formerly represented by them. The caller
must supply appropriate backing and lifetime. Exotic aliases such as a receiver
overlapping the return slot are not validated by the original or Source body;
normal return claims assume a valid return stack.

## ABI and Source form

The original input is one pointer in ECX with no stack arguments. Normal return
is the full original receiver in EAX, ECX is zero, and EDX, EBX, ESI, EDI and EBP
are untouched. ESP is unchanged until RET consumes the return address. No x87,
SSE, FS chain or stack-local state is accessed. XOR clears CF/OF/SF and sets
ZF/PF; AF is undefined. The following MOVs and RET do not change these flags.
No incoming flags are consumed.

The API is `void* __fastcall initialize_native_mission_lua_variant_008849b0(void*)`.
Its single pointer is delivered in ECX; no artificial EDX parameter or callback
is introduced. A guarded naked inline-assembly body preserves the full ordered
instruction sequence, raw values and machine-visible register result. No typed
class, virtual hierarchy, global storage, RAII or allocator is invented. The
immediate profile value creates no intended Source relocation; the primary must
verify actual compiler output and all COFF relocations before admitting it.

Source locations:

- `include/bsp/native_mission_lua_variant_initializer.hpp`
- `src/native_mission_lua_variant_initializer.cpp`

The implementation has no call dependency. Its numerical profile store still
does **not** supply a callable Source profile or a usable polymorphic element.
No queue, vector, allocator, destruction path or owning root is wired to it here.
The raw vector's reserve and virtual retirement boundaries remain separate
contracts. A future owning composition needs actual compatible storage,
profile/call target, mutation and lifetime evidence; this leaf alone closes none
of those transitive requirements.

## Provenance, checks and limits

The companion report includes the full original machine span, every instruction,
the five stores, ABI results and exact canonical Git pins. It replays the accepted
vector and prefix reports' input pins at their original revisions, then compares
each path with this packet's published baseline. Changed historical inputs are
reported explicitly rather than being treated as current. Owned header, Source
and document pins are computed over exact prospective Git blob bytes; the JSON
report excludes a self-hash.

Worker checks are limited to full Native span/PE agreement, instruction coverage,
pin replay, JSON parsing, owned-file scope and `git diff --check`. The eight Source
assembly instructions were reviewed against the eight Native instructions.
Primary compilation and complete byte comparison now pass, as recorded below.
No test executable, installation change, original-call execution, fixture result,
live game result or new reconstruction/ABI credit is claimed.


## Primary admission

The complete emitted Source leaf is 27 bytes / eight instructions, byte-for-byte
equal to the accepted live Original span and current installed PE. It contains
no relocation, named external boundary, child or extra instruction. Its physical
symbol, entire object and complete Core member have one unique positive public
definition. The normal MSVC Win32 build passed all three existing checks.

This adds one bounded reconstruction covering all 27 Original bytes. The normal
entry/register/store schedule matches statically; Original execution, binary
placement, callable profile, owner/vector integration and gameplay remain
unvalidated. The root is absent from the game map. Existing LNK4006 spawn-request
duplication remains unrelated. The primary report supersedes historical worker
pending admission. Full physical evidence and the shared build receipt are in
the primary report and its retained local artifact set.
