# Restored cleanup function ownership

Addresses: `0095DB40`, `0082E280`.

The previous vehicle and ship audits recorded decoded cleanup tails outside
Ghidra's stored function bodies. Those ownership gaps are now repaired in the
existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.

| Function | Previous stored end | Verified new end | Complete span |
| --- | --- | --- | ---: |
| `0095DB40` | `0095DB88` | `0095DBC6` | 135 bytes |
| `0082E280` | `0082E2C7` | `0082E311` | 146 bytes |

Before mutation, the complete proposed ranges matched the installed executable
and live Ghidra bytes. Full documentation was archived under
`local/cleanup_body_documentation_before_b2.json`. The range ends are the
decoded final RET instructions, with all branches inside the reviewed range;
they are not inferred from a decompiler return after free.

The prior camera cleanup repair had already cleared both free-call overrides.
For `0082E280`, the locked flow repair cleared the returning-free overrides and
decoded the tail but correctly reported that stored ownership was still short.
The existing `ghidra_define_function.py --recreate` path then recreated both
functions from their existing instructions, under the write lock. Prior names,
prototypes and plate comments are recorded; plate comments are restored by the
tool. Full after-documentation is retained separately. Neither function had a
recovered prototype or reviewed descriptive name to replace.

Readback proves `0095DB40..0095DBC6` and `0082E280..0082E311` are now owned by
the respective entries. Live flow reports 42 and 54 instructions, with zero
gaps. The project was saved, both exports refreshed and the snapshot refreshed.
No executable bytes or global callee no-return flags changed.

The newly owned calls are `0095DBA6 -> 005CD640`,
`0095DBAE -> 00BF6989`, `0082E2E9 -> 00492A00`, and
`0082E2F2 -> 00BF65AC`. All eight direct calls across both full bodies pass
the live call-site verifier. Their range-erase and element-lifetime dependencies
still need reconstruction; this repair supplies reliable body evidence rather
than source, ABI, exception-runtime or gameplay equivalence.

Records: `reports/native_cleanup_body_repairs_orch4_b2.json` and
`reports/native_cleanup_body_flow_orch4_b2.json`. These supersede the stored-body
limitations in the previous vehicle/ship audit reports, preserving those reports
as records of their earlier state.
