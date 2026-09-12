# Executable singleton lifetime ownership

The rebuilt executable now owns one `GameSingletonHost` for the run. Its `SingletonLifetimeDomain`, gameplay-effect publication00F87664 and deleting context survive menu destruction and remain available at WinMain's final manager shutdown step008F8449. The menu borrows that context. This replaces the menu-local domain whose destruction and validation callbacks did nothing.

## Concrete ownership and dispatch

`GameStartupHost` creates the host context; the lifetime manager itself remains lazy through the existing00415350 source provider. `GameMenuHost` receives a reference and exposes its effect context to the004E4000 front-end-shell path. There is no second menu-owned lifetime domain or copied effect publication.

The only registration path currently admitted by this executable host is `get_gameplay_effect_manager_004c1650`. It registers a `GameplayEffectManager`, whose original profileD0DA64 has slot0=008703E0. The host binds that exact source owner type to completed `scalar_delete_gameplay_effect_manager_008703e0` with its live `GameplayEffectManagerContext`. Native profile identities stay data; they are not called as rebuilt C++ vtables. Before admitting any additional owner type to this host, its concrete deleting binding must be added.

The existing deleter invokes0086FE20, releases the weak map nodes/head without releasing definition payloads, unconditionally clears00F87664, installs the base profile, and frees owner storage when flags&1. The lifetime manager passes flag1. Validation now reaches actual `_invalid_parameter_noinfo`; source CRT handler ownership remains distinct from original encoded109DD64.

`application_shutdown` still releases the menu before the final manager step. `destroy_singleton_lifetime_manager` now drains the application-owned domain through its existing shutdown provider. The effect context and publication remain alive throughout callbacks. The host context destructor also drains before automatic destruction removes that context, covering fallback teardown; after normal shutdown the publication is null and this second check does nothing.

## Evidence and observed run

Fresh guarded Ghidra memory queries verified the existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, against the installed executable SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. The37-byte capture covers the36-byte008F8449..008F846C shutdown fragment plus the first instruction of the following mutex cleanup. The fragment retains manager publication through destruction/free and clears it afterward; D0DA64[0] resolves to008703E0, whose complete30-byte body calls0086FE20 and conditionally frees on the low flag bit.

The strict Win32 build and both existing CTests passed; all eight reference seeds matched. A copied, hash-verified executable ran with `--frames 180 --press-start-frame 30`, the installed data root and an isolated settings root under `local/game_singleton_runtime/`. It created the window/device, presented180 frames, entered the front-end shell, requested the effect-manager context once and exited0. The log records application shutdown, application destruction, final manager shutdown, then exactly one drain with `registered_slots=1 effect_publication=null manager_publication=null`, followed by mutex/thread cleanup. No new test or ad hoc probe was added.

The executed PE SHA256 is `347b9fe58a1cca32c9bace63fd33da20bccd786042f868e5c0a4fe381e0c884a`. Source snapshots, the five relevant actual compiler commands/objects,365 compiler read dependencies, linked artifacts and runtime logs are retained under `local/game_singleton_runtime/`. Source capture was after the build; it is not described as a prebuild snapshot. The [audit](../reports/game_singleton_runtime_ownership.json) links the retained evidence.

## Remaining reconstruction boundary

This is executable ownership integration through existing typed C++ manager/effect-container interfaces. It does not mark the general raw BD0400 destructor or all native singleton profiles reconstructed. The raw registry's B1B710 source, for example, still requires additional bindings that the original slot0 call does not supply. The full raw getter/registry getter and canonical raw owner-dispatch migration remain separate work.

The run verifies this title/menu shutdown path, not visual parity or full gameplay. Other application-singleton teardown remains explicitly unimplemented in the same runtime log. The overall runnable-game reconstruction goal is still incomplete.
