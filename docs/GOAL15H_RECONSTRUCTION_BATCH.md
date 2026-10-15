# GOAL15H raw storage and scene-path reconstruction

Thirteen complete Native functions now have concrete raw Source kernels across five registered bsp_core modules. They cover POD8 storage initialization/iterator subtraction, all five raw-array capacity/growth/append/resize/reserve operations, entity path/ancestor access, scene-point transformation and both authored-path point wrappers. The older typed interfaces remain separate.

| Source module | Complete functions | Physical span bytes | Fixed bytes |
| --- | ---: | ---: | ---: |
| native_pod8_storage_init | 3 | 122 | 106 |
| native_pod8_raw_array | 5 | 506 | 466 |
| native_entity_path_access | 2 | 126 | 126 |
| native_scene_path_point | 1 | 211 | 195 |
| native_authored_path_points | 2 | 89 | 77 |

The 1,054-byte total includes three unreachable alignment bytes retained in the raw-array append span; Ghidra admits 1,051 function-body bytes. All 970 fixed bytes agree between registered COFF objects, target-verified live bytes and original installed disk bytes. Twenty CALL operands and one absolute operand are individually checked for relocation kind, actual Source symbol and Original target. The scene-vector adapter is separately checked for its full 13-byte shape and genuine kernel tail target. No other operands are excluded.

The full MSVC Win32 build and all three existing CTests pass. Five archive-only combined fixtures pass through the real current providers: storage/owning length failure and fresh iterator reloads; all five raw arrays including x87 signaling-NaN quieting/alias rereads and capacity publication before real allocation failure; actual Source callable-vtable reloads and signed ancestor wrap; dirty pose/captured matrix/returning SDK repair and reciprocal behavior; authored indexed/last transformations including a backed empty-last underflow. All receipts include Source, object, executable and consumed library hashes. No tracked test framework or replacement provider is added.

The 111 Native transfer rows and nine explicit Markdown claims pass without failed, unchecked or warning rows. Independent review found no discrepancy in the scene-point instruction schedule, 13-byte adapter, actual stack-cell reciprocal reuse or outgoing provider bindings.

The primary created the reviewed 14-byte00418700 and 71-byte00420AC0 functions under the Ghidra write lock, retaining the previous absence evidence. Thirteen physical prototypes now expose actual consumed arguments and returns, without Source-only EDX placeholders. Existing names/comments and old prototype values are preserved in rollback records; descriptive roles remain hypotheses. All65 owned/direct-caller exports are refreshed, the project saved, and snapshot/index rebuilt.

The decompiler's raw-array integer-copy interpretation is corrected to the literal x87 schedule. Resize18F30 always writes size, including shrinking. The path4A CALL is7ACA23;7ACA21 only restores ECX. Ancestor counter arithmetic wraps before signed testing and its actual EAX return is preserved. Scene-point validation retains its captured index/matrix while reloading repaired live point slots.

These are registered Source build and bounded behavior results. They do not establish Native private CRT/math state, factory/vtable/virtual deployment, RTTI/SEH, address ABI or game-runtime equivalence. Some recovered routines have zero Original incoming references; no runtime adoption is inferred. Next raw avoidance clip/winding readiness is being recovered independently, with actual missing children recorded rather than substituted. Overall reconstruction remains active.
