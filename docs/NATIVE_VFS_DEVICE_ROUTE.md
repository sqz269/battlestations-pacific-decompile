# Actual VFS device selection and provider name resolution

Addresses: `00BDD850`, `00BDBC70`, `00BDB680`, `00BDB670`, `00BDBE00`,
`00BF0FB0`; existing lookup dispatch extended at `00BDD0A0`, `00BD90D0`,
`00BDBC00`.

The six new actual-storage bodies total 898 original bytes. They compose with
the existing actual mount traversal, native strings, current pool and provider
implementations. No mount tree, provider, archive, private string pool or
replacement catalog is created. Descriptive names remain hypotheses.

| Routine | Inclusive body | Original ABI | Coverage |
|---|---|---|---|
| Device selection, BDD850 | BDD850..BDD980, 305 bytes | ECX manager; stack name/unused; RET8; EAX device/-1 | complete |
| Resolve visitor, BDBC70 | BDBC70..BDBDFB, 396 bytes | ECX visitor; stack payload/suffix; RET8 | complete |
| Visitor destructor, BDB680 | BDB680..BDB6D7, 88 bytes | ECX visitor; RET | complete |
| Visitor result, BDB670 | BDB670..BDB673, 4 bytes | ECX visitor; RET; raw AL byte4 | complete |
| Deleting wrapper, BDBE00 | BDBE00..BDBE1D, 30 bytes | ECX visitor; stack flags; RET4; EAX captured visitor | complete |
| Provider logical name, BF0FB0 | BF0FB0..BF0FFA, 75 bytes | ECX provider; stack suffix/output; RET8; AL0/1 | complete |

The existing 672-byte `visit_native_vfs_lookup_mounts_00bdd0a0` is extended to
dispatch D683F4; its traversal is reused rather than copied or counted again.
The existing BD90D0 provider-contains switch is factored for BF0FB0 reuse and
gains actual MPAK BB4B20. The existing BDBC00 name-probe dispatch gains the
sibling's actual five-byte BB40C0 false leaf. The older date/name/exists profile
paths retain their implementations. Unsupported current targets remain
explicit source exceptions, not recovered native failure behavior.

## Visitor storage and construction

BDD850 itself produces the 14h-byte stack visitor: identity D683F4 at +0,
result byte zero at +4, native name length/data zero at +8/+Ch, device FFFFFFFF
at +10h. Padding +5..+7 is not initialized. Live D683F4 contains BDBE00,
BDBC70 and BDB670 at slots +0/+4/+8. The context borrows the actual profile
storage, and every traversal dispatch rereads the current identity and slot.

BDD850 owns the visitor before constructing its separate temporary name. A
failure during that initial copy destroys the visitor but does not destroy the
incomplete name. Once copying finishes, it normalizes the copy through BEE690
and invokes the existing BDD0A0 using the captured manager. The original input
name and unused second stack argument are never written. Both observed direct
callers, BB580E and BB5C35, pass the same local name pointer twice; BDD850 reads
only the first stack argument and returns with RET8.

After traversal it tests the current result byte, captures temporary data, and
disarms the name before release. The successful branch captures device +10h
before either name or visitor cleanup. The unsuccessful branch does not read
device +10h and returns -1. Visitor cleanup follows name cleanup on both
branches. The result is not recomputed from storage that cleanup may mutate.

## Mount callback and provider resolver

BDBC70 first reads the mount payload's recorded name length. A zero length
captures payload+8 provider and current provider-table +24h, then invokes that
entry with the original suffix and visitor+8 as output. It publishes the raw
AL result to visitor+4 after the call.

A nonzero mount name constructs an empty native provider-name temporary and
arms its cleanup before the provider call. It stores raw AL, constructs `/`
from the verified CE7898 bytes, concatenates the current mount name with that
slash, then concatenates the returned header with the current provider name.
It copies the returned final name into visitor+8 even when AL was zero. The
four temporaries are destroyed in reverse order: final name, prefixed mount,
slash, provider name. The constructor-return boundaries, current header reads
and cleanup-state changes are retained.

