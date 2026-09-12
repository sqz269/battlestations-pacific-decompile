#include "bsp/gui_listbox_runtime.hpp"
#include "bsp/gui_text_lifetime.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
float x87_argument(float value) {
    __asm {
        fld value
        fstp value
    }
    return value;
}
float vertical_step(float height, const float* distance, float accumulator) {
    __asm {
        mov eax, distance
        fld height
        fadd dword ptr [eax]
        fadd accumulator
        fstp accumulator
    }
    return accumulator;
}
float horizontal_step(const float* width, const volatile double* divisor,
    const volatile double* gap, float accumulator) {
    float narrowed;
    __asm {
        mov eax, width
        fld dword ptr [eax]
        mov eax, divisor
        fdiv qword ptr [eax]
        fstp narrowed
        fld narrowed
        mov eax, gap
        fadd qword ptr [eax]
        fadd accumulator
        fstp accumulator
    }
    return accumulator;
}
float center_offset(float accumulator, const volatile double* half) {
    __asm {
        fld accumulator
        fchs
        mov eax, half
        fmul qword ptr [eax]
        fstp accumulator
    }
    return accumulator;
}
} // namespace

void set_gui_widget_local_xy_00aa7d00(GuiWidgetOwner& owner, float x, float y) {
    auto& p = owner.layout().transform.position;
    const float z = p.z;
    p.x = x; p.y = y; p.z = z;
    owner.recompose_00aa7220();
    owner.refresh_bounds_00aa70e0();
}
void set_gui_widget_local_y_00aa78f0(GuiWidgetOwner& owner, float y) {
    owner.layout().transform.position.y = y;
    owner.recompose_00aa7220();
}
void request_gui_sound_00941250(
    const std::function<SoundRequestQueue&()>& get_004c1b90,
    bool first_cl, bool second_dl) {
    // 941252 gives DL precedence, including when both flags are set. Do not
    // fetch/create the actual singleton on the all-clear path.
    if (second_dl) get_004c1b90().requested[2] = 1;
    else if (first_cl) get_004c1b90().requested[1] = 1;
}

class GuiListboxRuntime::Operation final {
public:
    explicit Operation(GuiListboxRuntime& runtime) : runtime_(runtime) {
        ++runtime_.active_calls_;
    }
    ~Operation() { --runtime_.active_calls_; }
private:
    GuiListboxRuntime& runtime_;
};

