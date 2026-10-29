#include "bsp/game_native_gui_render_application.hpp"
#include "bsp/game_native_raw_default_text_owner.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_fonts.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_gui_effect_application.hpp"
#include "bsp/game_native_gui_group_application.hpp"
#include "bsp/game_native_gui_sampler_application.hpp"
#include "bsp/game_native_gui_node_application.hpp"
#include "bsp/game_native_gui_text_process.hpp"
#include "bsp/game_native_gui_text_state.hpp"
#include "bsp/game_native_material_process.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_text_application.hpp"
#include "bsp/game_hosts_text.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/camera_configuration.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/gui_widget_clip_refresh.hpp"
#include "bsp/native_camera_reference.hpp"
#include "bsp/native_camera_view_set.hpp"
#include "bsp/native_camera_frustum.hpp"
#include "bsp/native_cockpit_helper_construction.hpp"
#include "bsp/native_gui_scene_storage.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/native_mesh_owner.hpp"
#include "bsp/native_mesh_section.hpp"
#include "bsp/native_node_scene_attachment.hpp"
#include "bsp/native_node_root_propagation.hpp"
#include "bsp/native_shader_binary_cache.hpp"
#include "bsp/native_weak_owner.hpp"
#include "bsp/post_effects_cc13.hpp"
#include "bsp/system_fog_owner.hpp"
#include "bsp/native_renderer_indexed_draw.hpp"
#include <d3d9.h>
#include <d3dx9shader.h>
#include <cstring>
#include <cstdlib>
#include <cwctype>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <new>
#include <new.h>
#include <string>
#include <utility>

// Host Win32 import-cell alias only: no storage or replacement function.
// The SDK's query declaration lacks dllimport, so its named function address
// identifies an EXE thunk. This alias reads the actual loader-resolved slot.
extern "C" { extern _PNH (__cdecl * const volatile bsp_cc16_query_new_handler_import)(); }
#pragma comment(linker, "/alternatename:_bsp_cc16_query_new_handler_import=__imp___query_new_handler")
extern "C" { extern std::new_handler (__cdecl * const volatile bsp_cc16_std_get_new_handler_import)() noexcept; }
#pragma comment(linker, "/alternatename:_bsp_cc16_std_get_new_handler_import=__imp_?get_new_handler@std@@YAP6AXXZXZ")

