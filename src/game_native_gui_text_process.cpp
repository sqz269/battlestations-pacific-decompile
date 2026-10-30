#include "bsp/game_native_gui_text_process.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include <cstddef>
#include <cstdlib>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native GUI Text process requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
using gui_widgets_cc13::GuiTypeCounterHost;
using gui_widgets_cc13::GuiTypeDescriptor;

static_assert(sizeof(NativeGuiTextPoolStorage) == 0x38);
static_assert(sizeof(GuiTypeDescriptor<2>) == 0x0c);
static_assert(sizeof(GuiTypeDescriptor<3>) == 0x10);
static_assert(offsetof(GuiTypeDescriptor<2>, native_name_address) == 8);
static_assert(offsetof(GuiTypeDescriptor<3>, native_name_address) == 12);

struct CounterHost final : GuiTypeCounterHost {
    TypeIdCounterLifetime& lifetime;
    explicit CounterHost(TypeIdCounterLifetime& owner) noexcept : lifetime(owner) {}
    TypeIdCounterStorage& counter() override { return *lifetime.get_006fac20(); }
};

// Canonical adapter for the ALREADY reconstructed AA8200 generic initializer.
// Only the one root ID is staged after its real bootstrap returns; no second
// descriptor/root owner is created. The helper preserves guard/name/parent/
// counter/own-ID order (AA8237 then AA823A). No register ABI is reproduced.
void initialize_element_00aa8200(GuiTypeDescriptor<2>& target, std::uint8_t& guard,
    LightTypeBootstrap& common, GuiTypeCounterHost& host) {
    std::uint32_t parent_id[1]; // Written by the real parent callback before helper reads it.
    gui_widgets_cc13::gui_type_initialize<2>(target, guard,
        gui_widgets_cc13::kGuiElementTypeName, parent_id,
        [&] {
            const auto storage = common.storage();
            common.initialize_root_00bea780(storage.root_0109db84);
            parent_id[0] = storage.root_0109db84.own_id;
        }, host);
}
} // namespace

GameNativeGuiTextProcess::GameNativeGuiTextProcess()
    : pool_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          pool_storage_00f8bdf0_) {}

GameNativeGuiTextProcess& game_native_gui_text_process() {
    static GameNativeGuiTextProcess process;
    return process;
}

int GameNativeGuiTextProcess::initialize_pool_once_00cd71f0() {
    std::lock_guard lock(pool_startup_mutex_);
    if (pool_state_ == State::returned) return pool_registration_status_;
    if (pool_state_ != State::unattempted)
        throw std::logic_error("GUI Text pool startup cannot replay an entered or retired lifetime");
    pool_state_ = State::threw;
    pool_.initialize_00ab77c0(); // CD71F0/CD71F5: ECX is the canonical F8BDF0.
    pool_registration_status_ = std::atexit(&destroy_pool_00ce0a80); // CD71FF.
    pool_state_ = State::returned;
    return pool_registration_status_;
}

void GameNativeGuiTextProcess::destroy_pool_00ce0a80() {
    auto& process = game_native_gui_text_process();
    if (process.pool_state_ != State::returned)
        throw std::logic_error("GUI Text CRT destruction requires its one initialized pool lifetime");
    process.pool_state_ = State::retiring;
    process.pool_.destroy_00ab7420(); // CE0A85: native tail jump, no Text destruction.
    process.pool_state_ = State::retired;
}

void GameNativeGuiTextProcess::initialize_types_once_00cd7210(
    TypeIdCounterLifetime& counter, LightTypeBootstrapStorage canonical_common_types) {
    // Pure admission before native guard/name/counter effects. No nested lock
    // spans the root/counter callbacks: callers externally serialize this API.
    if (pool_state_ != State::returned)
        throw std::logic_error("GUI Text types require completed CD71F0 pool startup");
    if (type_state_ == State::returned) {
        bind_or_check_type_domain(counter, canonical_common_types);
        return;
    }
    if (type_state_ != State::unattempted)
        throw std::logic_error("GUI Text type startup cannot replay an entered initialization");
    if (group_type_state_ == State::threw)
        throw std::logic_error("GUI Text type startup cannot enter after interrupted Group initialization");
    bind_or_check_type_domain(counter, canonical_common_types);
    type_state_ = State::threw;
    CounterHost host(counter);
    LightTypeBootstrap common(counter, canonical_common_types);
    // CD7210: guard F8BDEC, name F8BE34, parent AA8200, two CURRENT parent
    // words, then 6FAC20. The helper's post-increment is sequenced before the
    // own-ID assignment: CD7255 counter store precedes CD7258 publication.
    gui_widgets_cc13::gui_type_initialize<3>(text_00f8be28_, text_guard_00f8bdec_,
        0x00d5c5dc, element_00f8bc88_.ids,
        [&] { initialize_element_00aa8200(element_00f8bc88_, element_guard_00f8bc80_, common, host); },
        host);
    type_state_ = State::returned;
}

void GameNativeGuiTextProcess::bind_or_check_type_domain(
    TypeIdCounterLifetime& counter, LightTypeBootstrapStorage canonical_common_types) {
    // Metadata only: no getter, Native guard/name/ID access, callback or pool.
    if (!counter_ && !root_ && !root_guard_) {
        counter_ = &counter;
        root_ = &canonical_common_types.root_0109db84;
        root_guard_ = &canonical_common_types.root_guard_0109db80;
        return;
    }
    if (counter_ != &counter || root_ != &canonical_common_types.root_0109db84 ||
        root_guard_ != &canonical_common_types.root_guard_0109db80)
        throw std::logic_error("GUI type startup cannot retarget its canonical root/counter domain");
}

