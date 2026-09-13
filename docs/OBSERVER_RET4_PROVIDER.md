# Exact observer RET4 provider

Addresses: 0042B120.

The complete function is three bytes `C2 04 00`: return while discarding one
stack DWORD, with no argument/register reads and no defined return value.
`native_observer_noop_0042b120()` provides that behavior through a parameterless
C++ interface. It does not infer the ignored argument's type or a native ECX
receiver, and is not a binary replacement.

All 20 previously recovered unit callback tables that select this function
at slot08 were refreshed through verified Ghidra CLI reads and matched the
installed PE. The complete function bytes also match. Constructor/leaf mapping
provenance remains in `reports/native_unit_observer_callbacks.json`; the exact
refreshed cells are in `reports/observer_ret4_provider.json`.

This provider is selected only by its actual captured native target. The two
other byte-identical providers0080DFC0 and00952050 retain distinct identities;
unknown callbacks do not default to any no-op.

Combined V build and the actual unit-lifecycle fixture will record invocation
separately from this complete native-byte proof.
