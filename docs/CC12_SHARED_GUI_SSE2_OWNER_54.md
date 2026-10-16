# Shared GUI SSE2 Source capability owner54

The existing GUI conversion capability now belongs to `GameStartupHost`, so
the frontend and future native Text can borrow the same bool through frontend
retirement, font retirement and shared singleton drain. This is typed Source
ownership/ref propagation. It does not recover or install the Original CRT
selector at `0109EEA4`, change conversion bodies, or impose a locale policy.

`src/game_hosts.cpp:1674` provides `gui_sse2_conversion() const noexcept`.
Its first actual request initializes a private `std::optional<bool>` with the
unchanged expression
`IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE`.
Subsequent requests return the SAME contained bool by `const bool&`; the
presence test distinguishes an initialized false value from no value. There
is no exposed uninitialized false, reset, second queried GUI value or fallback
constant. The lifted capability performs zero queries when unused and one on
first use. Existing independent input/decal CPU queries are outside this
packet and remain unchanged; this is not a claim of one query for the entire
process or of identical allocation-fault-time behavior.

The only tracked `GameFrontendHost` constructor caller is
`src/game_hosts.cpp:2654`. It passes `gui_sse2_conversion()` directly as the
new required reference argument. The public frontend constructor and its
`Impl` constructor forward that reference unchanged
(`include/bsp/game_hosts_frontend.hpp:158`, `src/game_hosts_frontend.cpp:642,212`).
The old owned/default-initialized bool and query were removed from the
frontend; its field is now `const bool&` (`:162`). There is no default argument
that could create another capability or extend a temporary's lifetime.

Existing readers are unchanged: sprite property conversion (`:288`), the
layout host's `crt_sse2_conversion()` reference getter (`:366`), and further
page/property loading (`:433`). The owner/ref/constructor chain establishes
that they identify the contained bool returned by the startup getter. No
layout path or frontend logical release/order changed.

The optional is the **first declared data member** of `GameStartupHost`
(`include/bsp/game_hosts.hpp:699`), preceding every potential borrowing host
member. It therefore survives their reverse-order member destruction as well
as the explicit destructor body. The reference accessor contract is exposed
at `:681`. The current owner object is not relocated by this change.

Lifetime evidence in `src/game_hosts.cpp`:

| Route | Existing release order, unchanged |
|---|---|
| Startup destructor `:1855` | Delete frontend at `:1882`, retire native fonts at `:1884`, drain native renderer/raw singleton manager at `:1889`. The startup capability remains alive throughout. |
| Normal `application_shutdown :2833` | Delete frontend at `:2924`, then retire native fonts at `:2926`. Startup object and capability still exist. |
| `destroy_singleton_lifetime_manager :2940` | Retire native fonts at `:2951`, then drain at `:2953`. Capability remains owned by the same live startup host. |
| Later member destruction | Borrowing members declared after the optional are destroyed first. Only afterward does the optional end its contained bool's lifetime. |

Future native Text should initialize its existing reference field directly
from `startup.gui_sse2_conversion()`, retaining that reference rather than
copying a bool into a separate owner. Its factory, pending operations and any
retained users must retire before `GameStartupHost` finishes destruction. This
common lifetime is longer than the frontend's old lifetime; no font/text host
files are changed here. The capability getter uses the existing externally
serialized GUI/startup lifetime. It has no lock/atomic protocol and makes no
new thread-safe or process-global ownership claim.

Validation: the canonical MSVC Win32 build and existing checks are recorded in
the JSON report. Focused review covered the whole changed getter/constructors,
the sole constructor caller, all current frontend conversion readers, and
normal/exceptional drain order. No new tests or executable probes were added
for this reference-only ownership change. No CMake, `game_main`, native Text,
font-host, Ghidra, ledger, game/audio/settings or installation edits were made.
Build success and source lifetime evidence do not establish native Text
activation, Original CRT state/ABI parity, rendering or gameplay.
