# Sound resource construction and asset loading

Addresses: 00a82ea0, 00a835b0, 00a83450, 00a823f0, 00a82720,
00bdc8b0, 00bd91f0. Typed interfaces: `sound_resource_asset.hpp`.

The reconstructed game code resolves a requested sound through the existing
VFS, falls back to `sound/gui/error.fsb`, checks existence, constructs its asset,
loads an FSB bank from VFS memory or an FEV Event project, and accepts the
resource only when its sample or project handle exists. FMOD operations remain
actual proprietary library boundaries. No FMOD implementation is reproduced.

## Native ownership and interfaces

| Native offset | Established behavior |
| --- | --- |
| +00 | Vtable D5B118, after base CEB130 construction |
| +04 | Intrusive reference count initialized to 1 |
| +08 | Owned FMOD Event project; initialized null |
| +0C | Owned FSB bank sound; initialized null |
| +10 | Borrowed subsound 0 of +0C; initialized null |
| +14/+18 | Owned eight-byte NativeString name |
| +1C | Unsigned PCM sample length, written by the FSB branch |
| +20 | FMOD default frequency multiplied by options+8 |
| +24 | PCM length divided by the stored effective frequency |
| +28 | Bank RAWBYTES length for FSB; FMOD allocation delta for FEV |

The original allocation is 2Ch. The C++ resource is an ownership projection,
not that layout: optional timing fields explicitly represent the native FEV
branch's unwritten +1C/+20/+24. Its destructor is not automatic. The separate
cleanup implementation releases +0C and +08, never the borrowed +10, and releases
the pooled name. Native virtual table bytes at D5B118 identify final-release
BD30E0, deleting destructor A85AC0, slot+8 BEFAF0, and size A818B0 at slot+0C.

| Routine | Original ABI and return |
| --- | --- |
| 00A82EA0 | Owner ECX unused; hidden output/name/options stack; EAX output; RET0C at A82FE7 (3 bytes) |
| 00A835B0 | Owner ECX unused; name/options stack; EAX resource or null; RET8 at A836F2 (3 bytes), successful RET8 at A83696 |
| 00A83450 | ECX resource; name/options stack; EAX resource; RET8 at A835A4 (3 bytes) |
| 00A823F0 | ECX resource; options stack; RET4 at A82715 (3 bytes) |
| 00A82720 | ECX resource; RET at A827AB (1 byte) |
| 00BDC8B0 | ECX VFS manager; name/size-output stack; EAX allocated bytes; RET8 at BDC9A4 (3 bytes) |
| 00BD91F0 | ECX unused; allocated bytes stack; RET4 at BD91FB (3 bytes) |

Resolver A82EA0 copies the name, calls canonical VFS resolve BDF4C0, and copies
either the mutated result or the literal fallback into a fresh native string.
The third stack argument is unused. The creator A835B0 copies the input even
though it uses the original in the VFS existence call. It allocates, constructs,
and tests +10 then +08. An invalid completed resource receives deleting-dtor+4
with flags1. C++ allocates through ordinary `new`, compatible with the separate
typed deleting destructor's `delete`. Temporary string storage is explicit.

Constructor A83450 copies the name and obtains its last four characters using
the already reconstructed native substring helper. It compares case-insensitively
with `.fsb`, then `.fev` (live D5B110 bytes `2e 66 65 76 00`). Other extensions
leave an invalid resource for creator cleanup. Constructor unwind releases the
constructed string subobject; it does not call the completed resource destructor.

## FSB mode, output and arithmetic evidence

A823F0 clears a 6Ch create-info record, writes cbsize=6Ch at offset0, and passes
its offset4 to the VFS whole-file loader as the byte-length output. All other
create-info fields remain zero. Initial mode depends on options+00:

| options+00 | Mode |
| --- | --- |
| 0 | 00000A4A |
| 1 | 00000A52 |
| Other | 00000802 |

