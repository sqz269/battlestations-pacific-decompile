# Installed shader scripts through Lua

The existing D3D9 probe now executes the installed debug shader and dummy combiner
instead of supplying their shader-code strings and input/interpolator fields by
hand. It uses stock Lua5.1.1 core/auxiliary/base code from the checksum-pinned
[upstream archive](https://www.lua.org/ftp/), compiled as C. The original contains
a Lua5.1.1 identification string; this does not prove it lacks local patches.
The adapter uses its own C API and lua_State, not original game addresses or ABI.
Dependency details and the upstream license are in `third_party/`.

`load_shader_lua_code` creates a fresh state per descriptor, opens base only,
sets PC=true and supplied X360COMP/optional REGION, registers DoFile, and evaluates
fundamentals followed by the descriptor. DoFile resolves and runs includes in the
same state. Chunk return values are discarded at the adapter boundary; globals
persist. Protected calls return errors rather than reproduce native panic/SEH.
Resolver failures, missing Shader tables and malformed records are explicit host
errors. The resolver must supply supported files; native basename registration,
additional-content overlays and cached fundamentals provenance remain unported.

The diagnostic resolver derives the installation root from the supplied atlas
DDS path. It maps only fundamentals.lua, dx9_lua.inc, common/debugshader.shfx and
lights/dummy.shfx. It does not implement a general asset resolver or infer that
all combiners reside in lights. The selected dummy combiner remains an explicit
fixture choice until native Combiners table conversion and loading are recovered.

The adapter extracts Constants, VS, PS, VSVersion and PSVersion only when their
Lua type is exactly string. Code strings retain their byte lengths; absent code
becomes empty, absent profiles go through the recovered generation3 defaults.
VertexInput and Interpolators use the actual lua_next API for outer and inner
iteration. Inner numeric keys qualify after float32/integer comparison; their
accepted ordinal determines field role. Coercing versus exact-number conversions,
name fallback/truncation, default index and zero mask follow the inspected helpers.
See `SHADER_FIELD_TABLES.md`. Nontable outer fields leave output unchanged;
malformed inner values, fewer than four accepted fields or out-of-range numeric
conversions are rejected. These replace undefined/unsafe native cases, not
established native errors. Stock hash iteration and unusual FP cases remain
unvalidated against the embedded runtime.

The installed dense debug tables produced two vertex inputs and one interpolator.
Each descriptor executed three chunks (fundamentals, include, descriptor). The
evaluated code/fields fed both full source generators, VS3/PS3 compilation, pixel
disassembly usage filtering and the camera-edit draw. COLOR0=15, centerFF407FBF,
outsideFF000000 and state restore passed, as did the Win32 build, two existing
CTests and all existing native matrix fixtures. Asset hashes and results are in
`reports/shader_lua_adapter.json`. No new test target was added.

Without an atlas path the D3D9 probe explicitly skips shader asset checks. With
the documented installed path, it exercises the complete adapter route above.
The reconstructed-function count does not include upstream Lua or claim a full
native shader loader: this integration is recorded as a descriptor fragment.
Remaining work includes descriptor flags, sampler/constants metadata, render
states, combiner selection/ownership, full file resolution and material/startup
integration. The game rebuild is not runnable yet.
