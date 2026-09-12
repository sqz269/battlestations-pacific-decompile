#pragma once
#include "bsp/gui_input_runtime.hpp"
#include "bsp/gui_widget_attach.hpp"
#include "bsp/render_tail.hpp"
#include "bsp/input_action_classifier.hpp"
#include <cstdint>
#include <functional>
#include <list>
#include <optional>

namespace bsp {
struct GuiResourceState;
struct GuiWidgetRelativeBoundsConstants;
class NativeStringStorage;
class GuiWidgetFrameRuntime;
struct GuiListboxPointerServices;

// The native fastcall writes ECX=previous46, EDX=next47, stack=accept4A,
// RET4. A completed adapter must return ALL three raw bytes; no zero sample
// is supplied when a producer is missing. The frame compares raw direction
// bytes before testing their individual nonzero predicates.
struct GuiListboxInputSample {
    std::uint8_t previous_46;
    std::uint8_t next_47;
    std::uint8_t activate_4a;
};
GuiListboxInputSample read_gui_listbox_input_00696470(
    const std::function<std::uint8_t(std::int32_t)>& actual_action_004d92b0);

// AA0F50, ECX actual manager, signed selector stack, EAX borrowed widget,
// RET4. Reads the existing AA5E20-produced manager slots; no cached selector.
GuiLayoutWidget* select_gui_highlight_widget_00aa0f50(
    const GuiResourceState&, std::int32_t selector) noexcept;
// A9ABD0, ECX actual native node, x87 float result, RET. Each parent product
// spills to float32. Uses the SAME native parent30/scalarAC words and registry.
float gui_node_hierarchy_factor_00a9abd0(GuiWidgetOwnerRuntime&, NativeNodeBinding&);
struct GuiListboxHighlightServices {
    const std::function<const GuiResourceState&()>& get_manager_004c12b0;
    const GuiWidgetRelativeBoundsConstants& relative_bounds;
    const volatile float& paging_y_00d7a23c;
    const volatile float& ordinary_y_paging_height_00d5bbf0;
    const volatile float& ordinary_height_00d5bbec;
};

// Actual borrowed identities only. Listener bodies and the concrete current2C
// dispatcher must be bound by the same application; no successful defaults.
class GuiListboxFrameCalls {
public:
    virtual ~GuiListboxFrameCalls() = default;
    // Native listener ECX, row/listbox stack, RET8.
    virtual void listener_current04(void*, GuiWidgetOwner& row,
        GuiWidgetOwner& listbox) = 0;
    // Native listener current10, row/listbox stack, RET8 (pointer68 path).
    virtual void listener_current10(void*, GuiWidgetOwner& row,
        GuiWidgetOwner& listbox) = 0;
    // Native listener ECX, first/second/listbox stack, RETC.
    virtual void listener_current0c(void*, bool first, bool second,
        GuiWidgetOwner& listbox) = 0;
    // Native ECX actual device, RET/AL. Keyboard D5B904 -> A95F00 ->
    // current28 A95ED0; gamepad D5B7F0/D5BB48 -> A93F60.
    virtual std::uint8_t device_current2c(InputDevice&) = 0;
};
struct GuiListboxFrameServices {
    GuiWidgetFrameRuntime& base_frames;
    const GuiListboxHighlightServices& highlight;
    GuiListboxFrameCalls& calls;
    const std::function<GuiListboxInputSample()>& input_callback_f8bc08;
    GuiInputSource input_groups_00f8bbf4;
    const volatile float& zero_00d7a218;
};

// 00941250: CL first flag, DL second flag, RET. The registered target of
// F8BC0C after 004DD6FB..004DD700. Uses the SAME request queue as 00941140;
// this is a request-byte producer, not playback or a replacement queue.
void request_gui_sound_00941250(
    const std::function<SoundRequestQueue&()>& get_004c1b90,
    bool first_cl, bool second_dl);

// Required operations on actual borrowed listener/widget owners. No empty
// default implementation is supplied. Their downstream bodies are boundaries.
class GuiListboxRuntimeCalls {
public:
    virtual ~GuiListboxRuntimeCalls() = default;
    // Native listener+114 current08(selected row or null, SAME listbox), RET8.
    virtual void listener_current08(void* listener_114,
        GuiWidgetOwner* selected_row, GuiWidgetOwner& listbox) = 0;
};

struct GuiListboxLayoutConstants {
    const volatile double& width_divisor_00cec380;
    const volatile double& horizontal_gap_00d7a2f8;
    const volatile double& half_00d7a280;
};
struct GuiListboxRuntimeServices {
    NativeNodeParentingRuntime& parenting;
    GuiListboxRuntimeCalls& calls;
    // The SAME currently installed F8BC0C binding. It is reloaded at each call;
    // bind 00941250 above only when the original initialization established it.
    const std::function<void(bool, bool)>& sound_callback_f8bc0c;
    GuiListboxLayoutConstants layout_constants;
    // Live D7A24C is needed only by the reached Text globals.live color branch
    // of automatic row control. No synthetic value for an unbound producer.
    const volatile float* row_state_one_00d7a24c{};
};

// Fields established by A9DF40, shared by the real companion. A9AC90 writes
// the three consecutive11C..11E bytes without normalizing their raw values.
// Later property/paged-list producers must write these same fields.
struct GuiListboxFields {
    std::int32_t highlight_index_118{-1};
    std::uint8_t dont_move_items_11c{};
    std::uint8_t center_vertical_11d{};
    std::uint8_t auto_control_11e{};
    std::uint8_t horizontal_11f{};
    std::uint8_t new_highlight_120{1}; // A9DF96; zero takes native null fault.
    std::uint8_t allow_hidden_121{}; // A9DFCA, navigation skip predicate.
    float highlight_width_124{};
    float highlight_height_128{};
    GuiWidgetPoint highlight_offset_12c{};
    std::uint8_t field_138{}; // A9E010=0; pointer68 requires incoming85 when set.
    float line_distance_13c{};
    std::uint8_t field_140{}; // A9E01E=0; pointer68 gates on current20 when set.
    std::uint8_t field_141{}; // A9E024=0; enables pointer68's second edge arm.
    std::function<GuiListboxInputSample()> input_callback_144; // A9E02A=null.
    std::uint8_t paging_148{};
    std::uint8_t wrap_navigation_149{1}; // A9DFFC; zero delegates to current88.
    std::uint8_t field_14a{1}; // A9E002=1; first pointer68 gate.
    // Constructor-unwritten storage has no invented value. E230 writes EC,
    // 158,15C,160,164,168 in native order; C540 writes F0..F8 after fitting.
    std::optional<std::uint32_t> raw_ec;
    std::optional<GuiWidgetPoint> highlight_position_f0;
    std::optional<std::int32_t> page_size_158;
    std::optional<GuiWidgetOwner*> previous_arrow_15c;
    std::optional<GuiWidgetOwner*> next_arrow_160;
    std::optional<std::int32_t> page_start_164;
    std::optional<float> page_delay_168;
};

// Actual owner adaptations of the existing named base routines. Both preserve
// MOVSS argument stores and the exact recompose/bounds distinction.
void set_gui_widget_local_xy_00aa7d00(GuiWidgetOwner&, float x, float y);
void set_gui_widget_local_y_00aa78f0(GuiWidgetOwner&, float y);

// Canonical Listbox companion, not a raw180h ABI substitute or another GUI
// tree. Its FC list borrows the actual row owners attached to owner.layout().
// A9DF40 establishes empty rows, selected=end, previous110=null, listener114=
// null and paging148=false. Only those constructor fields are projected here.
// Full properties A9E400 and scalar/copy teardown
// remain required integration work. Owner and every borrowed row must stay
// alive; remove a row here before its actual deleting destructor is invoked.
class GuiListboxRuntime final {
public:
    GuiListboxRuntime(GuiWidgetOwner&, GuiListboxRuntimeServices);
    ~GuiListboxRuntime() noexcept;
    GuiListboxRuntime(const GuiListboxRuntime&) = delete;
    GuiListboxRuntime& operator=(const GuiListboxRuntime&) = delete;
    GuiWidgetOwner& owner() const noexcept { return owner_; }
    bool has_active_operation() const noexcept { return active_calls_ != 0; }
    GuiListboxFields& fields() noexcept { return fields_; }
    const GuiListboxFields& fields() const noexcept { return fields_; }

