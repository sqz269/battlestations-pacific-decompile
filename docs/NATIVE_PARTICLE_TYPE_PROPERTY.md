# Native PARTICLE common properties and texture loading

Addresses: 00b015c0, 00b01350, 00af4360, 00af3a20, 00af3b50, 00b00880, 00b00ee0, 00b00f30, 00af3750, 00af37d0, 00af38b0, 00af3960, 00aefb20, 00aee0f0, 004cad40, 0043bbf0, 0043e9a0

The module reconstructs 17 complete native bodies through explicit real service boundaries. It uses the same native pooled-string storage and particle record allocator as `native_particle_type_base.hpp`, the actual atlas manager/item identity, and the named D3DX half-conversion export. Interfaces are new C++; original FH3 and invalid-input fault behavior are not binary replacement claims.

| Address | Routine | Native ABI | Coverage |
|---|---|---|---|
| `00b015c0` | `load_native_particle_type_property_00b015c0` | ECX actual definition; stack pooled4h suffix; RET4/AL recognized | complete |
| `00b01350` | `load_native_particle_type_texture_00b01350` | ECX actual definition; stack nullable filename; RET4/AL status | complete |
| `00af4360` | `find_native_particle_layer_00af4360` | ECX layer owner; BY-VALUE stack length,data; RET8/EAX index, consumes string | complete |
| `00af3a20` | `count_native_particle_texture_frames_00af3a20` | ECX filename; EDX unused; RET/EAX count | complete |
| `00af3b50` | `construct_native_particle_texture_frame_name_00af3b50` | ECX output8h, EDX filename, stack index; RET4/EAX output | complete |
| `00b00880` | `construct_native_particle_uv_record_00b00880` | ECX actual1Ch output; stack uv_min,uv_max; RET8/EAX output | complete |
| `00b00ee0` | `append_native_particle_type_record_00b00ee0` | ECX descriptor; stack actual1Ch record; RET4 | complete |
| `00b00f30` | `clear_native_particle_type_records_00b00f30` | ECX descriptor; RET; retains allocation | complete |
| `00af3750` | `has_native_particle_frame_suffix_00af3750` | EDI filename; RET/AL; custom register storage required | complete |
| `00af37d0` | `construct_native_particle_texture_stem_00af37d0` | ECX output8h, EDX filename; RET/EAX output | complete |
| `00af38b0` | `construct_native_particle_texture_extension_00af38b0` | ECX output8h, EDX filename; RET/EAX output | complete |
| `00af3960` | `construct_native_particle_texture_prefix_00af3960` | ESI output8h, EAX stem; RET/EAX output; custom register storage required | complete |
| `00aefb20` | `find_native_particle_atlas_item_00aefb20` | ECX actual atlas manager; stack filename; RET4/EAX borrowed raw30h item | complete |
| `00aee0f0` | `matches_native_particle_atlas_item_00aee0f0` | ECX actual raw30h item; stack query8h; RET4/AL match | complete |
| `004cad40` | `replace_native_particle_string_substrings_004cad40` | ECX actual8h string; stack search8h,replacement8h,count; RET0C | complete |
| `0043bbf0` | `resize_native_particle_string_fill_0043bbf0` | ECX actual8h string; stack requested length,signed low-byte fill; RET8 | complete |
| `0043e9a0` | `matches_native_particle_string_at_0043e9a0` | ECX candidate8h; stack pattern8h,start; RET8/AL; EDX is scratch | complete |

`AF3750` requires EDI filename and `AF3960` requires ESI output/EAX stem. Their report prototypes are descriptive custom-storage declarations; the integrator must not apply them as ordinary stack arguments. All thiscall prototypes list stack arguments only. Existing `BSP_TextureAtlas_FindItem`, `BSP_String_ReplaceSubstrings`, and `BSP_NativeString_MatchesAtOffset` names are preserved.

## Property behavior

B015C0 repeatedly obtains token0 from the actual four-byte pooled suffix until it recognizes Texture, Layer, Shader, Stops, RandomRotationDirection, EmitLight, Distort, AnimOnOff, AnimRndStartFrame, AnimRandomPlay, AnimLoop, or AnimFade. Boolean properties parse token1 through CRT `atol` and store one byte for signed result >0. Offsets are respectively 28,29,64,65,4C,60,61,62,63. Unknown keys return false without property writes. A recognized Texture returns true even if its empty/missing filename makes B01350 return false. Each temporary retains the original pooled allocation/release order.

