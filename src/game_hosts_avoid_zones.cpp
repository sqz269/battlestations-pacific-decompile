// Packet cc9_avoid_zone_registry, docs/AVOID_ZONE_REGISTRY.md. Names are
// hypotheses. See the header for the ABI of each routine.
#include "bsp/game_hosts_avoid_zones.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>

#include "bsp/cc13_avoid1.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/native_list_count_guards.hpp"
#include "bsp/native_registry_list_producers.hpp"
#include "bsp/native_registry_list_tidy.hpp"
#include "bsp/native_zone_group_clear.hpp"
#include "bsp/oct18_tail_a.hpp"
#include "bsp/oct18b_nA1_early.hpp"
#include "bsp/singleton_lifetime.hpp"
// kPc24PlaneTasksCc13Bound sites here (pc24w1-sweep3-3): 0041BC20
// avoid_zone_sample_0041bc20, the two row lerps and the column lerp.
#include "bsp/pc24_planetasks_cc13.hpp"
// kPc24UnitsHostCc13Bound (pc24w1-units-1): 00417FA0 avoid_zone_world_to_cell_00417fa0.
#include "bsp/pc24_unitshost_cc13.hpp"

namespace bsp::game {

namespace {

float float_bits(std::uint32_t bits) noexcept {
    float value = 0.0f;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

// 00423960's typed view over the host's own layer record (the image's 28h-byte
// layer is +04h half extent, +08h n, +0Ch cell, +10h scale, +18h grid vector).
class TerrainGridLayerRecordDimensionFields final
    : public bsp::oct18_tail_a::TerrainLayerDimensionFields {
public:
    explicit TerrainGridLayerRecordDimensionFields(bsp::TerrainGridLayerRecord& layer)
        : layer_(layer) {}
    void set_half_extent_04(float value) override { layer_.half_extent = value; }
    void set_dimension_08(std::int32_t value) override { layer_.dimension = value; }
    void set_scale_10(float value) override { layer_.file_scalar = value; }
    void set_cell_width_0c(float value) override { layer_.cell_size = value; }
    // 004219A0 resizes the byte vector with fill 0, then 00BF79F0 zeroes [begin, end).
    void resize_and_clear_grid_18(std::uint32_t bytes) override { layer_.grid.assign(bytes, 0); }

private:
    bsp::TerrainGridLayerRecord& layer_;
};

// 00424680 with [00CE3990] (0.17453286, ten degrees): 00423960(10) sets the
// half extent [00CE3968] 12000.0, the cell 24000.0 [00CE3960] / 10, the scale
// [00CE3958] 2.0 and a zeroed 10 * 10 grid; +14h is tan(angle) through 00412E20.
bsp::TerrainGridLayerRecord default_layer_00424680() {
    bsp::TerrainGridLayerRecord layer;
    layer.class_name = "TerrainGridLayer";
    {
        // 004246B4 00423960 terrain_layer_set_dimension_00423960(10) over the record; OFF keeps the inline stores.
        // The image's constants, as its bytes read: 00CE3968 463B8000h, 00CE3960
        // 40D7700000000000h, 00CE3958 40000000h, 00CE396C 45160000h (unread here).
        static const volatile float extent_00ce3968 = 12000.0f;
        static const volatile double diameter_00ce3960 = 24000.0;
        static const volatile float scale_00ce3958 = 2.0f;
        static const volatile float default_width_00ce396c = 2400.0f;
        const bsp::oct18_tail_a::GridConstants constants{
            extent_00ce3968, diameter_00ce3960, scale_00ce3958, default_width_00ce396c};
        TerrainGridLayerRecordDimensionFields fields(layer);
        bsp::oct18_tail_a::terrain_layer_set_dimension_00423960(fields, 10, constants);  // 004246AE PUSH 0Ah
    }
    layer.slope_limit = static_cast<float>(std::tan(static_cast<double>(0.17453286f)));
    return layer;
}

// The same layer for the image's angle argument, read as the float bits the
// constructor loads (00CE3990 3E32B8C3h, 00CE398C 3EB2B8C3h, 00CE3988 3F9C61ABh).
bsp::TerrainGridLayerRecord default_layer_00424680(std::uint32_t angle_bits) {
    bsp::TerrainGridLayerRecord layer = default_layer_00424680();
    layer.slope_limit = static_cast<float>(std::tan(static_cast<double>(float_bits(angle_bits))));
    return layer;
}

}  // namespace

AvoidZoneCell avoid_zone_world_to_cell_00417fa0(const bsp::TerrainGridLayerRecord& layer,
                                                float x, float z) noexcept {
    AvoidZoneCell c;
    // 00417FA0-00417FB1 / 00417FB3-00417FCA: FADD then FDIV in the x87, each
    // stored to a float.
    float u, v;
    if constexpr (bsp::kPc24UnitsHostCc13Bound) {
        // Each op rounded (PC24).
        using namespace bsp::cc13x87;
        u = static_cast<float>(x87_div24(x87_add24(x, layer.half_extent), layer.cell_size));
        v = static_cast<float>(x87_div24(x87_add24(z, layer.half_extent), layer.cell_size));
    } else {
        u = static_cast<float>((static_cast<double>(x) + layer.half_extent) /
                               layer.cell_size);
        v = static_cast<float>((static_cast<double>(z) + layer.half_extent) /
                               layer.cell_size);
    }
    // 00417FCD-00417FDA: (float)(n - 2.0).
    const float hi = static_cast<float>(static_cast<double>(layer.dimension) - 2.0);
    // 00417FBE COMISS 0.0 > u -> 0, else u > hi -> hi (00417FED).
    c.u = (0.0f > u) ? 0.0f : ((u > hi) ? hi : u);
    c.v = (0.0f > v) ? 0.0f : ((v > hi) ? hi : v);
    return c;
}

float avoid_zone_cell_height_0041bae0(const bsp::TerrainGridLayerRecord& layer,
                                      int ix, int iz) noexcept {
    const std::int32_t first = 0;
    const std::int32_t last = layer.dimension - 1;
    // 0041BAE4-0041BAFA and 0041BB00-0041BB0C: < 0 -> 0, > n - 1 -> n - 1.
    // Reuse the recovered signed clamp with the actual sample coordinates and
    // layer bound. bsp.py show 00415660 --asm: the below-minimum arm returns
    // before reading maximum, exactly like these original inline branches.
    // This is C++ helper reuse; the image's 0041BAE0 inlines the comparisons.
    bsp::oct18b_nA1::clamp_int_in_place_00415660(ix, first, last);
    bsp::oct18b_nA1::clamp_int_in_place_00415660(iz, first, last);
    const long long index = static_cast<long long>(layer.dimension) * iz + ix;  // 0041BB0E
    if (index < 0 || index >= static_cast<long long>(layer.grid.size())) return 0.0f;
    // 0041BB2D MOVZX, FILD, FMUL [layer+10h], FSTP float.
    return static_cast<float>(static_cast<double>(layer.grid[static_cast<std::size_t>(index)]) *
                              layer.file_scalar);
}

float avoid_zone_sample_0041bc20(const bsp::TerrainGridLayerRecord& layer,
                                 float x, float z) noexcept {
    const AvoidZoneCell c = avoid_zone_world_to_cell_00417fa0(layer, x, z);
    // 0041BC49 / 0041BC4F CVTTSS2SI: truncation.
    const int ix = static_cast<int>(c.u);
    const int iz = static_cast<int>(c.v);
    // 0041BC59 FISUB then FSTP float: the u fraction.
    const float fu = static_cast<float>(static_cast<double>(c.u) - ix);
    const float h11 = avoid_zone_cell_height_0041bae0(layer, ix + 1, iz + 1);  // 0041BC6D
    const float h01 = avoid_zone_cell_height_0041bae0(layer, ix, iz + 1);      // 0041BC7A
    const float h10 = avoid_zone_cell_height_0041bae0(layer, ix + 1, iz);      // 0041BC8A
    const float h00 = avoid_zone_cell_height_0041bae0(layer, ix, iz);          // 0041BC97
    // 0041BCE1-0041BCED: the v fraction, FISUB then FSTP float.
    const float fv = static_cast<float>(static_cast<double>(c.v) - iz);
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // The same three lerps with each FSUBP / FMULP / FADDP rounded at PC24
        // (0041BCAD/B5/B9, 0041BCC9/CD/CF, 0041BCDF/F1/F3), each stored float.
        using namespace bsp::cc13x87;
        const float a = static_cast<float>(x87_add24(h00, x87_mul24(fu, x87_sub24(h10, h00))));
        const float b = static_cast<float>(x87_add24(h01, x87_mul24(fu, x87_sub24(h11, h01))));
        return static_cast<float>(x87_add24(a, x87_mul24(fv, x87_sub24(b, a))));
    } else {
        // 0041BCA0-0041BCBB: a = h00 + fu * (h10 - h00), stored as a float.
        const float a = static_cast<float>(static_cast<double>(h00) +
            static_cast<double>(fu) * (static_cast<double>(h10) - h00));
        // 0041BCBF-0041BCD1: b = h01 + fu * (h11 - h01), stored as a float.
        const float b = static_cast<float>(static_cast<double>(h01) +
            static_cast<double>(fu) * (static_cast<double>(h11) - h01));
        // 0041BCF1-0041BCF9: a + fv * (b - a), stored as a float.
        return static_cast<float>(static_cast<double>(a) +
            static_cast<double>(fv) * (static_cast<double>(b) - a));
    }
}

// Chain 465: the process registry owns the original-shaped list and layers.
// The host's existing records own the grid bytes; native vector pointers borrow
// those exact buffers, and every query still sees the records that produced
// the list. Moving a record during vector growth preserves its grid allocation.
// This binds typed C++ bodies, not the original ECX/vtable or allocator ABI.
struct GameAvoidZoneRegistry::NativeRegistry final : bsp::Cc13AvoidZoneRegistryHost,
    bsp::NativeAvoidZoneRegistryClearHost {
    explicit NativeRegistry(std::vector<bsp::TerrainGridLayerRecord>& layers)
        : layers_(layers) {
        try {
            // Original 0042474B..0042476B: ECX is the 14h-byte owner; the
            // body itself selects all three binary angle constants and links
            // each returned layer. Do not also build the old default list.
            bsp::cc13_avoid_registry_construct_00424730(registry_, *this);
            published_ = &registry_;
        } catch (...) {
            clear_layers_0041ded0(registry_);
            destroy_layer_list_0041d710(registry_);
            throw;
        }
    }

    ~NativeRegistry() override {
        // Original 0041F0FB/0041F103/0041F10C: clear layers, destroy the
        // sentinel, then withdraw the process registry before its base phase.
        bsp::cc13_avoid_registry_destroy_0041f0d0(*published_, *this);
    }

    void load(const std::vector<bsp::TerrainGridLayerRecord>& file_layers) {
        auto& registry = *published_;
        clear_layers_0041ded0(registry); // 004248C1 also removes defaults.
        for (const auto& record : file_layers) {
            layers_.push_back(record);
            auto layer = std::make_unique<bsp::Cc13TerrainGridLayerNative>();
            project_layer(*layer, layers_.back());
            auto node = std::unique_ptr<bsp::Cc13TerrainGridLayerListNode,
                decltype(&bsp::singleton_lifetime_free)>(
                    buy_node_0041c1f0(registry, registry.head, registry.head->previous, layer.get()),
                    &bsp::singleton_lifetime_free);
            increment_size_00420140(registry, 1);
            registry.head->previous = node.get();
            node->previous->next = node.get();
            layer.release();
            node.release();
        }
        // Original 00424975 -> 0041E000; 0041E02F selects head->next's
        // layer, and 0041E095..0041E0CF scans its real grid/scale into +10h.
        bsp::cc13_avoid_registry_end_load_0041e000(registry, *this);
    }

    float max_cell_height() const noexcept { return published_->max_cell_height; }

private:
    static void project_layer(bsp::Cc13TerrainGridLayerNative& native,
        bsp::TerrainGridLayerRecord& record) noexcept {
        native.vtable = 0x00ce38ccu;
        native.half_extent = record.half_extent;
        native.dimension = record.dimension;
        native.cell_size = record.cell_size;
        native.file_scalar = record.file_scalar;
        native.slope_limit = record.slope_limit;
        native.grid_proxy_18 = 0; // no original checked-iterator proxy ABI
        native.grid_first = record.grid.empty() ? nullptr : record.grid.data();
        native.grid_last = record.grid.empty() ? nullptr : record.grid.data() + record.grid.size();
        native.grid_end = native.grid_last;
    }

    bsp::Cc13TerrainGridLayerListNode* buy_head_node_0041bfd0(
        bsp::Cc13AvoidZoneRegistryNative&) override {
        // bsp.py show 0041BFD0: allocate 0Ch, self-link, leave payload unread.
        return bsp::buy_native_registry_list_head_0041bfd0();
    }
    void* allocate_layer_00bf681b(std::uint32_t bytes) override {
        return ::operator new(bytes);
    }
    void free_layer_00bf65ac(void* block) noexcept override { ::operator delete(block); }
    bsp::Cc13TerrainGridLayerNative* construct_layer_00424680(void* block, float angle) override {
        std::uint32_t bits;
        std::memcpy(&bits, &angle, sizeof(bits));
        layers_.push_back(default_layer_00424680(bits));
        auto* layer = new (block) bsp::Cc13TerrainGridLayerNative;
        project_layer(*layer, layers_.back());
        pending_layer_.reset(layer);
        return layer;
    }
    bsp::Cc13TerrainGridLayerListNode* buy_node_0041c1f0(
        bsp::Cc13AvoidZoneRegistryNative&, bsp::Cc13TerrainGridLayerListNode* next,
        bsp::Cc13TerrainGridLayerListNode* previous,
        bsp::Cc13TerrainGridLayerNative* const& value) override {
        // 0041C1F0 captures the supplied value slot after allocating the node.
        auto* node = bsp::buy_native_registry_list_node_0041c1f0(next, previous, &value);
        if (pending_layer_.get() == value) pending_layer_.release();
        return node;
    }
    void increment_size_00420140(bsp::Cc13AvoidZoneRegistryNative& registry,
        std::uint32_t count) override {
        // bsp.py show 00420140: its 0Ch list starts at registry+4h and checks
        // uint32(3FFFFFFFh-current), rather than the old UINT_MAX host limit.
        bsp::increment_native_list_count_00420140(&registry.list_proxy_04, nullptr, count);
    }
    void clear_layers_0041ded0(bsp::Cc13AvoidZoneRegistryNative& registry) override {
        // bsp.py show 0041DED0: delete the front payload, reload/unlink the
        // current front, free it, then decrement the actual registry count.
        bsp::native_avoid_zone_registry_clear_layers_0041ded0(registry, *this);
        layers_.clear(); // release borrowed grids only after their native views
    }
    void destroy_layer_list_0041d710(bsp::Cc13AvoidZoneRegistryNative& registry) override {
        // No list exists if the constructor's head allocation threw. Normal
        // destruction supplies the same registry+4h header as 0041F103.
        if (registry.head != nullptr)
            bsp::tidy_native_registry_list_0041d710(&registry.list_proxy_04);
    }
    void scalar_delete_layer_vtable_0(bsp::Cc13TerrainGridLayerNative& layer,
        std::uint32_t flags) override {
        // These host projections borrow layers_' grid buffers (project_layer),
        // so their existing typed delete releases the view; clear_layers frees
        // the owning records only after the native list traversal completes.
        if ((flags & 1u) != 0) delete &layer;
    }
    void free_list_node_00bf65ac(bsp::Cc13TerrainGridLayerListNode* node) override {
        bsp::singleton_lifetime_free(node); // pairs with both native producers
    }
    void store_registry_singleton_00e17620(void* value) override {
        published_ = static_cast<bsp::Cc13AvoidZoneRegistryNative*>(value);
    }
    void invalid_parameter_00bf6713() override {
        throw std::invalid_argument("invalid avoid-zone layer grid/list");
    }

    std::vector<bsp::TerrainGridLayerRecord>& layers_;
    bsp::Cc13AvoidZoneRegistryNative registry_{};
    bsp::Cc13AvoidZoneRegistryNative* published_ = nullptr;
    std::unique_ptr<bsp::Cc13TerrainGridLayerNative> pending_layer_;
};

GameAvoidZoneRegistry::GameAvoidZoneRegistry() {
    native_ = new NativeRegistry(layers_);
}

GameAvoidZoneRegistry::~GameAvoidZoneRegistry() {
    delete native_;
}

GameAvoidZoneRegistry& GameAvoidZoneRegistry::instance() {
    static GameAvoidZoneRegistry registry;  // 004C17D0, [00E17620]
    return registry;
}

void GameAvoidZoneRegistry::load_scene_layers(
    const std::vector<bsp::TerrainGridLayerRecord>& file_layers) {
    native_->load(file_layers);
    ++scene_loads_;
}

std::optional<float> GameAvoidZoneRegistry::max_cell_height() const noexcept {
    if (scene_loads_ != 0) return native_->max_cell_height();
    return std::nullopt;
}

int GameAvoidZoneRegistry::select_layer_by_slope_0041df40(float value,
                                                          bool value_is_angle) const noexcept {
    return bsp::select_terrain_grid_layer(layers_, value, value_is_angle);
}

const bsp::TerrainGridLayerRecord* GameAvoidZoneRegistry::layer(int index) const noexcept {
    if (index < 0 || static_cast<std::size_t>(index) >= layers_.size()) return nullptr;
    return &layers_[static_cast<std::size_t>(index)];
}

int GameAvoidZoneRegistry::squadron_layer_34c() const noexcept {
    return select_layer_by_slope_0041df40(1.5f, true);  // 007F1DB2-007F1DC5, [00CE380C]; stored at 007F1DCA
}

}  // namespace bsp::game
