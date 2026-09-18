# Global configuration sound assignment and float-vector storage

Addresses: 008DBE90, 0087D7B0, 00A83FD0, 0045A120, 00459CE0, 00412FD0, 00456CE0, 00457F00, 00455830, 00455850

## Sound assignment

`assign_global_config_sound_008dbe90` reconstructs the complete 171-byte body
008DBE90..008DBF3A. Native ECX is embedded GlobalConfig+2C0 storage; the two
stack arguments are the signed slot index and native string. It returns AL1
and RET8. Caller 0087F6F9 establishes this receiver and argument order.

The routine acquires a sample through the current F8BBE8 cache, then captures
the destination slot. When old and new differ, it publishes new, atomically
retains it, and atomically releases old, calling old's current vtable+0 at
zero. Finally it releases the current temporary cache result. Equal pointers
skip assignment and its retain/release pair, but still release the temporary.
The index uses unchecked DWORD arithmetic; it is not clamped to three slots.
Callbacks can change the destination, and those changes survive return.

Source calls the existing concrete A83FD0 cache implementation and borrows its
string allocation/factory services and current owner. The existing cache is a
host-map projection; this packet does not claim native tree layout or re-port
the cache. `GameplayEffectComponentLifetime` supplies current-vtable final
release and can be provided by the existing `SoundSampleRuntime`.

Native FH3 metadata DD56B4 describes one state with unwind action CA33D0:
`LEA ECX,[EBP+8]; JMP 004C3810`, releasing and clearing the actual temporary.
Source supports corresponding C++ cleanup when assignment callbacks throw.
Acquisition failure does not arm it; final temporary release occurs after
disarming it. Two source-only callback-exception checks cover those distinctions.
The original FH3 handler was not executed in the comparison probe.

## Float-vector insertion

The Globals.lua difficulty arrays use four actual 10h checked headers, with
opaque word0 and begin/end/capacity pointers at4/8/C. The loader's slow append
path calls 0045A120, which validates its iterator and calls 00459CE0 for one
element. The source adapter covers valid end insertion, using the existing
checked storage implementation and its CRT-backed allocation/free services.
It retains 1.5x growth and begin/capacity/end publication after old backing is
released. It does not expose the original general insertion/iterator ABI.

Evidence found an important float specialization: 00459CE0 first captures
the argument with MOVSS, preserving bits; existing elements move bytewise
through 00456CE0/memmove_s; newly filled elements pass through x87 FLD/FSTP32
in 00457F00 or 00455830. Thus signaling NaNs become quiet and set the x87
invalid flag with exceptions masked. The initial DWORD-only binding failed
the comparison for that reason. The corrected storage kind converts at the
fill step, after allocation and old-element copying. Existing DWORD storage
continues to preserve bits without invoking x87.

Ghidra had a CALL_RETURN override at the returning free call00459DE0, omitting
the three-byte ADD ESP,4 at00459DE5 and truncating the decompiler's nonempty
growth branch. The write-locked flow repair restored it and saved/refreshed
the export. The prior override and exact repair are recorded in
`reports/native_global_config_fields_r116_flow.json`.

The seven STL names are descriptive hypotheses based on these bodies. Their
library contracts are reused through source storage adapters; no complete
general STL reconstruction is claimed. The loader's direct spare-capacity
MOVSS append remains part of the future full loader, distinct from this
insertion helper's x87 fill.

## Validation and limits

- Nine function spans, 1,689 bytes, matched live Ghidra and the original PE;
  62 additional live/PE bytes establish the unwind action and FH3 metadata.
- Eight copied native bodies total1,001 bytes and23 direct CALL relocations.
  The sound body also makes five indirect calls, covered by the import and
  current-vtable contracts. Original vector storage uses real CRT allocation,
  free and memmove_s boundaries. Malformed-iterator, length, allocator-overflow
  and original exception paths are not executed.
- Ten sound comparisons passed using the actual source cache, native string
  operations and atomic reference counts, with controlled sample constructors
  and zero-reference callbacks. Cases include hits, misses, equal pointers,
  null factory results, nonzero/zero old counts, callbacks that change slots,
  and signed/cross-array unchecked indices. This is not FMOD sample playback.
- Seventeen full snapshots of four vectors passed after68 end insertions,
  including empty, allocated-empty, full and spare-capacity states, multiple
  reallocations, signed zero, quiet/signaling NaNs and infinities. Compared
  counts, capacities, allocation-change identity, payloads and x87 status.
- The combined comparison streams match19,220 bytes. Two source-only unwind
  checks and one DWORD bit-preservation regression also pass.
- Strict MSVC Win32 build and all three existing CTests pass. No new CTest
  cases or permanent test framework were added.

The report records exact artifacts, inputs, call rows, logs and immutable
archives. This packet closes two dependencies of full0087D7B0 configuration
population. That loader and its game-constructor binding remain open. No
application run or gameplay claim, drop-in original ABI/FH3/SEH proof,
unmasked floating-point trap test, or allocation-failure parity is supplied.
