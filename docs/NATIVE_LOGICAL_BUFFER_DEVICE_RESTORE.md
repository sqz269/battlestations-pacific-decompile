# Actual logical buffer restoration after device reset

`native_logical_buffer_device_restore2` reconstructs both complete bodies:

| Native entry | Full extent | Reconstructed interpretation |
| --- | --- | --- |
| `00B49DC0` | `00B49DC0..00B49F6F`, 432 bytes | Logical vertex buffer restoration |
| `00B4A040` | `00B4A040..00B4A181`, 322 bytes | Logical index buffer restoration |

Names describe established behavior; they are not recovered original symbols.
Both entries originally use ECX for the actual logical owner, take no stack
arguments, and return with plain RET. The C++ entries add the required actual
service context in EDX. They are not binary drop-in replacements, are not
`noexcept`, and add no cleanup, HRESULT branch, or rollback.

## Borrowed storage and call contracts

The vertex logical fields are physical owner `+58`, flags `+60`, count `+64`,
declaration pointer `+68`, and shadow pointer `+6C`. Declaration stride is a
DWORD at `+CC`. Index fields are physical owner `+08`, flags `+10`, count `+14`,
format `+18`, and shadow `+1C`. Index format `65h` selects stride two, `66h`
selects four, and other values select zero. Products and pointer arithmetic
wrap at 32 bits. Raw accesses use single x86 DWORD loads/stores, including
unaligned storage.

The context borrows the current renderer publication corresponding to
`00F8D394`, the actual physical Attach and Lock contexts, and four immutable
nine-DWORD profile views. Attach and Lock share the same actual lifetime domain.
The recovered profile identities and used slots are:

| Profile | Kind | Lock `+08` | Unlock `+0C` | Attach `+14` | Capacity `+18` | COM getter `+1C` |
| --- | --- | --- | --- | --- | --- | --- |
| `00D61E10` | private index | `00B4B850` | `00B4B820` | `00B4C250` | `00B4B800` | `00B4B840` |
| `00D61E34` | private vertex | `00B4BA00` | `00B4B9D0` | `00B4C370` | `00B4B9B0` | `00B4B9F0` |
| `00D61E58` | pooled index | `00B4B850` | `00B4B820` | `00B4C250` | `00B4B800` | `00B4B840` |
| `00D61E7C` | pooled vertex | `00B4BA00` | `00B4B9D0` | `00B4C370` | `00B4B9B0` | `00B4B9F0` |

These native addresses are identity tokens. The source maps only these exact
profile/slot combinations to existing compiled implementations; it never calls
a numeric token as a host function pointer. Other profiles/slots are outside
the explicit binding domain. There is no guessed fallback.

The required `native_pool_stack_bits` constructor argument models the native
uninitialized pool local for unsupported low flag nibbles. It is an input,
read only when `(flags & 15) > 3`, with no default value. Vertex reads the bits
at native entry ESP-4; index uses entry ESP-12. Vertex later reuses this local
as the zeroed Lock base output. Index has separate pool, usage, and base locals.
The input is not an externally writable stack output.

## Complete ordering

Both entries return immediately when the logical flags have `1000h` in the
`F000h` field, the physical pointer is null, or its current COM getter returns
nonnull. Otherwise vertex translates pool/usage inline; index calls the full
actual `00B20A80` translator with kind seven. Pool nibble zero forces flag
`10000h` before usage derivation; nibbles one through three write their own
pool value. Unsupported nibbles preserve the specified native stack bits.

The current renderer is captured and passed to the actual `00B1FEF0` borrowed
device getter. The local COM output is initialized to null. Vertex captures
the device table before reading current declaration stride/count; index
captures the format/derived stride before the device table, then reads
pool/usage/count. Create uses table slot `+68` for vertex or `+6C` for index,
the wrapped byte product, usage, FVF zero or captured index format, pool,
the temporary output address, and a null shared handle. Its HRESULT is ignored.

After Create, both entries reload the current logical size fields and physical
owner before Attach. Vertex captures the Attach table/slot before reading the
fresh logical flags; index reads fresh flags before capturing the table/slot.
Attach receives the current temporary COM output, fresh logical flags and the
fresh wrapped byte capacity. The full actual Attach runs, including COM
retention, diagnostic strings, the resource-support singleton, and its guard.

After Attach returns, the temporary output cell is read again. Its **current**
COM object is unconditionally released through its current `+08` slot. A null
output faults here, after Attach. The borrowed device is never released.