Only after all that work does BDBC70 retest CURRENT visitor byte4. If nonzero,
it reloads CURRENT payload+8 provider and copies that provider's +10h device to
visitor+10h. A false result leaves the existing device field untouched even
though the callback name may have been replaced.

BF0FB0 captures the actual provider's current table +10h contains target before
invocation. False returns zero and leaves output untouched. Success copies the
CURRENT original suffix into output with an identity guard, actual resize
(`preserve=1`), a fresh source-length guard, and current destination count and
data pointers. It returns normalized AL1. This is a logical-name operation;
the provider's physical-path replacement operation at slot +18h is separate.
The earlier semantic implementation in `physical_directory.cpp` remains
separate from this new actual-storage body.

The newly recovered inline copies at BDD8C3, BDBD25 and BF0FE6 call native
BF7680, whose BF769A branch handles backward overlap. These source fragments
therefore use host `memmove`; they do not port CRT internals. They omit only
the zero-byte host copy, consistent with the existing actual string interface.
Existing shared concatenation and normalization bodies retain their separately
documented implementation and limits.

## Cleanup evidence

| Handler / FuncInfo / unwind map | Verified transitions |
|---|---|
| CC63B0 / E00874 / E00864 | BDD850 state1 -> 0 through CC63A8, name at EBP-28h -> 41DD20; state0 -> -1 through CC63A0, visitor at EBP-20h -> BDB680 |
| CC6130 / E004EC / E004CC | BDBC70 state3 -> 2: EBP-1Ch; 2 -> 1: EBP-14h; 1 -> 0: EBP-24h; 0 -> -1: EBP-2Ch; all four funclets jump to 41DD20 |
| CC6038 / E00388 / E00380 | BDB680 state0 -> -1: CC6030 reloads saved this at EBP-10h then jumps to existing BD8FE0 base reset |

BDB680 captures current name data +Ch, reads current length +8h only for a
nonnull block, and returns length+1 through the actual pool with the original
preserve/unused value 1. It writes D68380 after that return and leaves name,
result and device fields unchanged. BDBE00 calls this destructor, tests flag
bit0, optionally calls existing host free, and returns the captured visitor.
Its existing `CG_scalar_deleting_dtor_00bdbe00` name is retained.

## Integration and validation

`NativeVfsDeviceRouteContext` borrows a `NativeVfsLookupRouteContext`, the
actual D683F4 table and a provider resolver. The lookup context's optional
`device` pointer must refer to the bound device context while this profile is
in use. `NativeVfsProviderResolveDispatch::invoke_resolve` consumes the already
captured slot24 entry/provider and actual suffix/output pointers. It supplies
no fallback. The runtime integrator maps BF0FB0 to this generic resolver and
BB68F0 to the sibling's actual MPAK member-directory selector.

The worker used the configured `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`, verified automatically before every live CLI
batch. Primary had already defined BDB670 and repaired BDBE00. BF0FB0's stale
pseudocode drops its assignment, while live assembly contains all 34
instructions and the read-only flow check finds zero gaps. The assembly was
used. The worker made no Ghidra mutations.

All six fresh live spans, 898 bytes, match the installed executable. Strict
MSVC Win32 Release `scripts/build.ps1` passed with
`MSBUILDDISABLENODEREUSE=1`, and both existing CTests passed after eight native
seeds matched disk. No new tests were added. All 29 direct call rows passed the
live checker with zero failures. Seven indirect sites, including the four
changed dispatch sites, were manually reviewed against their complete listings
and are explicitly distinguished from the checked direct targets.

There is no new native/source fixture, runtime-vtable wiring or gameplay claim
in this packet. The actual pool release is noexcept, so throwing pool getters
during cleanup and native FH3/SEH are not established by these C++ interfaces.
Concurrent mutation and native stack-spill aliases remain separate proof
domains. The completed MPAK open source/report was not modified.
