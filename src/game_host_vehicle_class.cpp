#include "bsp/game_host_vehicle_class.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>

#include "bsp/cc13_front4.hpp"
#include "bsp/cc13_sweepA.hpp"
#include "bsp/cc13_seg41.hpp"
#include "bsp/cc13_seg41_land.hpp"
#include "bsp/engine_seg4_6_9_cc13.hpp"
#include "bsp/gameplay_seg61_62_cc13.hpp"
#include "bsp/local_player_unit_lists.hpp"
#include "bsp/native_damageable_class_construction.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_vehicle_class_base_constructor.hpp"
#include "bsp/profile_seg2_cc13.hpp"
#include "bsp/save_settings_cc13.hpp"
#include "bsp/sound_lifetime_access.hpp"

// Goal 3 chain 234. See the header for the object, its producer and lifetime.
// Every constructor below is the existing reconstructed body; this file only
// supplies the storage 00964790 hands them and the host callbacks their bodies
// name, each of which forwards to the next existing body down the chain:
//   leaf (e.g. 00951AA0) -> plane base 007D1E50 / ship base 00963380 /
//   land-fort 00749180 -> vehicle base 00749050 -> damageable base 0087C640.

namespace bsp::game {
namespace {

using bsp::cc13_front4::Raw;

[[noreturn]] void unreached(const char* method) {
    throw std::logic_error(std::string("vehicle-class descriptor construction reached ") +
        method + ", which no descriptor constructor calls");
}

// 0087C640's two image vtable identities: the refcounted base it stores first
// and the damageable vtable it leaves at +0h (the same VAs every other
// reconstructed refcounted constructor stores, e.g. cc13_seg47.cpp:109).
constexpr std::uint32_t kRefCountedVtable_00ceb130 = 0x00CEB130u;
constexpr std::uint32_t kDamageableVtable_00d0e13c = 0x00D0E13Cu;

// 00963380's _eh_vector_constructor_iterator_ (00BF7CD1) arguments:
// twenty 30h-byte sub-objects at +138h, constructor 0095B920, destructor
// 009560F0 (docs/VEHICLE_CLASS_DESCRIPTORS.md, "00963380, the ship base").
constexpr std::uint32_t kShipSlotConstructor_0095b920 = 0x0095B920u;
constexpr std::uint32_t kShipSlotDestructor_009560f0 = 0x009560F0u;
// 0095B920's whole body (0095B938..0095B952): nine zero dword stores.
constexpr std::uint32_t kShipSlotZeroOffsets[] = {
    0x04, 0x08, 0x0C, 0x14, 0x18, 0x1C, 0x24, 0x28, 0x2C};

using LeafConstructor = std::uint8_t* (*)(bsp::VehicleClassConstructorHost&, std::uint8_t*);
struct LeafRow {
    std::uint32_t address;
    LeafConstructor construct;
};
// The kind rows' constructor addresses (kVehicleClassDescriptorTable) with
// the reconstructed body at each. 00749180 (LandFort) takes FortClassHost and
// is dispatched separately; Destroyer and Cruiser have no out-of-line call
// (00964790 inlines them, see construct_descriptor).
constexpr LeafRow kLeafConstructors[] = {
    {0x00963C20u, &bsp::construct_landing_ship_class_00963c20},
    {0x00963CB0u, &bsp::construct_cargo_class_00963cb0},
    {0x00963D30u, &bsp::construct_battleship_class_00963d30},
    {0x00963DB0u, &bsp::construct_submarine_class_00963db0},
    {0x00963E40u, &bsp::construct_torpedo_boat_class_00963e40},
    {0x00963EC0u, &bsp::construct_mothership_class_00963ec0},
    {0x00951AA0u, &bsp::construct_recon_plane_class_00951aa0},
    {0x009536E0u, &bsp::construct_small_recon_plane_class_009536e0},
    {0x00953770u, &bsp::construct_large_recon_plane_class_00953770},
    {0x00951AF0u, &bsp::construct_fighter_class_00951af0},
    {0x00951B40u, &bsp::construct_dive_bomber_class_00951b40},
    {0x00951C20u, &bsp::construct_torpedo_bomber_class_00951c20},
    {0x00951D00u, &bsp::construct_kamikaze_class_00951d00},
    {0x00951DE0u, &bsp::construct_level_bomber_class_00951de0},
    {0x0095FEE0u, &bsp::construct_airfield_class_0095fee0},
    {0x0095FF50u, &bsp::construct_shipyard_class_0095ff50},
    {0x0095FFC0u, &bsp::construct_land_vehicle_class_0095ffc0},
    {0x00951E30u, &bsp::construct_command_building_class_00951e30},
    {0x00960050u, &bsp::construct_dummy_target_class_00960050},
};
constexpr std::uint32_t kLandFortConstructor_00749180 = 0x00749180u;
constexpr std::uint32_t kLandFortIsKindOf_00749030 = 0x00749030u;

void write_u32(std::uint8_t* self, std::size_t offset, std::uint32_t value) {
    std::memcpy(self + offset, &value, sizeof(value));
}
std::uint32_t read_u32(const std::uint8_t* self, std::size_t offset) {
    std::uint32_t value = 0;
    std::memcpy(&value, self + offset, sizeof(value));
    return value;
}

// 007D1E50's Front4Host: the plane base calls only 00749050 (007D1E69).
class PlaneBaseBinding final : public bsp::cc13_front4::Front4Host {
public:
    explicit PlaneBaseBinding(const bsp::NativeDamageableClassConstructionAccess& damageable)
        : damageable_(damageable) {}
    void* vehicle_class_construct_base_00749050(Raw object) override {
        return bsp::construct_native_vehicle_class_base_00749050(object.p, damageable_);
    }
    void* global_address(std::uint32_t) override { unreached("Front4Host::global_address"); }
    void* global_pointer(std::uint32_t) override { unreached("Front4Host::global_pointer"); }
    void invalid_parameter_00bf6713() override { unreached("00BF6713"); }
    void debug_printf_004254b0(const char*, const char*) override { unreached("004254B0"); }
    void frontend_push_interface_request_004cc460(Raw, std::int32_t, void*) override {
        unreached("004CC460");
    }
    void unit_list_erase_node_004837d0(Raw, void*) override { unreached("004837D0"); }
    void entity_adjust_world_list_counters_009245a0(Raw, std::uint32_t, std::uint32_t) override {
        unreached("009245A0");
    }
    void scene_node_invalidate_subtree_pose_0042ed50(void*) override { unreached("0042ED50"); }
    void matrix_copy_004134f0(float*, const float*) override { unreached("004134F0"); }
    float* matrix_multiply_00413920(const float*, float*, const float*) override {
        unreached("00413920");
    }
    float* matrix_build_look_at_00b63f10(float*, const float*, const float*, float, float,
        float) override {
        unreached("00B63F10");
    }
    float* matrix_rotation_z_00b64780(float*, const float*) override { unreached("00B64780"); }
    void matrix_orthonormalize_0085d3d0(float*) override { unreached("0085D3D0"); }
    void hud_movie_screen_ensure_movcam_005cc000(Raw) override { unreached("005CC000"); }
    void movcam_set_target_007a6910(void*, void*) override { unreached("007A6910"); }
    bool ai_planner_owns_group_00a1c8b0(Raw, void*) override { unreached("00A1C8B0"); }
    void observer_unregister_pair_006952a0(void*, void*) override { unreached("006952A0"); }
    void stl_list_remove_00a1d1d0(Raw, void* const*) override { unreached("00A1D1D0"); }
    void stl_list_clear_00a16b60(Raw) override { unreached("00A16B60"); }
    void* sized_storage_pool_00419cc0() override { unreached("00419CC0"); }
    void sized_storage_pool_return_block_00bd1510(void*, void*, std::uint32_t,
        std::uint32_t) override {
        unreached("00BD1510");
    }
    void callback_owner_destroy_00695870(Raw) override { unreached("00695870"); }
    bool vcall_entity_is_kind_5c(void*, std::uint32_t) override { unreached("vcall +5Ch"); }
    void entity_request_join_formation_0077c8d0(void*, void*) override { unreached("0077C8D0"); }
    bool ai_command_send_empty_squadron_home_009ffeb0(void*) override { unreached("009FFEB0"); }
    const float* ai_group_leader_point_00a10c20(void*) override { unreached("00A10C20"); }
    void ai_command_issue_move_to_member_00a02020(void*, const float*) override {
        unreached("00A02020");
    }
    void debug_draw_line_rgba_floats_0085f990(const float*, const float*, const float*,
        const float*) override {
        unreached("0085F990");
    }
    void debug_draw_arc_around_axis_00860ad0(const float*, const float*, float, float, float,
        const float*, std::uint32_t) override {
        unreached("00860AD0");
    }

private:
    const bsp::NativeDamageableClassConstructionAccess& damageable_;
};

// 00749180's FortClassHost: construction calls only 00749050 (its first
// call); the destructor-side methods belong to 00748EC0, which never runs here.
class LandFortBaseBinding final : public bsp::cc13_seg41::FortClassHost {
public:
    explicit LandFortBaseBinding(const bsp::NativeDamageableClassConstructionAccess& damageable)
        : damageable_(damageable) {}
    void vehicle_class_base_construct_00749050(std::uint8_t* cls) override {
        bsp::construct_native_vehicle_class_base_00749050(cls, damageable_);
    }
    void vehicle_class_base_destruct_0095f1c0(std::uint8_t*) override { unreached("0095F1C0"); }
    void destroy_range_004cc760(std::uint32_t, std::uint32_t, std::uint8_t*,
        std::uint8_t*) override {
        unreached("004CC760");
    }
    void free_00bf65ac(void*) override { unreached("00BF65AC"); }
    long interlocked_decrement(volatile long*) override { unreached("InterlockedDecrement"); }
    void release_slot0(bsp::cc13_seg41::RefCounted*) override { unreached("vtable slot 0"); }

private:
    const bsp::NativeDamageableClassConstructionAccess& damageable_;
};

// The VehicleClassConstructorHost every leaf constructor body takes.
class LeafConstructorBinding final : public bsp::VehicleClassConstructorHost {
public:
    LeafConstructorBinding(const bsp::NativeDamageableClassConstructionAccess& damageable,
        PlaneBaseBinding& plane, LandFortBaseBinding& fort)
        : damageable_(damageable), plane_(plane), fort_(fort) {}
    // 00951E30 (CommandBuilding) derives from LandFort.
    void call_00749180(std::uint8_t* self) override {
        bsp::cc13_seg41::land_fort_class_construct_00749180(self, fort_);
    }
    void call_00749050(std::uint8_t* self) override {
        bsp::construct_native_vehicle_class_base_00749050(self, damageable_);
    }
    void call_00963380(std::uint8_t* self) override {
        bsp::construct_ship_class_base_00963380(*this, self);
    }
    // 00BF7CD1 is the CRT's _eh_vector_constructor_iterator_ (library, bound
    // rather than rewritten). The only use on this path is 00963380's: it
    // runs 0095B920 over the 14h slots. 0095B920 has no reconstructed body;
    // its whole body is the nine zero stores listed above, and the storage
    // 00964790 hands 00963380 is already zeroed (00470B80 / 00BF79F0), so the
    // iteration changes no byte. That is checked here instead of re-storing.
    void call_00bf7cd1(std::uint8_t* first, std::uint32_t stride, std::uint32_t count,
        std::uint32_t constructor, std::uint32_t destructor) override {
        if (constructor != kShipSlotConstructor_0095b920
            || destructor != kShipSlotDestructor_009560f0 || stride != 0x30u) {
            unreached("00BF7CD1 with a constructor other than 0095B920");
        }
        for (std::uint32_t i = 0; i < count; ++i) {
            const std::uint8_t* slot = first + static_cast<std::size_t>(i) * stride;
            for (const std::uint32_t offset : kShipSlotZeroOffsets) {
                if (read_u32(slot, offset) != 0) {
                    throw std::logic_error("00963380 ship slot storage was not zeroed");
                }
            }
        }
    }
    void call_007d1e50(std::uint8_t* self) override {
        bsp::cc13_front4::plane_class_construct_base_007d1e50(plane_, Raw{self});
    }

private:
    const bsp::NativeDamageableClassConstructionAccess& damageable_;
    PlaneBaseBinding& plane_;
    LandFortBaseBinding& fort_;
};

} // namespace

struct GameVehicleClassDescriptorCache::Bindings final
    : bsp::profile_seg2_cc13::VehicleClassRegistryConstructionHost,
      bsp::cc13_sweepA::PartyRowsStorageHost,
      bsp::cc13_sys::LazySingletonHost {
    Bindings(GameVehicleClassDescriptorCache& owner, bsp::NativeStringRawPoolContext& strings,
        bsp::NativeSingletonDeletionBindings& deletions)
        : owner_(owner), deletions_(deletions),
          lifetime_(strings.actual_manager_publication_01090aa0),
          retirement_{this, nullptr, &retire_registered},
          damageable{kRefCountedVtable_00ceb130, kDamageableVtable_00d0e13c, strings},
          plane(damageable), fort(damageable), leaf(damageable, plane, fort) {
        if (deletions_.vehicle_class_registry != nullptr)
            throw std::logic_error("vehicle class registry already has a live cache owner");
        deletions_.vehicle_class_registry = &retirement_;
        try {
            // bsp.py show 00437F50 / 00437E10: exact 401Ch allocation,
            // constructor CE417C, then registration of the returned receiver.
            // This is the existing first 00964790 descriptor resolve. Its
            // cache header now lives in the getter's returned allocation;
            // the previous direct construction of an inline image is gone.
            bsp::profile_seg2_cc13::vehicle_class_registry_get_00437f50(
                publication_, *this);
        } catch (...) {
            // The generic Source getter has no EH guard. Release the captured
            // shared section if allocation/construction/registration throws.
            section_.reset();
            if (constructed_) std::free(registry().ptr(0x4010));
            if (publication_ != nullptr)
                lifetime_.get_manager_00415350().unregister_object(publication_);
            deletions_.vehicle_class_registry = nullptr;
            throw;
        }
    }
    ~Bindings() override {
        // The outer owner retires while its vector and entries are still alive.
        if (deletions_.vehicle_class_registry == &retirement_)
            deletions_.vehicle_class_registry = nullptr;
    }

    bsp::profile_seg2_cc13::Raw registry() noexcept {
        return {static_cast<std::uint8_t*>(publication_)};
    }
    void lock() override { section_.emplace(lifetime_); }
    void unlock() override { section_.reset(); }
    void* operator_new(std::uint32_t size) override {
        allocation_.reset(static_cast<std::uint8_t*>(operator_new_00bf55be(size)));
        return allocation_.get();
    }
    void* construct(std::uint32_t constructor, void* memory) override {
        if (constructor != 0x00437e10u || memory != allocation_.get())
            throw std::logic_error("unexpected vehicle class registry constructor");
        bsp::profile_seg2_cc13::vehicle_class_registry_construct_00437e10(
            {static_cast<std::uint8_t*>(memory)}, *this);
        constructed_ = true;
        return memory;
    }
    void register_lifetime(void* object) override {
        retirement_.registered_owner = object;
        lifetime_.get_manager_00415350().register_object(object);
    }
    void retire() {
        if (publication_ == nullptr) return;
        // The registry borrows the real vector. Retire its references before
        // either allocation disappears; no Original descriptor dtor is claimed.
        owner_.resize_cache(0);
        lifetime_.get_manager_00415350().unregister_object(publication_);
        std::free(registry().ptr(0x4010));
        publication_ = nullptr;
        retirement_.registered_owner = nullptr;
        allocation_.reset();
    }
    void party_rows_reserve_00435960(bsp::profile_seg2_cc13::Raw vector,
        std::uint32_t count) override {
        bsp::cc13_sweepA::party_rows_reserve_00435960(
            {vector.p}, static_cast<std::int32_t>(count), *this);
    }
    void* operator_new_00bf55be(std::uint32_t size) override {
        void* const allocation = std::malloc(size);
        if (allocation == nullptr) throw std::bad_alloc();
        return allocation;
    }
    void free_00bf6989(void* allocation) override { std::free(allocation); }

    static void retire_registered(void* context, void* registered, std::uint32_t flags) {
        auto& self = *static_cast<Bindings*>(context);
        if (registered != self.publication_ || flags != 1u)
            throw std::logic_error("vehicle class registry retirement receiver/flags mismatch");
        self.retire();
    }
    struct FreeAllocation {
        void operator()(std::uint8_t* memory) const noexcept { std::free(memory); }
    };
    GameVehicleClassDescriptorCache& owner_;
    bsp::NativeSingletonDeletionBindings& deletions_;
    bsp::SoundLifetimeAccess lifetime_;
    std::optional<bsp::CapturedSoundLifetimeSection> section_;
    void* publication_{}; // Existing mission-scoped registry, now a getter publication.
    std::unique_ptr<std::uint8_t, FreeAllocation> allocation_;
    bsp::VehicleClassRegistryRetirementBinding retirement_;
    bool constructed_{};
    const bsp::NativeDamageableClassConstructionAccess damageable;
    PlaneBaseBinding plane;
    LandFortBaseBinding fort;
    LeafConstructorBinding leaf;
};

struct GameVehicleClassDescriptorCache::Entry {
    std::unique_ptr<std::uint8_t[]> image;
    std::uint32_t size{0};
    bsp::VehicleClassDescriptor descriptor;
};

GameVehicleClassDescriptorCache::GameVehicleClassDescriptorCache(
    bsp::NativeStringRawPoolContext& strings, bsp::NativeSingletonDeletionBindings& deletions)
    : bindings_(std::make_unique<Bindings>(*this, strings, deletions)) {}

GameVehicleClassDescriptorCache::~GameVehicleClassDescriptorCache() {
    bindings_->retire();
    for (const auto& entry : entries_) {
        entry->descriptor.type_name.release_to(bsp::crt_string_storage());
    }
}

int GameVehicleClassDescriptorCache::cache_size() const noexcept {
    return bindings_->registry().at<std::int32_t>(0x8);
}

void GameVehicleClassDescriptorCache::resize_cache(int size) {
    bsp::cc13_save::resize_descriptor_cache_00437bd0(cache_, size,
        [this](void* descriptor) { release_slot(descriptor); });
    // The vector owns allocation/reallocation; the native registry borrows
    // that same live allocation. Publish after resize, before any subsequent
    // resolver read, so neither storage nor reference-count work runs twice.
    const auto registry = bindings_->registry();
    registry.ptr(0x4) = cache_.data();
    registry.at<std::int32_t>(0x8) = static_cast<std::int32_t>(cache_.size());
    registry.at<std::int32_t>(0xc) = static_cast<std::int32_t>(cache_.capacity());
}

bsp::VehicleClassDescriptor* GameVehicleClassDescriptorCache::cached(
    int class_index) const noexcept {
    if (class_index < 0 || class_index >= cache_size()) return nullptr;
    auto* const data = bindings_->registry().at<void**>(0x4);
    return static_cast<bsp::VehicleClassDescriptor*>(data[class_index]);
}

void GameVehicleClassDescriptorCache::store_cached(int class_index,
    bsp::VehicleClassDescriptor* descriptor) {
    if (class_index < 0) throw std::invalid_argument("vehicle class index is negative");
    if (class_index >= cache_size()) {
        resize_cache(class_index + 1);
    }
    void*& slot = bindings_->registry().at<void**>(0x4)[class_index];
    if (slot != nullptr && slot != descriptor) release_slot(slot);
    slot = descriptor;
}

// The reference the slot held: InterlockedDecrement(+4h), vtable slot 0 at
// zero (0096511B / 00437BD0). The host keeps the entry until the cache dies
// (header, DIVERGENCE), so only the count is modelled, and only on the
// separate VehicleClassDescriptor copy: the image's +4h is not touched (see
// the split-copy note in the header).
void GameVehicleClassDescriptorCache::release_slot(void* descriptor) noexcept {
    if (descriptor == nullptr) return;
    auto* semantic = static_cast<bsp::VehicleClassDescriptor*>(descriptor);
    if (semantic->ref_count > 0) --semantic->ref_count;
}

bsp::VehicleClassDescriptor* GameVehicleClassDescriptorCache::construct_descriptor(
    int kind_index, std::uint32_t size) {
    if (kind_index < 0 || kind_index >= bsp::kVehicleClassDescriptorCount) {
        throw std::out_of_range("vehicle class kind index out of range");
    }
    const bsp::VehicleClassDescriptorRow& row = bsp::kVehicleClassDescriptorTable[kind_index];
    if (size != row.descriptor_size) {
        throw std::invalid_argument("descriptor size differs from the kind row");
    }
    auto entry = std::make_unique<Entry>();
    entry->size = size;
    // 00470B80 / 00BF55BE + 00BF79F0: zeroed storage of exactly `size` bytes.
    entry->image = std::make_unique<std::uint8_t[]>(size);
    std::uint8_t* const self = entry->image.get();

    if (row.constructor == 0) {
        // Destroyer 009649C2..009649CF and Cruiser 00964A1F..00964A2C, inlined
        // in 00964790: ECX = storage, CALL 00963380, then the vptr pair
        // (primary, primary - 4 at +6Ch).
        bsp::construct_ship_class_base_00963380(bindings_->leaf, self);
        write_u32(self, 0x00, row.vtable);
        write_u32(self, 0x6C, row.vtable - 4u);
    } else if (row.constructor == kLandFortConstructor_00749180) {
        bsp::cc13_seg41::land_fort_class_construct_00749180(self, bindings_->fort);
    } else {
        LeafConstructor construct = nullptr;
        for (const LeafRow& leaf : kLeafConstructors) {
            if (leaf.address == row.constructor) construct = leaf.construct;
        }
        if (construct == nullptr) {
            throw std::logic_error("kind row names a constructor with no body here");
        }
        construct(bindings_->leaf, self);
    }
    if (read_u32(self, 0) != row.vtable) {
        throw std::logic_error("constructed descriptor vtable differs from its kind row");
    }
    ++constructed_;
    entries_.push_back(std::move(entry));
    return &entries_.back()->descriptor;
}

void GameVehicleClassDescriptorCache::destroy_descriptor(bsp::VehicleClassDescriptor* descriptor) {
    for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        if (&(*it)->descriptor != descriptor) continue;
        for (void*& slot : cache_) {
            if (slot == descriptor) slot = nullptr;
        }
        (*it)->descriptor.type_name.release_to(bsp::crt_string_storage());
        entries_.erase(it);
        return;
    }
}