namespace bsp::game {
namespace {
// Private qualification route over the existing GUI Text opt-in. Keep OFF in
// main until broader raw Text content/rendering replaces the legacy consumer.
// The ON candidate exercises one unpublished DEFAULT lifecycle without a new
// environment option, scene/overlay, cache owner or public test-only interface.
constexpr bool kRawDefaultTextQualificationCc16Bound = false;
// Separate private fixture for the genuine primary Model prefix. Its owner
// selects the receipt-authorized lifetime route; it adds no content or layout.
constexpr bool kRawPrimaryPrefixQualificationCc16Bound = false;
// Explicit AB7700 body fixture after the genuine prefix, with ctor bounds0.
constexpr bool kRawPrefixGeometryQualificationCc16Bound = false;
// Genuine Font producer/owner metadata only, over the same empty geometry
// fixture. FontShader remains FRESH; this does not assign Text108 or call it.
constexpr bool kRawFontMetadataQualificationCc16Bound = false;
// Closed real Font/credit work fixture. No public option; keep OFF in main.
constexpr bool kRawFontWorkQualificationCc16Bound = true;
constexpr auto kRawFontWorkChoiceCc16Bound = GameNativeRawDefaultTextOwner::FontWork::invalidate_shader;
static_assert(static_cast<unsigned>(kRawDefaultTextQualificationCc16Bound) +
    static_cast<unsigned>(kRawPrimaryPrefixQualificationCc16Bound) +
    static_cast<unsigned>(kRawPrefixGeometryQualificationCc16Bound) +
    static_cast<unsigned>(kRawFontMetadataQualificationCc16Bound) +
    static_cast<unsigned>(kRawFontWorkQualificationCc16Bound) <= 1u);
using Word = std::uint32_t;
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
template<class T> const volatile T& cell(GameNativeReadOnlyData& data, Word address) {
    return *static_cast<const volatile T*>(data.data_at(address, sizeof(T)));
}
const volatile Word* table(GameNativeReadOnlyData& data, Word address, std::size_t bytes) {
    return static_cast<const volatile Word*>(data.data_at(address, bytes));
}
template<class T> T read(const void* value, std::size_t offset = 0) {
    T result;
    std::memcpy(&result, static_cast<const std::byte*>(value) + offset, sizeof result);
    return result;
}
bool gui_trace_requested() noexcept {
    char value[2]{};
    return GetEnvironmentVariableA("BSP_NATIVE_GUI_TRACE", value, sizeof value) == 1 && value[0] == '1';
}
template<class T> struct DiagnosticComCredit {
    T* value{};
    ~DiagnosticComCredit() { if (value) value->Release(); }
};
std::string gui_trace_directory() {
    char directory[MAX_PATH]{};
    const DWORD length = GetEnvironmentVariableA("BSP_NATIVE_GUI_TRACE_DIR", directory, sizeof directory);
    return length && length < sizeof directory ? std::string(directory, length) : std::string{};
}
template<class T> void trace_gui_shader(GameHostLog& log, T* shader, const char* stage, const std::string& directory) {
    if (!shader || directory.empty()) return;
    UINT size{};
    const HRESULT sized = shader->GetFunction(nullptr, &size);
    if (FAILED(sized) || !size || size % sizeof(DWORD)) return;
    std::vector<DWORD> code(size / sizeof(DWORD));
    const HRESULT copied = shader->GetFunction(code.data(), &size);
    if (FAILED(copied)) return;
    const std::string base = directory + "/gui_first_" + stage;
    std::ofstream binary(base + ".bin", std::ios::binary);
    binary.write(reinterpret_cast<const char*>(code.data()), size);
    auto* module = GetModuleHandleW(L"d3dx9_40.dll");
    decltype(&D3DXDisassembleShader) disassemble{};
    auto address = module ? GetProcAddress(module, "D3DXDisassembleShader") : nullptr;
    std::memcpy(&disassemble, &address, sizeof disassemble);
    DiagnosticComCredit<ID3DXBuffer> assembly;
    const HRESULT result = disassemble ? disassemble(code.data(), FALSE, nullptr, &assembly.value) : E_POINTER;
    if (SUCCEEDED(result) && assembly.value) {
        std::ofstream output(base + ".asm", std::ios::binary);
        output.write(static_cast<const char*>(assembly.value->GetBufferPointer()), assembly.value->GetBufferSize());
    }
    log.notef("native_GUI_trace shader=%s bytes=%u disassemble=%08lx path=%s", stage, size,
        static_cast<unsigned long>(result), base.c_str());
}
void trace_gui_constants(GameHostLog& log, IDirect3DDevice9* device, const std::string& directory) {
    if (directory.empty()) return;
    std::array<float, 256 * 4> vertex{};
    std::array<float, 224 * 4> pixel{};
    const HRESULT vs = device->GetVertexShaderConstantF(0, vertex.data(), 256);
    const HRESULT ps = device->GetPixelShaderConstantF(0, pixel.data(), 224);
    std::ofstream output(directory + "/gui_first_constants.txt");
    if (SUCCEEDED(vs)) for (unsigned i = 0; i < 256; ++i)
        output << "VS " << i << ' ' << vertex[i*4] << ' ' << vertex[i*4+1] << ' ' << vertex[i*4+2] << ' ' << vertex[i*4+3] << '\n';
    if (SUCCEEDED(ps)) for (unsigned i = 0; i < 224; ++i)
        output << "PS " << i << ' ' << pixel[i*4] << ' ' << pixel[i*4+1] << ' ' << pixel[i*4+2] << ' ' << pixel[i*4+3] << '\n';
    log.notef("native_GUI_trace constants VS=%08lx PS=%08lx path=%s/gui_first_constants.txt",
        static_cast<unsigned long>(vs), static_cast<unsigned long>(ps), directory.c_str());
}
// Stock Get* observations only. Every temporary COM credit is balanced; this
// does not bind a replacement target, shader, stream, texture or render state.
void trace_gui_device(GameHostLog& log, void* actual_device) {
    auto* device = static_cast<IDirect3DDevice9*>(actual_device);
    DiagnosticComCredit<IDirect3DSurface9> target, back;
    const HRESULT rt = device->GetRenderTarget(0, &target.value);
    const HRESULT bb = device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back.value);
    D3DSURFACE_DESC desc{};
    const HRESULT described = target.value ? target.value->GetDesc(&desc) : E_POINTER;
    D3DVIEWPORT9 viewport{};
    const HRESULT vp = device->GetViewport(&viewport);
    log.notef("native_GUI_trace target=%p back=%p getRT=%08lx getBack=%08lx desc=%08lx size=%ux%u format=%08x viewport_result=%08lx viewport=%lu,%lu,%lu,%lu,%g,%g",
        target.value, back.value, static_cast<unsigned long>(rt), static_cast<unsigned long>(bb),
        static_cast<unsigned long>(described), desc.Width, desc.Height, static_cast<unsigned>(desc.Format),
        static_cast<unsigned long>(vp), viewport.X, viewport.Y, viewport.Width, viewport.Height, viewport.MinZ, viewport.MaxZ);
    DiagnosticComCredit<IDirect3DVertexBuffer9> vertices;
    DiagnosticComCredit<IDirect3DIndexBuffer9> indices;
    UINT offset{}, stride{};
    const HRESULT vb = device->GetStreamSource(0, &vertices.value, &offset, &stride);
    const HRESULT ib = device->GetIndices(&indices.value);
    D3DVERTEXBUFFER_DESC vertex_desc{};
    D3DINDEXBUFFER_DESC index_desc{};
    const HRESULT vd = vertices.value ? vertices.value->GetDesc(&vertex_desc) : E_POINTER;
    const HRESULT id = indices.value ? indices.value->GetDesc(&index_desc) : E_POINTER;
    log.notef("native_GUI_trace VB=%p HRESULT=%08lx offset=%u stride=%u desc=%08lx bytes=%u usage=%08lx pool=%u IB=%p HRESULT=%08lx desc=%08lx bytes=%u format=%08x",
        vertices.value, static_cast<unsigned long>(vb), offset, stride, static_cast<unsigned long>(vd), vertex_desc.Size,
        vertex_desc.Usage, static_cast<unsigned>(vertex_desc.Pool), indices.value, static_cast<unsigned long>(ib),
        static_cast<unsigned long>(id), index_desc.Size, static_cast<unsigned>(index_desc.Format));
    DiagnosticComCredit<IDirect3DVertexShader9> vertex_shader;
    DiagnosticComCredit<IDirect3DPixelShader9> pixel_shader;
    DiagnosticComCredit<IDirect3DVertexDeclaration9> declaration;
    DWORD fvf{};
    const HRESULT vs = device->GetVertexShader(&vertex_shader.value);
    const HRESULT ps = device->GetPixelShader(&pixel_shader.value);
    const HRESULT decl = device->GetVertexDeclaration(&declaration.value);
    const HRESULT fvfr = device->GetFVF(&fvf);
    log.notef("native_GUI_trace VS=%p HRESULT=%08lx PS=%p HRESULT=%08lx declaration=%p HRESULT=%08lx FVF=%08lx HRESULT=%08lx",
        vertex_shader.value, static_cast<unsigned long>(vs), pixel_shader.value, static_cast<unsigned long>(ps),
        declaration.value, static_cast<unsigned long>(decl), fvf, static_cast<unsigned long>(fvfr));
    const auto directory = gui_trace_directory();
    trace_gui_shader(log, vertex_shader.value, "vs", directory);
    trace_gui_shader(log, pixel_shader.value, "ps", directory);
    trace_gui_constants(log, device, directory);
    if (declaration.value) {
        D3DVERTEXELEMENT9 elements[MAXD3DDECLLENGTH + 1]{};
        UINT count = MAXD3DDECLLENGTH + 1;
        const HRESULT result = declaration.value->GetDeclaration(elements, &count);
        if (SUCCEEDED(result)) for (UINT i = 0; i < count; ++i)
            log.notef("native_GUI_trace declaration i=%u stream=%u offset=%u type=%u method=%u usage=%u index=%u", i,
                elements[i].Stream, elements[i].Offset, elements[i].Type, elements[i].Method, elements[i].Usage, elements[i].UsageIndex);
    }
    // Read the existing HAL data after the original draw. No replacement
    // geometry or write lock is used; the original declaration decodes it.
    if (!directory.empty() && vertices.value && stride && vertex_desc.Size >= offset + stride * 4) {
        void* data{};
        const HRESULT result = vertices.value->Lock(offset, stride * 4, &data, D3DLOCK_READONLY);
        if (SUCCEEDED(result)) {
            struct Unlock { IDirect3DVertexBuffer9* buffer; ~Unlock() { buffer->Unlock(); } } unlock{vertices.value};
            std::ofstream output(directory + "/gui_first_vertices.bin", std::ios::binary);
            output.write(static_cast<const char*>(data), stride * 4);
        }
        log.notef("native_GUI_trace vertex_read HRESULT=%08lx", static_cast<unsigned long>(result));
    }
    if (!directory.empty() && indices.value && index_desc.Size >= 12) {
        void* data{};
        const HRESULT result = indices.value->Lock(0, 12, &data, D3DLOCK_READONLY);
        if (SUCCEEDED(result)) {
            struct Unlock { IDirect3DIndexBuffer9* buffer; ~Unlock() { buffer->Unlock(); } } unlock{indices.value};
            std::ofstream output(directory + "/gui_first_indices.bin", std::ios::binary);
            output.write(static_cast<const char*>(data), 12);
        }
        log.notef("native_GUI_trace index_read HRESULT=%08lx", static_cast<unsigned long>(result));
    }
    const std::pair<D3DRENDERSTATETYPE, const char*> states[] = {
        {D3DRS_COLORWRITEENABLE, "colorwrite"}, {D3DRS_ZENABLE, "z"}, {D3DRS_ZWRITEENABLE, "zwrite"},
        {D3DRS_ZFUNC, "zfunc"}, {D3DRS_CULLMODE, "cull"}, {D3DRS_ALPHATESTENABLE, "alphatest"},
        {D3DRS_ALPHABLENDENABLE, "alphablend"}, {D3DRS_SRCBLEND, "srcblend"}, {D3DRS_DESTBLEND, "destblend"},
        {D3DRS_SCISSORTESTENABLE, "scissor"}};
    for (const auto& state : states) {
        DWORD value{};
        const HRESULT result = device->GetRenderState(state.first, &value);
        log.notef("native_GUI_trace state=%s value=%08lx HRESULT=%08lx", state.second, value, static_cast<unsigned long>(result));
    }
    for (DWORD slot = 0; slot != 2; ++slot) {
        DiagnosticComCredit<IDirect3DBaseTexture9> texture;
        const HRESULT result = device->GetTexture(slot, &texture.value);
        D3DSURFACE_DESC texture_desc{};
        const HRESULT texture_result = texture.value && texture.value->GetType() == D3DRTYPE_TEXTURE ?
            static_cast<IDirect3DTexture9*>(texture.value)->GetLevelDesc(0, &texture_desc) : E_POINTER;
        log.notef("native_GUI_trace texture=%lu COM=%p HRESULT=%08lx desc=%08lx size=%ux%u format=%08x", slot,
            texture.value, static_cast<unsigned long>(result), static_cast<unsigned long>(texture_result),
            texture_desc.Width, texture_desc.Height, static_cast<unsigned>(texture_desc.Format));
    }
}
void trace_gui_indexed_draw(const NativeRendererIndexedDrawObservation& draw, void* context) noexcept {
    try {
        auto& log = *static_cast<GameHostLog*>(context);
        const char* terminal = "unknown";
        switch (draw.terminal) {
        case NativeRendererIndexedDrawTerminal::skippedInhibited: terminal = "inhibited"; break;
        case NativeRendererIndexedDrawTerminal::skippedLost: terminal = "lost"; break;
        case NativeRendererIndexedDrawTerminal::skippedEmpty: terminal = "empty"; break;
        case NativeRendererIndexedDrawTerminal::skippedStreamCount: terminal = "stream_count"; break;
        case NativeRendererIndexedDrawTerminal::deviceCalled: terminal = "device_called"; break;
        }
        log.notef("native_GUI_trace draw terminal=%s renderer=%p inhibit=%u lost=%u primitive=%u base=%d minimum=%u vertices=%u start=%u primitives=%u logical=%p count=%u tag=%08x device=%p HRESULT=%08x",
            terminal, draw.actual_renderer, draw.inhibit, static_cast<unsigned>(draw.lost), draw.primitive_type,
            draw.base_vertex, draw.minimum_vertex, draw.vertex_count, draw.start_index, draw.primitive_count,
            draw.logical_stream, draw.logical_count, draw.logical_tag, draw.actual_device, static_cast<unsigned>(draw.returned_hresult));
        if (draw.terminal == NativeRendererIndexedDrawTerminal::deviceCalled)
            trace_gui_device(log, draw.actual_device);
    } catch (...) {
        // Diagnostic logging cannot change the original ignored-HRESULT path.
    }
}
std::shared_ptr<const GuiTable> vector_table(std::initializer_list<double> values) {
    auto result = std::make_shared<GuiTable>();
    for (double value : values) result->array.emplace_back(value);
    return result;
}
// The standalone bring-up submits literal, non-localized text. Context tokens
// require the application's current Lua owner and remain a rejected boundary;
// choosing an unrelated page VM here would fabricate that owner.
struct LiteralTextLocale final : LocaleTextRuntimeHost {
    std::string context_string_00b692c0(const std::string&) override {
        throw std::logic_error("standalone native Text has no current Lua-context binding");
    }
    char16_t crt_uppercase_00c0391c(char16_t value) override {
        return static_cast<char16_t>(std::towupper(static_cast<std::wint_t>(value)));
    }
};
// Every identity comes from the mission frame's existing command graph. The
// renderer validates the active post-world scope and retires the world command
// before accepting this one overlay; AA3E00 still supplies the consumed label.
class MissionGuiQueue final : public GameNativeGuiQueue {
public:
    explicit MissionGuiQueue(GameNativeRendererApplication& renderer) : renderer_(renderer) {}
    void* queue_004c11f0() override { return renderer_.mission_overlay_queue(); }
    void* pass_context_00b0d0d0() override { return renderer_.mission_overlay_pass_context(); }
    void submit_scene_00b1f4d0(void* queue, void* scene, void* camera,
        void* context, void* label, bool& entered) override {
        renderer_.submit_mission_overlay_scene(queue, scene, camera, context, label, entered);
    }
private:
    GameNativeRendererApplication& renderer_;
};
} // namespace

