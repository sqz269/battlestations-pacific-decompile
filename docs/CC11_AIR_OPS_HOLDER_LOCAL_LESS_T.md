# Complete ordinary holder local-position helper

`native_air_ops_holder_local_less_t_006bcc90` reconstructs all 73 bytes and 22 instructions of `006BCC90..006BCCD9`, with an exclusive end. Native ECX is the actual holder; destination and source points are stacked, EAX returns the destination, and RET8 removes the two arguments. The Win32 Source fastcall declaration adds an unused EDX argument.

The body calls the existing complete raw `transform_native_point_004142e0` on the actual matrix at holder+48h, producing three local scratch floats. It then uses ordered x87 FLD/FSUB/FSTP operations against fresh holder+A4/A8/AC. It stages all source coordinates before writing output, preserves ambient precision/rounding, and retains sequential output alias effects. Copying all three translation words before output, using SSE subtraction, or projecting a semantic holder would change the contract.

This is a natural prerequisite for `009AFAF0` and the native holder/touchdown callers. Existing semantic `GameUnitsHost` landing geometry remains independently qualified; this packet installs no translated holder, pose cache or lifetime binding. Whole approach geometry, actual holder creation, native faults/EH, executable ABI integration and gameplay remain unverified. Probe and build receipts are recorded in `reports/cc11_air_ops_holder_local_less_t.json`.
