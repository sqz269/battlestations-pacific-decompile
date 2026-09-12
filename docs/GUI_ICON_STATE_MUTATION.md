# Retained Icon state textures and native sizing

Addresses: 00AB2690, 00AB27A0, 00AB17B0. Names are hypotheses. These are
new C++ interfaces over the existing `GuiIconRuntime::Impl`, not native ABI
replacements. Every live analysis batch verified `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`; Ghidra remained read-only.

| Routine | Coverage | Native inputs / return | Final instruction |
| --- | --- | --- | --- |
| 00AB2690 | complete supported retained Icon projection | ECX Icon; DWORD state, texture, UV pointer | 00AB2791 RET0C |
| 00AB27A0 | complete supported retained Icon projection | ECX Icon; output pointer, DWORD state; EAX same output | 00AB27E4 RET8 |
| 00AB17B0 | complete supported texture callback projection | ECX output, EDX texture; stack UV pointer; EAX output | 00AB1852 RET4 |

Live body ends are00AB2793,00AB27E6,00AB1854. The report records every
native call and all direct callers. Mutator callers use state0,2,3 or a
forwarded DWORD; both state-size callers use0. The menu calls are00596DDD
and00596DF4 inside005966F0. Existing `GuiIconState` agrees with the
00AB66C0 producer; there is no replacement record layout.

`set_state_texture_00ab2690` checks the full unsigned index, writes the new
pointer, retains it, then releases exactly that state's old ownership token.
Equal pointers skip ownership changes; null is permitted. Duplicate states
retain independent tokens. Ownership slots are published before old-release
callbacks. UVs remain borrowed until after release, when all four DWORDs are
captured before any destination write. Authored V/U flips use ordered
UCOMISS comparisons and the native asymmetric FLD/FSTP versus MOVSS swaps.
Only equality with the sign-extended active state reaches the existing
retained geometry rebuild. Constructor vtable00D5C4C0 has current80 at
00D5C540, containing00AB3CB0; this path creates no substitute renderer.

`state_texture_size_00ab27a0` captures the texture and borrows its state UV.
Width precedes height. Each unsigned dimension uses x87 FILD, conditional
2^32 correction, division by double960/720 and a float spill. UV differences
spill before the sign mask; width is written before vertical UV reads,
including overlapping output. Optimized MSVC assembly was inspected.

Callbacks preserve runtime, record and captured-resource lifetime. Retain
and release cannot throw; logical lookup is observational. Property reload
rejects borrowed-record invalidation. Native SEH/OOM, malformed pointers and
other derived vtables are excluded; failed resource lookup may expose partial
state and requires discarding the page.

Validation: Win32 Release build, both existing CTests and8/8 seed checks
passed. A manifested local fixture used actual D3D9 textures/cache and an
evaluated Lua5.1 Icon, covering duplicate/equal/null references, release-time
UV mutation, reload guard, captured texture across callbacks, full DWORD
range rejection and output aliasing. Geometry/base hooks throw if reached:
active rebuild, native differential and in-game rendering remain untested.
No permanent tests were added. Rerun `local/run_gui_icon_state_probe.ps1`;
logs are `local/gui_icon_state_{probe,build}.log`. The report verifier passed
38 call rows; indirect dispatches retain their explicit evidence boundary.
