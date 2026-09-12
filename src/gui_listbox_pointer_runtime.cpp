#include "bsp/gui_listbox_pointer_runtime.hpp"
#include "bsp/gui_widget_frame_runtime.hpp"
#include <stdexcept>

namespace bsp {
namespace {
struct ActivePointer {
    std::uint32_t& count;
    explicit ActivePointer(std::uint32_t& active) : count(active) { ++count; }
    ~ActivePointer() { --count; }
};
bool ordered_zero(float value) noexcept {
    std::uint8_t proceed;
    // A9CABA..A9CAC6. Equal ->40h odd parity; nonzero ->00h and
    // unordered ->44h both even parity. Only ordered zero continues.
    __asm {
        fld value
        fldz
        fxch
        fucomip st(0), st(1)
        fstp st(0)
        lahf
        test ah, 44h
        setnp proceed
    }
    return proceed != 0;
}
bool page_delay_ready(const float& delay) noexcept {
    const float* address = &delay;
    std::uint8_t ready;
    // COMISS(positive0,delay); JC exits for delay>0 OR unordered.
    __asm {
        mov eax, address
        xorps xmm0, xmm0
        comiss xmm0, dword ptr [eax]
        setnc ready
    }
    return ready != 0;
}
void copy_delay(float& target, const volatile float& source) noexcept {
    float* destination = &target;
    const volatile float* input = &source;
    __asm {
        mov eax, input
        movss xmm0, dword ptr [eax]
        mov eax, destination
        movss dword ptr [eax], xmm0
    }
}
template<class T> T& produced(std::optional<T>& field, const char* message) {
    if (!field) throw std::logic_error(message);
    return *field;
}
} // namespace

std::uint8_t read_gui_listbox_pointer_activation_00696430(
    const std::function<std::uint8_t(std::int32_t)>& action) {
    return action(0x55); //69643B,4C43C0; raw AL copied at696440
}

void GuiListboxRuntime::pointer68_00a9ca60(GuiWidgetOwner* child,
    const GuiListboxPointerServices& services) {
    if (&services.frame.base_frames.widgets() != &owner_.runtime() ||
        !services.frame.base_frames.operation_active(owner_))
        throw std::logic_error("Listbox68 requires its actual frame runtime owner borrow");
    ActivePointer operation(active_calls_);
    if (!fields_.field_14a || owner_.scene_flags().hidden) return;
    if (fields_.field_138 && (!child || !child->scene_flags().active)) return;
    if (child && &child->runtime() != &owner_.runtime())
        throw std::logic_error("Listbox68 incoming widget belongs to another owner domain");

    const auto mouse = services.frame.input_groups_00f8bbf4.device(1, 0); //A9CAA8
    // Current mouse A99FE0 always spills binary32 before returning ST0,
    // including code10 atA9A0A0/A0A4. This float adds no precision loss.
    if (!ordered_zero(mouse.value_current24(10))) return; //A9CAB8
    const auto down = mouse.query_current20(0); //A9CAD5, SAME captured device
    if (!down && fields_.field_140) return;

    if (fields_.paging_148) {
        std::optional<std::uint8_t> direction;
        if (child == produced(fields_.previous_arrow_15c, "Listbox68 reads unproduced arrow15C"))
            direction = 0;
        else if (child == produced(fields_.next_arrow_160, "Listbox68 reads unproduced arrow160"))
            direction = 1;
        if (direction) {
            if (!down || !page_delay_ready(produced(fields_.page_delay_168,
                "Listbox68 reads unproduced delay168"))) return;
            navigate84_00a9da60(*direction, services.frame.calls); //A9CB40
            // Native reads current D5BBA0 only AFTER navigation callbacks.
            copy_delay(produced(fields_.page_delay_168, "Listbox68 writes missing delay168"),
                services.page_delay_00d5bba0); //A9CB42..4C
            return;
        }
    }

    // Capture only this immediate callable so it may replace its publication
    // while executing. It is never cached across other native callbacks.
    const auto activation = services.activation_callback_00f8bc84;
    if (activation()) { //A9CB5C; incoming stack slot reused for output byte
        if (selected_row_00425e50() != child) { //A9CB6B
            select_row_00a9c740(child); //A9CB77, itself calls80 for nonnull
            refresh80_00a9c220(false); //A9CB88, preserve second refresh
        }
        if (listener_114_ && selected_row_00425e50() &&
            !selected_row_00425e50()->scene_flags().hidden) {
            // Native captures114's table, calls pure425E50, then reloads114.
            // The helper has no callback in the valid single-thread domain;
            // captured target and fresh receiver therefore remain associated.
            auto* row = selected_row_00425e50(); //A9CBB6
            services.frame.calls.listener_current04(listener_114_, *row, owner_); //A9CBC5
        }
        const auto sound = services_.sound_callback_f8bc0c;
        sound(false, true); //A9CBCB even without listener/visible selected row
    }
    if (!fields_.field_141) return; //A9CBD1, reloaded after activation/sound
    const auto current_mouse = services.frame.input_groups_00f8bbf4.device(1, 0); //A9CBE4
    if (!current_mouse) return;
    //67C170(1) reads current0D then previous10D directly, no swapped-button
    // query, current validity flag, or reuse of the earlier mouse publication.
    if (!current_mouse.mouse_current_down(1) || current_mouse.mouse_previous_down(1)) return;
    if (listener_114_ && selected_row_00425e50() &&
        !selected_row_00425e50()->scene_flags().hidden) {
        auto* row = selected_row_00425e50(); //A9CC26, pure helper as above
        services.frame.calls.listener_current10(listener_114_, *row, owner_); //A9CC35
    }
    const auto sound = services_.sound_callback_f8bc0c;
    sound(false, true); //A9CC3B, fresh current sound callback
}
} // namespace bsp
