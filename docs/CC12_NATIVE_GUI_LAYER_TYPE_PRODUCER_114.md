# Explicit Source Layer type role (packet 114)

`GameNativeGuiTextProcess` now owns one Layer guard, one 16-byte Layer descriptor
and separate completion metadata alongside its existing Element/Text roles.
The explicit initializer uses the SAME retained VFS counter/root/root-guard
domain, and the completion-only getter returns the SAME live three-word member
accepted by packet 108. There is no normal-host invocation or activation.

## Admission, identity and exposure

`initialize_layer_type_once_00cd73f0(TypeIdCounterLifetime&,
LightTypeBootstrapStorage)` requires completed pool and Text startup. Before any
Layer Native-cell effect it checks the exact stored counter object, root
descriptor and root-guard addresses. A completed repeat returns only after those
same-domain checks. An entered failure or reentry cannot replay initialization.
The temporary `LightTypeBootstrap` binds the supplied reference view without a
Native effect; it does not create another root or counter.

The new private fields are the sole Source Layer role. Their zero initialization
models loader preimages, not a successful default descriptor. The Source attempt
is marked entered before reading its guard. An unexplained nonzero private guard
is sticky and rejected without exposing lineage. This is explicit Source
policy: Original CD73F0 simply returns on any nonzero guard and does not prove
that an earlier attempt completed. Neither rule rolls prior effects back.

`layer_lineage_00f8bf88()` checks returned completion metadata before returning a
required `const volatile uint32_t (&)[3]` reference to the SAME Layer member.
It creates no ID copy, query-only descriptor or default. The existing typed and
live query APIs are unchanged. `native_operation_interrupted()` now includes an
entered Layer failure so callers can preserve the reached domain.

## Original schedule and Source implementation

Whole Original CD73F0 is `[00CD73F0,00CD743F)`, 79 bytes / 16 instructions / two
calls. It consumes no arguments, overwrites incoming ECX, returns with plain
`RET`, and contains no local exception frame, cleanup or `atexit` registration.
Packet 112 records the actual CRT table slot CE349C pointing to this entry.

For an admitted clear guard the Source body preserves:

1. BYTE Layer guard = 1, then DWORD name = D5CB0C (`cGuiLayer` evidence word).
2. The existing reconstructed AA8200 adapter on the SAME Element member.
3. BOTH current volatile Element ID reads, in order, before either Layer parent
   store; then the two parent stores in order.
4. The SAME real `TypeIdCounterLifetime::get_006fac20()` provider, followed by a
   current DWORD owner+4 read, unsigned modulo-32 increment/store, and only then
   Layer own-ID publication.
5. Source returned-completion metadata only after that final publication.

The existing generic initializer's per-word copy loop was not used as a claim of
the required two-load capture order. Two local scalar captures directly express
the Original EAX/ECX schedule; there is no parallel lineage array. Existing
Element/Text startup bodies, counter, root, host composition and CMake stay
unchanged. The existing descriptor size/name-offset assertions pass.

A parent failure leaves the guard/name and any reached Element/root/counter
effects sticky. A counter-provider failure follows the two parent stores and
may leave its own publication/registration effects. Source completion stays
unreturned in either case. The initializer has no new cleanup or rollback.
These are ordinary Source C++ exception boundaries, not a Native FH3/SEH,
nonlocal-exit or hardware-fault transport claim.

## Static and compilation evidence

Root reviewed the full Source/API diff and optimized initializer/getter/
interrupted slices: PASS. Strict MSVC Win32 `/MD /O2 /W4 /WX /EHsc` compile passed.
In the strict object the initializer's important offsets are:

| Offset | Effect |
| --- | --- |
| 07..48 | Completed prerequisites and exact domain identity preflight. |
| 73 / 9B | Entered metadata store, then actual BYTE guard read. |
| AC / B8 / CB | Guard store, name store, actual Element adapter call. |
| D0 / D5 | BOTH current Element DWORD loads. |
| DB / E1 | Captured Layer parent stores, after both loads. |
| EB / F0 | Real counter getter, then current owner+4 read. |
| F6 / F9 / FF | Counter store, Layer own-ID store, returned metadata store. |

The strict Source member returns with `RET 24h`; Original has no arguments and
plain `RET`. The new Source API/backing objects are not an Original binary ABI or
fixed-VA replacement. Static ordering does not establish Original global CRT
timing or absolute numeric type IDs.

`scripts/build.ps1` passed, and all three existing CTest checks passed in 8.17
seconds. The canonical `bsp_game` production object contains the initializer,
getter and interruption symbols. The existing TU registration is unchanged.
No new test, probe, initializer/getter execution or Native differential was
added; unused methods need not survive final executable dead-code removal.

Fresh verified BSP queries matched the whole 79-byte body to the unchanged
installed Original. Body SHA-256:
`834216a540048a72140a02574ae495b1bb4fd04b913064b25bb89f850862c343`.
Executable SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Lifetime, annotation and receipt

The caller must externally serialize initialization, reentry, lineage use and
domain retirement, and retain the SAME entire borrowed counter/root/VFS/host
graph through callbacks, uses and shared shutdown. Stored identity pointers do
not retain that graph. Completed pool/Text metadata cannot authorize entry after
the counter domain retires. All reached storage must be genuine and live, and
the returned counter owner nonnull at its +4 accesses. No fallback is supplied.

Only leased CD73F0 was annotated through the locked tool, saving old name/comment
history and the project, preserving its inventory comment and refreshing the
exports. `BSP_GuiLayer_TypeIdStaticInit` is a descriptive hypothesis replacing the
inventory name `CG_static_init_00cd73f0`. Only its new name/function ledger rows
were added; unowned rows remain unchanged.

The receipt is `reports/cc12_native_gui_layer_type_producer_114.json`. Source,
strict and canonical objects, optimized slices, Original bytes, logs, annotation
history and refreshed exports are sealed before commit in
`local/cc12_native_gui_layer_type_producer_114_artifact_manifest.json`.
The remaining boundaries are actual Original fixed-VA ownership, absolute CRT
timing/IDs, upper Page composition and production activation; none is claimed.
