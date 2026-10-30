# Native code-unit construction through the actual raw pool

This packet adds one overload of `construct_gui_text_code_unit_00ab81c0` in the existing [header](../include/bsp/gui_text_glyph_child.hpp) and [CPP](../src/gui_text_glyph_child.cpp). It directly composes the already recovered raw pool getter, allocation and return bodies. The existing `ActualNativeStringPoolStorage&` API/body, its byte helpers, and the historical [glyph-child evidence](GUI_TEXT_GLYPH_CHILD.md) remain unchanged. There is no new Native body, helper/vendor/duplicate credit, owner, allocator or application route.

```cpp
void* construct_gui_text_code_unit_00ab81c0(
    void* actual_utf16_header,
    std::uint32_t code_unit_word,
    NativeStringRawPoolContext& context);
```

The actual header is ordinary eight-byte Win32 backing: DWORD UTF16 length at `+0`, buffer pointer at `+4`. The context borrows the application's SAME pool publication `01090AA8`, live return gate `01090AA4`, and raw manager publication `01090AA0`. A typed Text/string owner, semantic lifetime adapter, cached pool, copied publication or new context owner is unnecessary. The new overload has no `noexcept`, catch, local cleanup or origin classifier.

The complete Native boundary is `00AB81C0..00AB8242`, 131 bytes, 46 instructions, six basic blocks, seven edges and five direct call sites. Native ECX is the captured header; one DWORD code-unit argument is at entry ESP+4; EAX returns the same header and `RET4` removes that argument. With ESI/EDI saved, steady ESP is entry ESP-8 and the late `MOV DX,[ESP+0C]` reads that argument's lowWORD. The saved Ghidra prototype is `undefined(void)` and is not used as ABI evidence. There are no x87/SSE operations, local EH states, import additions or read-only literals.

| Native phase | Required Source operation |
| --- | --- |
| `AB81C4`, `AB81CA` | Zero length, then pointer, before the first getter. Abandon any prior backing without returning it, as this constructor does. |
| `AB81D1..AB81E1` | Resolve the genuine raw `00419CC0` through SAME AA8/AA0, call whole `00BD1120` for four bytes, preserve its returned allocation. Native pushes unused DWORD1 then size4; the existing callee does not read that unused home. |
| `AB81E3..AB8202` | Reload CURRENT unsigned length after allocation. At least one copies exactly one WORD from CURRENT data forward to the captured allocation. Zero reads CURRENT data and omits the Native zero-byte `BF7680` call under the existing raw-wide qualification. |
| `AB8205..AB8215` | Reload CURRENT old data after the copy. Only nonnull old data reads CURRENT length and captures DWORD-wrapped `length*2+2`; both operands precede the returning getter. |
| `AB8216..AB821D` | Resolve `00419CC0` again even for large/disabled returns, then call whole `00BD1510` with the captured old pointer/size and SAME live AA4. Its native third stack DWORD is unused by that existing body. |
| `AB8222` | Read the code-unit lowWORD after the conditional return has finished. No early truncation or code-unit-domain guard. |
| `AB8227`, `AB822A`, `AB8230` | Publish the captured allocation, then length1, then the zero WORD terminator at captured allocation+2. |
| `AB8236..AB8242` | Reload CURRENT published data after the terminator store, write the saved lowWORD through that pointer, return the captured header identity. |

Single-WORD copying can alias the header or allocation, so later reads use current backing rather than saved pre-copy fields. The terminator can also alias the published pointer's bytes; the final store consequently uses the fresh header pointer. Zero code units still produce length1. All DWORD inputs and upper16 patterns are accepted. No count cap, range check, overflow/null fallback, mode substitution or FC0 policy is added.

The added raw-byte helpers use the existing ordinary-backing memcpy helpers with MSVC compiler read/write barriers. They do not remove volatile qualification from a volatile object or create a copied canonical header. The by-value DWORD is an ordinary C++ argument object, read at the late Native epoch; this interface does not expose/authenticate the original physical Native argument home or its private stack aliases. The zero-byte CRT call is omitted after Source operand capture. These qualifications preclude Original callable-ABI, exact machine/fault/atomic-access, private FH3/SEH or vendor-call-effect claims.

The returning getter is outside the existing `00BD1510` Source transport's inherited `noexcept` boundary. Getter exceptions therefore propagate through the new overload instead of crossing `NativeStringStorage::release`. The existing raw getter/allocator/return implementations and their Win32/CRT/exception-domain qualifications are unchanged; there is no new assertion that all reached escapes are Native. No failure record or public Native permission is minted. Keep the reached zero/copy/return/publication effects on escape. Do not infer header cleanup, automatically return the captured allocation, roll back, retry, replay destruction or invent a terminal policy.

The caller must retain the actual header/reached backing, the same genuine pool/manager/gate cells and their full lifetime/retirement graph through normal return and partial failure. Existing pool construction/deletion bindings remain mandatory. This overload owns no pool, storage graph, allocation metadata or returning header destructor.

Static evidence is in [the report](../reports/cc16_native_code_unit_raw_pool.json). Each saved live query went through `bsp.py`'s verified client for project `bsp`, `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 LE32, image base `00400000`. The saved batch had Ghidra autostart disabled. It contains the complete live prototype/listing/bytes and unchanged raw exports. The installed PE SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`; the 131-byte region SHA256 is `496b74b0d518f60f6b8e7490288b2516d672ac748a9735d05941e0ef6488333f`. Live bytes equal the disk region; all live instruction addresses/mnemonics/operands equal the saved export.

The report maps all 46 instructions and five Native call sites, pins 19 source/evidence inputs and 26 exact Source lines, and embeds the executable text-only inverse. Removing only the new header declaration/comment block, new CPP overload/private helpers, and `<intrin.h>` include restores both complete old Git LF files exactly to baseline `7743e27d0ec3598a695578b3d1f4e93161501975`. The old header SHA256 is `dbb7e982686b7cb5d5ccfcb20a1d6fcef9d652a2603e458f7f6964c51570cb8c`; old CPP is `82534201568102ad041dfbe16e266d6b306d094b4ba792ecd7026280fdb645bb`. The embedded inverse uses UTF-8/LF with one terminal LF, SHA256 `e29f6433fedc70bd1dd1ff162f901a7c7d1dbcd09c6f4799d37306b7adc94723`.

The new header Git LF SHA256 is `42d4ad6a32778b97583f74efac51853072943c2aa9929df44b0f57189c78b895`; new CPP is `c4e494d1ffdaf20257d8e5a77bebf352925ba3be53d0f595a5e58f6a8dcdc6f4`.

The smallest next caller packet is a direct Glyph call using its SAME acquired ordinary F24 header backing, SAME `Context.strings` and already captured code-unit DWORD, at the current `Calls.code_unit_00ab81c0` site. That packet must preserve whole-Glyph Native epochs, retained custody and private failure-origin routing after the constructor worker releases the Glyph files. No Glyph file, CMake/application/ledger/Ghidra state or other production body changes here. Genuine application pool/lifetime binding, origin permission, official compile/review/common checks, Original ABI/runtime/game/render and whole-frontend activation remain gates. The worker performed only static evidence/inverse checks; no build, executable, test, probe or game run occurred.
