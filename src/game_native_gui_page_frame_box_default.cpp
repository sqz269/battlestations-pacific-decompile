#include "bsp/game_native_gui_page_frame_box_default.hpp"
#include "bsp/game_native_gui_text_process.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/gui_text_lifetime.hpp"
#include "bsp/native_gui_frame_box_records.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Page DEFAULT FrameBox requires MSVC Win32.
#endif
static_assert(sizeof(void*) == 4);

namespace bsp::game {
namespace {
using Word = std::uint32_t;
using FactoryPhase = NativeGuiFrameBoxFactoryPhase;
using ReferencePhase = NativeGuiFrameBoxIdentityPhase;
using VectorPhase = NativeGuiFrameBoxVectorTidyPhase;
constexpr Word base_profile = 0x00d5c130;
constexpr Word box_profile = 0x00d5d130;
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void* at(void* actual, std::size_t offset) noexcept {
    return static_cast<unsigned char*>(actual) + offset;
}
Word read(void* actual, std::size_t offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(actual, offset));
}
const volatile Word& cell(GameNativeReadOnlyData& data, Word address) {
    return *static_cast<const volatile Word*>(data.data_at(address, sizeof(Word)));
}
struct Region { const volatile void* identity; std::size_t bytes; };
bool disjoint(Region left, Region right) noexcept {
    const auto a = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(left.identity));
    const auto b = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(right.identity));
    return a + left.bytes <= b || b + right.bytes <= a;
}
NativeGuiWidgetPool<GuiFrameBoxPoolTraits>& ready_pool(NativeGuiFrameBoxFactoryPoolProcess& process) {
    require(&process == &NativeGuiFrameBoxFactoryPoolProcess::instance(),
        "DEFAULT FrameBox requires the SAME canonical F8C188 process");
    return process.pool(); // Ready-only, no initialize/trim/pool storage copy.
}
} // namespace

struct GameNativeGuiPageFrameBoxDefault::Impl final {
    NativeGuiWidgetLifetimeContext& page;
    NativeGuiWidgetLifetimeBindings& dispatcher;
    NativeRenderActualOwnerRegistry& owners;
    NativeStringRawPoolContext& strings;
    GameNativeReadOnlyData& data;
    NativeGuiFrameBoxFactoryPoolProcess& process;
    NativeGuiFrameBoxDefaultBindings& defaults;
    NativeGuiWidgetPool<GuiFrameBoxPoolTraits>& pool;
    const volatile Word* const base;
    const volatile Word* const profile;
    NativeGuiFrameBoxFactoryOperation factory;
    // These independent stable Source objects are installed before reference
    // admission. No Page context/native cells or unknown raw bytes are copied.
    std::optional<NativeGuiFrameBoxVectorTidyOperation> vector;
    std::optional<NativeGuiFrameBoxLifetimeContext> lifetime;
    std::optional<NativeGuiFrameBoxIdentityContext> identity;
    std::optional<NativeGuiFrameBoxReference> reference;
    void* result{}; // Opaque diagnostic after scalar/factory slot return.
    bool active{}, failed{}, construct_entered{}, construct_returned{};
    bool bind_entered{}, bind_complete{};

