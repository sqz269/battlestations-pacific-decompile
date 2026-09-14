#include "bsp/native_material_program_compiler_actual.hpp"
#include "bsp/native_material_compiler_providers.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_renderer_format_check.hpp"
#include "bsp/native_shader_device_reset.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual material compiler requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
using I = std::int32_t;
using Frame = NativeMaterialProgramCompilerActualFrame;
using Context = NativeMaterialProgramCompilerActualContext;
template<class T> T read(const void* p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const char*>(p) + offset);
}
template<class T> void put(void* p, U offset, T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<char*>(p) + offset) = value;
}
void require(bool condition, const char* text) {
    if (!condition) throw std::logic_error(text);
}
void site(Frame& a, U address) noexcept {
    a.native_site = address;
    a.parent->native_site = address;
}
template<class T> T& child(std::unique_ptr<T>& output) {
    require(!output, "actual compiler child cannot replay");
    output = std::make_unique<T>();
    return *output;
}
void* allocate(U bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
const char* string_data(const void* name, const Context& c) noexcept {
    const char* value = read<const char*>(name, 4);
    return value ? value : c.actual_empty_0108d6f2;
}
NativeShaderDescriptorStorage& descriptor(Frame& a, U offset) noexcept {
    return *read<NativeShaderDescriptorStorage*>(&a.parent->builder, offset);
}
void literal(Frame& a, U index, const char* value, U length, U address, bool capture) {
    auto& t = a.names[index];
    require(!t.entered, "native compiler temporary cannot replay");
    t.entered = true;
    put<U>(&t.name, 0, 0);
    put<void*>(&t.name, 4, nullptr);
    site(a, address);
    resize_native_string_header_0041dd40(&t.name, a.context->strings, length, true);
    t.live = true;
    char* data = read<char*>(&t.name, 4);
    // Every literal captures its data before copying. Repeated shadow names
    // keep this pointer in EBX through lookup, but reload length on release.
    t.captured_data = data;
    if (capture) {
        t.captured = true;
        t.captured_length = read<U>(&t.name);
        if (data) std::memcpy(data, value, static_cast<std::size_t>(t.captured_length) + 1);
    } else if (data) {
        const U size = read<U>(&t.name);
        std::memcpy(data, value, static_cast<std::size_t>(size) + 1);
    }
}
void concat(Frame& a, U index, U suffix, U address) {
    auto& t = a.names[index];
    require(!t.entered, "native compiler concatenation cannot replay");
    t.entered = true;
    site(a, address);
    concatenate_native_string_headers_004261a0(&a.parent->local_name, &t.name,
        &a.names[suffix].name, a.context->strings);
    t.live = true;
}
void release_name(Frame& a, U index, U address) {
    auto& t = a.names[index];
    site(a, address);
    // Some native temporaries retain their pre-call data/length in registers;
    // others reload length/data immediately before their normal pool return.
    char* data = t.captured ? t.captured_data : read<char*>(&t.name, 4);
    if (data) {
        const U length = t.captured ? t.captured_length : read<U>(&t.name);
        a.context->strings.release(data, length + 1u);
    }
    t.live = false; // The native header itself deliberately remains stale.
}
NativeMaterialPassStorage* finish(Frame& a, bool failure, U failure_site = 0x00b3b6ea) {
    a.exception_state = 0xffffffffu;
    site(a, failure ? failure_site : 0x00b3c368);
    auto& name = a.parent->local_name;
    char* data = read<char*>(&name, 4);
    if (data) a.context->strings.release(data, read<U>(&name) + 1u);
    a.parent->local_name_live = false;
    a.native_null_return = failure;
    a.phase = Frame::Phase::complete;
    return failure ? nullptr : a.pass;
}

// The native inline resize(0) keeps array backing and abandons old pointer
// elements without destroying them. Negative capacity follows the real
// reserve/copy/free branch; negative count follows its wrapping fill loop.
void clear_array(Frame& a, NativeShaderDescriptorArray& rows, U stride, I minimum,
    U allocate_site, U free_site, U completion_site) {
    a.array_header = &rows;
    const I capacity = read<I>(&rows, 8);
    if (capacity < 0 && capacity < minimum) {
        const U bytes = static_cast<U>(minimum) * stride;
        site(a, allocate_site); a.array_native_site = allocate_site;
        auto* fresh = static_cast<char*>(allocate(bytes));
        a.array_allocation = fresh;
        for (I index = 0; index < read<I>(&rows, 4); ++index) {
            void* destination = reinterpret_cast<void*>(reinterpret_cast<U>(fresh) + static_cast<U>(index) * stride);
            if (destination) {
                const void* source = reinterpret_cast<const void*>(read<U>(&rows) + static_cast<U>(index) * stride);
                std::memcpy(destination, source, stride);
            }
        }
        site(a, free_site); a.array_native_site = free_site;
        singleton_lifetime_free(read<void*>(&rows));
        put<void*>(&rows, 0, fresh);
        put<I>(&rows, 8, minimum);
        a.array_allocation = nullptr;
    }
    const I count = read<I>(&rows, 4);
    if (count < 0) {
        U offset = static_cast<U>(count) * stride;
        do {
            void* destination = reinterpret_cast<void*>(read<U>(&rows) + offset);
            if (destination) {
                if (stride == 4) put<U>(destination, 0, 0);
                else { put<std::uint8_t>(destination, 0, 0xff); put<std::uint8_t>(destination, 1, 0xff); }
            }
            offset += stride;
        } while (static_cast<I>(offset) < 0);
    }
    while (read<I>(&rows, 4) > 0) put<U>(&rows, 4, read<U>(&rows, 4) - 1u);
    put<U>(&rows, 4, 0);
    a.array_native_site = completion_site;
}
bool has_vertex_sampler(Frame& a) noexcept {
    for (const U offset : {0x70u, 0x74u}) {
        auto* owner = &descriptor(a, offset);
        if (read<U>(owner, 0xc8) == 0) continue;
        auto* const data = read<void* const*>(owner, 0xc4);
        U index = 0;
        do {
            if (read<std::uint8_t>(read<void*>(data, index * 4u), 0xc)) return true;
            ++index;
        } while (index < read<U>(&descriptor(a, offset), 0xc8));
    }
    return false;
}
void reflect(Frame& a, bool pixel, const void* bytecode, U address) {
    site(a, address);
    auto& operation = child(a.reflections[pixel ? 1 : 0]);
    // Reload current pass metadata at the exact native call site.
    auto* owner = read<NativeCompiledShaderStorage*>(a.pass, pixel ? 0x74u : 0x70u);
    reflect_native_compiled_shader_00b3aea0(static_cast<const U*>(bytecode), *owner,
        a.context->reflection, operation);
}
void reflect_cached(Frame& a, bool pixel, const void* row, U address) {
    site(a, address);
    auto& operation = child(a.reflections[pixel ? 1 : 0]);
    // B3B84A/B3BDB5 capture metadata before loading the cache row bytecode.
    auto* owner = read<NativeCompiledShaderStorage*>(a.pass, pixel ? 0x74u : 0x70u);
    const auto* bytecode = read<const U*>(row, 8);
    reflect_native_compiled_shader_00b3aea0(bytecode, *owner,
        a.context->reflection, operation);
}
void require_stream_entry(void* stream, U slot, U target, const Context& c) {
    require(read<U>(stream) == 0x00d691b0 && c.actual_physical_stream_profile_00d691b0
        && c.actual_physical_stream_profile_00d691b0[slot / 4] == target,
        "actual compiler requires the recovered current physical-stream entry");
}
void write_cache(Frame& a, bool pixel) {
    auto& c = *a.context;
    if (c.source_mode_0108d6f1) return;
    const U suffix = pixel ? 6u : 2u, joined = suffix + 1;
    literal(a, suffix, pixel ? ".pso" : ".vso", 4, pixel ? 0x00b3bea3 : 0x00b3bb8d, false);
    a.exception_state = pixel ? 7u : 3u;
    concat(a, joined, suffix, pixel ? 0x00b3bedf : 0x00b3bbc9);
    // The second gate, size and cache identity are captured after concatenation.
    const bool skip = c.source_mode_0108d6f1 != 0;
    const U size = pixel ? a.pixel_size : a.vertex_size;
    auto* const cache = c.actual_cache_0108d6ec;
    a.exception_state = pixel ? 8u : 4u;
    if (!skip) {
        void* stream = read<void*>(cache, 8);
        site(a, pixel ? 0x00b3bf0f : 0x00b3bbf7);
        require_stream_entry(stream, 0x64, 0x00be4460, c);
        (void)write_native_compiler_stream_string_00be4460(stream, &a.names[joined].name,
            nullptr, c.actual_physical_stream_profile_00d691b0, c.actual_stream_empty_0109db64);
        stream = read<void*>(cache, 8);
        site(a, pixel ? 0x00b3bf1b : 0x00b3bc03);
        require_stream_entry(stream, 0x54, 0x00be40d0, c);
        (void)write_native_compiler_stream_word_00be40d0(stream, size, nullptr,
            c.actual_physical_stream_profile_00d691b0);
        stream = read<void*>(cache, 8);
        site(a, pixel ? 0x00b3bf2c : 0x00b3bc14);
        require_stream_entry(stream, 0x28, 0x00bf4f50, c);
        (void)write_native_physical_stream_00bf4f50(stream,
            pixel ? a.pixel_bytecode : a.vertex_bytecode, size, nullptr);
        put<U>(cache, 4, read<U>(cache, 4) + 1u);
    }
    a.exception_state = pixel ? 7u : 3u;
    release_name(a, joined, pixel ? 0x00b3bf53 : 0x00b3bc3b);
    a.exception_state = 0;
    release_name(a, suffix, pixel ? 0x00b3bf7a : 0x00b3bc62);
}
void cached_shader(Frame& a, bool pixel) {
    auto& c = *a.context;
    const U suffix = pixel ? 4u : 0u, joined = suffix + 1;
    literal(a, suffix, pixel ? ".pso" : ".vso", 4, pixel ? 0x00b3bd1e : 0x00b3b7b3, true);
    a.exception_state = pixel ? 9u : 5u;
    concat(a, joined, suffix, pixel ? 0x00b3bd5a : 0x00b3b7ef);
    auto& lookup = child(a.lookups[pixel ? 1 : 0]);
    site(a, pixel ? 0x00b3bd66 : 0x00b3b7fb);
    auto* row = find_native_shader_binary_cache_record_00b34890(c.actual_cache_0108d6ec,
        &a.names[joined].name, c.source_mode_0108d6f1, lookup);
    release_name(a, joined, pixel ? 0x00b3bd86 : 0x00b3b81b);
    a.exception_state = 0;
    release_name(a, suffix, pixel ? 0x00b3bda5 : 0x00b3b83a);
    // Native missing cache row is dereferenced; it is not a compiler-null return.
    reflect_cached(a, pixel, row, pixel ? 0x00b3bdbf : 0x00b3b854);
    site(a, pixel ? 0x00b3bdca : 0x00b3b85f);
    auto* device = static_cast<IDirect3DDevice9*>(get_native_renderer_device_00b1fef0(c.actual_renderer_00f8d394));
    // The native COM table is captured before the current row code reload.
    const auto* table = read<const volatile U*>(device);
    const DWORD* code = read<const DWORD*>(row, 8);
    site(a, pixel ? 0x00b3bde1 : 0x00b3b876);
    if (pixel) {
        using Create = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, const DWORD*, IDirect3DPixelShader9**);
        const auto create = reinterpret_cast<Create>(table[0x1a8 / 4]);
        (void)create(device, code, &a.pixel_shader);
    } else {
        using Create = HRESULT (STDMETHODCALLTYPE*)(IDirect3DDevice9*, const DWORD*, IDirect3DVertexShader9**);
        const auto create = reinterpret_cast<Create>(table[0x16c / 4]);
        (void)create(device, code, &a.vertex_shader);
    }
}
void construct_shader_owner(Frame& a, bool pixel) {
    auto& c = *a.context;
    const U index = pixel ? 1u : 0u;
    site(a, pixel ? 0x00b3bf97 : 0x00b3bc7f);
    void* raw = allocate(0x10);
    a.shader_owners[index] = static_cast<NativeD3d9ShaderStorage*>(raw);
    a.exception_state = pixel ? 10u : 6u;
    if (raw) {
        site(a, pixel ? 0x00b3bfb6 : 0x00b3bc9e);
        a.shader_owners[index] = pixel
            ? construct_native_pixel_shader_00b5f9b0(raw, a.pixel_shader, c.shader_construction)
            : construct_native_vertex_shader_00b5faf0(raw, a.vertex_shader, c.shader_construction);
        c.registration.bind_shader(c.registration.context, *a.shader_owners[index]);
        a.shader_registered[index] = true;
    }
    a.exception_state = 0;
    if (pixel ? a.pixel_shader != nullptr : a.vertex_shader != nullptr) {
        site(a, pixel ? 0x00b3bfd7 : 0x00b3bcbf);
        if (pixel) { (void)a.pixel_shader->Release(); a.pixel_com_released = true; }
        else { (void)a.vertex_shader->Release(); a.vertex_com_released = true; }
    }
    auto& operation = child(a.slots[index]);
    site(a, pixel ? 0x00b3bfde : 0x00b3bcc8);
    auto& owners = c.pass_construction.lifetime.retained_owners;
    if (pixel) set_native_material_pass_pixel_shader_00b5f080(a.pass->base, a.shader_owners[index], owners, operation);
    else set_native_material_pass_vertex_shader_00b5f0c0(a.pass->base, a.shader_owners[index], owners, operation);
    site(a, pixel ? 0x00b3bfe7 : 0x00b3bcd1);
    release_native_render_actual_owner(owners, a.shader_owners[index]);
    if (pixel) a.pixel_owner_released = true; else a.vertex_owner_released = true;
}