Next the current physical owner is captured and the Lock base local is zeroed.
Its table is captured before calling capacity. Lock uses a newly loaded
physical owner but the table captured before that capacity call, with arguments
`(capacity, 0, 1, &base, 1)`. A second capacity is obtained from the current
owner/table after Lock. Actual `memcpy` copies that many bytes from the current
logical shadow to the captured Lock destination. Unlock uses the current
physical owner/table after copying. Finally the current logical shadow is
passed to actual `singleton_lifetime_free`, and only after free returns is the
logical shadow field cleared. Callback replacements during free are overwritten
by this final zero store. No old pointer is substituted for any fresh read.

Fresh live assembly and installed PE bytes establish the previously omitted
returning-free continuations `00B49F5F..00B49F6A` and `00B4A171..00B4A17C`:
each twelve-byte span adjusts the stack, restores a register, clears the
logical shadow, and restores another register. The preparation records both
full bodies and these exact gap bytes. Ghidra repair/annotation is coordinated
by the primary integrator; this packet made no Ghidra mutations.

## Verification and boundaries

Preparation verified project `bsp`, program `/battlestationspacific.exe`, and
29 complete live/installed spans totaling 3,444 bytes. This includes 754 owned
bytes, the vertex pool branch table, all four profiles, and actual provider
references. Strict MSVC Win32 `/W4 /WX /fp:strict`, the existing two CTests,
and eight native seed checks passed. The actual built library was frozen
before compiling the ignored fixture; no source provider was replaced.

The fixture runs the unchanged 432/322-byte native bodies at a uniform code
delta, routing external dependencies to the complete compiled providers. Four
explicit callable profile tables bind native calling conventions/context;
the source continues to use distinct immutable original-token views.
The absolute vertex pool table at `00B49F70` lives in dedicated unused fixture
PE storage at `00B41000..00B50FFF`. Only its four target DWORDs are relocated.
No owned operand is patched and no process-heap reservation is modified.

Twenty-four paired cases compare 172,352 DWORDs and 730 event frames per
implementation. They cover three skip paths, normal restoration, unsupported
pool bits with two valid seeds and an invalid seed, wrapped byte multiplication,
zero product/invalid index format, and observer exceptions after real successful
Lock or Unlock. The mutation case observes Create 32 bytes, Attach 24, Lock 16,
and copy eight while changing the physical owner, escaped temporary output,
and current shadow at real provider boundaries. It also checks Lock/Unlock
depth wrap and the returning-free overwrite.

A real NVIDIA GeForce RTX 5090 D3D9 HAL device supplies every buffer, descriptor,
Create, AddRef, Release, Lock, and Unlock result. EXE import observers forward
actual malloc, free, memcpy, EnterCriticalSection, and LeaveCriticalSection.
The actual shared lifetime registers the actual support owner and shuts it
down through its actual deleter. Invalid pool/zero-product Create returns
`8876086Ch`; both implementations then execute full Attach and fault on a null
temporary Release. Observer exceptions are fixture injections after successful
driver calls, not claims of driver exceptions.

The audit compares complete selected COFF bodies with the fixed linked PE and
checks 49 stages of complete runtime code/profile/table/bridge postimages.
The detailed immutable manifest, provenance, observations and pins are in
`reports/native_logical_buffer_device_restore_audit.json`. Fixture/build
artifacts remain ignored under `local/` and `build/`; no permanent test or
shared CMake change was added.

This establishes reconstruction, strict build and bounded differential fixture
agreement. It does not establish original caller ABI compatibility, arbitrary
profiles, concurrent mutation between pure leaves, all flag/format/pool/usage
combinations, failed Lock behavior in this parent routine, allocation failures
during Attach, a complete game reset, gameplay, or visual correctness. Existing
provider packets retain their own narrower ABI and validation boundaries.

## Primary integration

The primary registered this source in CMake and passed the strict Win32 build,
both existing CTests and eight fresh native seeds. It independently verified
112 worker pins, 22 current source/provider files and 29 fresh live-Ghidra/PE
spans (3,444 bytes). The actual main library
`387db51b99a9f2eec2d94ba2dff26ca3552784737ecf28b28cb0576224e6327b`
and eleven exact archive-member objects passed the unchanged fixture: all
24 pairs, 172,352 literal DWORDs and 730 event frames match. All 84 complete
linked COFF functions and 4,900 whole runtime code/profile postimages were
verified. The required unsupported-pool stack input and real D3D9/CRT failure
paths remain explicit, including unconditional null-release faults after invalid
Create calls. All 754 owned native code bytes remain unchanged. Four external
helper bridges and the four-DWORD branch-table binding are declared.

Ghidra incorrectly ended the two returning-free calls. The primary preserved
all prior comments and labels, cleared only the evidenced CALL_RETURN overrides,
recovered the full tails, recreated the two complete functions and saved the
project. New descriptive names and evidence are saved; full reconstruction
records and forced exports include the repaired tails. No permanent tests
were added. Original-caller ABI, complete device reset and gameplay remain
unclaimed.
