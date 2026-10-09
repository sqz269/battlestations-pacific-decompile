# Shared receiver base constructor readiness

Root reviewed the complete parent and six-instruction child, replayed all 764 Original bytes and 132 operations against the full-hash Original image, and checked the retained capture and Source pins. The parent remains Source-held; the child was admitted separately by the later 509-input primary review; its GPR entry was defined after this readiness capture. Prior Source121 evidence is frozen history following a newer Root build. Existing names/comments were preserved in the locked, saved GPR annotation; Original bytes, prototypes and function count are unchanged.

Packet: `cc12_unit_numbering_shared_receiver_base_constructor`.

The complete normal body at `0095CC90` is understood at the instruction level. The parent remains **Source-held** because the actual base owner, callback pairs, exception cleanup and production lifetime have not been supplied. One concrete next Source packet is ready: the directly passed array-element constructor at `00952640`. This readiness packet implements neither routine and grants no Source, Original-ABI, build, fixture, startup or gameplay credit.

## Evidence and scope

The original image is `I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`, SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Verified CLI queries used project `C:/Users/sqz269/bsp.gpr` and program `/battlestationspacific.exe`; the function count was 64,729.

- `0095CC90..0095CF7E`: 751 bytes, 126 instructions, three physical calls, one normal return and no explicit branch or loop. PE bytes equal live bytes; all PE, live and saved-export instruction starts agree.
- `00952640..0095264C`: 13 bytes, six instructions, no call and one return. PE bytes and all six live instruction starts agree. This address is not currently a Ghidra function start; its fresh local saved instruction listing is explicitly distinguished from a prior function export.
- The worker reviewed every instruction. Local byte, listing and context captures are pinned in the companion report. No prototype, listing, flow, name, comment or saved-project mutation was made.
- `0087B670`, `00BF7CD1`, the other array callbacks, global cells and exception handler `00CA9416` were not opened. The child is the single approved read-only Native extension. Other workers' `00928A00/00928B73` ranges were not touched.

## Entry, normal control flow and storage

Let R be entry ECX and S be entry ESP. The routine reads a raw flag at S+4 and a descriptor word at S+8. It forwards R and the flag to `0087B670`, preserving EBX and ESI around its own body. The parent call sequence expects that callee to consume one stack argument and each array iterator call to consume five. These call-site expectations do not establish the unopened callees' complete contracts.

The routine installs an FS:[0] exception-chain record with handler `00CA9416`, initial state -1 and a saved R local. Its state becomes a full DWORD zero immediately before the first array call and a low-byte value 2 before the second. It restores the previous FS:[0] value on the normal path. No exception map, funclet, reverse destruction or unwind behavior was reviewed; the two state stores alone do not prove those paths.

There are three physical calls:

| Site | Target | Actual call-site contract |
| --- | --- | --- |
| `0095CCB3` | `0087B670` | ECX=R; one pushed flag from S+4 |
| `0095CD23` | `00BF7CD1` | first argument R+394h, stride Ch, count Ch, constructor `00952640`, destructor `00957080` |
| `0095CD5E` | `00BF7CD1` | first argument R+53Ch, stride 18h, count Ah, constructor `004C2D40`, destructor `004BD350` |

The first placement covers R+394h through R+423h. Three direct DWORD zero stores initialize R+424h/+428h/+42Ch. The second placement covers R+53Ch through R+62Bh. The descriptor is read from the original S+8 slot and stored directly at R+538h before the second array call. This body performs no direct descriptor increment; ownership and eventual release remain separate questions.

The body makes 70 explicit receiver write instructions; the report preserves all of them in order, including duplicate writes. The relevant field provenance is:

| Field | Own body evidence | Boundary |
| --- | --- | --- |
| R+0 | `0095CCE7` stores numeric profile `00D1A698` | A profile integer is not a callable C++ vtable or completed owner |
| R+C4h | `0095CE01` stores DWORD 5 | Later derived constructors can overwrite it |
| R+35Ch | No explicit store in this body | No final numbering value can be inferred |
| R+360h | No explicit store in this body | No model allocation, binding or publication is established |
| R+38Ch | `00D1A578` then `00D1A650` before the first array call | Preserve the two writes and their order |
| R+390h | DWORD 1 | Actual direct write |
| R+538h | Descriptor word from S+8 | Actual direct write, ownership unresolved |

Other profile stores are R+10h=`00D1A680`, R+24h=`00D1A678`, R+170h=`00D1A674`, R+1E4h=`00D1A66C` and R+310h=`00D1A654`. They are numeric evidence only. The body does not blanket-zero the owner, and its highest direct write does not establish allocation extent.