    Impl(GameNativeGuiPageFrameBoxDefault& owner, NativeGuiWidgetLifetimeContext& actual_page,
        NativeGuiWidgetLifetimeBindings& actual_dispatcher, NativeRenderActualOwnerRegistry& registry,
        NativeStringRawPoolContext& raw_strings, GameNativeReadOnlyData& ro,
        NativeGuiFrameBoxFactoryPoolProcess& actual_process, NativeGuiFrameBoxDefaultBindings& cells)
        : page(actual_page), dispatcher(actual_dispatcher), owners(registry), strings(raw_strings),
          data(ro), process(actual_process), defaults(cells), pool(ready_pool(process)),
          base(static_cast<const volatile Word*>(data.data_at(base_profile, 9 * sizeof(Word)))),
          profile(static_cast<const volatile Word*>(data.data_at(box_profile, 2 * sizeof(Word)))),
          factory(process, defaults) {
        const Region writable[]{{this, sizeof(*this)}, {&owner, sizeof(owner)}};
        const Region borrowed[]{{&page, sizeof(page)}, {&dispatcher, sizeof(dispatcher)},
            {&owners, sizeof(owners)}, {&strings, sizeof(strings)}, {&data, sizeof(data)},
            {&process, sizeof(process)}, {&defaults, sizeof(defaults)}, {&pool, sizeof(pool)},
            {base, 9 * sizeof(Word)}, {profile, 2 * sizeof(Word)},
            {&defaults.actual_one_00d7a24c, sizeof(Word)},
            {&defaults.actual_frame_x_00d5d11c, sizeof(Word)},
            {&defaults.actual_frame_y_00d5d10c, sizeof(Word)}};
        require(disjoint(writable[0], writable[1]), "FrameBox metadata backing overlaps its owner");
        for (const auto target : writable)
            for (const auto source : borrowed)
                require(disjoint(target, source), "FrameBox metadata cannot overlap its borrowed Page graph");
        require_domains();
    }
    void require_domains() const {
        auto& types = game_native_gui_text_process();
        require(&page.bindings == &dispatcher && page.actual_base_table_00d5c130 == base &&
            &page.actual_base_lineage_00f8bc88 == &types.element_lineage_00f8bc88() &&
            &page.actual_text_descriptor_00f8be28 == &types.text_lineage_00f8be28()[0],
            "FrameBox requires SAME actual Page dispatcher and canonical RO/type address aliases");
        require(&defaults.actual_one_00d7a24c == &cell(data, 0x00d7a24c) &&
            &defaults.actual_frame_x_00d5d11c == &cell(data, 0x00d5d11c) &&
            &defaults.actual_frame_y_00d5d10c == &cell(data, 0x00d5d10c) &&
            &pool == &ready_pool(process),
            "FrameBox requires SAME borrowed scalar cells and already-ready allocating pool");
        require(page.nodes.uses_raw_name_pool() && &page.nodes.require_raw_name_pool() == &strings &&
            same_native_string_raw_pool_cells(strings, game_native_string_process().raw_context()),
            "FrameBox requires SAME Page node/raw-string publication/gate/manager cells");
    }
    void begin() {
        if (failed) throw std::logic_error("FrameBox cannot reuse failed Source history");
        if (active) {
            failed = true;
            throw std::logic_error("FrameBox rejects reentrant Source entry");
        }
        active = true;
    }
    void fail() noexcept { failed = true; active = false; }
    void finish() {
        require(!failed, "FrameBox cannot complete after a caught reentrant denial");
        active = false;
    }
    void require_completed_factory() const {
        // Saved independent metadata FIRST; result may already name freed
        // storage after an interrupted operation. No payload read here.
        require(construct_entered && construct_returned && result &&
            factory.phase() == FactoryPhase::returned && !factory.source_identity() &&
            factory.slot_identity() == result && factory.returned_identity() == result &&
            factory.constructor_entered() && !factory.pool_return_entered(),
            "FrameBox publication requires SAME nonnull whole normal DEFAULT construction");
    }
    void require_empty_default() const {
        require(!read(result, 0x4c) && !read(result, 0x70) && !read(result, 0x6c) &&
            !read(result, 0x88) && !read(result, 0x8c) && !read(result, 0x90) &&
            !read(result, 0xf4) && !read(result, 0xf8) && !read(result, 0xfc) &&
            !read(result, 0x104) && !read(result, 0x108),
            "FrameBox DEFAULT admission excludes primary/parent/children/timed/state/name arms");
        void* const head = reinterpret_cast<void*>(read(result, 0x68));
        require(head && !(reinterpret_cast<std::uintptr_t>(head) & 3u) &&
            read(head) == reinterpret_cast<Word>(head) && read(head, 4) == reinterpret_cast<Word>(head),
            "FrameBox DEFAULT requires its genuine finite empty AA9390 ring");
        // No read of opaque list64, F0/proxy, EE..F3,101..103 or trailer11C.
    }
};

GameNativeGuiPageFrameBoxDefault::GameNativeGuiPageFrameBoxDefault(
    NativeGuiWidgetLifetimeContext& page, NativeGuiWidgetLifetimeBindings& dispatcher,
    NativeRenderActualOwnerRegistry& owners, NativeStringRawPoolContext& strings,
    GameNativeReadOnlyData& data, NativeGuiFrameBoxFactoryPoolProcess& process,
    NativeGuiFrameBoxDefaultBindings& cells)
    : impl_(std::make_unique<Impl>(*this, page, dispatcher, owners, strings, data, process, cells)) {}
GameNativeGuiPageFrameBoxDefault::~GameNativeGuiPageFrameBoxDefault() noexcept = default;

