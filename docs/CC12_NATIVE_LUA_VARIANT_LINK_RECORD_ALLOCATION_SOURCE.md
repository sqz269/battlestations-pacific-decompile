Primary compiled review: Registered allocation helper is91 bytes/24 instructions and uses the actual singleton allocator. All five narrow stores occur in the required order and preserve the other38 block bytes. The actual request, three indexed relocations and complete body were reviewed.

Normal MSVC Win32 build 2026-10-09T18:27:44.966886+00:00 to 2026-10-09T18:28:02.224133+00:00 passed three existing checks. Shared evidence pins105 Source/build inputs and four artifacts, replays all23 prior objects unchanged and captures two unchanged existing provider objects plus three new objects (28 total). Core contains32 selected positive roots. New public roots are absent from the application map; no consumer or forced retention was added. Original ABI, production lifetime, startup and gameplay are unproved. The candidate section below is an immutable worker-time snapshot; its Source97 artifacts are historical.

# Qualified Lua link-record allocation Source candidate

`006EDEA0..006EDED6` allocates 52 bytes, clears three DWORDs, writes two
separate bytes and returns the allocation pointer. This candidate implements
that success-domain schedule using the existing `singleton_lifetime_allocate`
provider. Its name describes the observed neighboring link operations; it
does not recover a complete Native node type.

The candidate owns only:

- `include/bsp/native_lua_variant_link_record_allocation.hpp`
- `src/native_lua_variant_link_record_allocation.cpp`
- this document
- `reports/cc12_native_lua_variant_link_record_allocation_source.json`

Primary registration, normal MSVC Win32 build, emitted-body review and Core
admission remain pending. This packet adds no CMake entry, ledger record,
Ghidra annotation, test, probe, caller or consumer.

## Native body and gate

Root independently captured and read all 55 bytes and 17 operations, compared
the saved instruction starts with raw PE decoding and compared live bytes
with the installed image. The approved body SHA-256 is
`259e4c15e1d271b55cf57f4e6cbd91da8fd1c2dd389415e077c14e739508c6f7`.
The gate artifacts are in the primary worktree's
`local/cc12_lua_link_record_allocation_primary/` directory:
`capture.json` and `Root_Astra_gate_approval.json`. The report pins both and
records this worker's independent PE/hash/instruction-start replay.

The body pushes `34h`, calls `00BF681B`, and removes its one stack argument.
It tests EAX before writing DWORD+0, tests the 32-bit EAX+4 address before
writing DWORD+4, and tests EAX+8 before writing DWORD+8. It then writes
BYTE+30h=1 and BYTE+31h=0 without a null test and returns with the allocation
pointer still in EAX. The function consumes no recovered caller argument;
this observation does not establish a drop-in caller ABI for the Source entry.

The worker opened no Native child body and made no Ghidra mutation. The
selection metadata for `006EDE90` identified unresolved `006EDD80`; that
separate helper and the accepted Lua dependency-cycle audits were not extended.

## Actual provider and qualified domain

The public Source entry is:

```cpp
void* allocate_native_lua_variant_link_record_006edea0();
```

It directly calls the existing provider declared in
`include/bsp/singleton_lifetime.hpp` and implemented in
`src/singleton_lifetime.cpp` with:

```cpp
{SingletonAllocationKind::object, 0x34, 0x34}
```

That provider uses actual host `std::malloc(host_bytes)`, retries through
`_callnewh(host_bytes)`, and throws `std::bad_alloc` when the handler returns
zero. Its accepted evidence is `reports/singleton_lifetime_audit.json`.
This receipt establishes a host CRT service boundary; its historical build
and Native allocator analysis are not new work or a new ABI claim here.

The accepted Source success domain is a real, suitably aligned, usable,
nonnull 52-byte allocation returned by that provider. Allocation or handler
exceptions propagate; the new entry has no `noexcept`, catch or translation.
It supplies no alternate allocator, callback or fabricated service. A
successful allocation preserves the provider's actual raw pointer identity.

Within that domain the Source executes these five separately sequenced
volatile stores:

| Order | Offset | Width | Value | Native store |
| --- | --- | --- | --- | --- |
| 1 | `00h` | DWORD | 0 | `006EDEAE` |
| 2 | `04h` | DWORD | 0 | `006EDEBB` |
| 3 | `08h` | DWORD | 0 | `006EDEC8` |
| 4 | `30h` | BYTE | 1 | `006EDECE` |
| 5 | `31h` | BYTE | 0 | `006EDED2` |

The other 38 bytes (`0Ch..2Fh` and `32h..33h`) are untouched by this helper.
There is no broad clearing, complete node layout, value initialization,
copy, owner, destructor or consumer. The returned allocation is the actual
provider result, not a projected wrapper or substituted address.

The Native null and wrapped-interior-address branches are outside this
usable-allocation success domain. They are not replaced with an invented
recovery behavior and are not claimed to be reproduced. Native `00BF681B`
binding, allocation/new-handler/CRT identity, exceptions and unwind identity,
registers, flags, stack layout, faults and their timing, arbitrary raw aliases,
original caller ABI, production lifetime, startup and gameplay remain unproved.

## Current baseline and validation boundary

The baseline is published main `a3050906b`, including the Source97 receipts:

- `reports/cc12_native_lua_variant_string_fields_cleanup_primary_review.json`
- `reports/cc12_native_squadron_launch_task_cleanup_primary_review.json`

Their shared normal build ran from `2026-10-09T17:59:46.563985+00:00` to
`2026-10-09T18:00:05.036630+00:00`, passed three existing checks, and captured
or replayed 23 whole objects with 27 unique positive Core definitions. Both
receipts describe the same 97 source-input pins and four artifact pins.
The worker replayed every input using the stored canonical-LF hash and all
four artifacts by exact byte count and SHA-256. Ninety-five inputs also
matched raw bytes; the two line-ending-only differences are named in the
candidate report. No object graph or long historical pin list is duplicated.

The current provider header and implementation are themselves among those
97 inputs and are pinned directly with the accepted allocator receipt. The
candidate additionally pins its own header, implementation and this document.
Prior Source88 artifacts remain historical and are not the current build
baseline. Baseline build evidence does not compile or admit this candidate.

Worker validation covers the approved 55-byte body, current input/artifact
hash replay, the exact provider call, and the five explicit volatile stores.
No build or execution was performed for this new source. Root owns the
normal build, emitted-code inspection and any subsequent admission decision.
