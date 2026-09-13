# Actual logical index construction and factory

This packet adds the complete normal constructor, renderer factory, pool allocation, pointer reserve, and a canonical terminal reference to the existing actual index destruction implementation. The factory returns the same raw24h logical stream it registers. Its counter remains actual+04 and its physical backing is a real private D3D9 index buffer owner. Existing mapping and destruction consume that same storage.

`NativeLogicalIndexCreationContext` borrows the unchanged `NativeLogicalIndexOwnerContext`, current profile cells, and the already established `NativeLogicalVertexDeviceRecreation` provider. That provider's B29670 operation is shared with vertex creation and remains a mandatory actual-device lifecycle boundary if the retry branch is reached. No renderer, native counter, semantic mesh stream, or payload vector is substituted.

## Evidence and coverage

Read-only BSP wrappers verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Full stored or live bodies and assembly were read; raw disk disassembly supplied the small omitted spans. There were no saved-analysis changes.

| Entry and inclusive end | Coverage | Original ABI |
| --- | --- | --- |
| B288B0..B28999 | Complete normal factory and construction-allocation unwind;77 listed instructions, zero gaps | ECX renderer; stack count,flags,format; EAX raw owner; RET0Ch |
| B4BF30..B4C1D5 | Complete normal constructor and guard/base C++ unwind order;208 instructions, zero gaps | ECX logical; stack count,format,flags; EAX same logical; RET0Ch |
| B4B380..B4B389 | Complete canonical-pool forwarder | Incoming ECX discarded; ECX=0108FE50; tail JMP B4B0A0 |
| B4B0A0..B4B1DB | Complete actual pool allocator, including returning-free gap | ECX pool; EAX raw28h slot; RET |
| B48F70..B48FAF | Complete slab producer;23 instructions, zero gaps | ECX slab; stack slab index; EAX same slab; RET4 |
| B22D70..B22DCE | Complete actual pointer-array reserve, including returning-free gap | ECX header; stack signed capacity bits; RET4 |

B22DC1..B22DC9 contains `ADD ESP,4`, pointer publication, and capacity publication after BF6989 returns. B4B136..B4B138 is another returning-free `ADD ESP,4`. B4B197..B4B19F is unreachable alignment between an unconditional jump and its target; it does not define another function. The report preserves the listed instruction/gap counts rather than claiming Ghidra was repaired.

All normal call sites are in the report with numeric `address`/`native` rows. Xrefs show B4BF30 has the single B288F4 caller; B4B0A0 has B4B385, and B48F70 has B4B0DD. All five callers of B22D70 were checked: B259A8/B27FD8 and both factory appends supply growth, while B259DE supplies a requested signed size. Its API preserves that full capacity contract. Factory B288B0 is published at renderer D5F0A8+60, word D5F108.

## Constructor and physical ownership

B4BF30 writes base CEB130, initializes actual+04 to1, installs D61DE0, and clears+08/+1C/+20. Trailing slot+24 remains the pool's slab index. Fields0C/10/14/18 are written only after physical attachment and temporary COM release.

Flags' low nibble0..3 selects the D3D pool. Nibble0 also adds10000 to temporary adjusted flags. Nibbles above3 read the native unwritten local at entry ESP-20h; the source requires its explicit input bits. Original flags, rather than adjusted flags, are ultimately stored in logical+10 and passed to physical Attach.

The prologue leaves ESP at entryESP-34h. B4BF92 reads the original argument from `[ESP+40h]`; B4BFB4 modifies only ECX. After B4C175 pushes byte count, B4C176 reads `[ESP+44h]`, still entryESP+0Ch, and B4C17A pushes those original flags for Attach. B4C195 reloads `[ESP+40h]` after the calls and B4C19C stores it into logical+10. No instruction writes adjusted flags back into that argument cell.

Usage bit1 comes from flags10. The flagF00 values100/200/300/400/500 add usage2/4000/40/100/80. FlagF000 equal1000 adds200, and adjustedF0000 equal10000 adds8. Format65 gives width2;66 gives4; other formats produce the native zero byte multiplier and still reach CreateIndexBuffer. Arithmetic is wrapped DWORD multiplication.

Unlike the vertex constructor, this body always creates a private physical owner, including when flags encode dynamic usage. It reads current F8D394, obtains device+1A10 through B1FEF0, and invokes actual device current6C with seven stdcall DWORDs: device, byte length, usage, format, pool, escaped temporary output address, null shared handle.