GuiListboxRuntime::GuiListboxRuntime(GuiWidgetOwner& owner,
    GuiListboxRuntimeServices services)
    : owner_(owner), services_(services), selected_(rows_.end()) {
    if (owner.layout().type != GuiWidgetType::Listbox ||
        owner.layout().transform.type_id != 11)
        throw std::invalid_argument("Listbox companion requires the actual type11 owner");
    if (&owner.runtime().environment().models.nodes != &services.parenting.nodes)
        throw std::invalid_argument("Listbox companion requires the same actual node owners");
}
GuiListboxRuntime::~GuiListboxRuntime() noexcept {
    if (active_calls_ != 0) std::terminate();
}
void GuiListboxRuntime::set_listener_00a9ac40(void* listener) noexcept {
    listener_114_ = listener;
}
std::uint32_t GuiListboxRuntime::row_count_104() const noexcept {
    return static_cast<std::uint32_t>(rows_.size());
}
GuiWidgetOwner* GuiListboxRuntime::selected_row_00425e50() const noexcept {
    return selected_ == rows_.end() ? nullptr : *selected_;
}
std::uint32_t GuiListboxRuntime::selected_data_d8_00a9c990() const noexcept {
    auto* const row = selected_row_00425e50();
    if (!row) return UINT32_MAX;
    static_assert(sizeof(void*) == 4, "BSP targets MSVC Win32");
    return static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(row->extra_fields().pointer_d8));
}
std::int32_t GuiListboxRuntime::selected_index_00a9c920() const noexcept {
    std::uint32_t ordinal = 0;
    for (auto it = rows_.begin(); it != rows_.end(); ++it, ++ordinal)
        if (it == selected_) return static_cast<std::int32_t>(ordinal);
    return -1;
}
GuiWidgetOwner* GuiListboxRuntime::row_at_00a9be00(std::int32_t index) const noexcept {
    std::uint32_t ordinal = 0;
    for (auto* row : rows_) {
        if (ordinal == static_cast<std::uint32_t>(index)) return row;
        ++ordinal;
    }
    return nullptr;
}
void GuiListboxRuntime::select_index_00a9c7c0(std::int32_t index) {
    Operation operation(*this);
    selected_ = rows_.end();
    std::uint32_t ordinal = 0;
    for (auto it = rows_.begin(); it != rows_.end(); ++it, ++ordinal) {
        if (ordinal != static_cast<std::uint32_t>(index)) continue;
        if (!(*it)->scene_flags().hidden) selected_ = it;
        break;
    }
    refresh80_00a9c220(false); // A9C848, even for absent/hidden/negative index.
}
void GuiListboxRuntime::select_row_00a9c740(GuiWidgetOwner* row) {
    if (!row) return;
    Operation operation(*this);
    if (!row->scene_flags().hidden) {
        for (auto it = rows_.begin(); it != rows_.end(); ++it) {
            if (*it == row) { selected_ = it; break; }
        }
    }
    refresh80_00a9c220(false);
}
void GuiListboxRuntime::refresh80_00a9c220(bool force) {
    Operation operation(*this);
    if (!force && previous_110_ == selected_row_00425e50() && !fields_.paging_148)
        return;
    if (listener_114_)
        services_.calls.listener_current08(listener_114_, selected_row_00425e50(), owner_);
    // A9C2C1..A9C2F5 re-read both the list and selection after listener code.
    if (!rows_.empty() && selected_ != rows_.end())
        services_.sound_callback_f8bc0c(true, false);
    previous_110_ = selected_row_00425e50(); // A9C2FD then A9C302.
}
void GuiListboxRuntime::append_row_00a9d750(GuiWidgetOwner& row,
    std::unique_ptr<GuiLayoutWidget>& detached) {
    if (&row.runtime() != &owner_.runtime() || &row == &owner_)
        throw std::invalid_argument("Listbox append requires a distinct actual row in its owner domain");
    // A9D480's successful list domain is at most 3FFFFFFF entries. The C++
    // allocation preflight precedes native-visible calls; allocation/throw
    // timing is excluded from this owning-handle transport.
    if (rows_.size() >= 0x3fffffffu)
        throw std::length_error("Listbox FC list length overflow");
    if (detached && detached.get() != &row.layout())
        throw std::invalid_argument("Listbox detached handle must own the same row");
    if (row.layout().parent && detached)
        throw std::invalid_argument("Listbox row cannot have two owning allocations");
    if (!row.layout().parent && !detached)
        throw std::invalid_argument("Listbox standalone row requires its owning allocation");
    Operation operation(*this);
    row.set_visible34(true); // A9D765, BEFORE native AA83A0/AAA5A0.
    if (row.layout().parent)
        detached = detach_gui_widget_child_00aa83a0(owner_.runtime(), services_.parenting,
            *row.layout().parent, &row.layout());
    append_gui_widget_child_00aaa5a0(owner_.runtime(), services_.parenting,
        owner_.layout(), detached); // A9D774, null insertion position.
    rows_.push_back(&row); // A9D791/A9D79C and A9D7A1/A9D7A7.
    if (rows_.size() == 1) {
        selected_ = rows_.front()->scene_flags().hidden ? rows_.end() : rows_.begin();
        refresh80_00a9c220(false); // A9D850.
    }
    layout7c_00a9c0a0(); // A9D859.
}
void GuiListboxRuntime::remove_row_00a9be60(GuiWidgetOwner& row) {
    if (&row.runtime() != &owner_.runtime())
        throw std::invalid_argument("Listbox remove requires the same actual owner domain");
    Operation operation(*this);
    row.set_visible34(false); // A9BE6F precedes FC removal.
    // Native BD50 removes all equal row pointers; the real widget tree stays.
    for (auto it = rows_.begin(); it != rows_.end();) {
        if (*it != &row) { ++it; continue; }
        if (it == selected_) selected_ = rows_.end();
        it = rows_.erase(it);
    }
    selected_ = rows_.end();
    previous_110_ = nullptr;
    refresh80_00a9c220(false); // A9BEA8 may therefore be an ordinary early return.
    layout7c_00a9c0a0(); // A9BEB1.
}
void GuiListboxRuntime::set_layout_flags_00a9ac90(std::uint8_t dont_move,
    std::uint8_t center_vertical, std::uint8_t auto_control) noexcept {
    fields_.dont_move_items_11c = dont_move;
    fields_.center_vertical_11d = center_vertical;
    fields_.auto_control_11e = auto_control;
}
void GuiListboxRuntime::layout7c_00a9c0a0() {
    if (fields_.dont_move_items_11c) return;
    Operation operation(*this);
    float offset = 0.0f; // A9C0B9 XORPS, A9C0C6 MOVSS.
    for (auto it = rows_.begin(); it != rows_.end(); ++it) {
        if (fields_.horizontal_11f) {
            set_gui_widget_local_xy_00aa7d00(**it, x87_argument(offset), 0.0f);
            auto* const text = (*it)->text_lifetime();
            if (!text)
                throw std::logic_error("horizontal Listbox row114 requires its actual Text companion");
            offset = horizontal_step(&text->text().measured_width,
                &services_.layout_constants.width_divisor_00cec380,
                &services_.layout_constants.horizontal_gap_00d7a2f8, offset);
        } else {
            set_gui_widget_local_xy_00aa7d00(**it, 0.0f, x87_argument(offset));
            offset = vertical_step(widget_size((*it)->layout().transform).height,
                &fields_.line_distance_13c, offset);
        }
    }
    if (fields_.center_vertical_11d) {
        offset = center_offset(offset, &services_.layout_constants.half_00d7a280);
        for (auto it = rows_.begin(); it != rows_.end(); ++it) {
            set_gui_widget_local_y_00aa78f0(**it, x87_argument(offset));
            // This remains a HEIGHT pass even after horizontal first layout.
            offset = vertical_step(widget_size((*it)->layout().transform).height,
                &fields_.line_distance_13c, offset);
        }
    }
}
void GuiListboxRuntime::loaded78_00a9c050() {
    Operation operation(*this);
    owner_.base_loaded78_00aa7170();
    layout7c_00a9c0a0();
    if (!rows_.empty()) selected_ = rows_.begin();
    refresh80_00a9c220(false);
}
} // namespace bsp
