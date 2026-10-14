# Main-scene execution prefix and actual camera history

This packet supplies the genuine B156C0 entry through B1586C and the complete
actual-storage B0DB00 history calculation it calls. It ends immediately before
the B1586D texture-holder acquisition. A true return means this exact boundary
was reached, not that a scene command executed successfully. The existing queue
must continue refusing D5E13C: B17240 preparation and the remaining B156C0 render
passes are not installed or claimed complete.

The implementation uses actual service/command/camera/clock storage and existing
raw camera, matrix and CRT kernels. It creates no renderer, scene, camera,
history, owner or callable table. The older callback-based scene-renderer
projection remains separate and is not used as a replacement execution body.

## Executed prefix

`execute_native_main_scene_prefix_00b156c0` implements B156C0..B1586C, a 429-byte
native prefix. Its original function has ECX service and one command stack
argument, RET4; this new source stage takes explicit borrowed context and a
persistent external continuation record.

1. Capture the actual F8D394 renderer publication and current profile+2C. The
   established D5F0A8 slot is B1FE20, the frame-active getter. An inactive frame
   returns without reading the command or service. This is distinct from the
   B20240 gate in the outer B17240 wrapper; the prefix does not add that gate.
2. Repeatedly obtain the current command-context camera through full raw B1BF40.
   Capture its +1D4 near-plane bits, perform the original x87 CE3800 argument
   store and set the same actual camera through raw B6FBF0. Capture current +190
   through full raw B6FE60, then replace the saved high byte with FF. The
   external record retains these exact words for the later original restores.
3. Obtain projected Y and X axes through full raw B70EA0/B70FE0 using the existing
   canonical axes/CRT domain. Normalize actual service+19C velocity through the
   existing 00419510 kernel. Preserve the original x87 dot products, spill
   boundaries and CEC9E0 double multiplication into service+1A8/+1AC.
4. Reload the actual 01090AB0 clock, load its accumulated float at +04 and retain
   its two original float32 stores around the next current-camera getter. Run
   full raw B0DB00 on the actual service+314 history.
5. Copy the history result at service+55C to service+5A8 through raw 004134F0.
   Obtain the next current camera's real B70490 view-projection cache, invert
   it through full raw B632D0 into the original temporary shape, and copy that
   inverse to service+5E8 through 004134F0. Return at B1586D.

Every original camera getter reloads the current command context. The prefix
does not snapshot a single camera or replace its world/cache/matrix storage.
The axes/history contexts must borrow the same actual CRT and constant cells;
their identity checks are source-domain admission, not native owner operations.

## Complete history body and rounded lerp

`update_native_camera_history_00b0db00` supplies all B0DB00..B0E924: 3,621 native
bytes and 817 instructions. Original ECX is the actual history, with time float
bits and camera on the stack, RET8. EDX adds the borrowed source context. The
294h storage is delay+00, count+04, eight 48h samples at +08 (world matrix, FOV,
time), output view-projection matrix+248 and prior position+288. Native-valid
counts 0..8 are required; the implementation adds no clamp or initialization.

The complete schedule preserves distance-based restart, forward x87 sliding of
the seven retained samples when count is eight, current world/FOV/time capture,
count publication, the original time-search comparisons, fraction clamp and
unordered handling. It then performs position and axis interpolation, all four
rounded square-root/normalization stages, both cross-product stages, interpolated
FOV projection, scaled-affine inverse and matrix product. It stores the full
result before the final world refresh and previous-position publication.

The original zero-duration and unordered fraction paths remain intact. There is
no fresh-history shortcut, alternate sample search, repaired axis or substituted
perspective calculation. Seven calls resolve to established raw/CRT domains:
B6DB70, 00419440, BF7030, B642F0, B63B30 and 00413920, plus the newly concrete
005803E0 lerp.

