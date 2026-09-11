# XLive startup service composition

Addresses: 00A40DF0, 00A4C250, 00A4C030, 00A4BDE0, 00A3EAE0, 00735510, 00735520, 00737D60.

`xlive_startup_services` connects already reconstructed manager, IPC and callback
bodies to `XLiveStartupAdapter`. It does not add recovered native functions or
claim a binary-compatible interface. Evidence remains in XLIVE_MANAGER_OWNER.md,
XLIVE_IPC.md, XLIVE_APPLICATION_CALLBACKS.md and XLIVE_STARTUP_ADAPTER.md.

`RecoveredXLiveManagerOwnerServices` invokes actual IPC creation/destruction with
the supplied pipe/system hosts. The application's required noexcept deleter owns
the projected manager allocation. It must not free borrowed context/storage.
The native IPC shutdown deadline still does not prove that its worker exited.

`BoundXLiveApplicationGlobals` reloads the same volatile F8ABE8 owner slot used by
registration and the platform pump, and returns that owner's canonical online
and sign-in storage. Its profile resolver is called separately for every current
game access. Missing publication is an explicit typed-host error.

`RecoveredXLiveStartupServices` completes all four abstract startup methods:
current renderer device, current present parameters, the retained +3AC IPC slot,
and reconstructed callback dispatch. Renderer accessors run independently at the
native call sites. They must expose the actual device and mutable parameters at
stable addresses for the SDK, without a copied parameter block. The complete
owner constructor already calls A4C250 directly through the same owner host;
there is no second IPC initialization inserted into that constructor.

The executable host is not wired here. `GameDeviceHost` currently exposes its
parameters by const reference; the application integrator must provide a proper
mutable-storage accessor before binding the SDK, and must supply the current game
profile and allocation owner. Pipe protocol/framing composition is still in
progress. Construction performs no SDK initialization, endpoint opening or thread
launch. Real startup has those effects only when explicitly invoked by the host.

Validation: MSVC Win32 C++17 build with warnings as errors and both existing
CTest checks passed. This establishes compilation and existing-check coverage;
it does not establish runtime startup or gameplay behavior.
