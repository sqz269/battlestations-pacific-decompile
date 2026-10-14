# Native sampler stage consumption

The texture loop in `00B43410` reads the stage field at sampler-row `+4`
as a byte. The two reads are `00B434B4` (disabled pixel-texture skip) and
`00B434F3` (vertex or pixel slot selection). The signed DWORD at row `+0`
selects the effect or material resource. These are separate fields.

Saved Ghidra bytes for `00B434A0..00B4352F` match the original executable:
144 bytes, SHA-256
`a8827d7b7f59e842cafe2b08df51bd6206d6fc129e5ffa83b68cc34e973cfce9`.
The project/program identity and receipt hash are recorded in
`reports/native_sampler_stage_consumption_cc12.json`.
The corresponding existing Source loop is
`src/native_material_pass_execution.cpp:253-287`.

`00B5F100` still writes the low byte of its stack word and then copies the
complete DWORD into row `+4`. `00B41E80` preserves all eight bytes of each
row. The established `00B5F3F0` retirement frees pair storage without
interpreting the stage field. Copying bytes does not establish their
meaning, and this bounded loop does not establish every reachable consumer.

The strict complete-word acquisition path remains unchanged. In particular,
the canonical root/mode sampler calls retain their shared physical-word
contract and unknown-entry guard. No universal zero, captured builder value,
or upper-byte substitute is admitted. A separate defined-stage Source
interface would require an audit of every reachable consumer first.

This packet adds evidence only: no C++ change, Ghidra mutation, GPU execution,
original ABI replacement, or gameplay validation.