U base_pixel_count(Frame& a) noexcept {
    auto* base = &descriptor(a, 0x70);
    const U count = read<U>(base, 0xc8);
    U result = 0;
    if (count) {
        const auto* const data = read<void* const*>(base, 0xc4);
        for (U i = 0; i < count; ++i)
            if (read<std::uint8_t>(read<void*>(data, i * 4u), 0xc) == 0) ++result;
    }
    return result;
}
void shadow_index(Frame& a, bool texture) {
    const U first = texture ? 10u : 8u, second = first + 1;
    const char* value = texture ? "ShadowTexture" : "ShadowMap";
    const U length = texture ? 13u : 9u;
    literal(a, first, value, length, texture ? 0x00b3c1bb : 0x00b3c0c4, true);
    auto& exists = child(a.lookups[texture ? 6 : 4]);
    site(a, texture ? 0x00b3c1e6 : 0x00b3c0ef);
    const bool found = find_native_sampler_ordinal_00b347e0(descriptor(a, 0x74),
        &a.names[first].name, exists) != 0xffffffffu;
    release_name(a, first, texture ? 0x00b3c1fc : 0x00b3c105);
    if (!found) return;
    literal(a, second, value, length, texture ? 0x00b3c222 : 0x00b3c12b, false);
    // literal retained EBX's pre-copy data pointer; length stays current.
    const U base_count = base_pixel_count(a);
    auto& lookup = child(a.lookups[texture ? 7 : 5]);
    site(a, texture ? 0x00b3c27d : 0x00b3c180);
    const U ordinal = find_native_sampler_ordinal_00b347e0(descriptor(a, 0x74),
        &a.names[second].name, lookup);
    put<U>(a.pass, texture ? 0x7cu : 0x78u, ordinal + base_count);
    if (a.names[second].captured_data)
        a.names[second].captured_length = read<U>(&a.names[second].name);
    a.names[second].captured = true;
    release_name(a, second, texture ? 0x00b3c29a : 0x00b3c19d);
}
void final_states(Frame& a) {
    auto& b = a.parent->builder;
    auto& c = *a.context;
    auto* first_descriptor = &descriptor(a, 0x70);
    put<U>(&b, 0x8c, 0); put<U>(&b, 0x90, 0); put<U>(&b, 0x94, 0);
    auto& first = child(a.samplers[0]);
    site(a, 0x00b3c018);
    append_native_material_descriptor_samplers_00b3b280(b, *a.pass, *first_descriptor, c.root_samplers, first);
    auto& second = child(a.samplers[1]);
    site(a, 0x00b3c024);
    append_native_material_descriptor_samplers_00b3b280(b, *a.pass, descriptor(a, 0x74), c.mode_samplers, second);
    auto& states0 = child(a.lookups[2]); site(a, 0x00b3c030);
    apply_native_descriptor_render_states_00b34920(a.pass, descriptor(a, 0x70), states0);
    auto& states1 = child(a.lookups[3]); site(a, 0x00b3c03c);
    apply_native_descriptor_render_states_00b34920(a.pass, descriptor(a, 0x74), states1);
    const U mask = read<U>(read<void*>(a.pass, 0x74), 0x84);
    for (U slot = 0; slot < 16; ++slot) {
        if (!(mask & (1u << slot))) {
            auto& operation = child(a.slots[slot + 2]); site(a, 0x00b3c060);
            remove_native_material_pass_sampler_slot_00b5eff0(a.pass->base, slot, operation);
        }
    }
    if (read<std::uint8_t>(&descriptor(a, 0x70), 0x1c)
        && !read<std::uint8_t>(&descriptor(a, 0x74), 0x1e)) {
        void* renderer = c.actual_renderer_00f8d394;
        site(a, 0x00b3c08d);
        require(read<U>(renderer) == 0x00d5f0a8 && c.actual_renderer_profile_00d5f0a8
            && c.actual_renderer_profile_00d5f0a8[0x104 / 4] == 0x00b1ff50,
            "actual compiler renderer capability entry is unsupported");
        const auto* caps = get_native_compiler_renderer_capabilities_00b1ff50(renderer);
        const bool alternate = read<std::uint8_t>(caps, 0x3d) != 0;
        site(a, 0x00b3c0ad);
        set_native_material_render_state_00b5ec40(a.pass, alternate ? 0xb5u : 0x9au,
            alternate ? 0x434f5441u : 0x314d3241u);
    }
    shadow_index(a, false);
    shadow_index(a, true);
    const U first_shadow_sampler = read<U>(a.pass, 0x78);
    if (first_shadow_sampler != 0xffffffffu) {
        const bool linear = read<std::uint8_t>(&b, 0xaa) != 0;
        const U addresses[2][5] = {{0x00b3c309, 0x00b3c318, 0x00b3c327, 0x00b3c336, 0x00b3c345},
                                  {0x00b3c2c8, 0x00b3c2d7, 0x00b3c2e6, 0x00b3c2f5, 0x00b3c345}};
        const U states[] = {6, 5, 7, 1, 2};
        const U values[] = {linear ? 2u : 1u, linear ? 2u : 1u, 0, 3, 3};
        for (U i = 0; i < 5; ++i) {
            site(a, addresses[linear ? 1 : 0][i]);
            const U sampler = i == 0 ? first_shadow_sampler : read<U>(a.pass, 0x78);
            set_native_material_sampler_state_00b5ed60(a.pass, sampler, states[i], values[i]);
        }
    }
}
} // namespace