struct GameNativeGuiRenderApplication::Impl {
    enum class Phase { prepared, preparing, ready, submitting, retiring, retired, closed, failed };
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameVfsHost& files;
    GameSingletonHost& singletons;
    GameNativeReadOnlyData& data;
    GameInputRuntime& input;
    GameFontHost& fonts;
    GameLocaleHost& locale;
    const bool& sse2;
    GameNativeVfsRawServices raw;
    GameNativeGuiTextState& globals;
    // Admit this immutable canonical view while renderer acquisitions are
    // quiescent. The mission preparation callback itself is a retained native
    // acquisition, so the ready-only getter must not be re-entered there.
    const volatile Word* material_profile;
    NativeMaterialDestructionAccess materials;
    NativeMaterialParameterAccess parameters;
    LiteralTextLocale locale_calls;
    std::unique_ptr<GameNativeGuiNodeApplication> nodes;
    std::unique_ptr<GuiMaterialBindingServices> style;
    std::unique_ptr<GuiWidgetClipRefreshServices> clip;
    std::unique_ptr<GameNativeGuiSamplerApplication> samplers;
    std::unique_ptr<GameNativeGuiEffectApplication> effects;
    std::unique_ptr<GameNativeTextApplication> text;
    std::unique_ptr<GameNativeGuiGroupApplication> group;
    std::shared_ptr<GuiTable> properties;
    GuiWidgetOwner* widget{};

