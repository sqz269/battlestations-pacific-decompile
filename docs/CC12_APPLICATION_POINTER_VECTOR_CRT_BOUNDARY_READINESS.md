# Application vector: fixed CRT entry points and Source providers

This read-only packet identifies the bounded native routes behind reserve's
allocation/free calls and the concrete current Source callback producer.
The selected native evidence is **15 bytes / three complete instructions**.
The allocator target exceeds the 32-byte limit; the free route reaches a
second jump destination beyond the permitted hop. Their policies remain open.

The renderer's shader-cache context does have callable host allocation/free
providers. The full-preload/actual Application-vector connection remains
separate. No Source, Ghidra, ledger or CMake changes, new credit, builds, tests,
probes, native execution or Application wiring are added.

Evidence: [machine-readable report](../reports/cc12_application_pointer_vector_crt_boundary_readiness.json).
Base: published main `365083d632ee8aaff7095a02292766b1f5271051`.
The accepted [reserve audit](CC12_APPLICATION_POINTER_VECTOR_RESERVE_NATIVE_READINESS.md)
is reused for its caller contract and retained Source evidence.

## Selected native routes and stopping points

Each query verified the existing `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`, x86 little-endian, image base `00400000`.
The initial live function count was 64,729. Both initial entries passed the
whole-body 32-byte gate at five bytes. The lease was extended to their direct
targets before requesting target metadata.

| Selected entry | Complete bytes | Instruction | Disposition |
| --- | --- | --- | --- |
| `00BF55BE`, `operator_new` | `E9 58 12 00 00` | `JMP 00BF681B` | Target metadata spans `00BF681B..00BF6883`, **105 bytes**; stop at metadata. |
| `00BF6989`, `_free` | `E9 1E FC FF FF` | `JMP 00BF65AC` | First target is five bytes, within the gate; inspect that whole target. |
| `00BF65AC`, `_free` | `E9 17 38 00 00` | `JMP 00BF9DC8` | One direct hop from the initial free entry is already consumed; do not follow this destination. |

All 15 selected bytes match the current installed PE. All three independent
decodes and signed relative-displacement calculations agree with the live
listing. Correct library names were retained. `00BF9DC8` received no metadata,
body or byte query; no allocator target listing/bytes were requested.

The selected instructions are direct relative jumps within the main PE. They
do not access an import cell. No IAT cell or import metadata was read, and no
import sweep occurred. This establishes the fixed internal routes shown above;
it does not identify any ultimate DLL import or heap implementation beyond them.

Tooling qualification: decompile requests made at the initial five-byte thunk
addresses unexpectedly returned expanded downstream pseudocode. That text is
excluded from selected evidence and all semantic conclusions. The report records
the event without reproducing it. Subsequent analysis used bounded listings,
bytes and target-size metadata; no further decompile request was made.

## Stack and result contract at the reserve boundary

The accepted original 95-byte reserve audit establishes the following caller
operations; the caller body was not queried again in this packet.

- Allocation: `00735EDF` pushes the low 32 bits of clamped request × four;
  `00735EE0` calls `00BF55BE`; `00735EE5` captures returned `EAX` in `EBX`;
  `00735EE9` caller-cleans four bytes.
- Free: `00735F09` reads CURRENT header data; `00735F0B` pushes that pointer,
  including null; `00735F0C` calls `00BF6989`; `00735F11` caller-cleans four
  bytes. Captured fresh data and capacity are published afterward. No free
  result is consumed.

Each selected `JMP` preserves the incoming stack, argument, return address,
registers and flags. There is no added return frame or argument adaptation.
Thus the selected routes preserve reserve's single-DWORD cdecl calling
expectation and allocation result handoff. A function-name or placeholder
prototype is not a fresh proof of the oversized callee's implementation ABI.

None of these jumps checks size, pointer or null, installs a handler, or catches
an exception. Passing null to the native free route is established; its ultimate
null behavior is not. Allocation success/failure, zero-size behavior,
new-handler retry policy, native exception identity, invalid-pointer behavior,
and heap provenance are not admitted from metadata or the excluded pseudocode.

`00BF55BE` converges at `00BF681B`, and `00BF6989` first reaches `00BF65AC`.
The existing Source labels use these pairs for array/object services, but those
labels do not establish `new[]`/`delete[]` semantics or allow allocations to be
freed across unrelated native and host domains.

## Concrete current Source binding

The [renderer translation unit](../src/game_native_renderer_application.cpp)
defines file-local `raw_allocate(uint32_t)` and `raw_free(void*)` at lines
114–115, before including
[game_native_renderer_shaders.inc](../src/game_native_renderer_shaders.inc).
The included `ShaderGraph` constructor supplies `&raw_allocate` to both
allocation slots and `&raw_free` to both free slots of its actual
`NativeShaderBinaryCacheContext`. The precise aggregate field order is pinned
in [native_shader_binary_cache.hpp](../include/bsp/native_shader_binary_cache.hpp).

The renderer implementation constructs this `shaders` member, and
`shader_cache_context()` returns `impl_->shaders.cache` by reference. These
are concrete callable Source providers with an available accessor; they are
not unresolved callback declarations or literal bindings to Original addresses.

`raw_allocate` passes `{object, size, size}` to
[singleton_lifetime_allocate](../src/singleton_lifetime.cpp). That Source body
loops over host `std::malloc(host_bytes)`, returns a nonnull result, calls the
host `_callnewh` after a null result, retries on a nonzero handler result, and
throws host `std::bad_alloc` when it returns zero. `raw_free` calls the
`noexcept` `singleton_lifetime_free`, which forwards to host `std::free`.
This describes current Source; comments referring to Original CRT behavior
do not establish that behavior in this bounded native packet.

For this concrete Source producer, a normal allocation return is nonnull.
Zero-size requests enter the same host loop, and allocation/new-handler failure
can leave exceptionally. The free wrapper forwards null to host `std::free`;
nonnull allocations must belong to the matching host allocation domain and
remain valid for release. The current host heap, installed new-handler and
exception objects are not thereby identified with the Original game's CRT.

Inside [the preload implementation](../src/native_shader_preload.cpp), lines
136–147 access the actual application+8 vector. On a capacity growth edge,
line 141 creates a local `NativeApplicationPointerVectorAllocation` from the
supplied cache's array-allocation and free slots; line 142 calls concrete
reserve. This local binding therefore exists. The vector interface itself
accepts externally supplied callbacks and does not validate their identity.

An exact-symbol search across `src`, `include` and `tests`, including `.inc`
files, found 18 matches in the two vector/preload headers and two implementation
files. It found no complete `NativeShaderPreloadContext` producer/full-preload
call and no external production call to public resize. The concrete cache
provider is available, but its connection through those missing call sites and
the actual Application-vector owner/lifetime is not established. This is a
bounded textual search, not a claim about unknown external consumers.

## Evidence reuse and remaining work

The prior accepted 232-byte / 98-instruction emitted resize/reserve chain was
not rebuilt or re-audited. Its report is pinned, and all 16 inherited canonical
references over ten unique paths were replayed. Thirteen directly consumed
repository files are pinned to the base commit, including the exact Source
producer, aggregate order, wrappers, reserve call site and helper implementation.
The report retains bounded excerpts and the complete search output.

A renewed Original allocator policy audit would need separate authorization
for the 105-byte `00BF681B` body. Original free implementation work would need
authorization beyond the observed `00BF9DC8` destination. This packet stops at
both boundaries. Native null/failure/new-handler/EH identity, cross-domain
compatibility, and full Application-vector production wiring remain unproved.
