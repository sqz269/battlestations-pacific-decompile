#include "bsp/native_render_observed_zero.hpp"
namespace bsp {
class NativeGuiIconTextureApplyStorageObservedZeroIssuer final {
public:
    static void dispatch(NativeRenderActualOwners& owners, void* actual, NativeRenderObservedZeroFrame& frame) {
        NativeRenderObservedZeroAccess::dispatch(owners, actual, frame);
    }
};
} // namespace bsp
#include "bsp/native_gui_icon_texture_apply_storage.hpp"
#include "bsp/native_gui_icon_storage.hpp"
#include "bsp/native_gui_icon_native_size_storage.hpp"
#include "bsp/native_gui_icon_rebuild_storage.hpp"
#include "bsp/native_crt_stricmp.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Phase = NativeGuiIconTextureApplyStorageFrame::Phase;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
void* at(const void* p, Word delta = 0) noexcept { return pointer(bits(p) + delta); }
Word read(const void* p, Word delta = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, delta));
}
Word live(const Word& home) noexcept { return static_cast<const volatile Word&>(home); }
void put(Word& home, Word value) noexcept { static_cast<volatile Word&>(home) = value; }
void write(void* p, Word value) noexcept { *static_cast<volatile Word*>(p) = value; }
Word count(Word last, Word first) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(last - first) >> 6);
}
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
void checked(void* actual, Word first, Word index) {
    // Native checks use this captured ECX, then a fresh F8. A returning
    // qualified BF6713 is not replaced by a successful default or extra throw.
    if (first == 0 || index >= count(read(actual, 0xf8), first))
        invoke_native_invalid_parameter_00bf6713();
}
const volatile Word* mapped_table(const game::GameNativeReadOnlyData& data, Word profile, Word bytes) {
    const auto* const table = static_cast<const volatile Word*>(data.data_at(profile, bytes));
    require(bits(table) == profile, "Icon apply requires its genuine mapped CURRENT read-only table alias");
    return table;
}
bool dimensions_are_ordered_zero(void* actual_state) {
    Word selected = 0; // Source control, never a Native home/default.
    __asm {
        mov ecx, actual_state
        movss xmm1, dword ptr [ecx + 34h]
        xorps xmm0, xmm0
        ucomiss xmm1, xmm0
        lahf
        test ah, 44h
        jp dimensions_done
        movss xmm1, dword ptr [ecx + 38h]
        ucomiss xmm1, xmm0
        lahf
        test ah, 44h
        jp dimensions_done
        mov selected, 1
    dimensions_done:
    }
    return selected != 0;
}
void copy_returned_size(void* returned_output, void* actual_state) {
    __asm {
        mov eax, returned_output
        mov ecx, actual_state
        fld dword ptr [eax]
        fstp dword ptr [ecx + 34h]
        fld dword ptr [eax + 4]
        fstp dword ptr [ecx + 38h]
    }
}
} // namespace

struct NativeGuiIconTextureApplyStorageFrame::Match {
    Match() {} // No zeroing of output/scratch/argument Native words.
    Match* next{};
    Word index{}; // Source history only.
    void* state{};
    void* incoming_texture{};
    void* old_texture{};
    NativeTextureSurfaceReferenceIncrement captured_increment{}, captured_decrement{};
    bool publication_written{}, increment_entered{}, increment_returned{};
    bool decrement_entered{}, decrement_returned{}, zero_entered{}, zero_returned{};
    long decrement_result{};
    Word zero_profile{}, zero_target{};
    NativeRenderObservedZeroFrame cpp_zero;
    NativeGuiIconNativeSizeScratch size_scratch;
    Word size_output[2], uv_argument_home; // Ordinary Native backing, no DMI.
    void* size_texture{};
    void* returned_output{};
    bool size_entered{}, size_returned{}, size_copied{}, flags_written{};
    std::unique_ptr<NativeGuiIconTextureApplyRebuildInvocation> rebuild;
    Word rebuild_profile{}, rebuild_target{};
    bool preparation_entered{}, rebuild_entered{}, rebuild_returned{};
};