    NativeViewportRegistry::Storage viewport_record;
    NativeString camera_name;
    bool camera_name_live{}, camera_retired{};
    void* camera_raw{};
    std::unique_ptr<NativeCameraOwner> camera;
    std::unique_ptr<NativeCameraReference> camera_reference;
    NativeGuiSceneStorageContext scene_storage;
    NativeGuiSceneIdentityContext scene_identity;
    NativeGuiSceneConstructFrame scene_frame{};
    NativeGuiSceneConstructAcquired scene_construction;
    NativeGuiSceneDestroyFrame scene_destroy_frame{};
    NativeGuiSceneLifetimeAcquired scene_destruction;
    std::optional<NativeGuiSceneReference> scene_reference;
    void* scene_raw{};
    NativeString scene_name;
    bool scene_name_live{}, root_linked{};
    NativeString label;
    bool label_live{};
    NativeRenderActualOwnerReleaseObservation scene_release;
    std::uint64_t frames{};
    Phase phase{Phase::prepared};
    std::exception_ptr failure;

    struct RootDispatch final : NativeNodeSceneChildDispatch {
        Impl& app;
        explicit RootDispatch(Impl& value) : app(value) {}
        const volatile Word* resolve_profile(Word profile) const override {
            if (profile == 0x00d634f8) {
                require(app.group != nullptr, "native GUI scene has no admitted Group environment");
                return app.group->groups().vtable_00d634f8;
            }
            require(profile == 0x00d62de8, "native Text scene requires its actual Model profile");
            return app.nodes->models().vtable_00d62de8;
        }
        void invoke_child(Word target, void* actual, Word requested, Word recursion) override {
            require(target == 0x00b6ed80 && !requested && !recursion,
                "native Text scene requires the genuine null-lighting Model attachment");
            if (app.group) {
                auto& reference = app.group->root();
                auto& owner = reference.group_owner();
                if (&owner.storage.node == actual) {
                    require(owner.phase == NativeGroupOwner::Phase::live &&
                        &owner.environment == &app.group->groups() &&
                        &owner.environment.nodes == &app.nodes->models().nodes &&
                        app.renderer.actual_owners().find(actual) == &reference,
                        "native GUI scene lost its same canonical Group");
                    auto& attachment = reference.scene_attachment();
                    require(attachment.attach_scene && !attachment.scene,
                        "native GUI Group cannot substitute a logical lighting owner");
                    attachment.attach_scene(owner.environment.nodes.scenes, attachment, nullptr, false);
                    return;
                }
            }
            auto* reference = app.widget->model_reference();
            require(reference && &reference->model_owner().storage.node == actual &&
                reference->model_owner().phase == NativeModelOwner::Phase::live &&
                &reference->model_owner().environment.nodes == &app.nodes->models().nodes,
                "native Text scene lost its same canonical Model");
            auto& attachment = reference->scene_attachment();
            require(attachment.attach_scene && !attachment.scene,
                "native Text scene cannot substitute a logical lighting owner");
            attachment.attach_scene(app.nodes->models().nodes.scenes, attachment, nullptr, false);
        }
    } roots;

