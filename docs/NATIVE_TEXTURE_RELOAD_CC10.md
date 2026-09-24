# Native 2D texture reload

Addresses: `00B3FA90`, `00B3F5D0`; read-only FH3 evidence `00CBEF80`,
`00CBEF88`, `00DF7860`, `00DF7868`.

| Routine | Original range, inclusive | ABI | Coverage |
| --- | --- | --- | --- |
| `reload_native_texture_00b3fa90` | `00B3FA90..00B3FD7A`, 747 bytes | ECX actual owner, RET, no semantic result | complete source control flow in declared provider/preimage domain |
| `construct_native_texture_reload_diagnostic_00b3f5d0` | `00B3F5D0..00B3F620`, 81 bytes | ECX output, stack pointer-to-COM/name, EAX output, RET8 | complete |

The new source interface composes the existing `NativeTextureLoadingContext`
and its actual owner, VFS, conversion, string pool, notification, cache and
resource-support providers. It does not create a renderer, texture owner,
companion, cache, stream reference counter or string pool. The canonical
`NativeTextureLoadOwners::Impl::Entry` keeps the same `raw` identity; consumers
of that raw owner observe the replacement COM at `+10`. No registration or
companion replacement is added. The platform focus dispatcher may select this
body for current `D61948/+08=B3FA90`; application WM_ACTIVATE binding remains
separate.

`B3F930` is the existing owner layout producer. Its named base/name `+08`, COM
`+10`, metadata `+14/+18/+24/+28/+2C/+30`, saved policy dimensions `+34/+38`,
requested mips `+3C`, cached surfaces `+40/+44/+48` and retained source `+4C`
are reused, without a competing overlay type. `B3F5D0` itself produces the
diagnostic's borrowed COM word followed by an actual eight-byte string header.

## Native ordering

1. Read current `0109CEEC` and open the owner's original `+08` name with flags
   2. Query current stream `+18`. A closed stream receives its current deleting
   slot `+04(1)` directly, without decrement; owner/cache/COM stay unchanged.
2. Convert the open source with full `BEF750`, decrement its actual `+04`
   reference, and dispatch its current zero slot only on zero. Capture renderer
   and its `+6C` slot before `B33E40`; call current `B32250` on that receiver.
   Use a fresh renderer global for `B27D40`, then call `B3E730`.
3. Query current memory length/data and invoke the real D3DX image-info import.
   HRESULT is ignored. Start requested dimensions at `FFFFFFFF/FFFFFFFF`, saved
   dimensions at image width/height, and mips at image mips. Quality reduction
   requires unsigned mips > 1, nonzero current renderer `+1D84`, and no substring
   `detail.dds`, `noseart`, or `interface/textures/gui/units`. Re-read quality;
   x86 shifts mask to 31, subtract mips as DWORD, and clamp each signed result to
   at least 1. The native CRT calls each clean up 8 bytes (`FBAA/FBCF/FBF0`).
4. Release current COM once if nonnull, then clear `+10`. Obtain the device from
   a fresh renderer using `B1FEF0`, re-query memory length/data, and call D3DX
   once with UNKNOWN format, MANAGED pool, `70004/FFFFFFFF` filters, zero usage
   and optional arguments, and the actual owner's `+10` as output. There is no
   retry or RGB conversion. The 15-argument stdcall import receives 60 bytes.
5. Only a nonnull current output updates metadata. Query memory size again for
   `+24`, current COM level count for `+14`, and fresh current COM level-zero
   description for actual dimensions and format. Save requested policy
   dimensions and zero `+30`. Ignore HRESULT even with a nonnull failure output.
   Null output preserves every old metadata field while leaving COM null.
6. Decrement/release the temporary memory owner. Construct the diagnostic from
   fresh owner COM and original name, call support again, and release its current
   string buffer using current length+1. Native `B3F5D0`'s overlap-safe BF7680
   copy has ADD ESP,0Ch at `B3F616`. Getter `00419CC0` consumes none of the three
   arguments pre-pushed for `BD1510`; that terminal uses RET0Ch.

The owner identity/reference word, flags, serial, requested-mips `+3C`, surface
cache and retained source `+4C` are not modified by this body. This is deliberately
different from initial `B2C2D0` loading. Cache removal and renderer-array removal
are the existing native operations and are not reversed or followed by an added
re-registration.

## Failure and output domain

`NativeTextureReloadOutputs` borrows two distinct caller-owned output objects,
disjoint from the owner, name, acquired state and service storage. The caller
supplies initialized native stack preimages for fields that a failing provider
does not write. These inputs make the ignored-HRESULT cases expressible without
inventing zeroed output or reading C++ indeterminate values. Successful real
D3DX/GetLevelDesc providers overwrite the consumed fields. Results are recorded
for inspection and do not alter control flow.

The retained acquired frame records the original owner and any acquired stream;
it adds no owner or reference. Re-entry is rejected. Native has no source/memory
or COM cleanup map on its main path, so provider failure retains partial writes
and acquisitions rather than rolling them back. Failed frames require external
resolution before their storage expires. Conversion-internal failure state is
still governed by the existing `BEF750` provider contract.

FH3 descriptor `DF7868` has magic `19930522`, one unwind state, map `DF7860`, no
try blocks; the sole map row is `{-1,CBEF80}`. State 0 is installed only after
`B3F5D0` returns at `B3FD38`; `CBEF80` addresses the diagnostic at EBP-54 and tail
jumps to existing `B3F4C0`, which releases only its current name buffer. Normal
cleanup switches to -1 before the pool getter/free. The C++ cleanup uses the same
existing noexcept actual-string interface; exceptions from a lazily recreated
pool's release, arbitrary SEH, original FH3 ABI and throwing COM are outside
that established source domain. The raw handler thunk `CBEF88..CBEF91` is not a
Ghidra function and is recorded for primary definition, without worker mutation.

Supported invalid-open deleting profiles are the existing physical `D691B0`,
memory `D642C0`, adopted `D68DB0`, and raw inflater `D64400` source providers.
All numeric table identities are read as data and routed to concrete source;
only actual D3D COM pointers/imports are called as host code. Unknown changed
numeric targets fail explicitly at the corresponding provider boundary.

## Evidence and validation

The report records every callsite, containing native body, direct helper and
indirect target domain. Full native pseudocode and all 258 assembly lines were
read; live prototypes establish the two inclusive body ends. Callee bodies were
checked before using their existing provider contracts. Worker Ghidra access was
read-only, using the existing `bsp.gpr` and `/battlestationspacific.exe`.

Validation results are recorded in `reports/native_texture_reload_cc10.json`.
The MSVC Win32 Release build and both existing CTests pass. The callsite checker
reports 24 checked rows and zero failures; symbolic indirect rows remain outside
its automatic proof. Live Ghidra and installed PE bytes agree for both full bodies.
The ignored `local/texture_reload_diagnostic_probe.cpp` executes the original
81-byte B3F5D0 body with production resize and overlap-safe CRT bridges, comparing
it to the current library over the actual constructed string pool. Three cases
(empty name, populated name, destination-name alias) pass. This is helper-only
differential evidence; it does not execute original B3FA90, original string/CRT
bodies or the complete VFS/D3D reload graph. The probe uses `/MD` and an embedded
manifest.

Compilation/report checks establish source and static-call evidence for reload.
This packet does not claim original-code differential execution, a real reload
through the complete cache/VFS/D3D graph, original ABI, GPU appearance or gameplay.
The primary's baseline executable currently stops before a game window at FMOD
Init61/CreateSound78; that unrelated runtime boundary is not reload proof.
