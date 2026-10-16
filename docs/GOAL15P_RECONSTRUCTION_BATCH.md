# GOAL15P tagged Source and machine evidence

Two more sweep packets add 24 complete C++ functions referencing 152 Native
body bytes and 65 instructions. Their actual registered objects contain 184
Source bytes and 69 instructions. Seventeen complete functions match all 80
Native bytes exactly; seven have complete normal-return Source semantics with
explicit remaining machine-state and fault-context differences. These are
separate evidence states. No Native-sized prefix, masked operand, hidden tail
or ignored helper is accepted as a whole-body match.

The B3 module contains twelve entries: nine exact bodies and three qualified
unsigned scaled-address computations. B4 contains twelve: eight exact bodies
and four qualified entries comprising one clear/return, two ordered triple-zero
stores and another scaled-address computation. There are no calls, unresolved
providers, globals, x87 instructions or branches in these 24 Native bodies.
The exact subset includes the byte return in AL and a full-EAX predicate;
Source result types preserve their observed widths.

For each scaled entry, independent affine interpretation of the actual entire
Native and registered Source instruction sequences proves EAX equals
word[0]+4*word[1]-4 modulo 2^32 for independent full-width word values. Both
read receiver+4 before receiver+0. This is symbolic instruction reasoning,
not sampled test cases or a re-run of the worker's verifier. Native leaves
ECX=word[0], preserves EDX and flags; Source retains the receiver in ECX,
puts word[1] in EDX and changes flags with ADD. Fault register contexts differ.
Backing must be readable and stable; no empty or range repair is added.

The clear writes a raw zero DWORD at +0x10 and returns the original address.
Native captures EAX before storing through it; Source stores through ECX and
captures EAX afterward. Both zero-three entries perform three separate DWORD
writes at 0,4,8 in that order and return zero. Native XORs EAX before all stores;
Source XORs it between the first and second stores. Valid-backed normal-return
effects agree; first-store fault register/flag contexts differ. No alignment,
typed object, wider store or fictional class is imposed by raw memcpy access.

Primary Source review and independent review cover both pinned packets.
The actual registered COFF proof accounts for every nonempty executable section,
each full section size and offset-zero public symbol, separately matching Native
bytes for the exact subset and full Source bytes for the qualified subset.
There are no relocations, masks or undefined externals. Fresh target-verified
Ghidra bodies and installed PE bytes supply the full Native comparison.
The registered MSVC Win32 Release build and all three configured checks passed.
No new tests, runtime fixtures or probes were added for these routine bodies.

Names describe observed operations, not recovered class or field identities.
Physical prototypes express the measured inputs and results; the Source C++
conventions and noexcept declarations are chosen interfaces. Old annotations
are retained before saving Ghidra names, comments and physical prototypes,
refreshing affected exports and taking a saved snapshot/index.

B3 inspected twenty new complete bodies, completed twelve and explicitly
deferred eight. Across its first four packets, 67 of 79 candidates were inspected,
41 Source entries covering 189 Native bytes completed, 26 deferred and twelve
left uninspected. B4 inspected/revisited nineteen bodies, completed twelve and
recorded all seven other dispositions. Qualified Source entries remain distinct
from exact machine copies. Deferrals, including the separate x87 getter and
absolute-global provider, remain work for later packets.

B3 qualification through handoff preparation took 1954.879 seconds; B4's full
handoff lower bound was 1798.54 seconds. These include failed code-generation
attempts, registered builds, scope steering, metadata and checks. Primary review,
annotation, builds and integration add further work. Different packets, Main
inputs and caches prevent a controlled throughput comparison. Tags and missing
ledger rows screen candidates; actual Source coverage and whole contracts
establish the work. The receipt keeps worker timing separate from primary proof.
Independent review found a B4 timestamp mismatch: the recorded report interval
is 1604.273599 seconds. The primary corrected that field and retained its old
1698.866493-second sample with its endpoint explicitly unrecorded. The later
1798.5428-second handoff remains separately supported.

No linked function-address identity/folding, production caller adoption,
installed binary replacement, complete Native binary ABI or gameplay is proved.
The seven qualified interfaces additionally leave explicit machine/fault behavior
gates open. The continuing goal and larger traffic/layout/world gates remain active.