GameVehicleClassDescriptorCache::Entry* GameVehicleClassDescriptorCache::find(
    const bsp::VehicleClassDescriptor* descriptor) const noexcept {
    for (const auto& entry : entries_) {
        if (&entry->descriptor == descriptor) return entry.get();
    }
    return nullptr;
}

const std::uint8_t* GameVehicleClassDescriptorCache::image(
    const bsp::VehicleClassDescriptor* descriptor) const noexcept {
    const Entry* entry = find(descriptor);
    return entry != nullptr ? entry->image.get() : nullptr;
}

// vtable[18h](kind) on the image descriptor (00964FD6, 0096507C, 0096508B,
// 0096516B). The slot is read through kUnitClassTable, which pairs each
// descriptor vtable with its IsKindOf (the PE's vtable+18h dwords agree for
// all 22 leaves); each IsKindOf body is a flat CMP chain (kIsKindOfRows), and
// LandFort's 00749030 has its own reconstructed body.
bool GameVehicleClassDescriptorCache::descriptor_is_kind(
    const bsp::VehicleClassDescriptor* descriptor, int kind) const {
    const std::uint8_t* const self = image(descriptor);
    if (self == nullptr) throw std::logic_error("IsKindOf on a descriptor this cache did not build");
    const std::uint32_t vtable = read_u32(self, 0);
    std::uint32_t is_kind_of = 0;
    for (const bsp::UnitClassRow& row : bsp::kUnitClassTable) {
        if (row.descriptor_vtable == vtable) is_kind_of = row.is_kind_of;
    }
    if (is_kind_of == kLandFortIsKindOf_00749030) {
        return bsp::cc13_seg41::land_fort_class_is_kind_of_00749030(
            static_cast<std::uint32_t>(kind));
    }
    const auto* body = bsp::gameplay_seg61_62_cc13::is_kind_of_row(is_kind_of);
    if (body == nullptr) throw std::logic_error("descriptor vtable has no IsKindOf row");
    // 007CFD00..007CFD1F: the plane base accepts 0Fh, 5 and 4. The eight
    // plane descriptor leaves (00953470..009537D0, kIsKindOfRows) inline
    // that same tail after their subtype checks. Reuse the existing body
    // with the actual query, replacing those comparisons on this path.
    // Select by the resolved descriptor's exact tail, retaining both prefix
    // ids for the small/large recon classes and all other families' dispatch.
    const auto count = body->id_count;
    if (count >= 3 && body->ids[count - 3] == 0x0f &&
        body->ids[count - 2] == 5 && body->ids[count - 1] == 4) {
        for (std::uint8_t i = 0; i < count - 3; ++i) {
            if (kind == static_cast<int>(body->ids[i])) return true;
        }
        return bsp::plane_class_is_kind_of_007cfd00(static_cast<std::uint32_t>(kind));
    }
    return bsp::gameplay_seg61_62_cc13::is_kind_of(*body, kind);
}

} // namespace bsp::game