NativeGuiIconTextureApplyStorageFrame::NativeGuiIconTextureApplyStorageFrame() {}
NativeGuiIconTextureApplyStorageFrame::~NativeGuiIconTextureApplyStorageFrame() {
    // Caller retains UNKNOWN and all active/transitive borrowers. This loop
    // destroys only Source metadata under the explicit full-last-use contract.
    while (matches_) {
        auto* const old = matches_;
        matches_ = old->next;
        delete old;
    }
}
NativeGuiIconTextureApplyStorageFrame::Match& NativeGuiIconTextureApplyStorageFrame::prepare_match() {
    auto* const fresh = new Match;
    // Read CURRENT head after allocation/new-handler; keep distinct nested
    // records instead of overwriting a previously reached invocation.
    fresh->next = matches_;
    matches_ = fresh;
    return *fresh;
}

void apply_native_gui_icon_texture_storage_00ab3840(void* actual,
    volatile Word (&homes)[3], NativeGuiIconTextureApplyStorageFrame& f,
    NativeGuiIconTextureApplyStorageContext& c) {
    auto& a = f.observation_;
    require(a.phase == Phase::fresh, "Icon apply requires its distinct fresh retained frame");
    a.phase = Phase::entered;
    a.actual_icon = actual;
    a.argument_home_identity = &homes[0];
    Word delta = 0, index = 0;
    put(f.native_state_24, 0); // Incoming by-value name is owned, native state0.
    try {
        for (;;) {
            a.active_native_site = 0x00ab3866;
            const Word first = read(actual, 0xf4);
            put(f.delta_spill_10, delta); // Native store follows first F4 read.
            if (first == 0 || index >= count(read(actual, 0xf8), first)) break;
            a.active_native_site = 0x00ab389e;
            checked(actual, first, index);
            const Word name_base = read(actual, 0xf4);
            const Word argument_length = homes[0];
            void* const state_name = pointer(name_base + delta + 4u);
            const Word state_length = read(state_name);
            bool matched = false;
            if (state_length == argument_length) {
                Word comparison;
                if (state_length == 0) comparison = argument_length == 0 ? 0u : 0xffffffffu;
                else if (argument_length == 0) comparison = 1u;
                else {
                    const Word argument_data = homes[1]; // Before current state DATA.
                    const Word state_data = read(state_name, 4);
                    a.active_native_site = 0x00ab38d9;
                    comparison = static_cast<Word>(compare_native_crt_strings_00bf7fbf(
                        static_cast<const char*>(pointer(state_data)),
                        static_cast<const char*>(pointer(argument_data)),
                        c.locale_changed_0109de1c, c.stricmp_callees));
                }
                matched = comparison == 0;
            }
            if (matched) {
                // Metadata only, before the real post-comparison F4/F8/state/
                // incoming-HOME capture. No Native acquisition or home seed.
                auto& call = f.prepare_match();
                call.index = index;
                a.active_native_site = 0x00ab3907;
                checked(actual, read(actual, 0xf4), index);
                const Word state_base = read(actual, 0xf4);
                const Word incoming = homes[2];
                void* const state = pointer(state_base + delta);
                const Word old = read(state);
                call.state = state;
                call.incoming_texture = pointer(incoming);
                call.old_texture = pointer(old);
                if (old != incoming) {
                    write(state, incoming); // Publish BEFORE either import.
                    call.publication_written = true;
                    if (incoming != 0) {
                        auto* const count_home = static_cast<volatile long*>(pointer(incoming + 4u));
                        a.active_native_site = 0x00ab3928;
                        const auto increment = c.increment_00ce221c;
                        call.captured_increment = increment;
                        call.increment_entered = true;
                        (void)increment(count_home);
                        call.increment_returned = true;
                    }
                    if (old != 0) {
                        auto* const count_home = static_cast<volatile long*>(pointer(old + 4u));
                        a.active_native_site = 0x00ab3936;
                        const auto decrement = c.decrement_00ce2220;
                        call.captured_decrement = decrement;
                        call.decrement_entered = true;
                        const long result = decrement(count_home);
                        call.decrement_result = result;
                        call.decrement_returned = true;
                        if (result == 0) {
                            call.zero_profile = read(pointer(old));
                            call.zero_target = mapped_table(c.data, call.zero_profile, 4)[0];
                            require(call.zero_target == 0x00bd30e0,
                                "Icon apply old texture requires its genuine captured canonical current0");
                            a.active_native_site = 0x00ab3947;
                            call.zero_entered = true;
                            NativeGuiIconTextureApplyStorageObservedZeroIssuer::dispatch(
                                c.icon.actual_owners, pointer(old), call.cpp_zero);
                            call.zero_returned = true;
                            // Never read old texture/companion after terminal.
                        }
                    }
                }
                a.active_native_site = 0x00ab3949;
                if (dimensions_are_ordered_zero(state)) {
                    call.size_texture = pointer(read(state)); // CURRENT after counter/zero callbacks.
                    put(call.uv_argument_home, bits(at(state, 0xc)));
                    a.active_native_site = 0x00ab3972;
                    call.size_entered = true;
                    call.returned_output = write_native_gui_icon_native_size_storage_00ab17b0(
                        call.size_output, call.size_texture, call.uv_argument_home,
                        call.size_scratch, c.dimensions, c.size_constants);
                    call.size_returned = true;
                    copy_returned_size(call.returned_output, state);
                    call.size_copied = true;
                }
                a.active_native_site = 0x00ab3982;
                *static_cast<volatile std::uint8_t*>(at(state, 0x3c)) = 0;
                *static_cast<volatile std::uint8_t*>(at(state, 0x3d)) = 1;
                call.flags_written = true;
                const auto selected = *static_cast<const volatile std::int16_t*>(at(actual, 0xec));
                if (index == static_cast<Word>(static_cast<std::int32_t>(selected))) {
                    a.active_native_site = 0x00ab3995; // Cold preparation precedes this capture.
                    call.preparation_entered = true;
                    require(c.prepare_rebuild != nullptr, "Icon apply requires its genuine cold raw80 preparer");
                    call.rebuild = c.prepare_rebuild(c.rebuild_provider, actual);
                    require(call.rebuild && &call.rebuild->frame.context == &call.rebuild->context &&
                        &call.rebuild->context.owners == &c.icon.actual_owners &&
                        &call.rebuild->context.strings == &c.icon.strings,
                        "Icon apply raw80 must retain the SAME actual renderer/string/owner graph");
                    // No early profile/target capture across preparation.
                    call.rebuild_profile = read(actual);
                    call.rebuild_target = mapped_table(c.data, call.rebuild_profile, 0x84)[0x80 / 4];
                    require(call.rebuild_profile == 0x00d5c4c0 && call.rebuild_target == 0x00ab3cb0,
                        "Icon apply requires genuine captured default current80");
                    put(call.rebuild->state_home, index); // Full native PUSH DWORD.
                    a.active_native_site = 0x00ab39a0;
                    call.rebuild_entered = true;
                    rebuild_native_gui_icon_geometry_storage_00ab3cb0(actual,
                        call.rebuild->state_home, call.rebuild->frame, call.rebuild->context);
                    call.rebuild_returned = true;
                }
            }
            delta = live(f.delta_spill_10); // Reload even on mismatch/equal texture.
            ++index;
            delta += 0x40u;
            a.index = index;
            a.restored_delta = delta;
        }
        a.active_native_site = 0x00ab39b1;
        void* const captured_data = pointer(homes[1]);
        a.captured_footer_data = captured_data;
        a.footer_data_captured = true;
        put(f.native_state_24, 0xffffffffu);
        a.footer_disarmed = true;
        if (captured_data) {
            const Word length = homes[0];
            put(f.footer_homes_[2], 1); // Native unread third argument.
            const Word bytes = length + 1u;
            put(f.footer_homes_[1], bytes);
            put(f.footer_homes_[0], bits(captured_data));
            a.captured_footer_bytes = bytes;
            a.active_native_site = 0x00ab39d0;
            a.getter_entered = true;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                c.icon.strings.actual_published_01090aa8,
                c.icon.strings.actual_manager_publication_01090aa0);
            a.getter_returned = true;
            a.active_native_site = 0x00ab39d7;
            a.return_entered = true;
            return_native_string_pool_00bd1510(pool, captured_data, bytes,
                c.icon.strings.actual_small_returns_disabled_01090aa4);
            a.return_returned = true;
        }
        a.phase = Phase::normal_returned;
    } catch (...) {
        a.escaped_native_site = a.active_native_site;
        a.failure = std::current_exception();
        a.phase = Phase::unknown_source_escape;
        throw; // No guessed Native parameter/FH3 cleanup or extra decrement.
    }
}
} // namespace bsp