void* GameNativeGuiPageFrameBoxDefault::construct_default_00aa1a40() {
    auto& p = *impl_;
    p.begin();
    try {
        require(!p.construct_entered && !p.bind_entered,
            "FrameBox DEFAULT factory entry is once only");
        p.require_domains();
        p.construct_entered = true; // Source scheduling, not Native-origin proof.
        p.result = allocate_native_gui_frame_box_00aa1a40(nullptr, p.factory);
        p.construct_returned = true; // Immediately, before any later throwing admission.
        require(p.factory.returned_identity() == p.result &&
            (p.result ? p.factory.phase() == FactoryPhase::returned :
                p.factory.phase() == FactoryPhase::returned_null),
            "FrameBox must preserve the actual whole factory result/null return");
        p.finish();
        return p.result;
    } catch (...) { p.fail(); throw; } // Existing factory alone owns ctor-escape slot action.
}
NativeGuiFrameBoxReference& GameNativeGuiPageFrameBoxDefault::bind_completed_default() {
    auto& p = *impl_;
    p.begin();
    try {
        require(!p.bind_entered && !p.vector && !p.lifetime && !p.identity && !p.reference,
            "FrameBox canonical publication is once only");
        p.bind_entered = true;
        p.require_completed_factory();
        p.require_domains();
        p.require_empty_default();
        const Word first = read(p.result, 0xf4);
        const Word last = read(p.result, 0xf8);
        const Word capacity = read(p.result, 0xfc);
        require(!first && !last && !capacity,
            "FrameBox retirement preparation requires actual whole DEFAULT null range postconditions");
        // Existing119 preparation validates metadata/regions only. It does not
        // read this header or F0; these null identities derive from the real
        // completed constructor. No synthetic span, raw seed or last-use claim.
        p.vector.emplace(at(p.result, 0xf0), reinterpret_cast<void*>(first),
            reinterpret_cast<void*>(last), nullptr, 0);
        p.lifetime.emplace(NativeGuiFrameBoxLifetimeContext{p.page, p.strings, p.owners, *p.vector});
        p.identity.emplace(NativeGuiFrameBoxIdentityContext{p.owners, *p.lifetime, p.pool, p.profile});
        p.reference.emplace(p.result, *p.identity); // Existing actual+04 reference; no retain/count write.
        require(p.owners.find(p.result) == &*p.reference,
            "FrameBox publication requires its SAME sole canonical registry companion");
        p.bind_complete = true;
        p.finish();
        return *p.reference;
    } catch (...) { p.fail(); throw; } // Complete Box remains held; no Native ctor cleanup/free.
}
void GameNativeGuiPageFrameBoxDefault::require_default_before_scalar() {
    auto& p = *impl_;
    p.begin();
    try {
        // Independent metadata/companion checks precede any actual slot read.
        require(p.bind_complete && p.reference && p.identity && p.lifetime && p.vector &&
            p.reference->phase() == ReferencePhase::bound &&
            p.vector->phase() == VectorPhase::fresh,
            "FrameBox DEFAULT scalar admission requires its SAME live completed binding");
        p.require_completed_factory();
        require(p.owners.find(p.result) == &*p.reference &&
            &p.identity->registry == &p.owners && &p.identity->lifetime == &*p.lifetime &&
            &p.identity->actual_matching_pool == &p.pool && p.identity->actual_profile_00d5d130 == p.profile &&
            &p.lifetime->widgets == &p.page && &p.lifetime->strings == &p.strings &&
            &p.lifetime->owners == static_cast<NativeRenderActualOwners*>(&p.owners) &&
            &p.lifetime->vector == &*p.vector && p.vector->header_identity() == at(p.result, 0xf0) &&
            !p.vector->expected_first_identity() && !p.vector->expected_end_identity() &&
            !p.vector->prepared_record_count(),
            "FrameBox DEFAULT admission requires its exact retained Page/registry/pool/119 graph");
        p.require_domains();
        require(read(p.result) == box_profile && p.profile[0] == 0x00bd30e0 &&
            p.profile[1] == 0x00ad29b0 && p.base[3] == 0x00a9e070 && p.base[8] == 0x00aa8320,
            "FrameBox DEFAULT scalar admission requires genuine current scalar/base selectors");
        p.require_empty_default();
        const Word descriptor = native_gui_text_descriptor_00ab6a30(p.page.actual_text_descriptor_00f8be28);
        const bool is_text = native_gui_widget_is_kind_of_00a9e070(
            descriptor, p.page.actual_base_lineage_00f8bc88);
        require(!is_text, "FrameBox DEFAULT rejects true base Text-descriptor legacy glyph cleanup");
        p.finish(); // Domain admission only; no flags/count/scalar/cleanup effects.
    } catch (...) { p.fail(); throw; }
}
bool GameNativeGuiPageFrameBoxDefault::native_operation_interrupted() const noexcept {
    const auto& p = *impl_;
    if (p.active || p.failed) return true;
    if (p.reference && p.reference->phase() != ReferencePhase::bound &&
        p.reference->phase() != ReferencePhase::retired) return true;
    return p.vector && (p.vector->phase() == VectorPhase::entered || p.vector->phase() == VectorPhase::failed);
    // Healthy live references, normal retired metadata and flags0 physical
    // custody are NOT permission to abandon the actual graph or queued uses.
}
} // namespace bsp::game