NativeMaterialProgramCompilerActualFrame::~NativeMaterialProgramCompilerActualFrame() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}

NativeMaterialPassStorage* NativeMaterialProgramCompilerActual::continue_material_pass_00b3b3c0(
    NativeMaterialProgramCompilerFrame& parent, NativeMaterialProgramCompilerResume resume,
    NativeMaterialProgramChild& retained) {
    require(&retained == &parent.tail_child && !retained,
        "actual compiler requires the original fresh tail_child");
    require(parent.phase == NativeMaterialProgramCompilePhase::required_tail && parent.resume == resume
        && parent.builder_live && parent.local_name_live,
        "actual compiler requires the reached prefix frame");
    require(resume == NativeMaterialProgramCompilerResume::source_after_vertex_inputs_00b3b513
        || resume == NativeMaterialProgramCompilerResume::cached_before_pass_allocation_00b3b536,
        "actual compiler continuation address is unsupported");
    auto& c = context_;
    require(c.registration.bind_pass && c.registration.bind_reflection && c.registration.bind_shader,
        "actual compiler requires canonical owner metadata registration");
    auto frame = std::make_unique<Frame>();
    Frame& a = *frame;
    retained = std::move(frame); // Persistent before the first native child.
    a.parent = &parent; a.context = &c; a.resume = resume;
    a.phase = Frame::Phase::running;
    auto& b = parent.builder;
    try {
        if (resume == NativeMaterialProgramCompilerResume::source_after_vertex_inputs_00b3b513) {
            auto& vs = child(a.vertex_system); site(a, 0x00b3b515);
            append_native_shader_vertex_system_fields_00b35be0(b, c.strings, vs);
            auto& select = child(a.interpolators[0]); site(a, 0x00b3b522);
            select_native_shader_interpolator_fields_00b36800(b, nullptr, nullptr, b.fields_1c, c.strings, select);
            auto& mapping = child(a.interpolators[1]); site(a, 0x00b3b52a);
            append_native_shader_interpolator_mapping_00b34aa0(b, b.fields_1c, mapping);
            auto& ps = child(a.pixel_system); site(a, 0x00b3b531);
            append_native_shader_pixel_system_fields_00b372d0(b, c.strings, ps);
        }
        site(a, 0x00b3b53b);
        a.raw_pass = allocate_static_native_material_pass_slot_00b41820();
        a.exception_state = 2;
        if (a.raw_pass) {
            auto& construction = child(a.pass_constructor);
            site(a, 0x00b3b552);
            a.pass = initialize_native_compiler_pass_00b44b10(a.raw_pass, c.pass_construction,
                c.texture_cache, c.actual_renderer_profile_00d5f0a8, construction);
            c.registration.bind_pass(c.registration.context, *a.pass);
            a.pass_registered = true;
        }
        a.exception_state = 0;
        for (U i = 0; i < 2; ++i) {
            site(a, i ? 0x00b3b5c8 : 0x00b3b56e);
            void* raw = allocate(0x88);
            a.reflection_owners[i] = static_cast<NativeCompiledShaderStorage*>(raw);
            if (raw) {
                a.reflection_owners[i] = initialize_native_compiled_shader_00b3b3c0_fragment(raw);
                c.registration.bind_reflection(c.registration.context, *a.reflection_owners[i]);
                a.reflection_registered[i] = true;
            }
            put<void*>(a.pass, i ? 0x74u : 0x70u, a.reflection_owners[i]);
        }
        a.texcoord_masks.fill(0); a.color_masks.fill(0);
        a.preliminary_pixel_orphan = nullptr;
        if (c.load_variants_0108d6f0 || c.source_mode_0108d6f1) {
            auto& source = child(a.preliminary_source); site(a, 0x00b3b688);
            build_native_shader_pixel_source_00b39880(b, b.fields_1c, b.fields_1c, c.source, source);
            const char* text = string_data(&b.source_4c, c);
            const char* profile = read<const char*>(parent.descriptor, 0x40);
            if (!profile) profile = c.actual_empty_0108d6f2;
            auto& compilation = child(a.preliminary_compile); site(a, 0x00b3b6c1);
            (void)compile_native_pixel_shader_00b61280(&b.name_9c, profile, text,
                &a.preliminary_pixel_orphan, a.texcoord_masks.data(), a.color_masks.data(), c.pixel_compilation, compilation);
            if (!a.preliminary_pixel_orphan) return finish(a, true);
        }
        const bool vertex_sampler = has_vertex_sampler(a);
        site(a, 0x00b3b763);
        const bool format = check_native_renderer_vertex_texture_render_target_00b20190(c.actual_renderer_00f8d394, 0x71);
        a.vertex_texture_unsupported = !format && vertex_sampler;
        a.vertex_shader = nullptr;
        if (!a.vertex_texture_unsupported) {
            if (!c.load_variants_0108d6f0 && !c.source_mode_0108d6f1) cached_shader(a, false);
            else {
                clear_array(a, b.fields_28, 4, 10, 0x00b3b88e, 0x00b3b8ba, 0x00b3b90d);
                auto& selection = child(a.interpolators[2]); site(a, 0x00b3b910);
                select_native_shader_interpolator_fields_00b36800(b, a.texcoord_masks.data(),
                    a.color_masks.data(), b.fields_28, c.strings, selection);
                clear_array(a, b.words_54, 2, 1, 0x00b3b923, 0x00b3b953, 0x00b3b998);
                clear_array(a, b.words_60, 2, 1, 0x00b3b9a9, 0x00b3b9dd, 0x00b3ba23);
                auto& mapping = child(a.interpolators[3]); site(a, 0x00b3ba26);
                append_native_shader_interpolator_mapping_00b34aa0(b, b.fields_28, mapping);
                clear_array(a, b.fields_1c, 4, 10, 0x00b3ba3c, 0x00b3ba69, 0x00b3baaf);
                auto& all = child(a.interpolators[4]); site(a, 0x00b3bab2);
                select_native_shader_interpolator_fields_00b36800(b, nullptr, nullptr, b.fields_1c, c.strings, all);
                auto& source = child(a.vertex_source); site(a, 0x00b3bab9);
                generate_native_shader_vertex_source_00b39110(b, c.source, source);
                const char* text = string_data(&b.source_4c, c);
                const char* profile = read<const char*>(parent.descriptor, 0x38);
                if (!profile) profile = c.actual_empty_0108d6f2;
                auto& compilation = child(a.vertex_compile); site(a, 0x00b3bae9);
                (void)compile_native_vertex_shader_00b60f60(&b.name_9c, profile, text,
                    &a.vertex_shader, c.vertex_compilation, compilation);
                if (!a.vertex_shader) return finish(a, true, 0x00b3bb14);
                a.vertex_size = 0; site(a, 0x00b3bb37);
                (void)a.vertex_shader->GetFunction(nullptr, &a.vertex_size);
                site(a, 0x00b3bb3e);
                a.vertex_bytecode = allocate(a.vertex_size);
                site(a, 0x00b3bb5c);
                (void)a.vertex_shader->GetFunction(a.vertex_bytecode, &a.vertex_size);
                reflect(a, false, a.vertex_bytecode, 0x00b3bb69);
                write_cache(a, false);
                site(a, 0x00b3bc73); singleton_lifetime_free(a.vertex_bytecode);
                // Preserve stale native buffer identity in diagnostic metadata.
            }
        }
        construct_shader_owner(a, false);
        a.pixel_shader = nullptr;
        if (!a.vertex_texture_unsupported) {
            if (!c.load_variants_0108d6f0 && !c.source_mode_0108d6f1) cached_shader(a, true);
            else {
                auto& source = child(a.pixel_source); site(a, 0x00b3bdf2);
                build_native_shader_pixel_source_00b39880(b, b.fields_1c, b.fields_28, c.source, source);
                const char* text = string_data(&b.source_4c, c);
                a.final_pixel_sentinel = 500;
                const char* profile = read<const char*>(parent.descriptor, 0x40);
                if (!profile) profile = c.actual_empty_0108d6f2;
                auto& compilation = child(a.pixel_compile); site(a, 0x00b3be30);
                (void)compile_native_pixel_shader_00b61280(&b.name_9c, profile, text,
                    &a.pixel_shader, &a.final_pixel_sentinel, &a.final_pixel_sentinel, c.pixel_compilation, compilation);
                if (!a.pixel_shader) return finish(a, true);
                a.pixel_size = 0; site(a, 0x00b3be51);
                (void)a.pixel_shader->GetFunction(nullptr, &a.pixel_size);
                site(a, 0x00b3be58); a.pixel_bytecode = allocate(a.pixel_size);
                site(a, 0x00b3be76);
                (void)a.pixel_shader->GetFunction(a.pixel_bytecode, &a.pixel_size);
                reflect(a, true, a.pixel_bytecode, 0x00b3be7f);
                write_cache(a, true);
                site(a, 0x00b3bf8b); singleton_lifetime_free(a.pixel_bytecode);
            }
        }
        construct_shader_owner(a, true);
        final_states(a);
        return finish(a, false);
    } catch (...) { a.phase = Frame::Phase::failed; throw; }
}
} // namespace bsp