Nonzero options byte+20 adds 00200000. A82492 calls System::createSound with
the file memory, mode, create-info and resource+0C output. Native memory queries
bracket creation, but the discarded debug print at 004254B0 is empty. The size
stored at resource+28 comes from bank getLength(unit8), not the memory delta.
VFS frees the copied bytes immediately afterward, before subsound operations.
FMOD_OPENMEMORY therefore owns/copies what it needs; this is not OPENMEMORY_POINT.

The loader calls getNumSubSounds (count is unused), getSubSound(index0), getMode,
getLength(PCM unit2), and getDefaults(frequency output only), in that order.
Every result equal to 2Bh triggers A7A460's actual two-output memory query.
Other setter errors are ignored by the recovered caller.

A825C9..A825DC computes and materializes the frequency product to float. The
PCM length is unsigned: A825DF loads a signed DWORD with FILD, A825E2 tests its
sign, and A825E4 adds the float at CE3978, whose live bytes `00 00 80 4f` equal
4294967296. A825EF divides this corrected length by the stored float frequency.
The implementation uses x87 operations to preserve this conversion, intermediate
store, current precision/rounding, and the final single-precision store.

Setter order is conditional 3D min/max distance (the returned mode's bit10h),
variations(options+30, options+34, 0), optional loop points (options byte+16),
and mode. Loop conversion loads end(+1C) before start(+18), temporarily sets
x87 rounding bits to truncation, stores signed qwords, restores the old control
word, and passes each low DWORD with MS unit1. The typed helper preserves those
x87 conversions, including native masked invalid-conversion behavior.

Final mode is `(mode & FFFFFDFF) | 100` for nonzero options byte+14, otherwise
`(mode & FFFFFEFF) | 200`. These masks and the setter order are from assembly;
the decompiler's enum-expression and stack-variable labels are not evidence.

## Event project and VFS contracts

A82720 samples FMOD memory, invokes EventSystem vslot24 using stdcall
`(eventSystem, name, nullptr, &resource+08)`, and stores the unsigned DWORD
difference between the second and first memory samples at +28. The installed
FMOD Event load export supplies that library boundary. Neither asset loader,
constructor nor creator increments global F8BBE4; no balancing increment was
invented for the separate native destructor's subtraction.

The live VFS table D68D04 has +04=BDF310, +08=BDD440, +10=BDC8B0,
+14=BD91F0. BDC8B0 copies the name, opens flags2, reads stream size+2C, allocates
that low DWORD count, issues one read+24 with a null actual-count pointer,
decrements/releases the stream, publishes size, and releases its name copy.
BD91F0 forwards its pointer to BF6989, the matching array deallocator.
The implementation composes canonical `open_resource_memory_00bdf310_fragment`
and `MemoryStream`, with their established provider and buffering contracts.
It does not reconstruct VFS anew or invent provider callbacks.

## Host limits and verification

The canonical VFS memory adapters reject unavailable/incomplete host data.
Native BDC8B0 has no null/short-read checks. FMOD output queries that fail before
writing raw length, mode, PCM length or frequency are likewise outside this
guarded projection: the host throws after the native 2Bh checkpoint instead of
reading indeterminate native stack values. This is an explicit host boundary,
not a recovered error branch. Constructor unwind after such a guard frees the
name only, following native constructed-subobject unwind; it does not synthesize
bank cleanup. Callers must supply valid loaded FMOD systems and successful
required output queries. No end-to-end exceptional FMOD resource recovery is
claimed. Routine setter errors and Event-load validity still follow native flow.

Ghidra was read only against C:/Users/sqz269/bsp.gpr and program
/battlestationspacific.exe, verified by bsp.py before live analysis/export batches.
All five sound bodies and BDC8B0 were exported with complete inspected endings.
BD91F0 was not defined in saved analysis when inspected. Disk disassembly proves
its 14-byte body BD91F0..BD91FD; the primary was sent this repair request.
No worker annotation, function creation or game-installation mutation occurred.

The Win32 Release build and both existing CTests passed. `verify-seeds` reported
matching installed-image bytes for every checked range. No new tests were added.
Exact validation status is recorded in `reports/sound_resource_asset.json`.
Installed default-error-FSB integration belongs to the primary sound startup
probe. Build success alone is not playback, installed-asset, or game validation.