void GameNativeGuiTextProcess::initialize_layer_type_once_00cd73f0(
    TypeIdCounterLifetime& counter, LightTypeBootstrapStorage canonical_common_types) {
    // Pure Source admission before native guard/name/parent/counter effects.
    if (pool_state_ != State::returned || type_state_ != State::returned)
        throw std::logic_error("GUI Layer type requires completed pool and Text type startup");
    if (counter_ != &counter || root_ != &canonical_common_types.root_0109db84 ||
        root_guard_ != &canonical_common_types.root_guard_0109db80)
        throw std::logic_error("GUI Layer type cannot retarget its canonical root/counter domain");
    if (layer_type_state_ == State::returned) return;
    if (layer_type_state_ != State::unattempted)
        throw std::logic_error("GUI Layer type startup cannot replay an entered initialization");
    layer_type_state_ = State::threw;
    CounterHost host(counter);
    LightTypeBootstrap common(counter, canonical_common_types);
    volatile auto& guard = layer_guard_00f8bf48_;
    if (guard != 0) {
        // Native simply returns on this guard. Within this Source owner's
        // private loader-zero invariant, only returned metadata admits success.
        throw std::logic_error("GUI Layer guard does not establish completed Source startup");
    }
    guard = 1;
    volatile auto& layer = layer_00f8bf88_;
    layer.native_name_address = 0x00d5cb0c;
    initialize_element_00aa8200(element_00f8bc88_, element_guard_00f8bc80_, common, host);
    const volatile auto& element = element_00f8bc88_;
    // CD7414/CD7419 capture BOTH current words before CD741F/CD7424 stores.
    const auto element_id = element.ids[0];
    const auto root_id = element.ids[1];
    layer.ids[1] = element_id;
    layer.ids[2] = root_id;
    volatile auto& actual_counter = host.counter();
    const auto next_id = actual_counter.next_id_04;
    actual_counter.next_id_04 = next_id + 1u;
    layer.ids[0] = next_id; // CD7435 counter store precedes CD7438 publication.
    layer_type_state_ = State::returned;
}

void GameNativeGuiTextProcess::initialize_group_type_once_00cd7460(
    TypeIdCounterLifetime& counter, LightTypeBootstrapStorage canonical_common_types) {
    // Preserve completed-role access before checking a different failed role.
    if (group_type_state_ == State::returned) {
        bind_or_check_type_domain(counter, canonical_common_types);
        return;
    }
    if (group_type_state_ != State::unattempted)
        throw std::logic_error("GUI Group type startup cannot replay an entered initialization");
    if (type_state_ == State::threw)
        throw std::logic_error("GUI Group type startup cannot enter after interrupted Text initialization");
    bind_or_check_type_domain(counter, canonical_common_types);
    group_type_state_ = State::threw;
    CounterHost host(counter);
    LightTypeBootstrap common(counter, canonical_common_types);
    volatile auto& guard = group_guard_00f8bf9c_;
    if (guard != 0)
        throw std::logic_error("GUI Group guard does not establish completed Source startup");
    guard = 1;
    volatile auto& group = group_00f8bfd8_;
    group.native_name_address = 0x00d5cc00;
    initialize_element_00aa8200(element_00f8bc88_, element_guard_00f8bc80_, common, host);
    const volatile auto& element = element_00f8bc88_;
    const auto element_id = element.ids[0];
    const auto root_id = element.ids[1];
    group.ids[1] = element_id;
    group.ids[2] = root_id;
    // The concrete same-counter getter is warm after completed Element. Keep
    // this suffix free of other fallible work; retained serialized publication
    // and the existing cold cross-role checks are required by that Source proof.
    volatile auto& actual_counter = host.counter();
    const auto next_id = actual_counter.next_id_04;
    actual_counter.next_id_04 = next_id + 1u;
    group.ids[0] = next_id;
    group_type_state_ = State::returned;
}

NativeGuiTextPool& GameNativeGuiTextProcess::pool_00f8bdf0() {
    std::lock_guard lock(pool_startup_mutex_);
    if (pool_state_ != State::returned)
        throw std::logic_error("GUI Text pool requires its completed live CD71F0 startup");
    return pool_;
}

const volatile std::uint32_t (&GameNativeGuiTextProcess::element_lineage_00f8bc88() const)[2] {
    if (type_state_ != State::returned)
        throw std::logic_error("GUI Element lineage requires complete same-domain Text type startup");
    return element_00f8bc88_.ids;
}

const volatile std::uint32_t (&GameNativeGuiTextProcess::text_lineage_00f8be28() const)[3] {
    if (type_state_ != State::returned)
        throw std::logic_error("GUI Text lineage requires complete CD7210 startup");
    return text_00f8be28_.ids;
}

const volatile std::uint32_t (&GameNativeGuiTextProcess::layer_lineage_00f8bf88() const)[3] {
    if (layer_type_state_ != State::returned)
        throw std::logic_error("GUI Layer lineage requires complete same-domain CD73F0 startup");
    return layer_00f8bf88_.ids;
}

const volatile std::uint32_t (&GameNativeGuiTextProcess::group_lineage_00f8bfd8() const)[3] {
    if (group_type_state_ != State::returned)
        throw std::logic_error("GUI Group lineage requires complete same-domain CD7460 startup");
    return group_00f8bfd8_.ids;
}

bool GameNativeGuiTextProcess::native_operation_interrupted() const noexcept {
    return pool_state_ == State::threw || pool_state_ == State::retiring ||
        type_state_ == State::threw || layer_type_state_ == State::threw ||
        group_type_state_ == State::threw;
}
} // namespace bsp::game
