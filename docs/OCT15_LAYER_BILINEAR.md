# Instruction-backed avoid-zone layer bilinear sample

Address: `0041BC20`, native body `0041BC20-0041BD02` (227 bytes, 79 listed
instructions). The complete Source kernel is
`layer_sample_bilinear_kernel_0041bc20`; existing game-host helper
`bsp::game::avoid_zone_sample_0041bc20` and its evidence record remain intact.
Descriptive names are hypotheses.

Original ABI: ECX layer, stack world x/z, ST0 float, `RET 8` at `0041BD00`.
New Source ABI: `__stdcall(layer,x,z)`, ST0 float, `RET 0Ch`. The extra Source
layer argument is loaded into ECX before the original frame setup. The two
input loads account for that extra word; dead argument/local slots still hold
the original arithmetic scratch values. This is not a native ABI/member binding.

| Native call site | Callee | Argument/result contract and readiness |
| --- | --- | --- |
| `0041BC40` | `00417FA0` | complete accepted raw WorldToCell kernel; ECX layer, EDX local cell output, stack x/z, Source `RET 8` |
| `0041BC6D` | `0041BAE0` | h11 at wrapped ix+1/iz+1; accepted raw height kernel, ECX layer, EDX x, stack z, Source `RET 4`, ST0 float |
| `0041BC7A` | `0041BAE0` | h01 at ix/iz+1 |
| `0041BC8A` | `0041BAE0` | h10 at wrapped ix+1/iz |
| `0041BC97` | `0041BAE0` | h00 at ix/iz |

Each Source height call reserves one unused padding word and its real z
argument, then discards the padding after Source `RET 4`. This keeps the
original shared scratch offsets and eight-byte total balance while using the
accepted EDX-x interface. Padding is not a semantic argument. Layer ECX is
explicitly restored for each height call, honoring Source caller-volatility.

`CVTTSS2SI` at `0041BC49/0041BC4F` forms ix/iz with hardware exceptional-range
behavior. `LEA` at `0041BC5D/0041BC60/0041BC84` forms wrapped DWORD neighbors.
Neighboring indices remain integer DWORDs. Signed C++ casts/increments are not
used. The native h11/h01/h10/h00 call and result-store order is retained.

Let F32 mean the explicit native float store. The arithmetic is:

```
fu = F32(cell.x - ix)
a  = F32(h00 + fu * (h10 - h00))
b  = F32(h01 + fu * (h11 - h01))
fv = F32(cell.z - iz)
h  = F32(a + fv * (b - a))
```

All unmarked difference/product/add intermediates stay on the x87 stack.
Fu is stored before the height calls and then retained across both row blends.
Fv is computed and stored after both blends. The assembly retains the original
operand/stack order and all height, fraction, row and final float stores; it
does not substitute double intermediates, algebraic reassociation or fused
operations. The DC-form scan found no DC register-form sites. Raw multiply/add
bytes at `0041BCB5` (`DE CB`), `0041BCCD` (`DE CA DE C1`) and `0041BCF1`
(`DE CA DE C1`) confirm the indexed pop destinations and additions.

The two accepted Source kernels are complete at their documented Source
boundary. WorldToCell retains its native x87/clamp schedule. CellHeight retains
wrapped indexing, byte-span checks and its returning-handler begin/late-scale
reloads. Its indirect `00BF6713` service uses the qualified current Source SDK
invalid-parameter entry: original encoded globals, diagnostics, dynamic-handler,
fault/return and repair identity are not established. There is no new callback,
zero substitute or invented grid/owner state in this sampler.

Callers supply actual mutable `Cc13TerrainGridLayerNative` producer-shaped
backing under those raw-kernel storage/handler obligations. The existing
semantic normal adapter's private backing is admitted only through its two
sampling methods. This packet does not expose or reinterpret that backing,
reinterpret `TerrainGridLayerRecord`/`std::vector`, or add duplicate admission.
The raw sampler remains available for backing outside valid-only admission.

Dependency fixture receipts are deliberately separate: WorldToCell has four
bounded original-byte output/return cases, and CellHeight has one valid-span
wrapped-row case. The latter uses dimension `0x40000000`, x=z=4, span 8 and
wrapped index 4, producing `40880000` without reaching the invalid sentinel;
that raw state is outside normal-adapter admission. The tracked primary receipt
is `reports/oct15_third_reconstruction_batch.json` on main `5a45adc36`.

Primary also compared this sampler against its full original 227-byte body,
with all five calls relocated to full original WorldToCell (159 bytes) and
CellHeight (105 bytes). Only the double-2.0 data operand and an unexercised
invalid-call sentinel were additionally relocated. One finite nonplanar 4x4
grid case passed: original and Source both returned binary32 `40AAF1AC` at
x bits `BFF47AE1`, z bits `BFD1EB85`; sentinel not reached. Evidence is in main
`local/oct15_layer_sampling_probe/bilinear_input.json`, `bilinear_result.json`
and `bilinear_probe.cpp`; the driver uses `/MANIFEST:EMBED` and masked default
FP state. This is one output-bit case, not abnormal/trap/status, sampler ABI,
CRT diagnostic, adapter, full-domain numerical or gameplay proof.

Live `bsp.py` verified project `bsp`, program `/battlestationspacific.exe`, x86
language and image base. Configuration names `C:/Users/sqz269/bsp.gpr`.
Readiness includes the named incomplete original CRT boundary above; no call
is silently assumed resolved by its name. Original incoming xrefs have nine
sites in four functions. No game frame/runtime consequence is inferred from
them. The worker made no Ghidra mutation or game wiring change. Win32 build
and all three existing suites passed; detailed checks are in the packet report.
