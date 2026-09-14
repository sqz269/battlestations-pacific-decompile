# Resource hierarchy parser integration BV

Addresses: B7EB90, B7F100, B87AE0, B7D160, B7D220; EH support CC1FF0,
CC1FF8, CC2000, CC2010, CC2018.

This batch connects raw hierarchy parsing to the existing actual pool, BT
traversal, BU values, string storage and reference/pointer-array services.
Five complete ordinary bodies total1,245 bytes. Ordered field overwrites,
duplicate index appends, current-resource selection and child/name-only
exception cleanup follow the listing and FH3 maps. Two handlers are defined
and saved; no no-return or function-flow mutation is required.

Strict compilation and one actual-service/original-math probe passed. It checks
hierarchy fields/defaults, unknown skipping, repeated append, current manager
publication, one source failure and explicit teardown. The two sphere-to-box
bodies pass576 paired cases. These checks do not prove whole native parser
execution, native FH3/SEH or gameplay. See `NATIVE_RESOURCE_HIERARCHY_PARSER_BV.md`
for the detailed contract and evidence limits.

The integration report records the exact final code revision, source manifest,
combined build and final-library probe. Raw root/item dispatch, manager/factory
and parser owners, B80720 loading and queue shutdown remain open.
