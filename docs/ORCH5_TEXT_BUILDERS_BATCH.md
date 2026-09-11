# Text builders and actual content integration

Addresses: 00ABA8D0,00AB9FD0,00ABA270,004768D0,00475F80,00AB8CE0,
00B19210,00B189F0,00B18AA0,00B18B00,00B18B20,00B73260,00B6DAB0,
00AB8EE0,00AB8250,00AB9650,00AA9730,00AB7700,00AB6AA0,00AB87D0,
00AA7E00,00B6D800,00AA6800.

Seven new sources are registered in the normal bsp_core Win32 build. They
provide actual Text material operations, single-line/wrapped geometry, all
initialized native position-decoding cases, bounded canonical child deletion,
Text type-dispatch leaves and the composed content entry. Existing content and
lifetime modules are extended without a second widget/resource owner hierarchy.

`GUI_TEXT_RUNTIME_CONTENT.md` describes the ordinary content path and explicit
pending boundaries. Single-line review corrected three native pointer arguments
that alias one float3, with stable storage separate from the pen accumulator.
Wrapped finalization uses the actual position decoder, including the verified
borrowed d3dx9_40 half-conversion import. Undefined native cases remain pending.

The AA9730 body was truncated after a false no-return free call. Local flow
repair alone did not extend its stored body; official function recreation then
restored AA9730..AA99B8 (649 bytes). Final listing has 204 instructions and 0 gaps.
Before/after documentation archives preserve its name, signature, calling
convention, parameters, plate/instruction comments, 27 old labels and 12 local
variable descriptions. Two new labels arise from the newly included tail.
No global no-return flag changed. Both raw name/loaded thunks are now defined.

The combined normal Release MSVC Win32 build passed with both existing tests.
Across eight reports, 2152 numeric call rows have zero failures. Numeric
indirect targets and symbolic indirect rows are counted separately in the JSON;
they rely on the documented profile/IAT evidence, not direct-CALL verification.
No new tests were added. These checks establish compilation and existing math
regression coverage, not execution of the new Text operations or gameplay.

Remaining work includes the AB98F0 optional-child factory/callee continuation,
nonzero native timed-entry ownership, unsupported child deletion, complete Text
property/current70/factory dispatch, original pool/string/SEH ABI and runtime
validation. Workers are continuing independent prerequisite packets.

Integration commit `6825816e` merged current main `8be28943` and passed the
combined normal build and both existing tests. The official annotation tool
saved 23 reviewed entries; prior values are archived in
`reports/orch5_text_builders_annotations.json`. Affected exports and the shared
snapshot/index were refreshed. The two raw thunks and complete base destructor
were exported again after their final annotations.
