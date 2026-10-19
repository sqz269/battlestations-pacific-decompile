# Oct18b nB middle rows

The exact 64 dispatch rows from `00759090` through `007F22E0` are enumerated in
`reports/oct18b_nB_mid.json`. The batch supplies 61 source interfaces in the
existing message, MovCam, movie-camera, path, bot-task, engine, paratrooper,
player-record and plane-squadron families. Two compiler/reference-counted
copies are classified `library_instantiation_out_of_scope`. Dispatch byte sizes
are retained as `dispatch_body_bytes`; the current live listing/prototype is
the body authority where analysis has since shortened a dispatched extent.
The remaining geometry body is in bounded contract recovery with the low worker,
with no placeholder implementation.

The evidence is the saved Ghidra pseudocode and x86 listing from the existing
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, plus per-row
caller/vslot references and direct call sites in the report. C++ and descriptive
Ghidra names are hypotheses. Prior live names/comments are retained by the
annotation tool's before/after log; the report retains each dispatch name.

The original receiver is ECX for member entries; the report records each RET
cleanup. Important decompiler omissions are the ignored stack word of
`007B6D00` (RET4), the third word of `007A66D0` (RET0Ch), the ten words of
`007ED000` (RET28h), and the hidden output pointer of `007B7830` (RET4).
`00780800` is identity with RET4; `0078EAA0` is identity with RET. AL-only
predicates make no promise about the native upper EAX bits.

The eight path codecs preserve the extended header, +1Ch boolean, optional
32-bit index and raw float[3] order. Plane-create optional +12Ch is -1 when
absent and an unsigned 8-bit value when present. The path-create reader reads
the existing 12-byte-stride array without reading a count or resizing it.
`007B2780` resizes to i+1 only after an archive entry exists: an empty archive
does not clear an existing vector. The squadron proxies read member zero even
when the live count is zero unless the original function explicitly tests it.
Fan-outs re-read the count after callbacks. The pose refresh reads the borrowed
game clock after child invalidation, as the listing does.

`src/oct18b_nB_mid.cpp` is limited to seven exceptions. `007593B0` cannot be added
to `cc13_seg30.hpp` while `agent/oct18b-nA2:oct18b-nA2-families` owns it;
`007B9070` cannot be added to `cc13_seg33a.hpp/.cpp` while
`agent/oct18b-nA3:oct18b-nA3-family` owns them. Both retain explicit existing
dependency boundaries. The two descriptor identities and session byte/false
leaves have exact data references but no stronger recovered class name. A data
reference or a candidate table alone establishes neither callback/descriptor
provenance nor class ownership; those names remain provisional.

`0078CFB0` is recovered as an exact raw-storage holder constructor, with
ECX=destination, stack owner, EAX=destination and RET4. Null owner clears the
holder; a non-null owner supplies a valid non-null reference at +0Ch, copied and
incremented at held+4 through the required InterlockedIncrement boundary. Its
class and authored-vs-template provenance remain unknown. Its only D2CE88 scan
match sits among non-code DWORDs, so it supports neither class nor library proof.

`007C4330` and `007C4390` copy a reference-counted pointer and three floats under
compiler EH registration. Actual placement-copy adapters `007C94C0/007C94D0`
move stack destination/source into ECX/EDX and call these bodies. Positively
reviewed adjacent `STL_UninitCopy_007C4220` assigns the same normalized 16-byte
pointer-plus-three-float record. Those compiler/template connections support
the two library exclusions; EH and candidate tables alone do not. Both copies
take ECX=destination, EDX=source and RET without a defined value result. They
increment through IAT `00CE221C`; no reconstructed game body or credit is added.

`007AE930` belongs to the pointer-array path geometry family near the existing
`point3_ptr_array_nearest_point_to_line_007aeec0`. ECX is the array, origin and
direction are stack pointers, result is ST0, RET8. Its remaining work is the full
x87 spill/NaN order and the hidden register/stack arguments at geometry calls
`0046F440`, `0046F940`, `00470150`; the pseudocode omits them.

`007EFBA0` is now reconstructed in `plane_squadron_entity`: ECX squadron,
command string and nullable target on stack, AL result, RET8. The complete
259-instruction listing identifies moveto, cleartarget, land, settarget, strafe,
bomb, torpedo and dogfight. It retains the blocked +3B0h test, base predicate,
virtual +168h command support, every comparison and early-out in image order.
The land arm uses the existing `0060C5B0` class+198h equality predicate and the
`006BF050` boundary (target ECX, DL=1). Bomb/torpedo arms reuse the existing
`007ED7E0/007ED8D0` loops with loadout0. Side/target/leader fields are read at
their original points; all remaining native/vslot services are required named
callbacks. No gameplay routing is added.

Validation uses `scripts/build.ps1`, the existing direct-call report checker,
and diff inspection. No new tests or game runs are requested for this batch.
These are source interfaces, not binary ABI replacements. Required callbacks
remain explicit; no host routing is added. Float source models round at the
observed float spills, but std::atan2 and double expressions are not proof of
x87 last-bit, floating-control-word or signaling-NaN equivalence. Build, static
and call-site evidence do not establish fixture, ABI or gameplay validation.
