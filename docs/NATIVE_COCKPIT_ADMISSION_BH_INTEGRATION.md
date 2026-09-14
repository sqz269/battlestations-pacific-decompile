# Validated cockpit host admission and viewport identity

Addresses: 00B3C800, 00CD7F00, 00B1F8F0

Scene and generated-model lifetime registries now reserve protected binding credits. Camera owner/reference overloads consume these credits at their existing binding points without native retains or host allocation. A caller-owned viewport registry provides stable actual-owner views and retires identities at the common destructor before lifetime end or free. Retired host views persist until explicit quiescence.

Exact combined source `9cb8a3e1b3093187bb9151270567d0b16ef7dbff` includes the current main base and passed strict MSVC Win32 compilation, both existing CTests, eight native seed comparisons and three current-library fixtures. All 2466 tracked build inputs, twelve source/header files and three libraries stayed unchanged through verification. The source, libraries, probes, recipes, logs and hashes are preserved in the archive pinned by `reports/native_cockpit_admission_bh_validation.json`. No repository tests were added.

The registry fixture consumed reserved credits after intervening ordinary registrations with zero forbidden host allocations. The viewport fixture covered flags0 retirement, stable inert views, same-address reuse, nonterminal and terminal releases, and failed-construction cancellation. These two fixtures establish host behavior only.

The existing native/source CW027F camera-helper pair now uses both camera credits and actual viewport registrations. Placement construction of prepared camera companions attempted zero guarded host allocations; the replacement viewport used the admitted allocation overload. Both viewport identities retired on their actual native terminal paths. Fifteen camera checkpoints, 16740 camera bytes per path, eleven ordered events and 36 normalized helper bytes agreed; 8880 mapped original code bytes remained unchanged. The first viewport is manually registered after successful B71A80 publication in this fixture, so production admission before that publication remains open. The helper begins with constructed storage and zero +08/+18 fields.

The B3C800 construction contract is now named and saved in Ghidra with its original thiscall signature. CD7F00 now has the verified int cdecl return signature; its model-pool implementation already existed. These are readiness/evidence corrections and add zero reconstructed native bodies.

Follow-up packets: add explicit first-viewport admission to the camera constructor without an ambient environment slot; implement a persistent cockpit construction block with two viewport records and prepared camera companions; then implement B3C800 using current-helper camera reloads and the reviewed native normal/EH ordering. Published native survivors must keep their records alive until actual retirement and host quiescence. Native FH3, original binary replacement ABI, full startup invocation and gameplay remain unvalidated.
