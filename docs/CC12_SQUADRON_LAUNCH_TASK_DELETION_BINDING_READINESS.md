# CC12 squadron-task deletion binding readiness

Two constructor-identified profile words have concrete Native slot-zero
targets: `00CFD99C -> 0071C4D0` and `00D08AE4 -> 007F1EE0`. Each four-byte
word agrees in live Ghidra and the configured Original PE's read-only section.
This resolves two table-word identities. Target ABI, cleanup, Source dispatch
bindings and execution remain unproved.

The separately captured Root Astra constructor gate owns 136 bytes / 41
operations at `007F1DE0`. It writes task+0=`00CFD99C` before passing the
captured task to `00876020`; it later writes `00D08AE4` before member child
`007F0F80`. That uninspected child may affect later memory, so the last owned
profile write does not establish the current post-child word unconditionally.
The gate remains pinned as a separate diagnostic record, not Source credit.

The accepted 100-byte `00874F00` caller captures current head N, performs
unlink effects, then freshly reads N+0 and that profile's current slot zero.
It calls that pointer with ECX=N and pushed flag 1. If this current N+0 equals
one of the two selected addresses and its word remains unchanged, the physical
edge targets the corresponding function below. This conditional edge does
not establish all nodes' profile domain, producer lifetime or deletion effects.

| Word | Four bytes | Target metadata only | Recorded children |
| --- | --- | --- | --- |
| `00CFD99C` | `D0 C4 71 00` | `0071C4D0..0071C4ED`, 30 B / 10 ops | `00875B30`, `00BF65AC` |
| `00D08AE4` | `E0 1E 7F 00` | `007F1EE0..007F1EFD`, 30 B / 10 ops | `007F1E70`, `00BF65AC` |

Both targets have current compiler-generated scalar-deleting-dtor metadata
labels. Those names and `_free` metadata do not prove RET4, preservation of
the caller's ESI/EDI, allocation provenance, cleanup order, return/throw or
no-return policy. No target, child, caller, handler, neighboring table word
or string body was read by this packet. A Root Astra whole-body gate must
account for possible listing omissions before routine implementation work.

Bounded current Source searches across `include/bsp` and `src` find only
the derived profile comment in `plane_squadron_host.hpp`; no actual matching
callable provider or child-profile dispatch binding was found. The existing
singleton dispatcher has a finite, different owner domain. A generic
unknown-profile free/callback fallback would not close these contracts.

Every live query verifies `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe`. The report retains eight input pins, five
captures, the Original image hash and both complete four-byte words. This
two-file evidence packet changes no Ghidra or Source, runs no build/test/probe,
and applies no ABI/startup/gameplay credit. Actual task ownership, handlers,
storage, producer/consumer bindings and runtime validation remain separate.