`lerp_native_float3_005803e0` supplies all 177 native bytes, with original ECX
output, EDX old vector, target/fraction stack arguments, RET8 and EAX output.
It retains the separate x87 binary32 difference/product/add stores, current
source reads and overwrite of its captured first public argument slot. The
older value-returning `main_menu_map_lerp_005803e0` interface is unchanged.

## Register and lifetime evidence

The listing's abbreviated `FMUL ST1` at B157EA is ambiguous without its bytes.
The installed instruction is **DC C9: `FMUL ST1,ST0`**, so it multiplies ST1 and
leaves the scale in ST0. A first bounded comparison exposed the wrong direction;
the final source uses the verified opcode direction at every one-register x87
arithmetic instruction. This preserves the negative half-scale in both projected
velocity constants.

The history helper saves an extra EBP for its context. Only the two public
arguments move by four bytes; all native scratch offsets remain unchanged. The
native search already saves/restores EBP around its endpoint pointer. Constant
translation preserves live EAX and adds no floating operation or spill. The
existing raw projection provider receives the original writable argument slots;
its new plain-RET ABI is balanced by an explicit 10h stack adjustment.

The B156C0 prefix has not armed a native cleanup state before its boundary.
Changing the camera near plane is not rolled back here. The fresh external
record, actual service/camera/history and borrowed domains must survive for a
future genuine continuation. The raw arithmetic/CRT callees must return normally;
C++ exceptions across the naked helpers, native FH3/SEH, floating traps and
arbitrary original private-stack aliases are not established.

## Validation

- Installed executable and saved Ghidra body bytes matched for the complete
  3,621-byte history, 177-byte lerp and 429-byte execution prefix. The verified
  project was `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
  Original constant bytes confirm near0.5, restart20, one1 and direction-0.5.
- Both translation units passed strict MSVC Win32 `/MD /W4 /WX /O2 /fp:strict`.
  A manifested source-entry link probe linked the real callee closure against
  the fresh `bsp_core.lib` and launched successfully.
- A static machine-listing audit matched all 648 unchanged tagged x87
  instructions in the history/lerp source and 41 in the prefix: 689 total.
  The context-bound constant loads and shifted public time slot are separately
  reviewed adaptations, not included in that unchanged-encoding count.
- One local fixture compared rebased original B156F6..B1586C plus complete
  original B0DB00/005803E0 bodies against the source prefix/history. All 23
  service/camera byte comparisons passed across ordinary rolling eight-sample
  history, zero-delay unordered values, reset-distance transitions and actual
  world-cache refresh. The fixture uses the same established raw camera/math/CRT
  callees, with argument/context bridges for their new source interfaces. It
  separately checks the source inactive-frame exit and continuation fields.
  The original outer gate, full renderer, native EH and CRT callback/exception
  branches are not differential-tested by this fixture. Floating controls use
  the ordinary masked environment; FP status/stack identity is not compared.
- `scripts/build.ps1` passed the current Release Win32 build and all three checks:
  reconstructed_math, native_math_differential and tool_tests. This worker leaves
  registration to the primary owner, so the new units are verified separately by
  strict compilation/linking. No persistent test suite was added.

`reports/native_main_scene_execution_prefix_cc12.json` records artifact hashes,
validation limits and annotation receipts. Outputs are under
`local/main_scene_execution/`. All fifteen preceding main-scene artifacts,
including its exact old core library, were preserved before this build under
`local/main_scene_receipt_3ccc58cbf/receipt.json`.

## Next executed clause

B1586D first reads service+4C and calls B4CB10, then repeats that getter and reads
the current texture's dimensions through slots3C/40. Later work computes texel
and far-corner constants, binds real targets, submits camera/system constants,
prepares and executes batch/material passes, and performs the concrete
post-effect/cockpit/distortion/stats paths. These remain required. No hidden-node
bypass, callback-only renderer, D5E13C admission, application invocation, visible
world pixels or gameplay equivalence is supplied by this prefix.
