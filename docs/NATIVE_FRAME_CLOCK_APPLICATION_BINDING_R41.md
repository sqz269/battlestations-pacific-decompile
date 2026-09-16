# Raw application frame clock

R41 replaces `GameStartupHost`'s embedded semantic `FrameClock` with one raw
80h owner constructed by the existing BEDFB0 provider at the phase-0 null gate.
The host owns the same private AB0 publication cell for construction, sound,
input, frame dispatch and deletion. There is no additional advancing clock.

## Persistent ownership

Private `ClockServices` borrows the original 44-byte D68D50 profile from the
existing verified numeric read-only data service and validates all eleven words
before allocation. That service already maps the D60000 band and must outlive
the host. The methods, publication and lifecycle contexts persist together;
the lifecycle borrows `GameSingletonHost`'s actual AA0 cell. Its pointer occupies
the existing deletion binding at +108; the 116-byte table is unchanged.

The source state is unattempted -> constructing -> constructed -> drained.
Contexts and the deletion binding exist before the raw allocation. The native
constructor publishes and registers; the caller adds no AB0 store. Repeated
local ensure calls admit the current constructed publication and skip allocation.
This does not establish whole-application reinitialization support.

Sound and input receive the R39 borrowed publication bindings. `GameFrameHost`
advances through slot08 and copies only the immediate slot1C 16-byte result.
The fixed-rate command-line gate dispatches signed milliseconds 50 through
slot24 immediately after settings loading and before sound construction. This
establishes that local order; the earlier native renderer phase is still partial.
Captured-owner consumers and optional unbound renderer/online consumers are not
universally converted into publication-reloading consumers.

## Failure and shutdown

An allocator exception marks failed without calling free. If the allocation
returned and its constructor throws, the catch marks failed and frees exactly
the captured allocation, corresponding to the caller's C86A3E cleanup action.
It never reads, clears or repairs AB0, whose bits can be stale. An unexpected
nonnull publication before construction also marks failed. A defensive null
allocation skips construction and publication; consumers reject the uncompleted
state. Profile/context prevalidation failure precedes publication and does not
manufacture a graph cleanup obligation.

The destructor, application shutdown, singleton-manager destruction and
`exit_process` first terminate failed/constructing state with `std::_Exit(1)`.
This explicit source policy retains an unproved failure graph and avoids manager
or CRT cleanup; it is not a reconstruction of native FH3 termination behavior.
Normal drain keeps all borrowed contexts and cells alive through
`GameSingletonHost` destruction, then releases `ClockServices`. State becomes
drained only after successful shutdown. The prior VFS failure guard and
diagnostic-sink startup/shutdown paths remain in place.

## Evidence boundary

See `reports/native_frame_clock_application_binding_r41.json` for original byte,
call and EH receipts, build results, the focused genuine-host fixture and its
immutable archive. Host member layout and the context-bearing frame constructor
are source interfaces, not the original application ABI. R35/R36/R38/R39 retain
the arithmetic, lifecycle and provider evidence. R41 does not prove full startup,
all exception edges, original register/FH3 ABI, active renderer/worker behavior,
or gameplay parity.
