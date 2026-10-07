# Raw x87 wrapped-angle addition

`native_angle_add_00438aa0` recovers the complete ordinary 110-byte `00438AA0..00438B0E` body through its ST0 result and RET8. Source adds a pure constant-alias context in ECX and uses EDX to form addresses. The original two float arguments remain stacked. This interface preserves the native arithmetic schedule rather than assuming that a typed host float result establishes its raw x87 contract.

The initial sum is spilled to binary32. Comparisons then use the actual double cells CE3D18, CE3D28 and CE3828. The lower and upper loops each retain the native double correction, stack exchanges and binary32 spill/reload for every iteration. There is no finite-input guard or iteration limit. Context metadata captures only addresses; the original load points observe the actual cells. Invalid storage, nonterminating inputs, unmasked exceptions and stack exhaustion remain outside the component comparison.

The existing typed `wrapped_angle_add_00438aa0` in `unit_rudder.cpp` remains available with its documented host boundaries. This new raw provider is a prerequisite for the holder path helper and other instruction-level callers. It does not activate the unrecovered path helper, whole geometry, native executable binding or gameplay. Evidence and independent native-body comparison receipts are in `reports/cc11_native_angle_add.json`.
