# Raw structured-node remaining predicate

`native_structured_node_has_remaining_00715bf0` reconstructs the complete
21-byte body at 00715BF0..00715C04. ECX points to a valid wrapper whose first
DWORD is the native node pointer. A null contained node returns false;
otherwise the result is whether node+20h is nonzero. The native EAX result
is exactly 0 or 1 and both paths end in plain RET. The C++ boolean interface
is separate from this native register ABI.

The listing has no attachment, stream, sign, bounds or wrapper-null check.
The raw source keeps the two ordered DWORD loads and conditional second load.
It neither reads a typed `StructuredNode` nor changes ownership or remaining
budget. In particular, a nonzero high-bit remaining value is true. The wrapper
and any nonnull node must designate the actual accessible Win32 storage.

The [report](../reports/native_structured_node_predicate.json) records the
complete installed-PE/live byte comparison and original ABI. Ghidra already
names the body `BSP_StructuredNode_HasRemaining`; that descriptive hypothesis
is retained. Its decompiler prototype omits the ECX input, so the source contract
uses the actual listing. No Ghidra mutation occurred in this packet.

This supplies one leaf needed by the raw hierarchy producer. Allocation,
stream reads, detach and intrusive release remain separate missing contracts
identified in the producer-frontier BG report. This packet adds no test or
probe and makes no native ABI, full producer, exception or gameplay claim.
Build and existing-test results are recorded in the report after verification.
