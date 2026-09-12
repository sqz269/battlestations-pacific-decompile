#include "bsp/gui_listbox_runtime.hpp"
#include "bsp/gui_text_lifetime.hpp"
#include "bsp/gui_listbox_row_control.hpp"
#include "bsp/gui_resources.hpp"
#include "bsp/gui_text_type_dispatch.hpp"
#include "bsp/gui_widget_relative_bounds.hpp"
#include "bsp/gui_widget_frame_runtime.hpp"
#include "bsp/platform_cursor.hpp"
#include <cstring>
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
float add_float(float a, float b) {
    __asm {
        fld a
        fadd b
        fstp a
    }
    return a;
}
float add_float_pair(float a, float b, const float* c) {
    __asm {
        fld a
        fadd b
        mov eax, c
        fadd dword ptr [eax]
        fstp a
    }
    return a;
}
float multiply_float(float a, const std::uint32_t* b) {
    __asm {
        fld a
        mov eax, b
        fmul dword ptr [eax]
        fstp a
    }
    return a;
}
void subtract_positive_delay(float& delay, float seconds, const volatile float& zero) {
    const volatile float* const threshold = &zero;
    float* const destination = &delay;
    float saved;
    __asm {
        mov eax, destination
        movss xmm0, dword ptr [eax]
        mov eax, threshold
        comiss xmm0, dword ptr [eax]
        movss saved, xmm0
        jbe finished
        fld saved
        fsub seconds
        mov eax, destination
        fstp dword ptr [eax]
    finished:
    }
}
template<class T> T& produced(std::optional<T>& field, const char* message) {
    if (!field) throw std::logic_error(message);
    return *field;
}
std::int32_t signed_word(std::uint32_t bits) {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
} // namespace

GuiListboxInputSample read_gui_listbox_input_00696470(
    const std::function<std::uint8_t(std::int32_t)>& action) {
    // Reload the actual game binding inside the required4D92B0 service for
    // EACH call, including its live deadline map and sound request side effects.
    const auto previous = action(0x46);
    const auto next = action(0x47);
    const auto activate = action(0x4a);
    return {previous, next, activate};
}

