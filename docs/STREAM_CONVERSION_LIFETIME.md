# Stream conversion lifetime and empty physical sources

Addresses: 008d43c0, 008d4470, 00bb8be0, 00bb8f90, 00bbc1d0, 00bbc320, 00bbc3e0, 00bd30e0, 00bd30f0, 00bef6d0, 00bef750, 00bef9c0.

`00bef750` borrows its input stream and returns a new memory wrapper. Its
memory-stream branch retains the existing backing; its other branch allocates
an independent backing and reads into it. The MPKG compressed-entry caller
releases its temporary inflater only after this conversion returns. No returned
memory wrapper depends on the lifetime of that inflater or its source wrapper.

This audit rechecked twelve complete function bodies and three vtable spans
against the installed PE on 2026-09-09. It also enables one bounded C++ change:
physical conversion accepts a zero-length source and returns a valid empty
backing. The byte-copy and MPKG materialization APIs retain their positive-size
preconditions. No native ABI or gameplay-equivalence claim follows.

## Evidence and scope

Every live query used `python tools/bsp.py ghidra ...`, whose client verifies
project `bsp`, program `/battlestationspacific.exe`, language
`x86:LE:32:default`, and image base `00400000` before querying. The configured
project is `C:/Users/sqz269/bsp.gpr`; the disk binary is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The tracked record is `reports/stream_conversion_lifetime_audit.json`.
Fresh byte dumps, PE comparisons, complete Capstone disassembly, original
comments and function metadata are in ignored
`exports/bsp/parallel_stream_lifetime/`. End addresses below are exclusive and
include every final RET operand. Padding was excluded only after checking that
the captured bytes after each complete body were all `CC`.

| Start | End | Bytes | Established ABI |
|---|---|---:|---|
| `00bb8be0` | `00bb8d52` | 370 | ECX archive; entry/flags on stack; EAX returned stream; RET8; flags unused |
| `00bbc1d0` | `00bbc31e` | 334 | ECX object; source/descriptor/two unused capacity slots on stack; EAX object; RET10h |
| `00bbc320` | `00bbc3da` | 186 | ECX inflater; no stack arguments; RET; no stable result contract |
| `00bbc3e0` | `00bbc3fe` | 30 | ECX inflater; flags on stack; EAX original storage; RET4 |
| `00bef750` | `00bef840` | 240 | ECX borrowed source; EAX new wrapper or null for null input; RET |
| `00bef6d0` | `00bef74c` | 124 | ECX backing; EAX new wrapper; RET |
| `00bd30e0` | `00bd30ee` | 14 | ECX object; no stack arguments; RET; dispatch deleting destructor with flag1 |
| `00bd30f0` | `00bd30f7` | 7 | ECX base; no stack arguments; RET; store base vtable |
| `008d43c0` | `008d4437` | 119 | ECX backing; signed length on stack; EAX backing; RET4 |
| `008d4470` | `008d44b2` | 66 | ECX backing; flags on stack; EAX original storage; RET4 |
| `00bef9c0` | `00befa36` | 118 | ECX memory wrapper; no stack arguments; RET |
| `00bb8f90` | `00bb8fae` | 30 | ECX memory wrapper; flags on stack; EAX original storage; RET4 |

The closure beyond the eight packet anchors is limited to the backing
constructor and the memory wrapper/backing destructors. Other MPKG routes,
allocation internals, type-query implementations and zlib internals were not
newly reconstructed. Cursor outcomes during reads use the established
`INFLATE_STREAM_READ_SEEK.md` contract; this audit does not re-open the excluded
cross-block backward-seek behavior.

## Source, inflater and conversion ownership

For a nonzero compression method, `00bb8be0` first seeks the transformed archive
source at archive `+0Ch` to the resolved entry offset. At `00bb8c39..00bb8c63`
it builds the three-DWORD descriptor from entry `+10h/+18h/+1Ch`, allocates a
`34h` inflater object and calls `00bbc1d0` with that same source wrapper.

