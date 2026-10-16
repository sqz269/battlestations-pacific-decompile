# GOAL15R traffic copy/fill integration

Four complete Source helpers close 171 Native bytes and 71 instructions:
`00486800` fill (30 bytes), `00486820` backward copy (47), `0048D660`
forward copy (48), and `0048E4A0` fill N (46). Each consumes three real stack
words and no incoming ECX/EDX receiver. Plain RET versus RET 0Ch is exposed
through cdecl versus stdcall Source interfaces. Physical EAX results are
recorded without claiming recovered Original C++ declarations or classes.

The full literal instructions retain argument-cell capture, borrowed-value
rereads, DWORD wrap, register saves, fault order and flags. Backward copy
uses a signed-positive count; forward copy preserves SAR flags across LEAs
and runs for every nonzero count. Fill uses exact endpoint equality; fill N
uses unsigned DWORD zero/nonzero, including high-bit counts. Neither fill
reloads an argument pointer after an overlapping store changes that cell.

Both copy operations use genuine selected SDK `memmove_s`. The whole public
Source bodies match all 163 fixed Native bytes, with only two actual CALL
operands (eight bytes) rebound. Every executable section is accounted:
strict COFF 263 bytes, registered COFF 265, including 94 registered bytes of
real SDK implementation. Actual linked code, import/CALL bindings, symbol
offsets and whole registered archive-member identity are checked.

Worker pin `f6fe9567f76b57eacddb076502437a60029d3260` received independent
max review. The primary built Source integration `8e9947377`; all three
existing checks passed. The archive-only fixture passed for actual overlapping
copies and both fills overwriting their actual outgoing third-argument pointer
cell, with captured-pointer behavior, physical EAX and stack cleanup checked
for public Source and the entire captured Native171 bodies. No new broad
test suite was added. Worker setup-through-handoff time was 1778 seconds;
review and primary integration are additional throughput costs.

Original names, prototypes and comments were preserved. The saved project
retains its four STL names and verified physical three-argument prototypes;
exports, snapshot and index were refreshed. Root evidence is in
`reports/goal15r_reconstruction_batch.json`; detailed worker evidence remains
in `reports/goal15r_traffic_dword_copy_fill.json` and its corresponding doc.

Selected current SDK errno, invalid-parameter, copying and EH behavior is a
qualified service boundary. Private Native CRT identity, code placement,
Original caller adoption and game behavior remain unproved. Allocator
`0047E970`, length failure `00490240`, and whole insertion/resize/traffic,
layout and world parents remain open.