Retry occurs only if the output remains null, HRESULT is nonzero, and HRESULT is neither8876017C nor8007000E. B29670 receives the current renderer. The subsequent device read also reloads the current renderer; the second HRESULT is ignored. There is no failure-success translation or null COM replacement.

The constructor allocates2Ch, writes the private physical D61E10 prefix/tail in the native order, publishes logical+08, invokes current14=B4C250 with the current temporary COM pointer, original flags and count*width, then unconditionally reloads and releases that temporary. The existing physical Attach owns the real COM reference, support singleton, and native diagnostic strings. Offset0C becomes0, followed by original flags10, count14, and format18.

The actual private profile's nine DWORDs and logical profile's eleven DWORDs were read. Logical current00/04 are BD30E0/B4C1F0. Private current14 is B4C250, current08/0C are B4B850/B4B820, and terminal04 is B4BB20. Other profiles are outside the admitted construction/reference domain.

## Pool and renderer registration

B48F70 initializes32 free slots in each544h slab. Every28h slot receives its slab index at+24. The WORD free-index array at+500 is31..0; WORD+540 becomes32. B4B0A0 enters the actual pool's critical section+0C and increments its tracked depth+24. New slabs append through the existing raw array+28/count+2C/capacity+30; capacity grows to `2*capacity+2` before allocation. Returning free is followed by replacement-pointer publication.

Allocation decrements the slab's WORD count, pops its index, and computes the actual28h slot. When a slab empties, the allocator scans later actual slabs for a nonzero count and otherwise leaves first-available+34 atFFFFFFFF. Lock/depth exit matches both native tails. There is no new exception lock guard or repaired allocation-failure behavior. The pre-existing initialized pool header remains a required input; the native global pool constructor is not reconstructed here.

B288B0 captures its receiver across construction, then appends the raw stream to receiver+1AB8. It inspects that receiver's current58=B1FE50 and appends to receiver+19C4 only when AL is exactly1. Both arrays are actual pointer/count/capacity headers; neither append retains the stream. Null native allocator results follow the original append behavior where execution permits it.

An optional `acquired_before_registration` cell is host creator bookkeeping. It begins empty and receives the completed raw identity immediately before the first append. If an append fails, the caller can bind or retire that creator through its canonical owner rather than lose its identity. The cell adds no native retain, rollback, or successful provider result. This is the stream-clone caller's required handoff before mapping.

## Unwind and canonical terminal

Constructor FuncInfo DF84D0 has two entries at DF84C0: state0 uses CBF9C0, state1 uses CBF9C8. The former reaches B49420/BD30F0 and restores CEB130. The latter reaches B21110 using the original local guard. Native construction does not arm physical/temporary COM rollback; the source does not add it. Guard cleanup precedes base cleanup. A second exception in an unwind action terminates.

Factory FuncInfo DF59F4 has one entry at DF59EC. CBD250 forwards the captured allocation through B49970 to existing B495E0 slot return. That cleanup is armed only around construction; renderer append failures retain the completed owner. Optional synchronization uses the existing actual entry/leave/cleanup routines and their captured-renderer/current-mode semantics. An entry-disabled/exit-enabled transition consumes an uninitialized native guard and remains outside the valid input domain.

`NativeLogicalIndexReference` borrows the actual+04 atomic cell without initializing or retaining it. It accepts the observed D61DE0/BD30E0/B4C1F0 terminal, invokes the existing destructor and actual pool return at zero, then calls the explicit companion-retirement callback. The creation/lifetime contexts must outlive this companion. The existing destructor continues to unregister primary renderer+1AB8 and release current physical+08; its established secondary-registry behavior remains unchanged.

## Validation limits

The ignored `local/index_creation_probe.cpp` links current production libraries and reuses the archived vertex fixture's real hidden-window D3D9 HAL setup. It crosses the32-slot allocator boundary, returns the raw slots, constructs one actual managed INDEX16 buffer, checks both renderer registrations and the real COM descriptor, maps/writes/unmaps/reads through production index mapping, and releases through the same-counter canonical terminal. It does not replace COM methods or production owners.

Renderer/global/pool-header and immutable profile publication are explicit fixture inputs. Native producers for those global objects, device recreation, returning failure paths, optional synchronization, and full game adoption remain unexercised. Build/fixture/call-check results and hashes are recorded in `reports/native_logical_index_creation.json`. No permanent test or Ghidra mutation is included. These are new MSVC Win32 source interfaces, not original binary entry replacements.
