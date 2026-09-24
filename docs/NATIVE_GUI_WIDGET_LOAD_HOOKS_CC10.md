# Raw base widget load hooks

| Entry | Complete bytes | Native behavior |
| --- | --- | --- |
| A9AC00 | 1, RET at A9AC00 | Base +74 post-construction hook returns |
| AA6A30 | 13, RET4 at AA6A3A | Copy low argument byte unchanged to widget+85 |
| AA7170 | 20, final JMP at AA717F | Current virtual+60(0), then tail AA70E0 |

These raw providers replace no logical objects and add no owner metadata.
The first two helpers retain their literal x86 leaf instructions. The active
setter preserves noncanonical byte values; it does not normalize a bool.
The single RET is a proven no-op only for A9AC00 and cannot stand in for an
unknown derived post-construction hook.

AA7170 captures the widget identity, reads its current profile and +60 target,
then calls that exact target with zero. AA6A30 selects the real raw setter;
all other targets require a genuine supplied implementation. Only after normal
callback return does the existing complete raw AA70E0 read the same widget's
current fields. Its model/gate/scratch/half-constant requirements still apply.
There is no dispatch fallback, synthetic scene field or logical owner cast.

All 34 bytes match live Ghidra and the installed PE. The indirect +60 call and
direct tail transfer were checked at their exact instruction addresses. Strict
MSVC Win32 Release and three existing CTests passed. No new tests were added;
AA70E0's earlier native/source comparisons remain separate callee evidence.
These wrappers have static and build evidence only. Explicit source bindings
and scratch change the outer interface; native ABI, escaping callbacks,
application wiring and game behavior are not claimed. Names are descriptive
hypotheses, and existing Ghidra comments are retained.
