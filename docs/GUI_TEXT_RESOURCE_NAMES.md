# Text font and shader names

`gui_text_resource_names.cpp` implements the two name setters against the same
`GuiTextLifetime`, stable font descriptors, actual font-resource association and
raw cached-shader slot. It adds no name cache, font table, font resolver callback
or shader owner. It does not rebuild geometry or load a replacement shader.

| Entry | Inclusive body / native ABI | Supported coverage |
| --- | --- | --- |
| `00AB8C30` | `00AB8C30..00AB8CD4`; ECX Text, name-header pointer stack, RET4 at8CD2/length3 | Complete setter sequence in the canonical typed-string domain. |
| `00AB8E70` | `00AB8E70..00AB8ED6`; ECX Text, name-header pointer stack, RET4 at8ED4/length3 | Complete setter sequence in the canonical typed-string domain. |

Both full bodies and all direct caller setup were read. No native bytes remain
unread. The repository wrappers verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` for each analysis batch; Ghidra was read-only.
Names are descriptive hypotheses. These interfaces use the existing canonical
`std::string` fields rather than inventing parallel NativeString members. Their
native pool allocation callbacks, wrapper layout and SEH behavior are excluded;
successful allocation and stable null-free DWORD-length names are required.

## Distinct invalidation rules

Font setter `00AB8C30` captures Text in EBX, input header in EDI and stored
font-name header+1C8 in ESI. It calls `00435C40`: recorded lengths must match,
zero lengths compare equal, and nonempty equality uses current CRT `_stricmp`.
The implementation calls the existing raw-header comparator with temporary
nonowning views of the same string bytes. No normalized or copied name index
is introduced. Equality returns before lookup, font/scalar stores and shader
release. The C++ bool return merely reports changed name; native callers do
not establish a useful return-value contract.

On change, native assignment occurs before `007371D0` and `00AC3570`. The
getter may run lifetime-manager callbacks, so lookup uses the **live stored
font_name after the getter**, not the original incoming name. The lookup result
is published to Text+108 before its current alpha scalar is read and stored at
Text+1D4. A genuine name miss publishes null and reads the live D7A24C constant
(verified bytes `00 00 80 3F`). Missing actual font ownership is an error, not
a manufactured name miss or default font.

Shader setter `00AB8E70` skips copying only for exact source/header identity.
Equal spelling is still an assignment and invalidation path. Even exact member
self-alias proceeds to the shader invalidation tail. The same `shader_name`
member is consumed later by the existing shader-selection routine.

Both tails capture the **current** raw shader+1EC after preceding work. Null
skips the release. Nonnull performs the existing atomic raw+04 decrement and
actual terminal-owner callback, then clears the live slot **after** that callback
(`00AB8CC5` / `00AB8EC7`). A callback's replacement publication is overwritten,
as native does; there is no early clear, second release or extra retain. The
existing `has_cached_shader` projection is kept consistent, but never gates
the actual release. Captured owners and Text must survive callbacks.

## Registry and field producer

The existing `get_font_registry_007371d0` operates on the supplied live F8BF44
publication and real `SingletonLifetimeManager`: double-check under its lock,
construct if needed, publish before lifetime registration, then return the
current publication. This module calls that concrete getter, not a generic
success callback. Its native full body was checked through `0073728C`.

The existing `FontRegistryStartup::find_font_00ac3570` searches its owned
descriptors in insertion order, compares equal lengths/current CRT equality,
and returns the first stable owner. The native list body was checked through
RET4 at `00AC3600` (end3602). Native intrusive-list ABI and corrupt iterator
behavior remain outside the typed registry's contract. No lookup of the
compatibility-vector descriptor copies is used.

Every nonnull result must already resolve by that **same descriptor address**
in `NativeFontResourceOwners`, with the same actual render-resource domain.
The published Text pointer is that descriptor identity, which the concrete
builders subsequently resolve to the sole actual FontData/image owner. The
registry's semantic `FontResources` textures are not read or converted here.
Registry/descriptors and actual font resources must outlive all borrowing Texts;
populating this association is a prerequisite, not a new loader in this packet.

Producer evidence distinguishes two floats: the Fonts Lua key `scale_ratio`
at D5CAD4 and `alphatexturescale` at D5CAC0 get independent FLD1 defaults.
The latter is spilled at `00AC3A65` to ESP+48, reloaded at `00AC3B22` and passed
as constructor argument3 at `00AC3B4D`. Full `00AD55C0..00AD5690` constructs
the font; `00AD5654` stores that argument at font+1C, while +18 gets scale_ratio.
Thus the setter copies the same descriptor's `alpha_texture_scale`, not
`scale_ratio` or font height. Text+108 remains borrowed, with no font retain.

## Every caller and remaining scope

The four direct sites are:

| Site | Native setter | True containing function | Input/receiver |
| --- | --- | --- | --- |
| `00ABB69A` | `00AB8C30` | `00ABB630..00ABBE44` | Property Font result; EDI is entry Text, ESI property reader. |
| `00AB9DC9` | `00AB8C30` | `00AB98F0..00AB9FC2` | Template Text via original Text+1B0, name+1C8; ESI newly constructed child. |
| `00AB9E1F` | `00AB8E70` | `00AB98F0..00AB9FC2` | Temporary `GuiFontBilinear.mshd`; same new child. |
| `0059512A` | `00AB8E70` | `00594BF0..005966CB` | Same shader literal; receiver from screen+2B8. |

Complete filtered register provenance was inspected across all three caller
listings. AB98F0 saves entry Text at ESP+10 (`00AB990E`) and restores EDI after
stream-address work; ESI becomes the constructor return at `00AB9D54`.
ABB630 captures EDI=entry ECX atB651. In594BF0, ESI=entry ECX at594C14 and
the result of AAB4C0 is published to+2B8 at5950E3 before reloading that receiver
at595114. Callee RET4 and each caller's single pushed header establish arity.
The literal at CEFD78 was verified directly. No caller is attributed merely
to the nearest earlier index entry.

Strict MSVC Win32 C++17 compilation passed with `/W4 /WX /EHsc /permissive-`
and the current integration header overlay. The report records exact call-site
checks. No tests were added. These new setters have no executable/factory wiring
yet; original binary ABI, native string allocation callbacks and game/render
equivalence are not claimed. The integrator owns source registration/build.