The constructor stores source at `+0Ch`, copies offset/compressed/decoded counts
to `+10h/+14h/+18h`, and initializes object reference count `+4` to one. It
allocates separate `10h` descriptors for `4000h` input and `10000h` output
buffers and a `38h` z_stream. The capacity arguments are unused; allocation
sizes are immediate constants. It passes ECX z_stream, EDX `-15`, and the
version/size stack arguments to raw `inflateInit2_` at `00bbc2d4`.

At `00bbc2e3`, construction increments source `+4` after decoder initialization.
It then seeks that same source wrapper again to the descriptor offset and
initializes compressed/decoded counters plus logical decoded position zero.
It does not clone the source or preserve its cursor. Allocation, decoder-init
and seek failures are not handled as recoverable failures by these native
bodies; the C++ adapter's explicit status handling is a host policy.

The archive caller passes the inflater as ECX to `00bef750` at `00bb8c78`.
After conversion, `00bb8c83` decrements inflater `+4`; if it becomes zero,
`00bb8c93` calls its virtual slot zero. It returns the converted wrapper only
after that destruction has completed. No additional retain of the returned
wrapper occurs in this caller.

| Transition on the ordinary compressed-entry path | Ownership result |
|---|---|
| Inflater construction | New inflater has count1; archive source gains one retained reference |
| Conversion backing construction | New independent backing has count1 |
| `00bef6d0` wrapper creation | New wrapper has count1; backing becomes count2 |
| Converter releases temporary backing | Backing returns to count1, owned by returned wrapper |
| Archive caller releases temporary inflater | Inflater reaches count0 and drops its retained source reference |
| Caller eventually releases returned memory wrapper | Wrapper releases backing; last backing owner destroys bytes and storage |

These are relative transitions. Existing source and shared-backing reference
counts need not initially equal one.

## Conversion and cursor effects

`00bef750` has three branches, with no input-source reference increment or
decrement in any branch:

1. Null ECX returns null at `00bef780`.
2. A successful source virtual `+0Ch` query with the ID at `0109dba0` passes
   source `+8` backing to `00bef6d0` (`00bef795..00bef798`). The source wrapper
   and its cursor remain untouched. The new wrapper shares only the backing
   and starts at its base. The type-query implementation itself was not audited.
3. Otherwise it calls source seek `+1Ch` with all three arguments zero, then
   source size `+30h`. It saves EAX as the original low-DWORD length. The returned
   EDX high word is saved to the stack but never used to allocate or read. It
   constructs a backing, makes exactly one virtual read `+24h` with destination
   backing `+8`, that original low length, and a null actual-count pointer, then
   wraps the backing and releases the converter's temporary backing reference.

The non-memory branch does not restore the source cursor, inspect seek/read
results, shrink the backing after a short read, or zero unread bytes. On the
bounded successful inflater path, conversion consumes the full advertised
decoded length; the new memory wrapper still starts at zero. The transformed
source's compressed cursor can advance through input prefetch and need not
equal an exact compressed-stream-end position. Destructor bodies perform no
seek or cursor restoration.

`00bef6d0` allocates `14h` bytes and initializes the new wrapper's count to one
and backing/end/cursor fields to zero. It has a release-old-backing branch,
although successful allocation just initialized that field to zero. It stores
the supplied backing at `+8`, increments backing `+4`, assigns cursor `+10h`
to data `+8`, and end `+0Ch` to data plus backing length `+0Ch`. It does not
retain an input stream wrapper. Allocation failure reaches a null dereference
in the native function; it is not a supported null-return contract.

## Complete destruction order

The freshly matched vtables confirm slot0 `00bd30e0` for inflater `00d64400`,
memory wrapper `00d642c0`, and backing `00d15ad8`. Their slot4 values are
`00bbc3e0`, `00bb8f90` and `008d4470`, respectively. `00bd30e0` only checks for
null and calls virtual `+4` with flag1. The callers perform the reference-count
decrement; this helper does not decrement again.

