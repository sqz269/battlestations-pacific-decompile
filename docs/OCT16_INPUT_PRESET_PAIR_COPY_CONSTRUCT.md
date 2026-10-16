# Preset pair copy construction

| Entry | Coverage | Original interface |
| --- | --- | --- |
| 0069E7A0 | Complete 99 defined bytes / 37 instructions, through 0069E802 | ECX destination, unconsumed EDX, one stacked source, RET4, EAX destination |

The new Source function constructs the actual 1Ch preset pair: a pooled-key
header at +0/+4 followed by five opaque four-byte mapped words at +8 through
+18. The existing `STL_StringPresetPair_CopyConstruct_0069E7A0` name remains;
the new descriptive Source name is provisional. The incomplete Native
`undefined(void)` prototype does not establish its physical arguments.

Compare destination and source addresses before unconditionally clearing
destination length +0 and data +4, in that order. Equal addresses skip key
construction but still copy the five mapped representations. This abandons
any old key backing, including on self-construction. It is not assignment.

On unequal addresses read CURRENT source length after those resets and invoke
the actual raw-pool resize service with preserve=1. After it returns, reread
CURRENT source length. If nonzero, capture CURRENT destination length, CURRENT
source data, then CURRENT destination data, in that order. The actual CRT copy
receives destination length bytes, without adding a terminator byte. Its EAX
is ignored. Finish with five interleaved four-byte source reads and destination
writes, then return the original destination address.

| Scope | Native site and actual target |
| --- | --- |
| Owned resize | 0069e7be CALL 0041dd40 |
| Owned key copy | 0069e7d3 CALL 00bf7680 |

Both services are published on Main 2995ccba1c0ede5bd92fc264baecab3deb2c85db;
the actual CRT-copy Source pin fe6f68c599408b2e782cc7151e50dc710fd633aa is an
ancestor. Use the real `NativeStringRawPoolContext` resize overload and actual
CRT-copy API. The context borrows canonical pool publication 01090AA8,
small-return gate 01090AA4 and manager publication 01090AA0. The separate
borrowed volatile feature cell is actual 0109EEA4. Its PE/Ghidra initial value
does not prove runtime feature truth; caller storage, lifetime, runtime state
and the copy service's CPU/OS and extent preconditions remain required.

The accepted resize Source retains its existing current-CRT overlap/zero-copy
policy, raw-manager and getter-failure domain. This packet does not change
those boundaries. It adds no adapter, provider, global or allocation model.
The key copy uses the actual published CRT service; constant-size `std::memcpy`
is used only for the five opaque mapped representations. Each mapped word has
its own four-byte `std::byte` temporary, rather than a whole-payload snapshot.
Thus indeterminate representation bytes need not be evaluated as integers,
pointers or floats. Native MOVs and emitted Source MOVs do not evaluate or
quiet a possible signaling-NaN representation. Header DWORD accesses alone
are volatile; the mapped bytes have ordinary representation-copy semantics,
without concurrent-writer or MMIO guarantees. Address offsets use wrapping
32-bit integer arithmetic and each reached address must designate valid storage.

There is no local Native EH state, cleanup or catch, and the Source adds none.
If resize throws, the two initial resets and provider progress remain, and
mapped copying has not started. Neither failure nor self-aliasing repairs or
frees abandoned backing. Source and destination may overlap only within the
accepted providers' valid extents and without overlapping active private
argument/return/provider storage. No null-header guard or alias normalization
is introduced. The old generic AV/NativeStringStorage pair and node projections
remain unchanged. Pair constructor 69E6E0 is not a Native child or a Source
dependency here. Node factory, placement action, insert fixup and map owner
remain outside this packet.

Source fastcall adds RawCtx and feature references, so it uses RET0Ch instead
of Native RET4 and still returns destination in EAX. A private volatile
predicate pins the comparison before both header resets. In this actual
compiler output SETNE reuses byte [EBP+0Bh], the high byte of the already
captured Source source-argument slot, while retaining its pointer in EDI.
It is a private compiler spill, not a Native field or public frame model.
The extra parameter loads, argument-slot mutation, registers/flags and private
fault order are qualified. Current C++ exceptions/CRT/heap and borrowed-cell
semantics do not establish Original ABI, private frames, FH3/SEH/RTTI, fault
recovery, runtime or game identity.

The default MSVC Win32 build and all three existing suites pass in 7.37 seconds.
These suites do not exercise this new entry specifically. No new tests,
probes, Native execution or fixture/game validation are claimed. All actual
CMake code was read: public 118B/46 instructions plus three unused helpers
51B/24, totaling 169B/70 across four sections, with exactly two proper REL32
provider bindings and no code padding or local EH. Five DWORD MOV pairs stay
interleaved. Compiler directives, debug/checksum metadata and an unreferenced
four-byte `__Avx2WmemEnabledWeakValue` BSS definition are retained separately;
that BSS is not file bytes or a new application feature cell. The companion
report freezes full Native, provider, object and build hashes. Final immutable-
pin primary acceptance is pending.
