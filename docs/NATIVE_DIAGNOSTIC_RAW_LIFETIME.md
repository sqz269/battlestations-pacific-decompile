# Diagnostic owner in the raw singleton domain

Addresses: `004C14C0`, `004BBCA0`, `00B4B850`, `00B4BA00`, `00BD0400`.

The renderer's physical-buffer diagnostic path can now use the same raw
`01090AA0` manager publication as its other owners. Previously the reconstructed
Lock context required a `SingletonLifetimeDomain` projection. It now borrows
`SoundLifetimeAccess`, which also accepts that existing semantic domain.

The getter captures the resolved manager's native `+10h` section, without
creating another manager or lock. Its original sequence remains: enter and
increment the section's physical `+18h` counter, recheck publication, allocate
four bytes, publish profile `CE752C`, obtain the manager again, read the current
publication for registration, leave the captured section, then read the result.
The existing explicit exceptional guard remains armed through normal leave.

Raw singleton shutdown now dispatches profile `CE752C` to the existing
`004BBCA0` deleter. The caller supplies the same diagnostic publication cell
used during construction. The manager pops the owner before calling its
deleter with flags 1; the deleter clears that cell unconditionally. There is
no added current-owner identity check. Missing bindings retain the existing
source contract error.

These are new C++ interfaces: the Lock context is now 16 bytes and the raw
deletion bindings are 108 bytes. Consumers were rebuilt together. The original
no-input getter, ECX owner functions and native exception machinery are not
drop-in replacements. Names remain descriptive hypotheses.

## Validation

The strict MSVC Win32 Release build and both existing CTests passed. All eight
native seed spans matched the installed PE. Fresh Ghidra reads matched five
complete original bodies plus the diagnostic vtable slot: 756 bytes total.
All direct call sites in those bodies were checked against live Ghidra;
indirect instructions and the `CE752C -> 004BBCA0` profile are retained.

One ignored lifecycle probe links the rebuilt production library. The actual
index Lock bounds diagnostic creates and registers the raw manager/diagnostic;
the vertex Lock dynamic-offset diagnostic reuses them. The raw vector contains
one diagnostic, and the native Windows section and tracked recursion counters
are zero afterward. Actual manager shutdown invokes the diagnostic deleter,
clears its publication, and drains the vector. The two null-COM paths use the
reconstructed sentinel behavior. No permanent tests were added.

The probe executes reconstructed source only. It does not exercise Direct3D
Lock, device startup, Reset, drawing, or gameplay. Allocation failure,
concurrency, publication mutation during provider calls and native FH3/SEH
coverage were not added. The prior semantic diagnostic and Lock fixtures are
documented separately in `NATIVE_DIAGNOSTIC_SINK_LIFETIME.md` and
`NATIVE_PHYSICAL_BUFFER_LOCK.md`.

See `reports/native_diagnostic_raw_lifetime.json` and the immutable capture
referenced there for source, build, module and live-byte provenance.

Immutable capture: `local/checkpoints/a3e3a4fb/raw-diagnostic-lifetime/validation.json`, SHA-256
`e7cbcab7ca8e13a51e069f056811951dea8be4a6e991744715a6e0f3cc74bdca`. It retains 4554 artifacts,
8 physical Win32 modules and 119 linked root source providers.

## Follow-up packet

Connect complete `B2AEB0` device startup and focused `B2ABD0` Reset to the
constructed renderer/effect fixture using one raw diagnostic publication in
both Lock and shutdown contexts. Reuse the graphics pool lifetimes already
committed on main as `955ebfe9`; preserve the same allocator-list domain.
The full startup/Reset capture remains outstanding.
