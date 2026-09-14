#include "bsp/native_material_constant_build.hpp"
#include "bsp/native_material_constant_build_leaves.hpp"
#include "bsp/native_material_texture_source_constants.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/native_renderer_texture_binding.hpp"
#include "bsp/native_renderer_binding_getters.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_inverse_world_getter.hpp"
#include "bsp/native_instance_collection.hpp"
#include "bsp/native_instance_group_upload.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_system_constant_gather.hpp"
#include "bsp/gui_material_binding.hpp"
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Signed = std::int32_t;
using Context = NativeMaterialConstantBuildContext;
using Frame = NativeMaterialConstantBuildFrame;
static_assert(sizeof(void*) == 4);
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
void* at(const volatile void* p, Word offset = 0) noexcept {
    return pointer(reinterpret_cast<Word>(p) + offset);
}
Word word(const volatile void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
std::uint8_t byte(const volatile void* p, Word offset) noexcept {
    return *static_cast<const volatile std::uint8_t*>(at(p, offset));
}
void* ptr(const volatile void* p, Word offset = 0) noexcept {
    return pointer(word(p, offset));
}
void put(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
void require(bool condition) {
    if (!condition) throw std::logic_error("native material constant build current virtual target is unsupported");
}
void reached(Frame& f, Word site) noexcept { f.reached_callsite = site; }
const volatile Word* renderer_table(void* renderer, Context& c) {
    require(word(renderer) == 0x00d5f0a8);
    return c.renderer_profile_00d5f0a8;
}
const volatile Word* source_table(void* source, Context& c) {
    const auto profile = word(source);
    if (profile == 0x00d64478) return c.caustics_profile_00d64478;
    if (profile == 0x00d644b4) return c.shore_profile_00d644b4;
    require(false);
    return nullptr; // Unreachable: never supplies a successful fallback.
}
void bind(void* renderer, Word target, Word sampler, void* texture, Context& c) {
    require(target == 0x00b24710);
    bind_native_renderer_texture_00b24710(renderer, sampler, texture, *c.texture_binding);
}
void copy_x87(void* destination, const volatile void* source) noexcept {
    __asm {
        mov eax, source
        mov edx, destination
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}
// Width2 is the native MOVSS/local scratch/REP MOVSD8 path. Other accepted
// widths perform individual FLD/FSTP crossings and retain overlap effects.
void transpose(void* destination, const void* source, Word rows) noexcept {
    if (rows == 2) {
        Word scratch[8];
        for (Word row = 0; row < 2; ++row)
            for (Word column = 0; column < 4; ++column)
                scratch[row * 4 + column] = word(source, column * 16 + row * 4);
        for (Word i = 0; i < 8; ++i) put(destination, i * 4, scratch[i]);
    } else if (rows == 3 || rows == 4) {
        for (Word row = 0; row < rows; ++row)
            for (Word column = 0; column < 4; ++column)
                copy_x87(at(destination, row * 16 + column * 4),
                    at(source, column * 16 + row * 4));
    }
}
void* reg(void* bank, Word index) noexcept { return at(bank, index << 4); }
void ensure_world(void* node, Frame& f, Word site) {
    if ((byte(node, 0x5c) & 2) == 0) {
        reached(f, site);
        refresh_native_camera_world_00b6db70(node);
    }
}
void scalar_bits(void* destination, Word first) noexcept {
    put(destination, 0, first);
    put(destination, 4, 0);
    put(destination, 8, 0);
    put(destination, 12, 0);
}
void light_row(void* destination, const void* array, Word index) noexcept {
    void* const light = ptr(ptr(array), index * 4);
    const Word x = word(light, 0x1ec), y = word(light, 0x1f0);
    const Word z = word(light, 0x1f4), w = word(light, 0x1f8);
    put(destination, 0, x); put(destination, 4, y);
    put(destination, 8, z); put(destination, 12, w);
    for (Word i = 0; i < 4; ++i) put(destination, 16 + i * 4, word(light, 0x184 + i * 4));
}
void unsigned_count_float(void* destination, Word count, const volatile float& bias) noexcept {
    float rounded;
    const volatile float* const actual_bias = &bias;
    __asm {
        fild dword ptr count
        test count, 80000000h
        jz nonnegative
        mov eax, actual_bias
        fadd dword ptr [eax]
    nonnegative:
        fstp rounded
        fld rounded
        mov edx, destination
        xorps xmm0, xmm0
        movss [edx+4], xmm0
        movss [edx+8], xmm0
        fstp dword ptr [edx]
        movss [edx+12], xmm0
    }
}
struct Failure {
    Frame& frame;
    ~Failure() noexcept {
        if (frame.phase == Frame::Phase::running) frame.phase = Frame::Phase::failed;
    }
};
} // namespace

void build_native_material_constants_00b42350(void* pass, void* entry,
    void* unused_override, Context& c, Frame& frame) {
    (void)unused_override;
    if (frame.phase != Frame::Phase::fresh)
        throw std::logic_error("native material constant build frame cannot be replayed");
    frame.phase = Frame::Phase::running;
    Failure failure{frame};

    // B42370..B423C3: binding pointer and owner are reloaded after each call.
    Word binding_index = 0;
    void* binding_slot = at(pass, 0x5c);
    while (binding_index < word(pass, 0x6c)) {
        void* binding = ptr(binding_slot);
        void* source = ptr(binding, 4);
        reached(frame, 0x00b42380);
        const Word getter = word(source_table(source, c), 0x2c);
        void* renderer = c.actual_renderer_00f8d394;
        require(getter == 0x00c302f0);
        void* texture = native_texture_source_current_texture_00c302f0(source);
        reached(frame, 0x00b42392);
        const Word target = word(renderer_table(renderer, c), 0x130);
        binding = ptr(binding_slot);
        bind(renderer, target, word(binding), texture, c);
        binding = ptr(binding_slot);
        source = ptr(binding, 4);
        void* const ps = ptr(pass, 0x74);
        reached(frame, 0x00b423b8);
        const Word writer = word(source_table(source, c), 0x30);
        void* const vs = ptr(pass, 0x70);
        if (writer == 0x00bbcc40)
            native_caustics_source_constants_00bbcc40(source, nullptr, entry, vs, ps,
                at(c.bank0108EBF4, 0xfffffffcu), at(c.bank0108DBEC, 0xfffffffcu));
        else if (writer == 0x00bbcbd0)
            native_shore_source_constants_00bbcbd0(source, nullptr, entry, vs, ps,
                at(c.bank0108EBF4, 0xfffffffcu), at(c.bank0108DBEC, 0xfffffffcu));
        else require(false);
        ++binding_index;
        binding_slot = at(binding_slot, 4);
    }

    reached(frame, 0x00b423d2);
    void* const parameter_table = native_material_parameter_table_00b17390(ptr(ptr(entry, 4), 0x20));
    const Word type = word(ptr(entry, 0x10), 0x198);
    void* parameter_slot = parameter_table;
    while (parameter_slot != at(parameter_table, word(parameter_table, 0x80) * 4)) {
        void* parameter = ptr(parameter_slot);
        const Word vertex_register = word(parameter, 0x14 + type * 4);
        if (static_cast<Signed>(vertex_register) > -1) {
            if (byte(parameter, 0x10) != 0) {
                void* shader = ptr(pass, 0x70);
                void* constant = ptr(shader, 0x78);
                Word rows = 0;
                while (constant != at(ptr(shader, 0x78), word(shader, 0x7c) * 32)) {
                    reached(frame, 0x00b42437);
                    if (static_cast<Word>(native_shader_constant_register_00b5b870(constant)) == vertex_register) {
                        reached(frame, 0x00b42442);
                        rows = native_shader_constant_register_count_00b5b880(constant);
                    }
                    constant = at(constant, 32);
                    shader = ptr(pass, 0x70);
                }
                parameter = ptr(parameter_slot);
                transpose(reg(c.bank0108EBF4, vertex_register), ptr(parameter, 8), rows);
            } else {
                const Word bytes = word(parameter, 0x0c) * 4;
                void* const source = ptr(parameter, 8);
                reached(frame, 0x00b425c7);
                std::memmove(reg(c.bank0108EBF4, vertex_register), source, bytes);
            }
        }
        parameter = ptr(parameter_slot);
        const Word pixel_register = word(parameter, 0x4c + type * 4);
        if (static_cast<Signed>(pixel_register) > -1) {
            void* const destination = reg(c.bank0108DBEC, pixel_register);
            if (byte(parameter, 0x10) != 0)
                transpose(destination, ptr(parameter, 8), 4);
            else {
                const Word bytes = word(parameter, 0x0c) * 4;
                void* const source = ptr(parameter, 8);
                reached(frame, 0x00b42665);
                std::memmove(destination, source, bytes);
            }
        }
        parameter_slot = at(parameter_slot, 4);
    }

    if (byte(ptr(pass, 0x70), 0x30) != 0xff) {
        void* const object = ptr(entry, 0x0c);
        reached(frame, 0x00b426a8);
        const Signed count = native_model_bone_count_00b8ff00(object);
        const Word first = byte(ptr(pass, 0x70), 0x30);
        void* destination = reg(c.bank0108EBF4, first);
        for (Signed index = 0; index < count; ++index) {
            reached(frame, 0x00b426d5);
            void* const node = native_model_bone_node_00b90620(object, nullptr, static_cast<Word>(index));
            ensure_world(node, frame, 0x00b426e4);
            transpose(destination, at(node, 0xf0), 3);
            destination = at(destination, 48);
        }
    }

    Word skin_register = byte(ptr(pass, 0x70), 0x32);
    if (skin_register != 0xff) {
        void* const object = ptr(entry, 0x0c);
        void* const animator = ptr(object, 0x130);
        bool optimized = false;
        if (animator) {
            const Word profile = word(animator); // Capture BEFORE B75E50.
            reached(frame, 0x00b42794);
            const Word required_type = native_optimized_animator_type_00b75e50(c.actual_types_010900fc);
            reached(frame, 0x00b427a7);
            require(profile == 0x00d62eb0);
            require(word(c.animator_profile_00d62eb0, 0x0c) == 0x00b782d0);
            optimized = native_optimized_animator_has_type_00b782d0(animator,
                c.actual_types_010900fc, required_type) != 0;
        }
        if (optimized) {
            void* const palette = ptr(object, 0x190);
            if (palette) {
                const Word bytes = word(ptr(animator, 0x34), 8) * 48;
                reached(frame, 0x00b427d3);
                std::memmove(reg(c.bank0108EBF4, skin_register), palette, bytes);
            }
        } else {
            Signed remaining = static_cast<Signed>(word(object, 0x188));
            skin_register = byte(ptr(pass, 0x70), 0x32);
            Word offset = 0;
            void* destination = reg(c.bank0108EBF4, skin_register);
            while (remaining > 0) {
                void* const node = ptr(ptr(object, 0x184), offset);
                ensure_world(node, frame, 0x00b42825);
                void* const inverse_bind = at(ptr(object, 0x184), offset + 0x20);
                reached(frame, 0x00b4283a);
                void* const inverse_world = get_native_node_inverse_world_00b6e0d0(object);
                Word intermediate[16], result[16];
                reached(frame, 0x00b4285c);
                void* const first = multiply_native_camera_matrices_00413920(inverse_bind,
                    nullptr, intermediate, at(node, 0xf0));
                reached(frame, 0x00b42863);
                multiply_native_camera_matrices_00413920(first, nullptr, result, inverse_world);
                transpose(destination, result, 3);
                offset += 0x60;
                destination = at(destination, 48);
                --remaining;
            }
        }
    }

    void* shader = ptr(pass, 0x70);
    if (byte(shader, 0x20) != 0xff) {
        Word first = byte(shader, 0x20), second = byte(shader, 0x21);
        Word remaining = second - first;
        const Word instance_type = word(ptr(entry, 0x10), 0x198);
        void* const descriptor = ptr(ptr(pass, 0x14), 0xc4);
        Word budget = word(descriptor, instance_type == 2 ? 0x24u : 0x20u);
        Word ordinal = 0, offset = 0x3c;
        void* section = ptr(entry, 4);
        while (ordinal < word(section, 0x4c)) {
            if (budget == 0 || remaining == 0) break;
            void* const logical = ptr(section, offset);
            const bool decoded = word(logical, 0x50) != 0;
            const Word available = second - first;
            reached(frame, 0x00b42966);
            require(word(logical) == 0x00d61d6c);
            require(word(c.logical_vertex_profile_00d61d6c, 0x24) == 0x00b48ce0);
            void* const declaration = native_logical_vertex_stream_get_declaration_00b48ce0(logical);
            reached(frame, 0x00b4296a);
            Word count = native_vertex_declaration_element_count_00b47900(declaration);
            if (static_cast<Signed>(count) > static_cast<Signed>(available)) count = available;
            Word index = 0;
            if (static_cast<Signed>(count) > 0) {
                const Word one = word(&c.actual_one_00d7a24c);
                void* scale = reg(c.bank0108EBF4, first);
                void* bias = reg(c.bank0108EBF4, second);
                first += count;
                second += count;
                // B429C2 executes once. B42A4F loops to B429C5, preserving
                // ECX=1; decoded calls explicitly restore it to1 as well.
                Word step = index + 1;
                do {
                    if (decoded) {
                        reached(frame, 0x00b429cf);
                        void* const decode = native_logical_vertex_decode_record_00b61e10(logical, nullptr, index);
                        copy_x87(scale, decode);
                        static_cast<void>(word(&c.actual_one_00d7a24c));
                        for (Word j = 1; j < 4; ++j) copy_x87(at(scale, j * 4), at(decode, j * 4));
                        for (Word j = 0; j < 4; ++j) copy_x87(at(bias, j * 4), at(decode, 16 + j * 4));
                        step = 1;
                    } else {
                        for (Word j = 0; j < 4; ++j) put(scale, j * 4, one);
                        for (Word j = 0; j < 4; ++j) put(bias, j * 4, 0);
                    }
                    budget -= step;
                    remaining -= step;
                    index += step;
                    scale = at(scale, 16);
                    bias = at(bias, 16);
                } while (static_cast<Signed>(index) < static_cast<Signed>(count));
            }
            section = ptr(entry, 4);
            offset += 4;
            ++ordinal;
        }
    }

    shader = ptr(pass, 0x70);
    if (byte(shader, 8) != 0xff) {
        void* const node = ptr(entry, 0x0c);
        const Word rows = byte(shader, 0x3e);
        ensure_world(node, frame, 0x00b42a9f);
        const Word first = byte(ptr(pass, 0x70), 8);
        transpose(reg(c.bank0108EBF4, first), at(node, 0xf0), rows);
    }
    if (byte(ptr(pass, 0x74), 8) != 0xff) {
        void* const node = ptr(entry, 0x0c);
        const Word rows = byte(ptr(pass, 0x70), 0x3e); // Native PS uses VS width.
        ensure_world(node, frame, 0x00b42c84);
        const Word first = byte(ptr(pass, 0x74), 8);
        transpose(reg(c.bank0108DBEC, first), at(node, 0xf0), rows);
    }
    Word scalar_register = byte(ptr(pass, 0x70), 0x33);
    if (scalar_register != 0xff) scalar_bits(reg(c.bank0108EBF4, scalar_register), word(entry, 0x18));
    scalar_register = byte(ptr(pass, 0x74), 0x33);
    if (scalar_register != 0xff) scalar_bits(reg(c.bank0108DBEC, scalar_register), word(entry, 0x18));

    scalar_register = byte(ptr(pass, 0x74), 0x34);
    if (scalar_register != 0xff) {
        float leading, fraction;
        copy_x87(&leading, entry);
        void* const threshold_owner = ptr(entry, 8);
        copy_x87(&fraction, &c.actual_fraction_00d7a238);
        reached(frame, 0x00b42ecf);
        volatile float result = native_stream_threshold_fade_00b73770(threshold_owner, &c.threshold, leading, fraction);
        void* const destination = reg(c.bank0108DBEC, scalar_register);
        copy_x87(destination, &result);
        put(destination, 4, 0); put(destination, 8, 0); put(destination, 12, 0);
    }

    shader = ptr(pass, 0x70);
    if (byte(shader, 9) != 0xff) {
        void* const node = ptr(entry, 0x0c);
        const Word rows = byte(shader, 0x3f), first = byte(shader, 9);
        reached(frame, 0x00b42f13);
        void* const inverse = get_native_node_inverse_world_00b6e0d0(node);
        transpose(reg(c.bank0108EBF4, first), inverse, rows);
    }
    scalar_register = byte(ptr(pass, 0x74), 0x37);
    if (scalar_register != 0xff) {
        reached(frame, 0x00b43072);
        void* const color = native_material_diffuse_00b179f0(
            *static_cast<NativeMaterialStorage*>(ptr(ptr(entry, 4), 0x20)), 0);
        for (Word i = 0; i < 4; ++i) put(reg(c.bank0108DBEC, scalar_register), i * 4, word(color, i * 4));
    }
    scalar_register = byte(ptr(pass, 0x70), 0x37);
    if (scalar_register != 0xff) {
        reached(frame, 0x00b430ab);
        void* const color = native_material_diffuse_00b179f0(
            *static_cast<NativeMaterialStorage*>(ptr(ptr(entry, 4), 0x20)), 0);
        for (Word i = 0; i < 4; ++i) put(reg(c.bank0108EBF4, scalar_register), i * 4, word(color, i * 4));
    }
    if (word(pass, 0x7c) != 0xffffffffu) {
        void* const renderer = c.actual_renderer_00f8d394;
        void* const service = c.actual_service_00f8d39c;
        reached(frame, 0x00b430e3);
        const volatile Word* const profile = renderer_table(renderer, c);
        void* const texture = native_shadow_owner_texture_00b0d130(service);
        reached(frame, 0x00b430f5);
        const Word target = word(profile, 0x130);
        bind(renderer, target, word(pass, 0x7c), texture, c);
    }
    if (word(pass, 0x78) != 0xffffffffu) {
        void* const light_environment = ptr(ptr(entry, 0x0c), 0x170);
        void* const head = ptr(light_environment, 0x1c);
        void* const first = ptr(head);
        if (first == head) {
            reached(frame, 0x00b43112);
            _invalid_parameter_noinfo();
        }
        reached(frame, 0x00b4311a);
        void* const shadow = get_raw_light_shadow_00b7aab0(ptr(first, 8));
        if (shadow) {
            const Word shadow_profile = word(shadow);
            void* const renderer = c.actual_renderer_00f8d394;
            reached(frame, 0x00b43132);
            const volatile Word* const render_profile = renderer_table(renderer, c);
            require(shadow_profile == 0x00d5b5d8);
            require(word(c.shadow_profile_00d5b5d8, 8) == 0x00a8fcf0);
            void* const texture = get_raw_shadow_depth_texture_00a8fcf0(shadow, c.actual_shadow_target_00f8bbf0);
            reached(frame, 0x00b43141);
            const Word sampler = word(pass, 0x78);
            bind(renderer, word(render_profile, 0x130), sampler, texture, c);
        } else {
            void* const texture = ptr(pass, 0x84);
            void* const renderer = c.actual_renderer_00f8d394;
            reached(frame, 0x00b4315e);
            const Word target = word(renderer_table(renderer, c), 0x130);
            bind(renderer, target, word(pass, 0x78), texture, c);
        }
    }

    reached(frame, 0x00b43163);
    void* const lights = native_node_point_light_array_00b6dc50(ptr(entry, 0x0c));
    const Word count_register = byte(ptr(pass, 0x70), 0x35);
    if (count_register != 0xff) {
        Signed count = static_cast<Signed>(word(lights, 4));
        if (count >= 4) count = 4;
        unsigned_count_float(reg(c.bank0108EBF4, count_register), static_cast<Word>(count), c.actual_unsigned_bias_00ce3978);
        const Word first = byte(ptr(pass, 0x70), 0x36);
        if (first != 0xff) {
            Word index = 0;
            void* destination = reg(c.bank0108EBF4, first);
            // The native four-row unroll has the same ordered reads/stores as
            // these four calls. Negative counts enter the unsigned tail.
            if (count >= 4) {
                for (; index < 4; ++index) {
                    light_row(destination, lights, index);
                    destination = at(destination, 32);
                }
            }
            while (index < static_cast<Word>(count)) {
                light_row(destination, lights, index);
                destination = at(destination, 32);
                ++index;
            }
        }
    }
    frame.phase = Frame::Phase::completed;
}
} // namespace bsp