    Impl(GameHostLog& output, GameNativeRendererApplication& app, GameVfsHost& vfs,
        GameSingletonHost& host, GameNativeReadOnlyData& mapped, GameInputRuntime& controls,
        GameFontHost& font_host, GameLocaleHost& locale_host, const bool& conversion)
        : log(output), renderer(app), files(vfs), singletons(host), data(mapped), input(controls),
          fonts(font_host), locale(locale_host), sse2(conversion), raw(vfs.borrow_raw_services()),
          globals(game_native_gui_text_state()),
          material_profile(app.borrow_gui_material_destruction_profile_00d5e520()),
          materials{app.actual_owners(), raw.strings,
              game_native_material_process().parameter_pool_00f8d3e4(),
              game_native_material_process().material_pool_00f8d3ac()},
          parameters{game_native_material_process().parameter_pool_00f8d3e4(), raw.strings,
              source_ucrt_text_name_comparer(), table(mapped, 0x00d61a00, 0x14)},
          scene_storage{host.weak_owners(), app.camera_environment().nodes.require_raw_name_pool(),
              app.actual_owners(), app.cockpit_viewport_release().decrement_iat_00ce2220,
              app.node_constants().one_00d7a24c, nullptr},
          scene_identity{scene_storage, table(mapped, 0x00d62d48, 8)}, roots(*this) {
        nodes = std::make_unique<GameNativeGuiNodeApplication>(log, renderer, files, data,
            [this](GuiWidgetOwner& owner) {
                require(text && owner.layout().type == GuiWidgetType::Text,
                    "native GUI text bring-up admits only its constructed Text factory");
                return text->make_type(owner);
            });
        style = std::make_unique<GuiMaterialBindingServices>(GuiMaterialBindingServices{
            nodes->widgets(), globals.aspect_ratio_00e12fc0(),
            [](GuiWidgetOwner&) -> GuiClipBoxParameterSources {
                throw std::logic_error("standalone native Text has no ClipBox ancestor");
            },
            [this](GuiWidgetOwner& owner) { return nodes->retain_widget(owner); },
            renderer.actual_owners()});
        clip = std::make_unique<GuiWidgetClipRefreshServices>(GuiWidgetClipRefreshServices{
            nodes->widgets(), materials, parameters, globals.aspect_ratio_00e12fc0()});
        nodes->widgets().environment().clip = clip.get();
    }
    void fail() noexcept { failure = std::current_exception(); phase = Phase::failed; }
    void compose_effect_services() {
        samplers = std::make_unique<GameNativeGuiSamplerApplication>(log, renderer, files,
            singletons, data, GameNativeGuiSamplerApplicationBindings{input,
                renderer.borrow_platform_publication_0109cf04()});
        effects = std::make_unique<GameNativeGuiEffectApplication>(log, renderer, files,
            samplers->effect_bindings());
        if (GameNativeGuiRenderApplication::nodes_requested())
            group = std::make_unique<GameNativeGuiGroupApplication>(log, renderer, files, data,
                nodes->parenting());
    }
    void make_text() {
        auto& font_resources = fonts.native_fonts();
        const auto& shadow = globals.shadow_color_00e12fd8();
        const auto& one = cell<float>(data, 0x00d7a24c);
        const auto& shadow_offset = cell<float>(data, 0x00d5c5c0);
        text = std::make_unique<GameNativeTextApplication>(log, renderer,
            GameNativeTextApplicationBindings{nodes->widgets(), nodes->parenting(),
                game_native_gui_text_process().pool_00f8bdf0(), materials, parameters, *style,
                font_resources, GuiTextFontRegistryAccess::borrowed_native_images(font_resources),
                locale.tables(), locale_calls, source_ucrt_text_content_comparison(), nullptr,
                source_ucrt_text_name_comparer(), sse2,
                *static_cast<void* const volatile*>(renderer.borrow_platform_publication_0109cf04().identity()),
                material_profile,
                table(data, 0x00d62d60, 8), raw.strings,
                static_cast<const char*>(data.data_at(0x00cefd78, 1)),
                globals.vertical_scale_00e12fd4(), globals.blend_factor_00f8be54(),
                cell<float>(data, 0x00d7a260),
                {shadow_offset, shadow, cell<float>(data, 0x00ce3e18), one,
                    cell<float>(data, 0x00ce3800)},
                {one, shadow_offset, shadow, cell<float>(data, 0x00cee07c),
                    globals.default_color_mask_00f8be48(), globals.default_color_00f8be38()},
                {cell<double>(data, 0x00d7a280), cell<double>(data, 0x00cec380),
                    cell<double>(data, 0x00cef1b8), cell<double>(data, 0x00d5c5c8)},
                {cell<double>(data, 0x00d7a358), cell<double>(data, 0x00d5c7a8),
                    cell<double>(data, 0x00d7a220), cell<double>(data, 0x00d06880),
                    cell<double>(data, 0x00cec380)},
                game_native_gui_text_process().text_lineage_00f8be28()}, *effects);
        fonts.bind_text_factory(text->factory());
        require(!fonts.registry().fonts().empty(), "native Text requires a loaded Font");
        const auto& font = *fonts.registry().fonts().front();
        require(font.native_resources && font.native_resources->native_image(),
            "native Text requires the selected image-shaped Font producer");
        properties = std::make_shared<GuiTable>();
        properties->named = {{"Font", GuiValue(font.descriptor.name)},
            {"Multiline", GuiValue(false)}, {"Visible", GuiValue(true)},
            // Explicit diagnostic placement inside AA3E00's 1..19984 depth
            // volume. This is authored bring-up content, not a native default.
            {"Pos", GuiValue(vector_table({0.05, 0.10, 0.0}))},
            {"Size", GuiValue(vector_table({0.9, 0.08}))},
            {"Color", GuiValue(vector_table({1.0, 1.0, 1.0, 1.0}))}};
        auto layout = std::make_unique<GuiLayoutWidget>();
        layout->key = "NativeGui_Text";
        layout->type = GuiWidgetType::Text;
        layout->source = properties.get();
        layout->transform.authored_x = 0.05f;
        layout->visible = true;
        widget = &nodes->create_standalone_text(std::move(layout));
        auto* platform = *static_cast<void* const volatile*>(
            renderer.borrow_platform_publication_0109cf04().identity());
        require(platform, "native Text requires the existing platform publication");
        bind_widget_properties_00aaa710(*properties, widget->layout(), false,
            read<std::uint8_t>(platform, 0x0d) != 0, sse2);
        nodes->widgets().base_properties_bound(widget->layout());
        nodes->widgets().properties_bound(widget->layout(), *properties);
        nodes->widgets().loaded78(widget->layout());
        require(text->submit_source(*widget, "Battlestations Pacific - native GuiText", false) ==
            GameNativeTextStatus::complete, "native Text bring-up retained an incomplete glyph child");
        if (gui_trace_requested()) {
            void* mesh_raw = gui_model_geometry_00b74640(widget->model_reference()->model_owner().storage.model, 0);
            auto* mesh = dynamic_cast<NativeMeshReference*>(&renderer.actual_owners().resolve_actual(mesh_raw));
            require(mesh && &mesh->storage() == mesh_raw && mesh->storage().draw_sections_54.count_04 > 0,
                "native GUI trace requires the actual Text mesh");
            void* section_raw = gui_geometry_element_00b732c0(mesh_raw, 0);
            auto* section = dynamic_cast<NativeMeshSectionReference*>(&renderer.actual_owners().resolve_actual(section_raw));
            require(section && &section->storage() == section_raw, "native GUI trace requires the actual Text section");
            const auto& raw_section = section->storage();
            void* stream = mesh->storage().vertex_stream_count_7c > 0 ? mesh->storage().vertex_streams_64[0] : nullptr;
            log.notef("native_GUI_trace built mesh=%p section=%p primitive=%u minimum=%u vertices=%u start=%u primitives=%u VB=%p IB=%p stream_tag=%08x stream_count=%u material=%p clip=%u",
                mesh_raw, section_raw, raw_section.primitive_08, raw_section.range_words_0c[0], raw_section.range_words_0c[1],
                raw_section.range_words_0c[2], raw_section.range_words_0c[3], stream, mesh->storage().index_stream_60,
                stream ? read<Word>(stream, 0x54) : 0, stream ? read<Word>(stream, 0x64) : 0,
                raw_section.material_20, static_cast<unsigned>(widget->extra_fields().clip_enabled_e8));
        }
    }
    static void retire_camera(void* context, NativeCameraReference& reference) noexcept {
        auto& app = *static_cast<Impl*>(context);
        app.renderer.actual_owners().unbind(app.camera_raw, reference);
        app.camera_raw = nullptr;
        app.camera_retired = true;
    }
    void make_camera() {
        auto& environment = renderer.camera_environment();
        camera_name_live = true;
        camera_name.assign_0041e870(raw.strings, "GuiCam_NativeText");
        camera_raw = environment.pool_0108ffb0.allocate_raw_slot_00b71770();
        require(camera_raw, "native GUI camera pool returned null");
        auto admission = renderer.actual_owners().prepare_binding(camera_raw);
        auto viewport = renderer.viewport_registry().admit(viewport_record);
        camera = std::make_unique<NativeCameraOwner>(camera_raw, NativeCameraPool::slot_bytes,
            environment);
        construct_native_camera_00b71a80(*camera, &camera_name, renderer.node_constants(),
            std::move(viewport));
        camera_reference = std::make_unique<NativeCameraReference>(*camera,
            NativeCameraCompanionDisposal{this, &retire_camera});
        renderer.actual_owners().publish(std::move(admission), *camera_reference);
        camera_name.release_to(raw.strings);
        camera_name_live = false;
        // The recovered GUI scene acquisition disables distance/height fog.
        auto* fog = allocate_system_fog_owner();
        set_system_fog_scalar_68_00b84d00(*fog, 0);
        set_system_fog_scalar_78_00b84d40(*fog, 0);
        set_system_fog_camera_owner_00b71940(camera->frame.fog_184, fog);
        release_system_fog_owner(*fog);
        set_camera_clear_flags_00b6fe10(camera->frame, 6);
        camera->storage.camera.clear_color_190 = 0;
    }
    void make_scene() {
        scene_raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x24, 0x24});
        require(scene_raw, "native GUI scene allocation returned null");
        scene_name_live = true;
        scene_name.assign_0041e870(raw.strings, "NativeGuiText");
        scene_frame.name_argument = static_cast<Word>(reinterpret_cast<std::uintptr_t>(&scene_name));
        construct_native_gui_scene_storage_00b724e0(scene_raw, scene_frame,
            scene_storage, scene_construction);
        scene_reference.emplace(scene_raw, scene_identity, scene_destroy_frame, scene_destruction);
        scene_name.release_to(raw.strings);
        scene_name_live = false;
        // B6D890 links only the parentless root, propagating the same scene to
        // Group children. It adds no reference; unlink before owned retirement.
        auto* const root = group ? &group->root().group_owner().storage.node :
            &widget->model_reference()->model_owner().storage.node;
        propagate_native_node_root_storage_00b6d890(
            root, scene_raw, roots);
        root_linked = true;
        if (group)
            log.notef("native GUI Group scene root: root=%p text_model=%p scene=%p",
                root, &widget->model_reference()->model_owner().storage.node, scene_raw);
    }

    struct Pass final : GuiRenderOrderHost, GuiOrthoProjectionHost {
        Impl& app;
        GameNativeGuiQueue& commands;
        CameraMatrix projection;
        std::array<Word, 2> dimensions{};
        Pass(Impl& value, GameNativeGuiQueue& queue) : app(value), commands(queue) {}
        bool has_pre_draw_object() override { return false; }
        void pre_draw_virtual40() override { throw std::logic_error("standalone GUI has no pre-draw object"); }
        std::array<Word, 2> device_size_00f8d394_30() override {
            auto& environment = app.renderer.camera_environment().viewport;
            require(environment.renderer_00f8d394, "native GUI pass lost renderer");
            const auto size = environment.renderer_access.parameters_00b1ff60(*environment.renderer_00f8d394);
            return {size.width_0c, size.height_10};
        }
        GuiBackBufferSize back_buffer_size_00f8d394_virtual80() override {
            const auto size = device_size_00f8d394_30();
            return {static_cast<std::int32_t>(size[0]), static_cast<std::int32_t>(size[1])};
        }
        void set_camera_look_at_00b71490(const void* camera, const GuiCameraPlacement& p) override {
            require(camera == app.camera_raw, "native GUI pass requires its same camera");
            CameraMatrix view;
            build_camera_look_at_00b63f10(view, {p.eye[0], p.eye[1], p.eye[2]},
                {p.target[0], p.target[1], p.target[2]}, {p.up[0], p.up[1], p.up[2]},
                app.camera->environment.crt, app.renderer.node_constants().one_00d7a24c);
            set_native_camera_view_00b71490(*app.camera, view.data());
        }
        void set_camera_ortho_00aa2020(const void* camera, const GuiOrthoVolume& volume) override {
            set_gui_ortho_projection_00aa2020(camera, volume, *this);
        }
        const void* platform() const {
            auto* value = *static_cast<void* const volatile*>(
                app.renderer.borrow_platform_publication_0109cf04().identity());
            require(value, "native GUI pass lost platform"); return value;
        }
        bool platform_widescreen_0d() override { return read<std::uint8_t>(platform(), 0x0d) != 0; }
        float platform_aspect_10() override { return read<float>(platform(), 0x10); }
        void* ortho_off_center_00b62a10(float l, float r, float b, float t, float n, float f) override {
            cc13fx::ortho_off_center_lh(projection.data(), l, r, b, t, n, f);
            return projection.data();
        }
        void set_projection_00b6fd60(const void* camera, void* matrix) override {
            require(camera == app.camera_raw && matrix == projection.data(), "native GUI projection identity changed");
            set_camera_projection_00b6fd60(app.camera->projection, projection);
        }
        void* camera_viewport_00b6fde0(const void* camera, const std::array<Word, 2>& size) override {
            require(camera == app.camera_raw && app.camera->storage.camera.viewport_180,
                "native GUI camera has no constructed viewport");
            dimensions = size; return app.camera->storage.camera.viewport_180;
        }
        void viewport_set_dimensions_00b1f940(void* viewport) override {
            require(viewport == app.camera->storage.camera.viewport_180, "native GUI viewport changed");
            set_native_viewport_dimensions_00b1f940(*static_cast<NativeViewportOwner*>(viewport), dimensions.data());
        }
        void set_pass_label_0041e870(std::string_view value) override {
            require(!app.label_live && value == "X", "native GUI pass label cannot overlap");
            app.label_live = true;
            app.label.assign_0041e870(app.raw.strings, "X");
        }
        void* render_command_queue_004c11f0() override { return commands.queue_004c11f0(); }
        void* render_pass_context_00b0d0d0() override { return commands.pass_context_00b0d0d0(); }
        void submit_scene_00b1f4d0(void* queue, const void* scene, const void* camera, void* context) override {
            require(app.label_live, "native GUI submission lost its actual label");
            // Prefix validation can fail before the consuming native call.
            // Once entered, B1F4D0 owns cleanup even when it throws.
            bool entered{};
            try {
                commands.submit_scene_00b1f4d0(queue, const_cast<void*>(scene),
                    const_cast<void*>(camera), context, &app.label, entered);
            } catch (...) {
                if (entered) app.label_live = false;
                throw;
            }
            require(entered, "native GUI queue did not enter its consuming submission");
            app.label_live = false;
        }
    };
};