Layer copies token1 into a second pooled text, constructs an eight-byte native string passed by value to AF4360, and writes its result at definition+74. The layer-owner address comes from definition+14, not +18. AF4360 scans inline pointers at owner+34 using signed count+54, compares each entry name at+8/+C with exact length and CRT case folding, consumes its by-value string buffer, and returns zero both for no match and the first entry. The base producer B01150 establishes the original word14 field.

Shader captures the current definition vtable+10 and invokes the required real target with token1 bytes. Reviewed Sprite/Axial/Floating targets B089E0/B06210/B07C80 compare an actual native string with Additive and write DWORD+7C. Object AF80E0 and Tracer B0A040 have installed/live bytes `C2 04 00` (RET4), but no created Ghidra functions; they remain service evidence, not worker mutations. Base slot10 is the existing CRT purecall. The Object model+20 callback belongs to the independent Object parser packet.

## Actual atlas and frame names

The raw lookup AEFB20 reads manager pointer array+4 and signed count+8. It returns the actual current item pointer, with no new owner or semantic TextureAtlasItem conversion. AEE4D0 produces descriptor string+0, borrowed texture+8 and name string+C/+10 in the 30h item; AEE620 writes UV floats14/18/1C/20. Its scaled packed24..2E data is distinct from the particle half values.

Lookup preserves last-dot removal excluding index0, pooled backslash replacement, repeated leading-slash removal, ASCII lowercasing, ordered whole-name case-insensitive OR slash-suffix case-sensitive matches, and basename fallback. String reconstruction composes existing actual-header resize/substring/concat/copy functions. Generic 004CAD40 retains count wrap, replacement-length continuation, current headers after allocation/release, and the native four-temporary release order. Original fixed buffers and unsafe empty-after-normalization inputs have no invented success path.

AF3A20 strips the extension and checks for three consecutive trailing zeroes strictly after index0. If present it formats prefix+`%03d` and counts contiguous raw atlas hits starting at zero. AF3960 itself counts three zeroes backward without requiring adjacency. AF3B50 builds prefix+formatted signed index+extension in native8h pooled headers. If no dot exists after index0, AF38B0 returns the whole filename as its extension; this quirk is retained. Current null-string fallback bytes E17BF0/F8C2C1/F8D37C/F8D390 are explicit borrowed bindings.

## Actual records and resource boundary

B01350 clears count in the actual definition+68 descriptor, retaining allocation. It appends one 1Ch UV record from the atlas hit or from (0,0,current1,current1) after the current renderer vtable64 service. The renderer result is still dereferenced and decremented, with current virtual00 called exactly when the reference becomes zero; no successful null-resource fallback is added. It counts frames, reserves the full reported count, appends later atlas frames until a miss, then applies two signed minima: +54=min(count-1,old54), +50=min(new54,old50). Capacity and current count remain distinct.

B00880 performs four ordered x87 FLD/FSTP copies before the real D3DX conversion. This quiets signaling NaNs; B01350 later frames use bit-preserving SSE stores before conversion. The final DWORD at record18 is never written by either producer. First/later native stack words are separately supplied and verified with a native trampoline. B00EE0 uses forward REP MOVSD semantics, seven DWORDs, with native wrapping capacity growth and descriptor rereads after reserve. B00F30 retains the native negative-capacity reserve path and signed count drain.

## Verification and limits

Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and installed PE were verified through the repository wrappers before capture. All 17 inclusive ranges (5888 bytes) match live Ghidra bytes and the installed image, with no PE relocation directory. Capstone decoded every instruction; only decoded call/absolute operands were rebound. No raw byte scanning, Ghidra naming/type/body creation, comments, or save was performed. Existing function bodies already cover all owned final instructions; no body repairs are requested.

Strict isolated MSVC Win32 compile and native differential passed with process exit0: `/MD /W4 /WX /fp:strict /O2`, embedded manifest, explicit main return0, and the real SysWOW64 d3dx9_40 export. Comparisons cover 84 filename cases,6 suffix cases,8 raw atlas cases,5 frame counts,5 x87 UV values (including signaling NaN),8 texture loads and15 common properties. Raw bytes, AL/EAX results, pool/array allocation-release order, and renderer/deleting/shader call traces agree. Commands and executable/source/library hashes are in the report and scratch `C:/Users/sqz269/bsp-aq-properties`.

`verify_report_calls.py` validates the report's exact decoded address/native rows. Full primary build/current integrated library replay is pending integration; the worker adds no CMake entries, shared metadata or permanent tests. Fixtures reach only supported normal allocations and services, not original FH3/allocation-failure behavior, arbitrary malformed strings, fixed-buffer overflow, real renderer resource internals, or gameplay. The game executable does not currently reach this imported raw-owner fixture; no frame-time cause or visual parity claim is made.
