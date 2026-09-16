# Bone resource item reader (R26)

`00B8AF30..00B8B04C` is the complete 285-byte reader initially installed at
`00D632B8+20h`. The R25 outer parser `00B8A990` allocates `2Ch`, constructs
the reference base, installs that profile and zeros only `+08/+0C`. This
reader assigns the native string at those two words and seven float32 fields
at `+10,+14,+18,+1C,+20,+24,+28`. No meaning beyond that storage is assumed.

## Interface and source composition

Original ABI: `ECX=actual item`, one stacked structured-node handle, `RET4`.
The listing does not consume EDX as an argument. The decompiler's EDX
parameters and `extraout_EDX` values are artifacts. EAX has no specified result.

`read_native_bone_resource_item_00b8af30` uses the existing complete bodies:

- `read_native_resource_handle_string_00bea010` against the actual handle;
- raw-pool `resize_native_string_header_0041dd40` and
  `destroy_native_string_header_0041dd20` against actual eight-byte headers;
- `read_native_resource_node_float_00be99d0`, which captures the current node
  on every invocation and uses the existing stream/accounting layer.

No semantic string owner, synthesized node, pool projection or default float
is introduced. `NativeBoneResourceItemReaderCalls` resolves exactly the
captured target `00B8AF30`; every other target, item and handle forwards to its
required remaining reader service. This composes with the R25 outer parser
without changing its files or the separate AnimationChannels reader.

## String effects and lifetime

The caller does not initialize the eight-byte temporary. BEA010 owns its
construction; cleanup is armed only after that call returns. The returned
header pointer is compared with `item+08`. When different, resize receives
the current source length and preserve byte 1. Source length is reloaded
after resize. Nonzero source length triggers a copy whose count is the
current destination length; source and destination data are also loaded after
resize, in native order. The copy excludes the terminator resize installed.

State 0 covers assignment of the completed temporary. On assignment failure,
only that temporary is destroyed. The item and any partially changed item
name are retained. Ordinary temporary destruction occurs after state -1 is
stored and before any float read, so a throwing pool getter is not retried.
The source catches only assignment failures and terminates if cleanup throws
during unwinding. It adds no cleanup around the reads or any item rollback.

Immediate native exception evidence:

| Region | Evidence |
|---|---|
| `00CC2940..00CC2947` | `LEA ECX,[EBP-2Ch]; JMP 0041DD20` destroys the temporary |
| `00CC2948..00CC2951` | loads `00DFBF80` and jumps to `00BF6B43` |
| `00DFBF78..00DFBF7F` | one unwind-map row: state -1, action `00CC2940` |
| `00DFBF80..00DFBFA3` | FuncInfo `19930522h`, maxState 1, no try blocks |

The handler instructions are present in Ghidra but `00CC2948` is not a
function entry. This packet does not mutate the shared listing or metadata.

## Floating-point schedule

Each of seven calls returns through x87 ST0. Its result is immediately
`FSTP float32`-spilled to a local; there is no live float spanning another
reader call. After the seventh spill, seven `FLD float32` / `FSTP float32`
pairs commit those locals to `item+10..+28`, in read order. The source uses
explicit x87 assembly for both phases, retaining signaling-NaN quieting and
the observed 32-bit rounding points. It uses integer locals to avoid added
compiler float conversions.

A failure in any scalar read leaves all seven payload fields at their entry
bytes. The item name has already been assigned and the string temporary has
already been returned. Stream position and node accounting retain effects of
completed operations; nothing restores them. Native cleanup of the allocated
item remains disarmed in the outer parser too.

## Evidence and validation

The companion report contains all 12 direct call rows and the `CC2943` cleanup
tail jump. The standard read-only checker accepted all 13 rows. Live bytes
from project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
match the installed PE for the entire 285-byte body, 18 bytes of immediate
exception code and 44 bytes of exception data: 347 bytes total.

Validation results and artifact hashes are recorded in
`reports/native_bone_resource_item_reader_r26.json`. The ignored fixture
uses a genuinely constructed native string pool, raw storage, existing
structured-node/string implementations and a byte-stream input service.
It compares the copied original reader's ordinary path to the source and
separately checks a source exception during the fourth scalar read. Original
calls in the copied body are redirected to ABI bridges to those same complete
source services; this does not validate their original machine code or FH3.

The strict Release Win32 build passed with `/MD /W4 /WX /fp:strict`; all
eight original-code seeds matched before building and all three CTests
passed. Both native/source comparisons passed bitwise for `1.5`, negative
zero, both infinities, the minimum float32 subnormal, a signaling NaN and a
quiet NaN. The signaling NaN became `7FC12345` in both bodies. Nonempty
`Bone\0X` and empty names matched, including final byte accounting and pool
return state. During the source's fourth scalar read failure, the completed
name remained and all seven payload words retained `A5A5A5A5`; budget was
978 after the three successful scalar debits. Non-Bone forwarding passed.

The new explicit-service source ABI is not a drop-in binary interface.
Original private stack addresses, GP registers/EFLAGS, native FH3/SEH,
hardware-fault handling, CRT identity and game execution remain unproven.