bool GameNativeGuiRenderApplication::text_requested() noexcept {
    static const bool requested = [] {
        char value[2]{};
        return GetEnvironmentVariableA("BSP_NATIVE_GUI_TEXT", value, sizeof value) == 1 && value[0] == '1';
    }();
    return requested;
}
bool GameNativeGuiRenderApplication::nodes_requested() noexcept {
    static const bool requested = [] {
        char value[2]{};
        return GetEnvironmentVariableA("BSP_NATIVE_GUI_NODES", value, sizeof value) == 1 && value[0] == '1';
    }();
    return requested;
}
GameNativeGuiRenderApplication::GameNativeGuiRenderApplication(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& vfs, GameSingletonHost& singletons,
    GameNativeReadOnlyData& data, GameInputRuntime& input, GameFontHost& fonts,
    GameLocaleHost& locale, const bool& sse2)
    : impl_(std::make_unique<Impl>(log, renderer, vfs, singletons, data, input, fonts, locale, sse2)) {}
GameNativeGuiRenderApplication::~GameNativeGuiRenderApplication() noexcept {
    if (impl_->phase != Impl::Phase::prepared && impl_->phase != Impl::Phase::closed) std::terminate();
}
void GameNativeGuiRenderApplication::compose_effect_services_in_ready_renderer() {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::prepared && !p.samplers && !p.effects,
        "native GUI effect services must be admitted once before mission preparation");
    p.phase = Impl::Phase::preparing;
    try {
        p.compose_effect_services();
        p.phase = Impl::Phase::prepared;
        p.log.note("native GUI effect services ready: profile=D5E520 acquisitions=deferred");
    } catch (...) { p.fail(); throw; }
}
void GameNativeGuiRenderApplication::prepare_text_in_open_shader_cache() {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::prepared && p.samplers && p.effects &&
        p.renderer.shader_cache_context().actual_cache_0108d6ec,
        "native GUI Text preparation requires the mission's same open shader cache");
    p.phase = Impl::Phase::preparing;
    try {
        // Mission configuration follows ready metadata composition. Bind its
        // retained Group view now, before this fresh graph's first submission.
        if (p.group) p.renderer.bind_mission_gui_group_environment(&p.group->groups());
        p.make_text();
        if (p.group) p.group->create_root("NativeGuiTextRoot", *p.widget->model_reference());
        p.make_camera(); p.make_scene(); p.phase = Impl::Phase::ready;
        p.log.notef("native GUI Text ready: text=%p model=%p scene=%p camera=%p glyph_buffers=HAL font=image",
            &p.widget->layout(), &p.widget->model_reference()->model_owner().storage.node, p.scene_raw, p.camera_raw);
    } catch (...) { p.fail(); throw; }
}
void GameNativeGuiRenderApplication::submit_mission_frame(GameNativeGuiQueue& queue) {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::ready, "native GUI frame requires ready Text owners");
    p.phase = Impl::Phase::submitting;
    try {
        Impl::Pass pass(p, queue);
        NativeRendererIndexedDrawObserverScope trace(p.frames == 0 && gui_trace_requested() ?
            &trace_gui_indexed_draw : nullptr, &p.log);
        submit_gui_pass_00aa3e00(p.scene_raw, p.camera_raw, pass);
        ++p.frames; p.phase = Impl::Phase::ready;
    } catch (...) { p.fail(); throw; }
}
void GameNativeGuiRenderApplication::retire_before_fonts() {
    auto& p = *impl_;
    if (p.phase == Impl::Phase::retired || p.phase == Impl::Phase::closed) return;
    if (p.phase == Impl::Phase::prepared) { p.phase = Impl::Phase::retired; return; }
    require(p.phase == Impl::Phase::ready, "native GUI retirement cannot replay interrupted work");
    p.phase = Impl::Phase::retiring;
    try {
        auto* const text_model = &p.widget->model_reference()->model_owner().storage.node;
        auto* const group_root = p.group ? &p.group->root().group_owner().storage.node : nullptr;
        if (p.root_linked) {
            propagate_native_node_root_storage_00b6d890(
                group_root ? group_root : text_model, nullptr, p.roots);
            p.root_linked = false;
        }
        if (p.group) {
            p.group->detach_text();
            p.group->retire_root();
        }
        p.text->retire_owned_text(p.nodes->take_standalone_for_retirement(*p.widget));
        p.widget = nullptr;
        release_native_render_actual_owner(p.renderer.actual_owners(), p.camera_raw);
        require(p.camera_retired, "native GUI camera remains referenced after command retirement");
        p.renderer.viewport_registry().forget_quiescent(p.viewport_record);
        release_native_render_actual_owner_observed(p.renderer.actual_owners(), p.scene_raw, p.scene_release);
        p.phase = Impl::Phase::retired;
        p.log.notef("native GUI Text retired: submitted_frames=%llu model=returned camera=returned scene=released",
            static_cast<unsigned long long>(p.frames));
        if (p.group)
            p.log.notef("native GUI Group retired: root=%p text_model=%p scene=unlinked text=returned group=returned",
                group_root, text_model);
    } catch (...) { p.fail(); throw; }
}
void GameNativeGuiRenderApplication::after_native_renderer_drain() {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::retired, "native GUI graph must retire before shared drain");
    if (p.effects) p.effects->after_native_renderer_drain();
    if (p.samplers) p.samplers->after_native_renderer_drain();
    if (p.scene_reference)
        require(p.scene_reference->retired() && p.scene_destruction.complete && p.scene_destruction.freed,
            "native GUI scene did not complete its actual terminal");
    p.effects.reset(); p.text.reset(); p.samplers.reset();
    p.scene_reference.reset(); p.camera_reference.reset(); p.camera.reset();
    p.group.reset(); // Its environment and parenting borrow outlive native drain.
    p.clip.reset(); p.style.reset(); p.nodes.reset(); p.properties.reset();
    p.phase = Impl::Phase::closed;
}
bool GameNativeGuiRenderApplication::ready() const noexcept { return impl_->phase == Impl::Phase::ready; }
bool GameNativeGuiRenderApplication::requires_process_retention() const noexcept {
    const auto& p = *impl_;
    return p.failure || p.phase == Impl::Phase::preparing || p.phase == Impl::Phase::submitting ||
        p.phase == Impl::Phase::retiring || (p.nodes && p.nodes->failure()) ||
        (p.text && p.text->failure()) || (p.effects && p.effects->failure()) ||
        (p.group && p.group->failure());
}
std::uint64_t GameNativeGuiRenderApplication::submitted_frames() const noexcept { return impl_->frames; }

