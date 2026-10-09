# CC12 GlobalConfig colour setter frontier

This is a bounded, read-only continuation of
[the direct-caller audit](CC12_GLOBAL_CONFIG_SECOND_ARRAY_DIRECT_CALLERS.md).
It follows the five virtual `+50h` calls reached by six GlobalConfig getter
borrows. Evidence is in
[the companion report](../reports/cc12_global_config_color_setter_frontier.json).
Names below describe native behavior; they are not recovered symbols.

The two audited native setters copy colour values. Neither retains the source
pointer, writes through it, nor releases it. Their complete direct helper chain
contains no GlobalConfig second-array publication. **This remains conditional
target evidence:** the inspected receiver producers do not establish the current
native vtable at any of the five indirect calls. No empty-array, whole-program
absence, or no-op callback claim follows.

## Receiver provenance

| Virtual call | Getter borrow | Actual receiver and producer | Remaining type evidence |
| --- | --- | --- | --- |
| `005ADFF7` | `005ADFD8` / `005ADFE4`: owner `+108h` / `+128h` | Entry `+18h`. If null, `005ADF13` clones the original controller's `+98h` template through `00AAB4C0`, then `005ADF18` stores the result. A non-null prior receiver is reused. | Template type tag/current table, or the reused receiver's current table. |
| `005AED48` | `005AED36`: owner `+158h` | Three slots at entry `+64h/+68h/+6Ch`. The loop clears an old object after virtual deletion, then clones controller `+A4h` or `+E8h` at `005AEC7E` / `005AEC94`, selected by entry `+2Ah`. | Both templates' actual type tags/current clone tables. Neither is classified as Text or Icon merely from its use. |
| `005D0904` | `005D08F1`: owner `+218h` | Chat controller `+5Ch`. Initializer `005D14D0` finds `SendMessage_Text` beneath its `+44h` `ChatModeOn_Group`; `005D15A8` stores the lookup result. | Loaded child construction and current table. |
| `005D094E` | `005D092B`: owner `+208h` or `+218h`, selected by controller byte `+8Ch` | Chat controller `+60h`. The initializer finds `SendMessage_Text` beneath its `+4Ch` `Lobby_Group`; `005D17EE` stores the lookup result. | Loaded child construction and current table. The call is at `005D094E`, not `005D0950`. |
| `005D0AF2` | `005D0AD7`: owner `+208h + (index << 4)` | First stack argument to `005D09A0`, held in ESI. All three listed direct callers in `005D0C60` pass a `00AAB4C0` clone result. | Actual source template tag/current clone table, including the outer-menu templates described below. The index domain is not established here. |

The map controller identity comes from `MOV EBP,ECX` at `005ADDCE`; the entry
argument is loaded into ESI at `005ADDFB`. The controller is also preserved in
the stack frame used by the later map-effect clone branches. These are native
receiver chains, not reconstructed facade object assumptions.

`005D0C60` takes the template as its first stack argument and the parent as its
second. It clones that template at `005D0CEB`, `005D0DF4`, or `005D0F0F`, and
passes the resulting widget to `005D09A0` at `005D0D21`, `005D0E2A`, or
`005D0F45`. The listed callers of `005D0C60` provide these origins:

- `005D1D52` selects chat controller template `+54h` or `+50h`, paired with
  parent `+44h` or `+48h`. The initializer stores `MessageTemplate_Text` lookup
  results in those template fields at `005D1618` and `005D1703`.
- `005E5CBF` provides outer-menu template `+1B4h`, parent `+134h`.
- `005E5D45` provides outer-menu template `+128h`, parent `+B4h`.

The uncontained call at `005D1D52` is included from its instruction context;
the named-function caller list alone omits it.

## What the native profiles prove

`00AA7E00` searches a widget's direct children by scene-node name and returns a
child pointer or zero. It neither creates a Text object nor checks the child's
type tag or table. Consequently `_Text` names and nearby direct Text-method
calls are supporting hints, not sufficient current-profile proof.

`00AAB4C0` has native ABI ECX = source widget, one stack argument = parent clone,
EAX = result, `RET 4`. It loads the source's type tag from `+60h` and calls
`00AA6560` with ECX = tag and EDX = source. This factory path gives a useful
conditional type witness:

