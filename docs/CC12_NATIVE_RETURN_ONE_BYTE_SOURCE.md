# Return-one byte Source candidate

`00876180..00876182` (`TRIV_body_00876180`) is exactly `B0 01 C3`:
`MOV AL,1; RET`. All three Native bytes and both operations were reviewed.
Root's full gate, live/PE agreement and saved starts are pinned from
`local/cc12_tick_adjacent_Native_gate/`. This packet covers only that leaf;
the nearby 86-byte and 76-byte bodies are outside its scope.

The ordinary C++ entry is `std::uint8_t return_native_one_byte_00876180()`.
It takes no arguments and returns byte value 1. It adds no `noexcept`, naked
wrapper, EAX input, global, field, recovered object type or consumer. Native
MOV preserves upper EAX bits and arithmetic flags, but those observations do
not expand the C++ result contract or promise that full EAX equals 1.

The owned header, implementation, this document and
`reports/cc12_native_return_one_byte_source.json` form an uncompiled candidate.
Three bytes is the Original Native body size, not a measured Source size.
Root must register/build it and check the emitted body for exact `B0 01 C3`.
Original ABI, startup and gameplay credit remain zero.

The report records the Source113 snapshot: the reparent and retreat-wrapper
primary receipts share 113 input pins, four artifacts, three checks, 33 whole
objects and 37 positive Core definitions. Their integrator build ran from
`2026-10-09T19:35:55.717080+00:00` to
`2026-10-09T19:36:12.082131+00:00`. All normalized input hashes and exact
artifact hashes were replayed; raw matches and CRLF/LF-only differences are
reported separately. Artifact claims apply at the recorded replay time;
later Root builds may replace the same paths. Prior baselines are historical.

Worker validation covers the complete three-byte gate, input/artifact pins,
the whole candidate and this document, and the four-file diff check. No new
build, test, probe, CMake, ledger, GPR, Ghidra or retention work is included.
