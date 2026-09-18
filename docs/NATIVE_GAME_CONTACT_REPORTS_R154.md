# Native game contact report receiver and vector12 helpers (R154)

The actual physics callback at **004D4CE0** now has a complete normal source
implementation and a callable one-slot table corresponding to **00CE78C0**.
It receives the real world-step's 88-byte contact records and copies each first
world-space point into the checked vector in the game object.

## Evidence and storage

Ghidra project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
Live bytes matched the original PE for 1,659 bytes in 15 spans: fourteen bodies
covering 1,655 bytes and the four-byte callback table. The missing callback
definition is 004D4CE0..004D4D61. The preceding function ends at 004D4CDD;
alignment bytes separate it from this callback. No enclosing function was split.

The constructor writes CE78C0 at game+1Ch and publishes that subobject to the
world. The checked vector is therefore at game+20h: proxy, data, end, capacity
end. The callback's original ABI is ECX receiver and stack records/count, RET8.
The count reaches resize as unsigned, then controls a signed report-copy loop.

| Address | Body | Bytes |
|---|---|---:|
| 004D4CE0 | Report callback | 130 |
| 004D1510 | Checked vector resize | 213 |
| 0041FA40 | General fill insert | 674 |
| 004C82B0 | General range erase | 88 |
| 00419330 | Forward move | 84 |
| 00415290 | Vector size | 30 |
| 00415720 | Vector allocation | 85 |
| 00419900 | Uninitialized copy | 41 |
| 0041E310 | Uninitialized fill wrapper | 53 |
| 0041C7B0 | Uninitialized fill loop | 41 |
| 0041F480 | Checked uninitialized copy wrapper | 37 |
| 0041C500 | Assign fill | 41 |
| 0041D950 | Checked backward copy wrapper | 43 |
| 00419710 | Backward copy | 95 |

Source preserves all normal insert paths: reallocating prefix/fill/suffix,
in-place short suffix, and in-place backward movement. Insert snapshots the
fill value with MOVSS before possible aliasing changes. Copy/fill helpers use
the native x87 load/store order. Growth, iterator checks, publication order,
zero-count behavior and allocation identity remain explicit.

The callback reserves an **uninitialized** three-word fill value before resizing.
Source preserves that omission; it does not replace it with zero. Report points
overwrite the completed elements. Intermediate floating exception flags can
depend on that stack preimage.

The returning free continuation at 0041FBC3..0041FBC5 was restored. The catch
free continuation at 0041FBFA..0041FC05 was also restored for analysis, exposing
the native rethrow. The 674-byte insert evidence span includes this separate
21-byte catch reference; its two calls are excluded from normal-body call rows.
Callee no-return annotations were left unchanged. Source
omits native FH3 registration/restoration and catch cleanup/rethrow; source
length-error and bad-allocation services do not certify native failure unwinding.

## Ownership and integration boundary

`NativeGameContactReportRuntime` owns the complete one-slot source table and
borrows matching allocator/call services. Binding writes only the table pointer
to an already initialized callback subobject. It owns no receiver or vector.
The owner must remain stable and outlive every receiver using its table.

`GameNativeDynProcess` now owns the world-step, collision and group contexts,
sharing its actual engine/profile cells, allocator and CRT services. It also
owns the contact-report table. Accessors require completed process startup.
No replacement world is constructed by these accessors.

Ordinary application game admission remains separate: the projected game state
is not a native game object, and its fixed-step host is not yet bound to this
world. The original constructor's table literal is unchanged until that real
owner is admitted.

## Validation

- Strict MSVC Win32 build and all three existing CTests passed.
- Independent COFF audit checked 654 native instructions, 53 branches and 13
  adapters, with explicit records for context arguments and omitted FH3 paths.
- Relocated original code versus source: 108 paired sequences, 1,512 paired
  operation states, **24,894,432 identical bytes**, and 1,512 allocator events
  across both sides. Twelve x87/MXCSR combinations cover signed zero, subnormal,
  infinity and masked quiet/signaling NaN inputs. Cases include aliasing fills,
  both in-place insert paths, reallocation, erase, no-ops, shrink, capacity reuse,
  and nonempty/empty callbacks through the source table.
- Explicit-fill vector operations compare FP status. Callback observations
  compare completed storage and allocation events, excluding exception status
  caused by unknown resize-fill stack bytes. Fault/unmasked-exception parity is
  not established.
- Actual producer fixture uses canonical process contexts and table, both solver
  modes, real collision listeners and pending-body removal. Four simulations
  with four substeps produce four observed initialized points after nonempty
  simulations, six listener calls and eleven worker allocations. Empty reports
  retain capacity. Fixture frees its receiver vector; native physics teardown,
  pool trimming and real atexit end with zero tracked allocations. The fixture
  closes 101 handles not closed by the reconstructed native lifetime path.
- One-frame application exited zero with a window and D3D device, zero presented
  frames, one skipped present and 45 unimplemented host methods. This is startup
  evidence; it does not exercise native game admission or prove gameplay.

Names describe evidence-backed interpretations, not recovered symbols. Full
contracts, direct-call checks, prior annotations, hashes and immutable artifact
receipts are in `reports/native_game_contact_reports_r154.json`.
