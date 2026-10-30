# Full raw locale manager retirement

This batch registers the raw 73C9F0 destructor and 73CAF0 scalar wrapper from
2bab249d1. Primary reviewed the complete CPP/HPP/docs against both full native
listings; independent review checked the packet's own selected EH and artifacts.
No application caller or canonical F8BC4C lifetime/profile wiring is added.

The destructor frees and clears B/A sidecars in native order, captures language
data/size before its real string-pool getter/return, then advances selected
cleanup state before each normal map, range and base call. It does not retry an
escaped stage. The names begin read stays under state1; state0 precedes end/range
work, and current begin is reloaded after range return before freeing. The final
base call runs after state-1. Selected unwind actions are map, names and base;
no sidecar/language rollback or invented cleanup is added.

The scalar interface borrows an actual caller-owned volatile DWORD flag object.
It reads only the current low byte after the complete destructor returns, frees
the captured receiver when bit0 is set, and returns address bits without any
post-free receiver read. This normalized interface adds explicit context/flags
references and does not reproduce the original stack/register ABI.

The real aa56 base, f411 cleanup pair,550 map clear, raw string-range/pool and
matched current heap providers are coupled. Actual allocation, alias, cell,
section/pool and failure-custody premises remain caller obligations. Current
ordinary C++ cleanup/fail-stop behavior is distinct from unrestricted Native
FH3/SEH/hardware-fault/private-spill compatibility.

Strict worker object compilation and the common MSVC Win32 Release build pass;
both existing checks pass in 14.24 seconds. No tests/probes were added and no
new body/game was executed for qualification. Ghidra comments were preserved,
updated and saved; affected exports were refreshed.

Canonical locale owner/profile activation, populated slab/slot provenance, raw
loading and actual startup remain open. All nine raw Text choices remain FALSE;
frozen ac60 stays on its Font48/Viper19 hold. No loaded-locale, runtime, visual,
full13/USN21/Page/P4, Original ABI, production or new-native-body credit follows.

Exact Source/build/peer evidence: reports/codex_full_raw_locale_retirement_integration.json.