The remaining straight-line stores include DWORD -9 at +528h, DWORD -1 at +630h/+6B8h/+530h/+534h, byte 1 at +6C8h and DWORD 1 at +6ECh. Byte writes and DWORD writes must retain their widths. Zero stores and all other destinations are listed in the report.

The float path reads global cells without calling helpers. Their current cell contents were not leased or read, so the report retains the addresses instead of guessing values. It reads `00F87574/78/7C` twice, once for +4C0h/+4C4h/+4C8h and again for +4CCh/+4D0h/+4D4h. Six interleaved FLD/FSTP pairs copy those fields to +4A8h/+4ACh/+4B0h and +4B4h/+4B8h/+4BCh. A proposed Source implementation must account for x87 exceptions and special values; replacing these pairs with assumed bit copies is not justified here. Values read from `00CE4ADC` into +704h/+700h are later overwritten from `00D7A260`.

After the last call there are no additional calls or branches. EAX is set to R at `0095CE98` and survives the remaining stores. ECX contains the old FS:[0] pointer loaded at `0095CE8E`; EBX and ESI are restored, then the routine executes RET 8. The current Ghidra prototype is incomplete (`undefined ...(void)`); the recovered register/stack contract above comes from the instructions.

## Existing Source providers and lifetime boundary

`construct_native_game_array_00bf7cd1` is a game-specific normal iterator over exactly three constructor/destructor pairs. Neither pair passed by this parent is supported. Its Source failure state retains a diagnostic completed prefix; it supplies neither the original CRT reverse cleanup nor these element destructors. Matching its numeric address to the Native call is not sufficient provider binding.

The existing gunnery header records the actual Ch-byte category-header offsets 0/4/8 as count/head/tail. It has no existing `00952640` constructor implementation. The destructor-level Source emits host calls for the +53Ch array, +424h list and +394h array; these abstract host operations are not actual destruction providers. Metadata identifies `00957080` as a thunk to `00955EB0` and `004BD350` as a thunk to `004B7EF0`; those bodies were not reviewed here.

Existing complete normal health/parts and scene initializers accept borrowed views and required providers. They still do not construct or publish the complete production receiver. The preceding LandFort readiness report's actual-owner and application findings remain bounded by their pinned Source reads; this packet does not turn semantic `UnitInstanceState` or `GameUnitSlot` values into Native backing storage. It adds no synthetic owner buffer, vtable integer adapter, callback façade or application fallback.

The full parent therefore still requires: the real `0087B670` base lifetime, both actual callback pairs and the relevant CRT iteration/unwind contract, valid descriptor ownership, reviewed global-value/FP treatment, complete owner storage and publication, and real teardown. Normal ordering is established; complete exceptional and owner lifetime closure is not.

## Concrete next Source packet

The directly passed `00952640` callback is an independent complete constructor leaf:

~~~asm
00952640  mov eax, ecx
00952642  xor ecx, ecx
00952644  mov [eax], ecx
00952646  mov [eax+4], ecx
00952649  mov [eax+8], ecx
0095264C  ret
~~~

It receives a valid actual writable Ch-byte category header in ECX, zeroes all three DWORDs, returns the same receiver in EAX, leaves ECX zero, touches no EDX/nonvolatile register and consumes no stack argument. There is no allocation, global read, call or explicit exception machinery. It can fault on invalid storage; it does not validate a pointer or free a populated list.

A next Source packet can define the real three-field storage and this complete leaf with a reviewed Win32 entry contract, including the observable ECX result. The parent call supplies direct placement evidence for twelve such records at R+394h. The descriptive count/head/tail names derive from the existing gunnery layout evidence; the six instructions alone prove three zeroed words. Admission still requires normal source/build/emitted-object review. Implementing this leaf will not resolve the iterator, destructor or complete parent.

## Snapshot and credit boundary

A fresh replay matched all 121 Root raw source inputs, all 121 worker LF-normalized inputs and all four pinned artifacts from `cc12_native_mlandfort_kind_primary_review.json`. The only worker raw difference is the recorded CRLF/LF report difference. The authority's normal build ran 2026-10-09 20:46:26.766022 through 20:46:43.654990 UTC. Its 37 selected whole objects, 41 positive Core roots and three existing checks are inherited receipt evidence, not a new object review or rerun. Source117 is historical.

The current production startup receipt records a genuine SDK windowed run: three ticks, two Presents, a 640x480 device/window, loop completion and exit code zero. It reached PressStartPoll and had zero mission frames. This is bounded startup smoke and proves neither complete receiver/model publication nor faithful startup or gameplay. This worker did not rerun it.

Only this document and its companion readiness report are committed. No C++/CMake, test, probe, build, ledger or GPR work occurred. Current Source statuses remain unchanged.
