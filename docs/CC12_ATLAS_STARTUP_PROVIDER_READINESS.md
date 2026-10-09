# Atlas startup provider boundary and callback order

The ordinary Source atlas loader now queries the current registry for each
matching candidate. One focused case reproduces the stale snapshot: the first
load callback registers the requested name; a second matching candidate must
then be skipped. The pre-fix normal build compiled but failed that case. The
corrected normal Win32 build passed all three existing checks.

At AF02F0, Native code checks the current manager count/base after each matching
candidate and compares against the requested name. The older assertion that
this comparison can never fire depended on an unproved texture-domain invariant.
Its old annotation is preserved, followed by the new qualification.

The fresh AF0060 physical span is 935 bytes and 304 instruction starts; typed
function metadata reports 289. Exact function body AddressSet is unavailable.
The selected ordering is verified, but this discrepancy holds whole Native
body/listing admission. AA5E20 spans 1,311 bytes and 442 starts, agreeing with
its typed count. Fresh saved/live/PE gates and complete live-byte replay are
retained. Exports were refreshed before review without historical pre-refresh
copies; the review does not claim independent old-export provenance.

The logged AA5E60 address is inside AA5E20. Its actual AA5EA2 call uses the
captured current renderer's slot64 and then publishes manager+28. The current
frontend D3D sprite/VFS bridge is a separate object domain. Production menu
atlas loading still lacks the actual F8C26C manager and AEF280 logical-texture
registration composition. Those dispatch/ownership prerequisites remain open.

The std::string/vector host interface covers the tested per-candidate callback
order. It does not establish NativeString/raw array aliasing, concurrent
in-walk mutation, original locale/FH3/fault/ABI or gameplay equivalence.
See CC12_ATLAS_MESH_SOURCE_PRIMARY_REVIEW.md and the JSON receipt for exact pins.