GuiLayoutWidget* select_gui_highlight_widget_00aa0f50(
    const GuiResourceState& manager, std::int32_t selector) noexcept {
    if (selector == 1) return manager.highlight_frame;
    if (selector == 2) return manager.highlight_circle;
    return nullptr;
}
float gui_node_hierarchy_factor_00a9abd0(
    GuiWidgetOwnerRuntime& owners, NativeNodeBinding& node) {
    if (node.storage.parent_30)
        return multiply_float(gui_node_hierarchy_factor_00a9abd0(
            owners, owners.node(node.storage.parent_30)), &node.storage.scalar_ac);
    float factor;
    std::memcpy(&factor, &node.storage.scalar_ac, sizeof(factor));
    return x87_argument(factor);
}

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
void GuiListboxRuntime::select_first_selectable_00a9c310() {
    Operation operation(*this);
    for (auto it = rows_.begin(); it != rows_.end(); ++it) {
        if ((*it)->scene_flags().hidden) continue;
        selected_ = it;
        refresh80_00a9c220(false);
        return;
    }
}
void GuiListboxRuntime::navigate88_00a9c400(std::uint8_t forward) {
    Operation operation(*this);
    if (!has_selectable_gui_listbox_row_00a9ba40(*this)) {
        selected_ = rows_.end();
        refresh80_00a9c220(false);
        return;
    }
    if (forward) {
        // A9C45D compares row identities, not the iterators. With duplicate
        // rows an earlier entry can equal the last row and stop here.
        if (selected_row_00425e50() == row_at_00a9be00(
            signed_word(row_count_104() - 1u))) return;
        if (selected_ == rows_.end())
            throw std::logic_error("A9C46D cannot increment the native end iterator");
        ++selected_;
        if (selected_ == rows_.end())
            throw std::logic_error("A9C486 cannot dereference the native end iterator");
        if ((*selected_)->scene_flags().hidden) { --selected_; return; }
    } else {
        if (selected_ == rows_.begin()) return;
        --selected_; // Native permits decrementing end for a nonempty list.
        if ((*selected_)->scene_flags().hidden) { ++selected_; return; }
    }
    refresh80_00a9c220(false);
}
void GuiListboxRuntime::navigate84_00a9da60(std::uint8_t forward,
    GuiListboxFrameCalls& calls) {
    Operation operation(*this);
    if (!fields_.wrap_navigation_149) {
        navigate88_00a9c400(forward);
        return;
    }
    if (!has_selectable_gui_listbox_row_00a9ba40(*this)) {
        selected_ = rows_.end();
        refresh80_00a9c220(false);
        return;
    }
    const auto require_row = [&]() -> GuiWidgetOwner& {
        if (selected_ == rows_.end())
            throw std::logic_error("A9DA60 cannot dereference the native end iterator");
        return **selected_;
    };
    const auto increment = [&]() {
        (void)require_row();
        ++selected_;
    };
    const auto decrement = [&]() {
        if (rows_.empty() || selected_ == rows_.begin()) {
            selected_ = rows_.end(); // Native writes sentinel before reporting.
            throw std::logic_error("A9DA60 native decrement reached the end sentinel");
        }
        --selected_;
    };
    const auto hidden_disallowed = [&]() {
        return require_row().scene_flags().hidden && !fields_.allow_hidden_121;
    };
    if (forward) {
        // Native end->begin precedes an increment, so this visits the second
        // node when selection starts at end; it is not SelectFirstSelectable.
        if (selected_ == rows_.end()) selected_ = rows_.begin();
        for (;;) {
            increment();
            if (selected_ == rows_.end()) {
                const auto page_size = static_cast<std::uint32_t>(produced(
                    fields_.page_size_158, "A9DB3C reads native unwritten158"));
                const auto total = paging_row_count_154();
                const auto remaining = total - page_size; // wrapping SUB.
                if (page_size < total) {
                    auto& raw = produced(fields_.raw_ec, "A9DB4C reads native unwrittenEC");
                    if (raw != remaining) ++raw;
                }
                if (!fields_.paging_148) selected_ = rows_.begin();
                else {
                    auto& start = produced(fields_.page_start_164, "A9DB6C reads native unwritten164");
                    if (start >= signed_word(remaining)) {
                        decrement(); // actual postfix-decrement A9AF90.
                        if (listener_114_)
                            calls.listener_current0c(listener_114_, false, true, owner_);
                        break;
                    }
                    start = signed_word(static_cast<std::uint32_t>(start) + 1u);
                    rebuild_page_00a9d870();
                    selected_ = rows_.end();
                    decrement();
                }
            }
            if (!hidden_disallowed()) break;
        }
        if (fields_.paging_148)
            while (hidden_disallowed()) decrement();
    } else {
        for (;;) {
            if (selected_ == rows_.begin()) {
                auto& raw = produced(fields_.raw_ec, "A9DCA4 reads native unwrittenEC");
                if (raw) --raw;
                if (!fields_.paging_148) selected_ = rows_.end();
                else {
                    auto& start = produced(fields_.page_start_164, "A9DCC0 reads native unwritten164");
                    if (start <= 0) {
                        if (listener_114_)
                            calls.listener_current0c(listener_114_, true, false, owner_);
                        break;
                    }
                    start = signed_word(static_cast<std::uint32_t>(start) - 1u);
                    rebuild_page_00a9d870();
                    selected_ = rows_.begin();
                    increment(); // Native advances first, then common decrement.
                }
            }
            decrement();
            if (!hidden_disallowed()) break;
        }
        if (fields_.paging_148)
            while (hidden_disallowed()) increment();
    }
    refresh80_00a9c220(false);
}
void GuiListboxRuntime::update40_00a9d030(float seconds,
    const GuiListboxFrameServices& services) {
    Operation operation(*this);
    if (&services.base_frames.widgets() != &owner_.runtime() ||
        !services.base_frames.operation_active(owner_))
        throw std::logic_error("Listbox40 requires its actual frame runtime owner borrow");
    if (owner_.scene_flags().hidden || !owner_.scene_flags().active) return;
    if (!owner_.implementation().is_visible38(owner_)) return;
    const auto& manager = services.highlight.get_manager_004c12b0();
    if (!manager.blocked_70)
        throw std::logic_error("A9D069 reads native constructor-unwritten manager70");
    if (*manager.blocked_70) return;
    services.base_frames.update_base_from_active_00aa87b0(owner_, x87_argument(seconds));
    if (fields_.paging_148)
        subtract_positive_delay(produced(fields_.page_delay_168,
            "A9D08A reads native unwritten168"), seconds, services.zero_00d7a218);
    if (!has_selectable_gui_listbox_row_00a9ba40(*this)) {
        selected_ = rows_.end();
        refresh80_00a9c220(false);
    } else if (selected_ == rows_.end()) select_first_selectable_00a9c310();

    // Copy the chosen function pointer transport so native144 may be replaced
    // during the call without destroying a currently executing std::function.
    // Only this immediate callback is captured; global08 is reloaded on each frame.
    const auto callback = fields_.input_callback_144 ? fields_.input_callback_144
        : services.input_callback_f8bc08;
    const auto input = callback();
    if (input.activate_4a) {
        const auto get_device = [&](std::int32_t type) {
            auto* const groups = services.input_groups_00f8bbf4;
            if (!groups || static_cast<std::size_t>(type) >= groups->size())
                throw std::logic_error("Listbox40 requires its actual input backend class vectors");
            return get_input_class_device_004ba6d0(*groups, type, 0);
        };
        auto* const keyboard = get_device(0);
        auto* const gamepad = get_device(2); // Both getters before either2C.
        if ((keyboard && services.calls.device_current2c(*keyboard)) ||
            (gamepad && services.calls.device_current2c(*gamepad))) {
            if (listener_114_ && selected_row_00425e50() &&
                !selected_row_00425e50()->scene_flags().hidden)
                services.calls.listener_current04(listener_114_,
                    *selected_row_00425e50(), owner_);
            services_.sound_callback_f8bc0c(false, true);
        }
    } else if (input.next_47 != input.previous_46 && selected_ != rows_.end()) {
        // Native tests next47 first when the raw bytes differ. Two distinct
        // nonzero bytes therefore navigate forward, rather than cancelling.
        if (input.next_47 || input.previous_46) {
            navigate84_00a9da60(input.next_47 ? 1 : 0, services.calls);
            refresh80_00a9c220(false); // Deliberate second notification.
        }
    }
    if (fields_.auto_control_11e && owner_.scene_flags().active &&
        !owner_.scene_flags().hidden) {
        for (auto it = rows_.begin(); it != rows_.end(); ++it) {
            const auto state = it == selected_ ? 1 : ((*it)->scene_flags().hidden ? 3 : 0);
            apply_gui_listbox_row_state_00a9ba90(**it, state, services_.row_state_one_00d7a24c);
            // Same existing FC lifetime contract: current node survives until
            // native519E00 advances it. Re-read the selection on the next row.
        }
    }
    auto* const selected = selected_row_00425e50();
    if (fields_.new_highlight_120) {
        const auto& current_manager = services.highlight.get_manager_004c12b0();
        auto* const layout = select_gui_highlight_widget_00aa0f50(
            current_manager, fields_.highlight_index_118);
        if (!layout)
            throw std::logic_error("A9D2F9 requires its actual selector1/2 highlight");
        const float depth = x87_argument(resolved_position(layout->transform).z);
        update_highlight_00a9c540(selected, fields_.paging_148, depth, services.highlight);
    } else {
        const auto& current_manager = services.highlight.get_manager_004c12b0();
        auto* const layout = select_gui_highlight_widget_00aa0f50(current_manager, 0);
        if (!layout)
            throw std::logic_error("A9D333 native selector0 returns null before current34 dereference");
        auto& highlight = owner_.runtime().owner(*layout);
        highlight.set_visible34(selected != nullptr);
        if (selected)
            fit_gui_widget_to_source_00ac0820(*selected, highlight, 0.0f,
                services.highlight.relative_bounds);
    }
}
void GuiListboxRuntime::update_highlight_00a9c540(GuiWidgetOwner* row,
    std::uint8_t paging, float depth, const GuiListboxHighlightServices& services) {
    Operation operation(*this);
    // Native reads118 AFTER GetOrCreate, so the getter may update this field.
    const auto& manager = services.get_manager_004c12b0();
    auto* layout = select_gui_highlight_widget_00aa0f50(manager, fields_.highlight_index_118);
    if (!layout) throw std::logic_error("C540 requires its AA0F50 widget before its null-row test");
    auto& highlight = owner_.runtime().owner(*layout);
    const float old_y = resolved_position(layout->transform).y; // First6750.
    const float old_x = resolved_position(layout->transform).x; // Second6750.
    set_gui_widget_resolved_position_00aa8240(highlight, {old_x, old_y, depth});
    if (!row) {
        highlight.set_visible34(false);
        return;
    }
    auto* node = row->node_binding(); // A9C5C3, AFTER initial position callbacks.
    if (!node) throw std::logic_error("C540 requires the current row scene node");
    float factor;
    if (node->storage.parent_30)
        factor = multiply_float(gui_node_hierarchy_factor_00a9abd0(owner_.runtime(),
            owner_.runtime().node(node->storage.parent_30)), &node->storage.scalar_ac);
    else {
        std::memcpy(&factor, &node->storage.scalar_ac, sizeof(factor));
        factor = x87_argument(factor);
    }
    highlight.set_visible34(factor != 0.0f); // unordered/NaN yields AL1 too.
    fit_gui_widget_to_source_00ac0820(*row, highlight,
        x87_argument(fields_.highlight_width_124), services.relative_bounds);
    const auto fitted = resolved_position(layout->transform);
    fields_.highlight_position_f0 = GuiWidgetPoint{
        x87_argument(fitted.x), x87_argument(fitted.y), x87_argument(fitted.z)};
    const auto& cached = *fields_.highlight_position_f0;
    const float x = add_float(cached.x, fields_.highlight_offset_12c.x);
    const float y = add_float(fields_.highlight_offset_12c.y, cached.y);
    const float z = add_float(fields_.highlight_offset_12c.z, cached.z);
    const float y_adjust = paging ? services.paging_y_00d7a23c
        : services.ordinary_y_paging_height_00d5bbf0;
    // Native captures this extra-height float BEFORE8240 callbacks.
    const float height_adjust = paging ? services.ordinary_y_paging_height_00d5bbf0
        : services.ordinary_height_00d5bbec;
    set_gui_widget_resolved_position_00aa8240(highlight,
        {add_float(x, 0.0f), add_float(y, y_adjust), add_float(0.0f, z)});
    const float width = widget_size(layout->transform).width; // First6740.
    const float height = add_float_pair(widget_size(layout->transform).height,
        height_adjust, &fields_.highlight_height_128); // Second6740, live128.
    set_gui_widget_current_size58(highlight, {width, x87_argument(height)});
}
std::uint32_t GuiListboxRuntime::paging_row_count_154() const noexcept {
    return static_cast<std::uint32_t>(paging_rows_.size());
}
GuiWidgetOwner* GuiListboxRuntime::paging_row_at_14c(std::int32_t index) const noexcept {
    std::uint32_t ordinal = 0;
    for (auto* row : paging_rows_) {
        if (ordinal == static_cast<std::uint32_t>(index)) return row;
        ++ordinal;
    }
    return nullptr;
}
void GuiListboxRuntime::enable_paging_00a9e230(std::int32_t page_size,
    NativeStringStorage& strings, int (*compare)(const char*, const char*)) {
    Operation operation(*this);
    fields_.raw_ec = 0;
    fields_.paging_148 = 1;
    // D9D0 clear followed by D560 range insertion, preserving row identities
    // and duplicates. Allocation/SEH and allocator reentry are not projected.
    ++paging_assignment_generation_;
    paging_rows_.clear();
    for (auto* row : rows_) paging_rows_.push_back(row);
    fields_.page_size_158 = page_size;
    const bool horizontal = fields_.horizontal_11f != 0;
    auto find_arrow = [&](const char* spelling, std::optional<GuiWidgetOwner*>& slot) {
        NativeString name;
        name.assign_0041e870(strings, spelling);
        try {
            auto* found = find_child_by_name_00aa7e00(owner_.runtime(), owner_.layout(),
                name, 1, compare);
            slot = found ? &owner_.runtime().owner(*found) : nullptr;
        } catch (...) {
            destroy_native_string_header_0041dd20(&name, strings);
            throw;
        }
        // The slot is published BEFORE the native temporary name is released.
        destroy_native_string_header_0041dd20(&name, strings);
    };
    find_arrow(horizontal ? "ScrollLeft_Icon" : "ScrollUp_Icon", fields_.previous_arrow_15c);
    find_arrow(horizontal ? "ScrollRight_Icon" : "ScrollDown_Icon", fields_.next_arrow_160);
    fields_.page_start_164 = 0;
    fields_.page_delay_168 = 0.0f;
    select_index_00a9c7c0(0);
    rebuild_page_00a9d870();
}
void GuiListboxRuntime::rebuild_page_00a9d870() {
    Operation operation(*this);
    void* const listener = listener_114_;
    listener_114_ = nullptr;
    auto* const selected = selected_row_00425e50();
    while (!rows_.empty()) remove_row_00a9be60(*rows_.front());
    auto it = paging_rows_.begin();
    for (std::int32_t i = 0; i < fields_.page_start_164.value(); ++i) {
        if (it == paging_rows_.end())
            throw std::logic_error("D870 page start exceeds its native14C list");
        ++it;
    }
    for (std::int32_t i = 0; i < fields_.page_size_158.value(); ++i) {
        if (it == paging_rows_.end()) break;
        const auto generation = paging_assignment_generation_;
        std::unique_ptr<GuiLayoutWidget> detached;
        append_row_00a9d750(**it, detached);
        if (generation != paging_assignment_generation_)
            throw std::logic_error("D870 callback replaced its current14C list node");
        ++it; // Native reads current.next only AFTER append/attach callbacks.
    }
    if (selected) select_row_00a9c740(selected);
    listener_114_ = listener; // Native restoration is not an exception guard.
    refresh80_00a9c220(false);
    auto* previous_arrow = fields_.previous_arrow_15c.value();
    if (!previous_arrow) throw std::logic_error("D870 requires current15C arrow");
    previous_arrow->set_visible34(true);
    auto* next_arrow = fields_.next_arrow_160.value(); // Reload AFTER15C callback.
    if (!next_arrow) throw std::logic_error("D870 requires current160 arrow");
    next_arrow->set_visible34(true);
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
void GuiListboxRuntime::append_row_with_data_00a9da30(GuiWidgetOwner& row,
    std::uint32_t data, std::unique_ptr<GuiLayoutWidget>& detached) {
    row.extra_fields().pointer_d8 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(data));
    append_row_00a9d750(row, detached);
}
void GuiListboxRuntime::set_active60_00a9cd20(bool active) {
    Operation operation(*this);
    owner_.base_set_active60_00aa6a30(active); // A9CD2C precedes BOTH tail tests.
    if (active || fields_.auto_control_11e == 0) return;
    for (auto it = rows_.begin(); it != rows_.end(); ++it) {
        const std::int32_t state = it == selected_ ? 1 : 3; // A9CD7D..A9CD88.
        apply_gui_listbox_row_state_00a9ba90(**it, state, services_.row_state_one_00d7a24c);
        // A9CDA7 reloads current.next only AFTER row-state callbacks.
    }
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
