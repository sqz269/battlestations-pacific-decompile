# Renderer capabilities consumed by settings

Packet `orch2_settings_capabilities`; implementation in
`include/bsp/settings_capabilities.hpp` and `src/settings_capabilities.cpp`.
Addresses and names are reconstruction hypotheses, not recovered symbols.
The module owns ordinary C++ vectors and is not a native renderer ABI replacement.

## Established operations

| Address | Original ABI | Recovered operation |
| --- | --- | --- |
| 00b27d80 | ECX renderer, RET | Enumerate and sort the resolution vector |
| 00b1ffd0 | cdecl(two pair pointers), RET | qsort width/height comparator |
| 00b1fff0 | ECX renderer, RET | Return renderer+1Ch resolution vector |
| 00b20000 | ECX renderer, RET | Return renderer+28h AA vector |
| 00b200b0 | ECX renderer, RET | Return renderer+1B48h settings shader ceiling |
| 00b200c0 | ECX ignored, one ignored DWORD, RET4 | Entire body is a no-op |
| 00b295c0 | ECX renderer, stack format, RET4 | Rebuild supported AA sample vector |
| 00b2c8e0 | ECX renderer, RET | Large capability producer; only settings shader fields reconstructed here |

`00b27d80` clears the opaque DWORD at renderer+19DCh, then calls
`IDirect3D9::GetAdapterModeCount(0, D3DFMT_X8R8G8B8)` (format 16h). It calls
`EnumAdapterModes` for each index in ascending order with the same adapter and
format. Only result zero is accepted. Width must be at least 640 and height at
least 480; both comparisons are unsigned. Refresh rate and the returned format
are not retained. `008d46c0` searches the existing stride-8 vector for an equal
width/height pair; absent pairs append. This helper remains externally owned.
The existing resolution vector is **not cleared**. The entire vector is then
sorted by `00b1ffd0`: signed 32-bit width subtraction, or height subtraction
when widths match. The reconstruction preserves subtraction wraparound.

The comparator originally existed as the label `LAB_00b1ffd0` without a Ghidra
function. Its complete body is 29 bytes, through the one-byte RET at
00b1ffech (exclusive end 00b1ffed); another branch returns at 00b1ffe8h.
The integrator defined it and saved the project under the Ghidra write lock;
`reports/settings_capabilities_function_definitions.json` records that work.
The worker performed no Ghidra mutations.

`00b2c8e0` calls `GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps)` at 00b2c91a.
The stack buffer baseline is ESP+1F4h. At 00b2c960, three intervening pushes
mean `[ESP+2CCh]` reads baseline+0CCh, the low WORD of
`D3DCAPS9.PixelShaderVersion`; 00b2c985 stores it at renderer+1B40h, record+28h.
At 00b2ca8c..00b2caa0, an unsigned comparison against 200h produces
`version < 200h ? 1 : 2` in renderer+1B48h. Thus hardware supporting ps_3_0
still exposes **2** as the settings shader ceiling. Record+28h is a packed
pixel-shader version, not a shader constant limit. These are the two shader
inputs used by settings loader 008d8190.

The gather API is explicitly a settings projection: it performs the established
GetDeviceCaps query and those two field calculations. The original routine's
other capability fields, ATOC format probe, adapter-description classification,
format vectors and later queries are outside this module. Native code ignores
GetDeviceCaps failure and then reads an uninitialized stack record. The C++
interface returns false on a negative HRESULT and leaves prior shader fields
unchanged; callers must handle that failure. It does not invent a fallback cap.

`00b295c0` clears the AA vector, appends zero, and queries sample values 2..15
(inclusive) in order. The exact COM call at 00b29629 is:

```
CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL, supplied_format, FALSE,
                          samples, &quality_levels)
```

The result is tested with `TEST EAX,EAX; JNZ`: only zero appends the sample,
including when the returned quality count is zero. The routine excludes values
1 and 16; it does not test the current windowed/fullscreen setting. Quality is
ignored. Native EBP preserves the format while the stack format argument is
reused as quality output; the implementation retains that data flow.
The caller owns the 15h/71h format choice and the timing of AA rebuilding.

## Startup binding

`Win32SettingsCapabilityQueries` borrows a live `IDirect3D9&`; the startup owner
must retain that COM reference. No D3D device is required. Use
`enumerate_settings_resolutions_00b27d80` before
`gather_settings_shader_caps_00b2c8e0`, matching their order in renderer
constructor 00b32410 (calls at 00b3285c and 00b3289b). The module does not
reconstruct that constructor's intervening or unrelated calls. Settings can
then copy the resolution table, read the shader fields, invoke the observed
no-op selector, rebuild AA with the native caller's selected format and copy
the resulting AA table. These operations replace the milestone's unsorted mode
list, major-version shader ceiling and empty AA approximation.

## Validation and limits

MSVC Win32 Release build passed with the repository's warning policy. All eight
native seed byte comparisons passed; both existing CTests passed. One ignored
fixture checks enumeration order, strict HRESULT-zero acceptance, size filters,
deduplication, retained existing pairs, sorting, shader threshold/masking,
explicit query failure and AA sample boundaries/arguments. It executes the
complete original comparator bytes for four differential comparisons, including
the signed wraparound case. The same fixture uses a real D3D9 interface on this
machine: 26 resolution pairs, packed pixel version 0300h, shader ceiling 2, and
AA vector `[0, 2, 4, 8]` for format 15h.

The live query is API validation, not gameplay or complete renderer validation.
No original game files were changed. The full 00b2c8e0 producer, renderer
allocation/container ABI, native binary replacement and game execution remain
outside this packet. Preserve ignored `local/settings_capabilities_fixture/`,
`local/settings_capabilities_build.log`, and shared `exports/bsp/functions/`
entries for the addresses above.