1. Native type 3 indexes `00AA65F4 + 8` to `00AA6581`, which moves EDX into ECX
   and jumps to `00AA1380`.
2. `00AA1380` requests `1F4h` bytes and invokes the copy constructor
   `00ABB2C0` for a non-null source, or `00AB9650` for a null source. Allocation
   failure returns zero.
3. `00ABB2FD` and `00AB9678` install native table `00D5C6C8` in their respective
   constructors. The actual table DWORD at `00D5C718` (`+50h`) is `00AB6B50`.

Thus a proved native Text profile selects the Text setter. The actual table
cells `00D5C180` (`00D5C130 + 50h`) and `00D5C510`
(`00D5C4C0 + 50h`) contain `00AA6870`. These byte facts do not assign either
table to the inspected caller receivers. Subsequent profile validity still
belongs to the actual receiver/lifetime proof.

## Complete setter and helper contracts

`00AA6870` takes ECX = GUI receiver and one stack source pointer, preserves
ESI/EDI, and returns with `RET 4`. It reads four DWORDs from source `+0/+4/+8/+Ch`
and copies them to receiver `+50h/+54h/+58h/+5Ch`. If the node at receiver `+4Ch`
has nonzero geometry and a nonzero element count, it also copies those four
DWORDs into the first material's diffuse quartet at material `+38h..+44h`.
The source pointer is held transiently in ESI; it is never stored in object or
global state.

`00AB6B50` has the same incoming ABI and `RET 4`. It holds the source in EDI,
calls `00AA6870`, and optionally writes the first shadow material's alpha.
The shadow node is receiver `+188h`; its geometry and element count must be
nonzero. At `00AB6B9F`, x87 loads receiver `+174h`, multiplies by source `+Ch`,
rounds to a stack float at `00AB6BAF`, then copies that float to diffuse `+Ch`
at `00AB6BBC`. This ordering is recorded as evidence, with no arithmetic port
or ABI compatibility claim.

Both material paths use only these fully inspected native helpers:

| Helper | Native effect |
| --- | --- |
| `00B74650` | Returns whether node `+180h` is nonzero. |
| `00B74640` | Returns node `+180h`; ignores one stack DWORD; `RET 4`. |
| `00B72B40` | Returns geometry `+58h`, the count used by these setters. |
| `00B732C0` | Returns `array[index]` from the pointer at geometry `+54h`; `RET 4`. |
| `00B179F0` | Returns material `+38h`; ignores one stack DWORD; `RET 4`. |

For index zero, the final path is node -> geometry -> first element ->
element `+20h` material -> material `+38h` diffuse storage. These helpers only
read pointers/counts or calculate an alias. No source colour pointer is passed
to them, and none writes object or global storage.

## Second-array boundary and remaining work

The complete audited target bodies perform value copies into GUI/material
storage. They contain no source-address retention, reference count operation,
source-based store, or instruction that publishes to GlobalConfig
`+2CCh/+2D0h/+2D4h`. The actual setter destinations are derived from the GUI
receiver, not from the GlobalConfig source pointer. This is a native store
classification, not a proof that malformed GUI pointers cannot alias arbitrary
memory.

The index used at `005D0AF2` is not given an inferred safe domain. An unexpected
index could read other GlobalConfig words. Copying those raw DWORD values into
colour storage does not retain the source address or establish ownership of
values they happen to encode.

All five current receiver profiles remain open. Closing them requires actual
loaded template/child construction and current table evidence, including the
reused map label and outer-menu chat templates. The whole-owner escape
`004326E7 -> 00BD0C30` from the preceding report also remains unexpanded.
This packet therefore cannot enable a no-op second-array callback or prove
that the array stays empty.

## Validation

Every live query verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` through the repository's Ghidra tooling. The report
preserves 34 valid instruction contexts and 31 byte spans totaling 1,588 bytes;
all spans exactly match the installed executable, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The two setters and five direct helpers decode completely through their return
instructions. Six getter borrows and five virtual call anchors were checked.

Only this document and its JSON report change. No source, Ghidra database,
ledger, build, native probe, runtime, or gameplay work was performed. Static
byte equality and store analysis are not execution or game validation.
