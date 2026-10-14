# Retained platform power-policy owner (CC12)

`PlatformPowerPolicyContext` now owns the saved and modified policy buffers
across the explicit `BED223` producer and `BEBF70` stop call. `GameWindowHost`
binds its old-window stop method to this context, replacing the neutral
unimplemented return. Its producer entry point is concrete and compiled; the
application startup call remains gated pending the genuine normal-exit restore
caller. No destructor or harness cleanup schedule was added.

## Native evidence and caller boundary

Read-only live batches verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Full saved constructor bodies `BECDA0`, `BE2AC0`
and `BE2960`, plus `BECDA0` assembly, do not initialize scheme `+4C` or either
policy range `+50..+DF` / `+E0..+16F`. The projected platform constructor's
`power_scheme = 0` is host normalization, not a recovered native store. A fresh
projected stop therefore cannot use it as an actual captured scheme.

The new owner deliberately leaves its two `POWER_POLICY` objects default
initialized, preserving their allocation bytes. Its constructor and destructor
make no OS calls. The explicit producer receives the platform object, rejects
rebinding to another receiver, and calls the existing exact
`initialize_platform_power_00bed223_fragment`. The producer sets the sentinel,
captures the real active scheme, ignores `ReadPwrScheme` failure, copies the
retained original policy, zeros the two video timeouts in the modified copy,
ignores activation failure, and disables the screensaver. It binds the receiver
even when scheme capture fails, because the native sentinel path is complete.

The stop method requires that same receiver to have completed the producer.
This is a source interface precondition outside the native body; invalid calls
throw before any API or state effect. Valid calls invoke complete `BEBF70`, using
the exact retained saved policy, then clear `+42` and post quit in native order,
including restoration failure. The owner is noncopyable so storage and receiver
identity cannot silently split.

The proved native caller is `BECF27`, gated by an existing HWND in `BECEE0`.
The producer belongs after actual renderer and render-entry cache initialization
at `BED223`. No normal-shutdown dispatch to slot `+8` is established by
`PLATFORM_STOP_SCHEDULE_CC10`; `BECE30`'s screensaver enable call restores no
policy. Consequently production startup activation still logs its explicit
unimplemented lifecycle gate. A future caller must establish that normal-exit
restore contract before invoking the compiled producer entry point.

## Validation

The ignored Win32 MSVC probe relocates all 37 original `BEBF70` bytes and the
84-byte `BED223..BED276` fragment from the installed PE. It preserves the
producer's EBP/EBX/ESI/EDI ABI and substitutes spies for every power, screensaver
and quit API. Three original/source scenarios compare API traces, captured
scheme, both 90h-byte policies and frame-clear-before-quit ordering: successful
read, failed scheme capture, and ignored read failure. Activation/restoration
failures are ignored in all scenarios. Placement construction over known bytes
proves no policy initialization; fresh and foreign receiver calls are rejected,
and destruction schedules no API. All three scenarios passed.

`reports/platform_power_policy_owner_cc12.json` records build/test results and
probe artifact hashes. This proves a projected retained owner and bounded
original/source behavior; it does not establish the native platform object ABI,
the missing normal-exit call, or whole-game lifecycle parity. The probe did not
mutate OS power policy, screensaver settings or the live game installation.
