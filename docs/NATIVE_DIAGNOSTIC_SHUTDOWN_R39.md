# Native diagnostic shutdown R39

## Result

`destroy_native_diagnostic_sink_007363b0` reconstructs the complete 164-byte
diagnostic shutdown schedule as a typed source entry. It borrows the same
`0109CF14` publication cell and `SoundLifetimeAccess` used by the getter. It
does not own either cell and does not create a manager.

An initial null publication returns without resolving the lifetime manager. A
nonnull publication resolves the first manager, captures its `+10` critical
section, enters it, updates physical depth `+18`, and arms ordinary source EH.
The body rechecks the publication, resolves a second manager, then captures the
current publication for genuine `00BCFCA0` unregister. It reloads `0109CF14`
after unregister, dispatches current slot zero with flags 1, clears the cell
again, and releases the first captured section.

## Current-profile dispatch

The native instruction at `00736429` calls slot zero from the current profile.
Source cannot invoke integer vtable addresses. The finite dispatcher uses only
complete existing providers:

- `00CE752C[0] -> 004BBCA0`, the diagnostic scalar deleter. It clears
  `0109CF14`, writes `00CE3818`, and frees for flags 1.
- `00CE3818[0] -> 00412440`, the canonical singleton-base scalar deleter. It
  writes `00CE3818` and frees for flags 1. The wrapper performs the native
  redundant publication clear afterward.

Any other nonnull current profile raises `std::logic_error` at the original
indirect-call boundary. A completed unregister stays completed, the publication
remains available for explicit recovery, and the captured source guard releases
the section. There is no guessed function pointer, executable-vtable call, or
no-op terminal.

## Original evidence

Read-only Ghidra used `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Fresh live bytes matched the installed executable:

- `007363B0-00736453`: 164 bytes, 44 instructions, SHA-256
  `fb860e3c2072743125d42daf505a685ae23ae71d528a93edc2c3cc689c327ad5`.
- `004BBCA0-004BBCC8`: 41 bytes, SHA-256
  `f462bb0027b9bfdceed4ade23ced0707acf58aa56d643492b1984d21f4da9c58`.
- `00412440-0041245E`: 31 bytes, SHA-256
  `61d4c09ccc46b0bf6c6f7f08a45e4e5e1d652ccba884505951000da209a35c1e`.
- `00BCFCA0-00BCFD10`: 113 bytes, SHA-256
  `e3fc97e499913fd8fac8f1ebb0804b4ced06333e49c628b44ba06316e5cc6463`.

The six shutdown call instructions are first manager `007363D2`, section enter
`007363EB`, second manager `00736406`, unregister `00736414`, current virtual
slot zero `00736429`, and section leave `0073643E`. The only original caller is
application shutdown at `0073830F`.

## Validation

The final baseline includes published R38 raw access, the frame-clock tail, and
the canonical host `0109CF14` cell plus `CE752C` drain admission. Strict MSVC
Win32 `/MD`, `verify-seeds`, and all
three existing CTests passed. COFF inspection confirms x86, `MSVCRT`, the new
typed symbol, ordinary EH data, and genuine references to the existing
unregister, diagnostic scalar, and base scalar providers. The focused ignored
fixture has an embedded manifest.

That fixture used the real raw manager, its real critical section and vector,
the real getter and both real scalar deleters. It established:

- initial null returns without constructing a manager;
- a warm derived owner is unregistered, scalar-deleted with flags 1, cleared,
  and leaves the section at zero depth;
- the canonical base profile uses `00412440` and completes the same wrapper
  schedule;
- aliasing the publication view to the genuine manager slot causes unregister
  to null the current view, proving the following reload skips scalar dispatch;
- an unsupported current profile throws after the real unregister, retains the
  publication, and releases the captured section.
- the published `GameSingletonHost` cell supports explicit R39 shutdown before
  an empty host drain, while a separate host safely retires an untouched
  `CE752C` owner through its admitted raw-drain fallback.

The alias case is a focused mutation observation, not a production ownership
shape. The fixture directly retires retained owners after the reload and error
cases, then destroys an empty genuine manager.

The host cases link the fresh production `game_hosts_singletons.obj` and
`game_observer_runtime.obj`. The ignored fixture retains the exact needed
`GameHostLog` leaves from `src/game_hosts.cpp` so it does not pull the unrelated
application graph. It substitutes no manager, deletion dispatcher, or terminal.

## Boundary

This packet does not change `GameSingletonHost`, shared deletion bindings,
`game_main`, or application shutdown. It does not bind the original
`0073830F` step because application host ownership and step-19 ordering require
their later audit. Published R38 phase 2 owns the stable `0109CF14` host cell and
manager drain admission; this packet only consumes them in ignored validation.

The C++ entry is not the original no-argument ABI. Ordinary C++ EH models the
established guard state, but original FH3/SEH identity, hardware faults,
arbitrary executable profiles, application execution, game shutdown, renderer
behavior, and gameplay remain unproved.