`00bbc320` installs inflater vtable, decrements its source reference, invokes
source slot0 only at zero, and clears source `+0Ch` after the callback returns.
It then calls `inflateEnd`, frees z_stream storage, frees input bytes and input
descriptor, and frees output bytes and output descriptor. Finally it installs
stream-base vtable `00d5c104`, calls `00bd30f0` to install `00ceb130`, restores
SEH state and returns at `00bbc3d9`.

At capture, Ghidra ended the inflater body at `00bbc380`, immediately after the
first free. The raw continuation `00bbc381..00bbc3d9` proves the remaining
frees and epilogue. `00bbc3e0` subsequently frees the inflater's own storage
only when flags bit0 is set, then places the original object address in EAX
on both paths before RET4. Ghidra's `extraout_EAX` rendering after free is not
the returned-value contract.

`00bef9c0` similarly decrements wrapper backing `+4`, invokes its slot0 at zero,
and only then clears wrapper `+8`. It installs stream-base then base vtables;
cursor/end fields are not cleared. `00bb8f90` conditionally frees wrapper
storage using flags bit0 and always restores EAX to the original storage.

Backing deleting destructor `008d4470` installs `00d15ad8`, frees backing data,
decrements object counter `0109db98`, subtracts stored length from byte counter
`0109db9c`, invokes base cleanup, and conditionally frees backing-object storage.
It also restores EAX to the original storage and uses RET4. The raw tail after
the first free is `008d4482..008d44b1`; Ghidra's captured pseudocode omits it.
The primary integration lane identified `CALL_RETURN` overrides at `00bbc37c`
and `008d447d`. Later analysis also reinstated an incorrect no-return flag on
the actual `_free`; its own complete returning body justified correcting that
flag. The audited overrides and truncated definitions are now repaired and
saved. See `LIFETIME_EFFECT_CACHE_INTEGRATION.md` for final readback evidence.
Allocator instrumentation, exception unwinding and counter thread safety
remain outside this audit.

## Bounded C++ change and verification

At `008d43fd..008d440a`, signed lengths at or below zero store logical length0
and allocate one physical byte. Positive lengths allocate and store that
length. The constructor adds the original signed input to the byte counter,
while destruction subtracts stored length; negative-size accounting and the
converter's original unsigned read count must not be generalized into a safe
negative-size API. Zero has no such mismatch: it allocates one byte, reads
zero bytes, and returns a valid wrapper with base equal to end.

`src/memory_stream.cpp` now keeps a one-byte allocation for logical length0
and removes the zero-length rejection from physical conversion. The existing
one-read conversion, cursor reset, initialized-prefix tracking and source
ownership remain in place. Sizes above `INT32_MAX` are still rejected, and
`memory_stream_from_bytes_00befa40_fragment` plus MPKG materialization still
require positive lengths. No generic stream interface was added.

The existing `probe_memory_texture` scenario in `src/d3d9_probe.cpp` creates
one empty temporary file beside the probe executable under the build tree and
cleans it up on scope exit. It first seeks the physical file to7, converts it,
checks source cursor0 and retained-open source state, closes the file, clones
the empty memory stream and releases the first wrapper. It checks shared
nonnull backing, size/init/cursor0, zero-distance end seek, rejected out-of-range
seek, and an EOF read that reports zero without altering a sentinel byte.
Its success marker is
`Empty physical conversion: retained_backing_cursor_and_read=1`.

The primary integration passed the Win32 build, both existing CTests and the
installed-resource probe, including the empty-file marker above. Results are in
`reports/lifetime_effect_cache_validation.json`. These are diagnostic checks;
native ABI compatibility and gameplay remain unvalidated.

The existing host inflater already releases its retained source before
`inflateEnd`. `MemoryStream` already shares backing with independent clone
cursors. MPKG's host `DecodedSource` uses a cloned archive-memory cursor and
materializes through a temporary vector; native uses the archive source wrapper
directly and reads into the final backing. Returned byte ownership agrees for
the supported success path, but those cursor/allocation details and native
intrusive ABI are not equivalent. A clean cross-block rewind remains a
separate host behavior choice and is not enabled by this lifetime audit.