    // A9AC40, ECX Listbox, borrowed listener argument, RET4. No retain/call.
    void set_listener_00a9ac40(void* listener) noexcept;
    void* listener_114() const noexcept { return listener_114_; }
    std::uint32_t row_count_104() const noexcept;

    // 425E50 is a correct existing STL instantiation name. Returns actual row,
    // null at end. A9C990 returns that row's LIVE D8 bits, FFFFFFFF at end;
    // A9C920 returns its list ordinal. These are deliberately separate values.
    GuiWidgetOwner* selected_row_00425e50() const noexcept;
    std::uint32_t selected_data_d8_00a9c990() const noexcept;
    std::int32_t selected_index_00a9c920() const noexcept;
    GuiWidgetOwner* row_at_00a9be00(std::int32_t ordinal) const noexcept;
    // ECX owner; ordinal/row pointer on stack; each RET4.
    void select_index_00a9c7c0(std::int32_t ordinal);
    void select_row_00a9c740(GuiWidgetOwner* row);
    // A9C310, ECX Listbox, RET. First row with hidden77==0, then current80(0).
    // Empty/all-hidden preserves current selection and does not notify.
    void select_first_selectable_00a9c310();
    // Complete native current88, RET4 raw direction. Moves at most one node;
    // a hidden target restores the old iterator without notifying. The forward
    // end test compares actual row POINTERS, retaining duplicate-row behavior.
    void navigate88_00a9c400(std::uint8_t forward);
    // Complete normal valid-list current84, RET4. Wrap149 delegates or runs
    // native paging/hidden traversal over these SAME FC/14C rows. EC/158 are
    // read even on some unpaged boundaries and must have a real producer.
    void navigate84_00a9da60(std::uint8_t forward, GuiListboxFrameCalls&);
    // Full normal A9D030 caller sequence. Enter through the same frame runtime
    // which holds its owner borrow across this derived routine and its base.
    // Missing native services, constructor-unwritten reads, invalid iterators
    // and the proven selector0 null-dereference branch fail explicitly.
    void update40_00a9d030(float seconds, const GuiListboxFrameServices&);
    // Complete147-instruction pointer current68 atA9CA60..A9CC46, RET4.
    // Incoming widget is read (unlike AA7190). Requires the actual frame
    // runtime owner borrow; retains SAME FC rows, listener114 and input source.
    // Valid single-thread/lifetime domain; pending dependencies throw in order.
    void pointer68_00a9ca60(GuiWidgetOwner* incoming_child,
        const GuiListboxPointerServices&);
    // A9C540, ECX Listbox, stack(row, low-byte paging, float depth), RETC.
    // Actual AA0F50 manager widget is dereferenced BEFORE testing row=null.
    // Uses its live current34/58 and native node hierarchy factor. No rendering
    // or missing manager/row-node fallback. Borrowed objects survive callbacks.
    void update_highlight_00a9c540(GuiWidgetOwner* row, std::uint8_t paging,
        float depth, const GuiListboxHighlightServices&);
    // E230 successful allocation/valid-list projection, RET4 page size signed.
    // Copies row POINTERS into the native separate14C list, resolves direct
    // arrow children by actual native names, initializes the paging window,
    // then selects0 and rebuilds. Native allocator/SEH and allocation reentry
    // are excluded. Missing arrows fail at their later native dereference.
    void enable_paging_00a9e230(std::int32_t page_size, NativeStringStorage&,
        int (*compare_names_00bf7fbf)(const char*, const char*));
    // D870 normal valid iterator domain. Rebuild SAME FC list from live14C
    // window using actual hide/remove and append/attach operations. Does not
    // change ownership of14C rows. Keep captured selected row and current14C
    // node alive across callbacks; replacing14C during traversal is excluded.
    void rebuild_page_00a9d870();
    std::uint32_t paging_row_count_154() const noexcept;
    GuiWidgetOwner* paging_row_at_14c(std::int32_t ordinal) const noexcept;
    // Current80 A9C220, low-byte force argument, RET4. Listener may change
    // selection/listener/rows; the post-callback tests and previous110 reload
    // observe the SAME current storage. Reentrancy is native behavior.
    void refresh80_00a9c220(bool force);
    // A9AC90, three raw low-byte arguments, RETC.
    void set_layout_flags_00a9ac90(std::uint8_t dont_move,
        std::uint8_t center_vertical, std::uint8_t auto_control) noexcept;
    // Full7C row layout. Horizontal rows require their actual Text companion's
    // measured_width114; other raw derived114 aliases remain unsupported.
    // Native x87 spills and live constants/fields are retained. Callbacks must
    // keep the current borrowed row/list node alive through its iteration.
    void layout7c_00a9c0a0();
    // A9C050: actual base78, layout7C, nonempty selected=first (without hidden
    // check), refresh80(0). A failed downstream operation is not completion.
    void loaded78_00a9c050();
    // Full A9CD20: base60 first, then clearing activation with live11E nonzero
    // applies state1 to the CURRENT selected node, state3 to each other node.
    // Callbacks may alter selection/other rows; keep the current list node and
    // borrowed widget alive through its subsequent native iterator advance.
    void set_active60_00a9cd20(bool active);

