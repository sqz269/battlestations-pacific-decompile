#include "bsp/native_gui_text_glyph_buffers_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_mesh_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vertex_declaration_cache.hpp"
#include <cstddef>
#include <cstring>
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
using Phase = NativeGuiTextGlyphBuffersStoragePhase;
using Acquired = NativeGuiTextGlyphBuffersStorageAcquired;
using Context = NativeGuiTextGlyphBuffersStorageContext;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);

void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void location(Acquired& acquired, Acquired::Boundary boundary, std::uint32_t site) noexcept {
    acquired.boundary = boundary;
    acquired.native_site = site;
}
void* pointer(std::uint32_t word) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(word));
}
NativeMeshStorage& current_mesh(void* captured, Context& context) {
    // Pure lookup in the EXISTING canonical registry. Its companion was
    // constructed by the actual Mesh producer, not from this raw operand.
    auto* const reference = dynamic_cast<NativeMeshReference*>(
        &context.owners.resolve_actual(captured));
    require(reference && static_cast<void*>(&reference->storage()) == captured,
        "AB8400 requires the current genuine Mesh in its same canonical registry");
    return reference->storage();
}
void* declaration_current38(void* renderer, const void* name, Context& context,
    GuiNativeDeclarationAcquired& acquired) {
    const auto* const profile = context.graphics.streams.vertices.actual_renderer_profile_00d5f0a8;
    require(renderer && *static_cast<const volatile std::uint32_t*>(renderer) == 0x00d5f0a8u &&
        profile && profile[0x38 / 4] == 0x00b317e0u,
        "AB8400 current38 requires genuine B317E0 renderer/declaration services");
    // The existing generic Text adapter takes NativeString&, so use its SAME
    // real raw-header B317E0 supplier and canonical registration directly.
    acquired.reference = load_native_renderer_vertex_declaration_00b317e0(
        renderer, name, context.graphics.declarations);
    if (!acquired.reference) throw std::bad_alloc(); // inherited factory domain
    context.graphics.streams.geometry.register_native_declaration_reference(
        acquired, context.graphics.declarations.declarations);
    return acquired.reference;
}
void release_creator(void*& creator, NativeTextureSurfaceReferenceIncrement captured,
    NativeGuiTextGlyphBuffersStorageRelease& acquired, Context& context) {
    void* const raw = std::exchange(creator, nullptr);
    require(raw && captured && !acquired.entered,
        "AB8400 creator return requires one captured genuine obligation/import");
    acquired.identity = raw;
    acquired.captured_decrement = captured;
    acquired.entered = true;
    acquired.result = captured(static_cast<volatile long*>(
        static_cast<void*>(static_cast<std::byte*>(raw) + 4)));
    acquired.decremented = true;
    if (acquired.result == 0) {
        acquired.zero_entered = true;
        dispatch_native_render_actual_owner_zero(context.owners, raw);
        acquired.zero_returned = true;
    }
    acquired.complete = true; // no raw access after possible terminal return
}
void preflight(Context& context, Acquired& acquired) {
    require(acquired.phase == Phase::fresh && !acquired.failure &&
        !acquired.declaration.reference && !acquired.declaration.companion &&
        !acquired.declaration.canonical_registration && !acquired.vertex.creator &&
        !acquired.vertex.companion && acquired.vertex.phase == NativeStreamClonePhase::empty &&
        !acquired.index.creator && !acquired.index.companion &&
        acquired.index.phase == NativeStreamClonePhase::empty,
        "AB8400 requires fresh retained invocation/acquisition metadata");
    require(&context.graphics.streams.geometry.actual_owners() == &context.owners &&
        context.graphics.declarations.strings.uses_raw_pool_cells(context.strings),
        "AB8400 requires the same actual registry and raw string publication cells");
    require_gui_text_native_renderer_domain(context.graphics,
        context.renderer_00f8d394,
        context.graphics.declarations.strings, context.owners);
}
} // namespace

NativeGuiTextGlyphBuffersStorageFrame::NativeGuiTextGlyphBuffersStorageFrame(
    Context& context, volatile std::uint32_t (&arguments)[2], Acquired& acquired) noexcept
    : context_(context), arguments_(arguments), acquired_(acquired) {}
NativeGuiTextGlyphBuffersStorageFrame::~NativeGuiTextGlyphBuffersStorageFrame() noexcept {
    if (requires_process_retention()) std::terminate();
}