void GameStartupHost::initialize_native_gui_render() {
    require(!GameNativeGuiRenderApplication::nodes_requested() || GameNativeGuiRenderApplication::text_requested(),
        "BSP_NATIVE_GUI_NODES=1 requires BSP_NATIVE_GUI_TEXT=1");
    if (!GameNativeGuiRenderApplication::text_requested()) return;
    char frame_flag[2]{};
    require(GameNativeRendererApplication::mission_ship_requested() &&
        GetEnvironmentVariableA("BSP_NATIVE_MISSION_FRAME", frame_flag, sizeof frame_flag) == 1 &&
        frame_flag[0] == '1',
        "BSP_NATIVE_GUI_TEXT=1 requires BSP_NATIVE_MISSION_SHIP=1 and BSP_NATIVE_MISSION_FRAME=1");
    if constexpr (kRawDefaultTextQualificationCc16Bound ||
        kRawPrimaryPrefixQualificationCc16Bound || kRawPrefixGeometryQualificationCc16Bound ||
        kRawFontMetadataQualificationCc16Bound || kRawFontWorkQualificationCc16Bound) {
        require(!GameNativeGuiRenderApplication::nodes_requested(),
            "raw Text qualification requires the GUI node overlay disabled");
        require(!native_gui_render_ && !native_raw_default_text_ && native_renderer_ &&
            vfs_ && singletons_ && native_data_ && input_runtime_,
            "raw Text qualification requires the existing ready startup owners");
        native_raw_default_text_ = std::make_unique<GameNativeRawDefaultTextOwner>(
            log_, *native_renderer_, *vfs_, *singletons_, *native_data_,
            GameNativeGuiSamplerApplicationBindings{*input_runtime_,
                native_renderer_->borrow_platform_publication_0109cf04()});
        if constexpr (kRawFontMetadataQualificationCc16Bound || kRawFontWorkQualificationCc16Bound) {
            require(fonts_ != nullptr, "raw Font metadata qualification requires the actual startup Font host");
            // Store the stable owner before the exclusive combined export.
            // Its typed handshake verifies readiness, image and domain identity.
            fonts_->bind_raw_text_owner(*native_raw_default_text_);
            log_.note("raw Font metadata qualification: producer_linked before_prepare=1 shader_fresh=required");
        }
        // Explicit caller budget for this one retained graph, not an Original
        // constant or a universal nesting bound. Exhaustion retains failure.
        if constexpr (kRawFontWorkQualificationCc16Bound)
            native_raw_default_text_->prepare_primary_prefix_geometry_font_work_in_ready_renderer(64, kRawFontWorkChoiceCc16Bound);
        else if constexpr (kRawPrefixGeometryQualificationCc16Bound || kRawFontMetadataQualificationCc16Bound)
            native_raw_default_text_->prepare_primary_prefix_geometry_metadata_in_ready_renderer(64);
        else if constexpr (kRawPrimaryPrefixQualificationCc16Bound)
            native_raw_default_text_->prepare_primary_prefix_metadata_in_ready_renderer(64);
        else
            native_raw_default_text_->prepare_metadata_in_ready_renderer(64);
        return;
    }
    require(!native_gui_render_ && native_renderer_ && vfs_ && singletons_ && native_data_ &&
        input_runtime_ && fonts_ && locale_, "native GUI Text requires the existing ready startup owners");
    native_gui_render_ = std::make_unique<GameNativeGuiRenderApplication>(log_, *native_renderer_,
        *vfs_, *singletons_, *native_data_, *input_runtime_, *fonts_, *locale_, gui_sse2_conversion());
    native_gui_render_->compose_effect_services_in_ready_renderer();
}
void GameStartupHost::prepare_native_gui_text_resources() {
    if (native_raw_default_text_) {
        if constexpr (kRawPrefixGeometryQualificationCc16Bound || kRawFontMetadataQualificationCc16Bound ||
            kRawFontWorkQualificationCc16Bound) {
            HMODULE query_module{}, call_module{}, standard_module{}, query_declaration_module{};
            constexpr DWORD module_flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
            const auto query_target = bsp_cc16_query_new_handler_import;
            const BOOL declaration_loaded = GetModuleHandleExA(module_flags,
                reinterpret_cast<LPCSTR>(&_query_new_handler), &query_declaration_module);
            const BOOL query_loaded = query_target && GetModuleHandleExA(module_flags,
                reinterpret_cast<LPCSTR>(query_target), &query_module);
            const BOOL call_loaded = GetModuleHandleExA(module_flags,
                reinterpret_cast<LPCSTR>(&_callnewh), &call_module);
            const auto standard_target = bsp_cc16_std_get_new_handler_import;
            const BOOL standard_loaded = standard_target && GetModuleHandleExA(module_flags,
                reinterpret_cast<LPCSTR>(standard_target), &standard_module);
            log_.notef("raw prefix geometry CRT resolved modules: query_ok=%d call_ok=%d standard_ok=%d query=%p call=%p standard=%p",
                query_loaded ? 1 : 0, call_loaded ? 1 : 0, standard_loaded ? 1 : 0,
                static_cast<void*>(query_module), static_cast<void*>(call_module), static_cast<void*>(standard_module));
            require(query_loaded && call_loaded && standard_loaded,
                "raw prefix-geometry qualification requires live CRT function modules");
            require(query_module == call_module,
                "raw prefix-geometry qualification requires the same resolved CRT heap module");
            char heap_path[MAX_PATH]{}, standard_path[MAX_PATH]{};
            const DWORD heap_length = GetModuleFileNameA(query_module, heap_path, MAX_PATH);
            const DWORD standard_length = GetModuleFileNameA(standard_module, standard_path, MAX_PATH);
            require(heap_length && heap_length < MAX_PATH && standard_length && standard_length < MAX_PATH,
                "raw prefix-geometry qualification requires complete live CRT module paths");
            log_.notef("raw prefix geometry CRT modules: heap=%s standard=%s", heap_path, standard_path);
            log_.notef("raw prefix geometry CRT query import: declaration_in_exe=%d resolved_slot=1",
                declaration_loaded && query_declaration_module == GetModuleHandleA(nullptr) ? 1 : 0);
            // Actual host allocation callbacks must be absent in this closed
            // fixture; query both CRT interfaces without changing either one.
            require(_query_new_handler() == nullptr && std::get_new_handler() == nullptr,
                "raw prefix-geometry qualification requires actual null CRT new handlers");
            log_.note("raw prefix geometry qualification: crt_new_handlers=null");
            if constexpr (kRawFontWorkQualificationCc16Bound)
                require(native_raw_default_text_->construct_primary_prefix_geometry_font_work_in_open_shader_cache(),
                    "raw Font-work qualification requires a nonnull native factory result");
            else
                require(native_raw_default_text_->construct_primary_prefix_geometry_in_open_shader_cache(),
                    "raw prefix-geometry qualification requires a nonnull native factory result");
        } else if constexpr (kRawPrimaryPrefixQualificationCc16Bound)
            require(native_raw_default_text_->construct_primary_prefix_in_open_shader_cache(),
                "raw primary-prefix qualification requires a nonnull native factory result");
        else
            require(native_raw_default_text_->construct_default_in_open_shader_cache(),
                "raw DEFAULT qualification requires a nonnull native factory result");
        return;
    }
    if (native_gui_render_) native_gui_render_->prepare_text_in_open_shader_cache();
}
void GameStartupHost::draw_native_gui_text_mission_overlay() {
    if (!native_gui_render_) return;
    require(native_renderer_ && native_gui_render_->ready(),
        "native GUI Text requires preparation in the shared mission frame before overlay submission");
    MissionGuiQueue queue(*native_renderer_);
    native_gui_render_->submit_mission_frame(queue);
}
void GameStartupHost::exit_if_native_gui_text_incomplete() noexcept {
    const bool raw_incomplete = native_raw_default_text_ && native_raw_default_text_->requires_process_retention();
    if (!raw_incomplete && (!native_gui_render_ || !native_gui_render_->requires_process_retention())) return;
    try { log_.note("native GUI Text interrupted; retaining Text/font/material/node/frame owners"); log_.close(); }
    catch (...) {}
    std::_Exit(1);
}
void GameStartupHost::retire_native_gui_text_before_fonts() noexcept {
    if (!native_gui_render_ && !native_raw_default_text_) return;
    try {
        if (native_raw_default_text_) {
            native_raw_default_text_->retire_before_native_drain();
            require(!native_raw_default_text_->requires_process_retention(),
                "raw DEFAULT Text retains unresolved native custody before parent drain");
        }
        if (native_gui_render_) native_gui_render_->retire_before_fonts();
    }
    catch (...) {
        try { log_.note("native GUI Text retirement failed; retaining all borrowed owners"); log_.close(); }
        catch (...) {}
        std::_Exit(1);
    }
}
void GameStartupHost::close_native_gui_text_after_drain() noexcept {
    if (!native_gui_render_ && !native_raw_default_text_) return;
    try {
        if (native_raw_default_text_) {
            native_raw_default_text_->after_native_renderer_drain();
            if constexpr (kRawFontMetadataQualificationCc16Bound || kRawFontWorkQualificationCc16Bound) {
                require(fonts_ != nullptr, "raw Font metadata close requires the retained startup Font host");
                fonts_->detach_raw_text_owner_after_parent_drain(*native_raw_default_text_);
                log_.note("raw Font metadata qualification: producer_detached after_parent_drain=1 before_owner_reset=1");
            }
            native_raw_default_text_.reset();
        }
        if (native_gui_render_) {
            native_gui_render_->after_native_renderer_drain();
            native_gui_render_.reset();
        }
    }
    catch (...) {
        try { log_.note("native GUI Text metadata close failed after renderer drain"); log_.close(); }
        catch (...) {}
        std::_Exit(1);
    }
}
} // namespace bsp::game
