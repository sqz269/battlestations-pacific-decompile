# GlobalConfig sample zero adapter Source

`NativeGlobalConfigSampleDispatch` now provides the domain-qualified effects
adapter identified in the [readiness packet](CC12_GLOBAL_CONFIG_SAMPLE_ZERO_ADAPTER_READINESS.md).
It borrows the retained canonical `SoundSampleRuntime` and owns a stateless
`NativeGlobalConfigCurrentDispatch`. Copy and move construction/assignment are
deleted. Its constructor only binds the reference; it performs no runtime
initialization, shutdown, registration or reference handling.

The implementation is confined to
`include/bsp/native_global_config_sample_dispatch.hpp` and
`src/native_global_config_sample_dispatch.cpp`. This is zero additional
Original-function credit.

## Dispatch behavior

- `stop_slot_08` explicitly qualifies the owned current provider's stop method
  and forwards the actual receiver and unmodified flag. It does not synthesize
  a stop operation for Source samples.
- `zero_references_slot_00` copies the actual receiver's current first DWORD
  with `std::memcpy`. For `D5B074`, it explicitly calls
  `SoundSampleRuntime::zero_references_slot_00` on the retained canonical runtime
  and immediately returns. Otherwise it explicitly calls the current provider's
  zero method.

The adapter performs no increment/decrement, retain, slot write, cache lookup,
payload-table installation or post-call payload read. It adds no fallback no-op
or catch. Existing GlobalConfig/Singleton/Sound source remains unchanged. The
existing sample zero method supplies full scalar cleanup/free after the caller's
actual `+4` decrement reaches zero; it does not decrement the sample again.

## Preconditions before dispatch

The caller must admit a disjoint union of valid non-null, live receivers:

1. A complete actual Source `0x7C` sample with current `D5B074`, obtained from
   this exact canonical runtime/factory/cache and retaining its producer's
   string, resource, FMOD, publication and allocator domain.
2. An actual raw object whose current table word differs from `D5B074` and is a
   genuine callable current-process table with the required receiver and flag
   contracts. Another numeric Original profile identity does not satisfy this.

The numeric branch only selects an already admitted representation. It proves
neither sample type/size, allocation base, ownership nor lifetime. Sample options
at `+8`, other subobjects, foreign-cache samples, resource/cache owners and
projected `SoundInstance`/`SoundLevelEntry` storage are not admitted. Cache
membership is not used as proof; a fresh sample can be absent after a duplicate
weak insertion.

Stop additionally requires the raw second domain's genuine current `+8` method.
The sample profile's `+8` word is literal `6F6F4C0A`, not a valid stop. The
non-null second-array producer remains unresolved; the array is not assumed
empty. Unchecked ObjectiveSounds indices that place samples in stop-bearing
slots remain outside this adapter's admitted stop domain.

Retain the adapter, canonical sample runtime, current cache/owner and related
publication, strings, resources, FMOD library/handles and accounting storage
through the last GlobalConfig release. Sound shutdown must not destroy these
first. The existing effects interface is `noexcept`; selected callbacks must
actually be nonthrowing. Existing sample helpers can otherwise throw, and this
adapter does not swallow that failure. General native EH/SEH equivalence and
full application drain ordering remain unproved.

## Verification boundary

Static checks verified the single first-DWORD copy, all three explicitly
qualified calls, four deleted copy/move operations, and absence of added
reference operations or post-call payload access. Both new files were absent
at the base commit; all 25 Source pins from the readiness packet still match.
The report preserves both new Source contents and hashes plus those unchanged
dependencies. `git diff --check` passed.

No CMake registration, app assignment, build, test, probe, Ghidra or ledger work
occurred here. The primary integrator owns registration, normal build and
emitted-code review; this packet makes no emitted ABI, runtime or game claim.
Evidence: [cc12_global_config_sample_zero_adapter_source.json](../reports/cc12_global_config_sample_zero_adapter_source.json).

## Primary registration and emitted-code review

The integrator registered the source in bsp_core and ran the normal MSVC Win32 build; all three existing checks passed. Complete emitted methods show a 28-byte constructor that binds the runtime, a 12-byte stop tail-call preserving receiver and flag, and 99 bytes of zero-method code plus 5 bytes of alignment padding. The sample arm directly calls the concrete canonical sample zero method; the other arm directly calls the owned current provider. Positive physical COFF definitions and unique whole-library members establish each named target. Neither arm adds a sample decrement or a payload read after dispatch.

The compiler adds a 29-byte security-cookie/CxxFrameHandler3 boundary; this does not prove Native EH equivalence. The adapter public roots are absent from the game map. It remains caller-admitted glue with zero new Original-function credit; canonical runtime lifetime, nonthrowing selected targets, raw second-array provenance and application drain ordering remain requirements.

Evidence: [cc12_sample_subtree_primary_review.json](../reports/cc12_sample_subtree_primary_review.json).