void create_native_gui_text_glyph_buffers_storage_00ab8400(
    NativeGuiTextGlyphBuffersStorageFrame& frame) {
    auto& context = frame.context_;
    auto& acquired = frame.acquired_;
    try {
        require(frame.phase_ == Phase::fresh, "AB8400 invocation cannot reenter or replay");
        location(acquired, Acquired::Boundary::admission, 0);
        preflight(context, acquired); // metadata only; no argument/native-cell read
        const void* const literal = context.data.data_at(0x00cfada4u, 17);
        frame.phase_ = acquired.phase = Phase::entered;

        frame.name_[0] = 0; // AB8426
        frame.name_[1] = 0; // AB842A
        // Keep actual8B header storage; no NativeString/typed-string cast.
        void* const name = const_cast<std::uint32_t*>(&frame.name_[0]);
        location(acquired, Acquired::Boundary::name_resize, 0x00ab842e);
        resize_native_string_header_0041dd40(name, context.strings, 0x10, true);
        void* const name_data = pointer(frame.name_[1]); // AB8433, before length
        if (name_data) {
            const auto bytes = frame.name_[0] + 1u;
            location(acquired, Acquired::Boundary::name_copy, 0x00ab8449);
            std::memcpy(name_data, literal, bytes);
        }

        acquired.renderer38 = context.renderer_00f8d394;
        acquired.name_cleanup_armed = true; // native state0 only at AB8461
        location(acquired, Acquired::Boundary::declaration, 0x00ab8465);
        try {
            (void)declaration_current38(acquired.renderer38, name, context, acquired.declaration);
        } catch (...) {
            acquired.name_cleanup_trigger = std::current_exception();
            acquired.name_cleanup_armed = false;
            acquired.exceptional_name_return_entered = true;
            location(acquired, Acquired::Boundary::name_return, 0); // private unwind PC unclaimed
            destroy_native_string_header_0041dd20(name, context.strings);
            acquired.exceptional_name_returned = true;
            location(acquired, Acquired::Boundary::declaration, 0x00ab8465);
            throw;
        }
        acquired.name_cleanup_armed = false; // AB846F: no retry on return failure
        location(acquired, Acquired::Boundary::name_return, 0x00ab8484);
        acquired.name_return_entered = true;
        destroy_native_string_header_0041dd20(name, context.strings);
        acquired.name_returned = true;

        acquired.renderer5c = context.renderer_00f8d394; // AB8490
        acquired.captured_capacity = frame.arguments_[0]; // AB8496: ONCE, kept in EDI
        acquired.capacity_read = true;
        location(acquired, Acquired::Boundary::vertex_factory, 0x00ab84aa);
        void* const vertex = create_gui_text_native_vertex_current5c(acquired.renderer5c,
            acquired.captured_capacity * 4u, 1u, acquired.declaration.reference,
            context.graphics, acquired.vertex);
        acquired.vertex_mesh = pointer(frame.arguments_[1]); // AB84AC, after factory
        location(acquired, Acquired::Boundary::vertex_publication, 0x00ab84b4);
        set_native_mesh_vertex_stream_00b73bb0(current_mesh(acquired.vertex_mesh, context),
            context.owners, 0, vertex);
        acquired.vertex_published = true;

        location(acquired, Acquired::Boundary::decrement_capture, 0x00ab84b9);
        acquired.captured_decrement = context.decrement_00ce2220; // one real CELL read
        acquired.vertex.companion = nullptr;
        acquired.vertex.phase = NativeStreamClonePhase::consumed;
        location(acquired, Acquired::Boundary::vertex_return, 0x00ab84c3);
        release_creator(acquired.vertex.creator, acquired.captured_decrement,
            acquired.vertex_release, context);
        location(acquired, Acquired::Boundary::declaration_return, 0x00ab84d5);
        release_creator(acquired.declaration.reference, acquired.captured_decrement,
            acquired.declaration_release, context);

        acquired.renderer60 = context.renderer_00f8d394; // AB84E4
        location(acquired, Acquired::Boundary::index_factory, 0x00ab84f9);
        void* const index = create_gui_text_native_index_current60(acquired.renderer60,
            acquired.captured_capacity * 6u, 1u, 0x65u, context.graphics, acquired.index);
        acquired.index_mesh = pointer(frame.arguments_[1]); // AB84FB: FRESH home
        location(acquired, Acquired::Boundary::index_publication, 0x00ab8502);
        set_native_mesh_index_stream_00b73b70(current_mesh(acquired.index_mesh, context),
            context.owners, index);
        acquired.index_published = true;
        acquired.index.companion = nullptr;
        acquired.index.phase = NativeStreamClonePhase::consumed;
        location(acquired, Acquired::Boundary::index_return, 0x00ab850b);
        release_creator(acquired.index.creator, acquired.captured_decrement,
            acquired.index_release, context);
        require(frame.phase_ == Phase::entered && acquired.phase == Phase::entered &&
            !acquired.failure, "AB8400 cannot complete after a caught invocation failure");
        frame.phase_ = acquired.phase = Phase::complete;
    } catch (...) {
        acquired.escaped_boundary = acquired.boundary;
        acquired.escaped_site = acquired.native_site;
        acquired.failure = std::current_exception();
        frame.phase_ = acquired.phase = Phase::failed;
        throw; // preserve native effects; diagnostics own no implicit cleanup
    }
}
} // namespace bsp