    // Partial A9D750: insert_position=null branch and common tail. Native RETC
    // proves (row, optional iterator, after-byte), not two arguments. For a
    // parented row the existing AA83A0 returns its owning handle. A standalone
    // row uses detached_allocation; successful AAA5A0 consumes that handle.
    // Current34(1), actual attach, FC append, first-row select/80, then7C.
    // Non-null iterator insertion and native allocation/SEH ABI are not claimed.
    void append_row_00a9d750(GuiWidgetOwner& row,
        std::unique_ptr<GuiLayoutWidget>& detached_allocation);
    // A9DA30 null-position branch: write SAME row+D8 before the current34,
    // actual attach and FC insertion sequence. Native non-null position still
    // requires resolving its cross-list insertion semantics, not an ordinal.
    void append_row_with_data_00a9da30(GuiWidgetOwner& row, std::uint32_t data,
        std::unique_ptr<GuiLayoutWidget>& detached_allocation);
    // A9BE60: current34(0), remove EVERY matching FC entry without detaching or
    // deleting the widget, selected=end, previous110=null, current80(0),7C.
    void remove_row_00a9be60(GuiWidgetOwner& row);
    // A9BEC0: temporarily clear114, delete FC rows through current04(1),
    // clear FC, select end, restore captured114, clear110, current80(false).
    // Same list nodes survive callbacks; deleting profiles must complete.
    void clear_rows_00a9bec0(GuiTextChildDeletion&);

private:
    friend bool has_selectable_gui_listbox_row_00a9ba40(const GuiListboxRuntime&) noexcept;
    class Operation;
    using Rows = std::list<GuiWidgetOwner*>;
    GuiWidgetOwner& owner_;
    GuiListboxRuntimeServices services_;
    Rows rows_;
    Rows paging_rows_; // Native14C list, separately constructed empty by A9DF40.
    std::uint32_t paging_assignment_generation_{}; // Host iterator-validity guard.
    Rows::const_iterator selected_;
    GuiWidgetOwner* previous_110_{};
    void* listener_114_{};
    GuiListboxFields fields_;
    std::uint32_t active_calls_{};
};
} // namespace bsp
