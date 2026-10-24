#include "bsp/native_font_resources.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_effect_handle_acquisition.hpp"
#include "bsp/native_font_storage.hpp"
#include "bsp/native_font_tree_insert.hpp"
#include "bsp/native_int_pointer_tree18_erase.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/stream_scalars.hpp"
#include <cstddef>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "native font resources require Win32");
static_assert(offsetof(FontGlyphData, gfx_texture_18) == 0x18);
static_assert(offsetof(FontGlyphData, alpha_texture_1c) == 0x1c);
static_assert(sizeof(FontGlyphData) == 0x20);

void require_empty_resources(const FontGlyphData& glyph) {
    if (glyph.gfx_texture_18 || glyph.alpha_texture_1c)
        throw std::invalid_argument("native font adoption requires scalar-only decoded glyphs");
}
void require_actual_reference(void* raw, NativeRenderActualOwners& owners) {
    if (!raw) return;
    if ((reinterpret_cast<std::uintptr_t>(raw) & 3u) != 0)
        throw std::invalid_argument("font texture requires aligned actual owner storage");
    std::uint32_t profile;
    std::memcpy(&profile, raw, sizeof(profile));
    if (profile != 0x00d61948 && profile != 0x00d61870 && profile != 0x00d618b0)
        throw std::invalid_argument("font image requires an established actual native texture profile");
    auto& companion = owners.resolve_actual(raw);
    auto* actual_count = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(raw) + 4));
    if (&companion.reference_count != actual_count || actual_count->load() <= 0)
        throw std::invalid_argument("font texture must use its same live native reference count");
}
struct FontImageNode {
    FontImageNode* left;
    FontImageNode* parent;
    FontImageNode* right;
    std::uint32_t key;
    FontGlyphData* payload;
    std::uint8_t color, nil;
    std::uint8_t padding[2];
};
struct FontRegistryImageNode {
    FontRegistryImageNode* next;
    FontRegistryImageNode* previous;
    void* font;
};
struct FontRegistryImageStorage {
    std::uint32_t native_vtable_00;
    std::uint32_t allocator_04;
    FontRegistryImageNode* head_08;
    std::uint32_t count_0c;
};
static_assert(sizeof(FontRegistryImageNode) == 0x0c);
static_assert(offsetof(FontRegistryImageNode, font) == 8);
static_assert(sizeof(FontRegistryImageStorage) == 0x10);
static_assert(offsetof(FontRegistryImageStorage, head_08) == 8);
static_assert(offsetof(FontRegistryImageStorage, count_0c) == 0x0c);
static_assert(sizeof(FontImageNode) == 0x18);
static_assert(offsetof(FontImageNode, payload) == 0x10);
std::uint16_t image_scaled_word(std::int32_t input, float scale) noexcept {
    std::int32_t converted;
    __asm {
        mov eax, input
        cvtsi2ss xmm0, eax
        mulss xmm0, scale
        cvttss2si eax, xmm0
        mov converted, eax
    }
    return static_cast<std::uint16_t>(converted);
}
}

// This IS the owned Font storage read by image-aware Text/layout consumers.
// The typed descriptor is lookup metadata, never cast to a Font or kept as
// another metric/glyph owner. +00 and padding are an admitted zero preimage.
struct NativeFontResources::Image final {
    std::uint32_t allocator_00;
    FontImageNode* head_04;
    std::uint32_t count_08;
    NativeString name_0c;
    std::uint16_t height_14, padding_16;
    float scale_18, alpha_scale_1c;
    NativeString data_20, alpha_28, gfx_30, root_38, language_40;
    std::uint8_t uppercase_48, padding_49[3];
    FontGlyphData space_4c, missing_6c, carriage_8c;
    static void verify_layout() noexcept {
        static_assert(sizeof(Image) == 0xac);
        static_assert(offsetof(Image, head_04) == 4);
        static_assert(offsetof(Image, name_0c) == 0xc);
        static_assert(offsetof(Image, height_14) == 0x14);
        static_assert(offsetof(Image, scale_18) == 0x18);
        static_assert(offsetof(Image, alpha_scale_1c) == 0x1c);
        static_assert(offsetof(Image, data_20) == 0x20);
        static_assert(offsetof(Image, alpha_28) == 0x28);
        static_assert(offsetof(Image, gfx_30) == 0x30);
        static_assert(offsetof(Image, root_38) == 0x38);
        static_assert(offsetof(Image, language_40) == 0x40);
        static_assert(offsetof(Image, uppercase_48) == 0x48);
        static_assert(offsetof(Image, space_4c) == 0x4c);
        static_assert(offsetof(Image, missing_6c) == 0x6c);
        static_assert(offsetof(Image, carriage_8c) == 0x8c);
    }
    FontImageNode* find(std::uint16_t key) const noexcept {
        NativeIntPointerTree18Iterator result{};
        find_native_font_glyph_iterator_00ad4540(const_cast<Image*>(this), &result, key);
        return static_cast<FontImageNode*>(result.node);
    }
};

void NativeFontResources::construct_image_00ad55c0(
    const FontDescriptor& descriptor, NativeRenderActualOwners& owners,
    game::GameNativeStringProcess& strings, std::unique_ptr<NativeFontResources>& result) {
    if (result || descriptor.name.find('\0') != std::string::npos ||
        descriptor.name.size() > (std::numeric_limits<std::uint32_t>::max)())
        throw std::invalid_argument("native Font image requires a DWORD null-free name");
    Image::verify_layout();
    result.reset(new NativeFontResources(descriptor, owners));
    result->image_strings_ = &strings;
    result->image_ = new (singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0xacu, sizeof(Image)})) Image{};
    struct Header { std::uint32_t length; const char* data; };
    const Header name{static_cast<std::uint32_t>(descriptor.name.size()), descriptor.name.c_str()};
    const void* const volatile name_header = &name;
    std::uint32_t scale, alpha;
    std::memcpy(&scale, &descriptor.scale_ratio, sizeof(scale));
    std::memcpy(&alpha, &descriptor.alpha_texture_scale, sizeof(alpha));
    const std::uint32_t uppercase = descriptor.uppercase_only ? 1u : 0u;
    const NativeFontConstructorArguments arguments{name_header, scale, alpha, uppercase};
    construct_native_font_storage_00ad55c0(result->image_, arguments, strings);
}

void NativeFontResources::save_image_paths_00ad4c30_fragment(
    const std::string& root, const std::string& language) {
    if (!image_ || phase_ != Phase::preparing || image_paths_saved_)
        throw std::logic_error("native Font paths require one fresh constructed image");
    image_paths_saved_ = true; // Entered failure is retained, not replayed.
    auto& strings = image_strings_->raw_context();
    image_->root_38.assign_0041e870(strings, root.c_str());
    image_->language_40.assign_0041e870(strings, language.c_str());
    image_->data_20.assign_0041e870(strings, descriptor_.data_file.c_str());
    image_->gfx_30.assign_0041e870(strings, descriptor_.gfx_file.c_str());
    image_->alpha_28.assign_0041e870(strings, descriptor_.alpha_texture.c_str());
}
void NativeFontResources::populate_image_00ad4c30_fragment(MemoryStream& stream,
    void*& gfx, void*& alpha) {
    if (!image_ || phase_ != Phase::preparing || !image_paths_saved_ || image_population_started_ ||
        &gfx == &alpha || image_->count_08 || !stream.has_backing() || !stream.fully_initialized())
        throw std::invalid_argument("native Font image population requires a fresh image and complete DAT");
    image_population_started_ = true;
    require_actual_reference(gfx, actual_owners_);
    require_actual_reference(alpha, actual_owners_);
    if (gfx && gfx == alpha && actual_owners_.resolve_actual(gfx).reference_count.load() < 2)
        throw std::invalid_argument("aliased Font images require two transferred references");
    const auto position = stream.position_00bef580();
    const auto size = stream.size_00bef600();
    if (position < 0 || position > size || size - position < 6)
        throw std::invalid_argument("native Font DAT header is incomplete");
    const auto records = stream_read_u32_00be4300(stream);
    const auto height = static_cast<std::int16_t>(stream_read_word_00be4320(stream));
    image_->height_14 = image_scaled_word(height, image_->scale_18);
    if (static_cast<std::uint64_t>(records) * 24u > static_cast<std::uint64_t>(size - position - 6))
        throw std::invalid_argument("native Font DAT record extent is incomplete");
    for (std::uint32_t index = 0; index < records; ++index) {
        const auto key = stream_read_word_00be4320(stream);
        auto* payload = new (singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 0x20u, sizeof(FontGlyphData)})) FontGlyphData;
        pending_image_glyph_ = payload;
        for (auto& value : payload->fields_00_0c) value = stream_read_float_00be4360(stream);
        payload->field_10 = stream_read_word_00be4340(stream);
        payload->scaled_field_12 = image_scaled_word(stream_read_word_00be4340(stream), image_->scale_18);
        payload->scaled_field_14 = image_scaled_word(stream_read_word_00be4320(stream), image_->scale_18);
        payload->gfx_texture_18 = gfx;
        payload->alpha_texture_1c = alpha;
        if (image_->find(key) == image_->head_04) {
            // The provider copies a DWORD key. The low WORD is the native
            // unsigned key; this new Source pair has a defined upper preimage.
            const NativeIntPointerTree18Pair pair{static_cast<std::int32_t>(key), payload};
            NativeIntPointerTree18InsertResult inserted{};
            insert_native_font_tree_unique_00ad4aa0(image_, &inserted, &pair);
        } else {
            singleton_lifetime_free(payload); // Original duplicate: first wins.
        }
        pending_image_glyph_ = nullptr;
    }
    image_dat_complete_ = true;
}
void NativeFontResources::finish_image_00ad4c30_fragment(void*& gfx, void*& alpha) {
    if (!image_ || phase_ != Phase::preparing || !image_dat_complete_ || &gfx == &alpha)
        throw std::logic_error("native Font completion requires one completed DAT population");
    auto* fallback = image_->find(0x91);
    if (fallback == image_->head_04 || !fallback->payload)
        throw std::invalid_argument("native Font DAT lacks required glyph 0091");
    image_->space_4c.scaled_field_12 = static_cast<std::uint16_t>(
        static_cast<std::int16_t>(image_->height_14) / 4);
    image_->space_4c.gfx_texture_18 = gfx;
    image_->space_4c.alpha_texture_1c = alpha;
    std::memcpy(&image_->missing_6c, fallback->payload, sizeof(FontGlyphData));
    image_->carriage_8c.gfx_texture_18 = gfx;
    image_->carriage_8c.alpha_texture_1c = alpha;
    gfx = nullptr;
    alpha = nullptr;
    phase_ = Phase::live;
}

NativeFontResources::NativeFontResources(const FontDescriptor& descriptor,
    NativeRenderActualOwners& owners) : descriptor_(descriptor), actual_owners_(owners) {}

std::unique_ptr<NativeFontResources> NativeFontResources::adopt_00ad4c30_fragment(
    const FontDescriptor& descriptor, std::unique_ptr<FontData>& decoded,
    void*& gfx, void*& alpha, NativeRenderActualOwners& owners) {
    if (!decoded || &gfx == &alpha || decoded->glyphs.find(0x91) == decoded->glyphs.end())
        throw std::invalid_argument("native font adoption requires one decoded font and two distinct reference slots");
    for (const auto& entry : decoded->glyphs) require_empty_resources(entry.second);
    require_empty_resources(decoded->space_lf_glyph);
    require_empty_resources(decoded->missing_glyph);
    require_empty_resources(decoded->carriage_return_glyph);
    require_actual_reference(gfx, owners);
    require_actual_reference(alpha, owners);
    if (gfx && gfx == alpha && owners.resolve_actual(gfx).reference_count.load() < 2)
        throw std::invalid_argument("an aliased font image requires two transferred native references");
    // Allocation and all validation precede ownership transfer. No FontData
    // copy/move construction: even callers' existing glyph addresses survive.
    auto result = std::unique_ptr<NativeFontResources>(new NativeFontResources(descriptor, owners));
    result->data_ = std::move(decoded);
    auto& data = *result->data_;
    for (auto& entry : data.glyphs) {
        entry.second.gfx_texture_18 = gfx;   //AD5004; borrowed, no retain.
        entry.second.alpha_texture_1c = alpha; //AD5007.
    }
    data.space_lf_glyph.alpha_texture_1c = alpha; //AD50CB.
    data.space_lf_glyph.gfx_texture_18 = gfx; //AD50D1; owning pair64/68.
    const auto& fallback = data.glyphs.find(0x91)->second;
    data.missing_glyph.gfx_texture_18 = fallback.gfx_texture_18;
    data.missing_glyph.alpha_texture_1c = fallback.alpha_texture_1c; //AD5109 copy.
    data.carriage_return_glyph.gfx_texture_18 = gfx; //AD5142.
    data.carriage_return_glyph.alpha_texture_1c = alpha; //AD5148.
    gfx = nullptr;
    alpha = nullptr;
    result->phase_ = Phase::live;
    return result;
}

NativeFontResources::~NativeFontResources() noexcept {
    if (phase_ != Phase::preparing) destroy_00ad53a0_fragment();
}
FontData& NativeFontResources::data() {
    if (!data_) throw std::logic_error("image Font has no typed FontData allocation");
    return *data_;
}
const FontData& NativeFontResources::data() const {
    if (!data_) throw std::logic_error("image Font has no typed FontData allocation");
    return *data_;
}
void* NativeFontResources::native_image() const noexcept { return image_; }
std::uint16_t NativeFontResources::scaled_height_14() const noexcept {
    return image_ ? image_->height_14 : data_->scaled_height;
}
const float& NativeFontResources::scale_ratio_18() const noexcept {
    return image_ ? image_->scale_18 : descriptor_.scale_ratio;
}
const float& NativeFontResources::alpha_texture_scale_1c() const noexcept {
    return image_ ? image_->alpha_scale_1c : descriptor_.alpha_texture_scale;
}
bool NativeFontResources::uppercase_only_48() const noexcept {
    return image_ ? image_->uppercase_48 != 0 : descriptor_.uppercase_only;
}
std::size_t NativeFontResources::glyph_count() const noexcept {
    return image_ ? image_->count_08 : data_ ? data_->glyphs.size() : 0u;
}
bool NativeFontResources::has_glyph(std::uint16_t key) const noexcept {
    return image_ ? image_->find(key) != image_->head_04 : font_has_glyph_00ad4500(*data_, key);
}
bool NativeFontResources::accepts_text_byte(std::uint8_t value) const noexcept {
    if (value == 0x20u) return true;
    return has_glyph(static_cast<std::uint16_t>(value < 0x80u ? value : 0xff00u | value));
}
std::int32_t NativeFontResources::signed_height_14() const noexcept {
    std::int16_t height;
    const auto bits = scaled_height_14();
    std::memcpy(&height, &bits, sizeof(height));
    return height;
}
const FontGlyphData& NativeFontResources::glyph(std::uint16_t key) const noexcept {
    if (image_) {
        if (key == 0x20 || key == 0x0a) return image_->space_4c;
        if (key == 0x0d) return image_->carriage_8c;
        auto* found = image_->find(key);
        return found == image_->head_04 ? image_->missing_6c : *found->payload;
    }
    return select_font_glyph_00ad4480(*data_, key);
}
void NativeFontResources::destroy_00ad53a0_fragment() {
    if (phase_ == Phase::destroyed) return;
    if (phase_ != Phase::live)
        throw std::logic_error("native font destruction requires a live non-reentrant owner");
    phase_ = Phase::destroying;
    if (image_) {
        NativeIntPointerTree18Iterator cursor{image_, image_->head_04->left};
        while (cursor.node != image_->head_04) {
            auto* node = static_cast<FontImageNode*>(cursor.node);
            if (node->payload) {
                singleton_lifetime_free(node->payload);
                node->payload = nullptr;
            }
            increment_native_int_pointer_tree18_00869a20(&cursor);
        }
        if (auto* captured = image_->space_4c.gfx_texture_18) {
            release_native_render_actual_owner(actual_owners_, captured);
            image_->space_4c.gfx_texture_18 = nullptr;
        }
        if (auto* captured = image_->space_4c.alpha_texture_1c) {
            release_native_render_actual_owner(actual_owners_, captured);
            image_->space_4c.alpha_texture_1c = nullptr;
        }
        auto& strings = image_strings_->raw_context();
        destroy_native_string_header_0041dd20(&image_->language_40, strings);
        destroy_native_string_header_0041dd20(&image_->root_38, strings);
        destroy_native_string_header_0041dd20(&image_->gfx_30, strings);
        destroy_native_string_header_0041dd20(&image_->alpha_28, strings);
        destroy_native_string_header_0041dd20(&image_->data_20, strings);
        destroy_native_string_header_0041dd20(&image_->name_0c, strings);
        destroy_native_int_pointer_tree18_0086fde0(image_);
        image_->~Image();
        singleton_lifetime_free(image_);
        image_ = nullptr;
        phase_ = Phase::destroyed;
        return;
    }
    // Native frees payloads first but retains tree nodes until after image
    // and string release. The typed map releases both together at this phase.
    data_->glyphs.clear();
    if (auto* captured = data_->space_lf_glyph.gfx_texture_18) {
        release_native_render_actual_owner(actual_owners_, captured);
        data_->space_lf_glyph.gfx_texture_18 = nullptr;
    }
    // Reload from the SAME owning glyph after any GFX terminal callback.
    if (auto* captured = data_->space_lf_glyph.alpha_texture_1c) {
        release_native_render_actual_owner(actual_owners_, captured);
        data_->space_lf_glyph.alpha_texture_1c = nullptr;
    }
    phase_ = Phase::destroyed;
}

NativeFontResources& NativeFontResourceOwners::publish(std::unique_ptr<NativeFontResources>& font) {
    if (image_registry_ || !font || font->image_ || font->phase_ != NativeFontResources::Phase::live)
        throw std::invalid_argument("cannot publish a null or retired actual font resource owner");
    for (const auto& current : fonts_)
        if (&current->descriptor() == &font->descriptor())
            throw std::logic_error("font descriptor already has its canonical resource owner");
    auto& result = *font;
    fonts_.push_back(std::move(font));
    return result;
}
NativeFontResources& NativeFontResourceOwners::resolve(const FontDescriptor* font) const {
    for (const auto& current : fonts_)
        if (&current->descriptor() == font) {
            if (current->phase_ != NativeFontResources::Phase::live)
                throw std::logic_error("current Text font resource owner has been retired");
            return *current;
        }
    throw std::logic_error("current Text font has no canonical actual resource owner");
}
NativeFontResources& NativeFontResourceOwners::resolve_image(const void* image) const {
    if (image) for (const auto& current : fonts_)
        if (current->image_ == image) {
            if (!current->live()) throw std::logic_error("current image Font has been retired");
            return *current;
        }
    throw std::logic_error("current Text Font image has no canonical resource owner");
}

// Source custody metadata, not another native registry/header/count/name map.
// The raw header/list is the only lookup authority in the selected arm.
struct NativeFontResourceOwners::ImageRegistry {
    enum class Phase { constructing, building, ready, retiring, retired };
    FontRegistryImageStorage* image{};
    FontRegistryImageNode* pending_node{}; // Retain an entered failed allocation.
    Phase phase{Phase::constructing};
};
NativeFontResourceOwners::~NativeFontResourceOwners() noexcept {
    if (image_registry_) {
        // Failed native graphs belong to the application's retained process
        // failure path. Never turn C++ destruction into native rollback.
        if (image_registry_->phase != ImageRegistry::Phase::retired) std::terminate();
        delete image_registry_;
    }
}
void NativeFontResourceOwners::construct_image_registry_00ac3690() {
    if (image_registry_ || !fonts_.empty())
        throw std::logic_error("native Font registry requires fresh empty custody");
    image_registry_ = new ImageRegistry; // Retained BEFORE raw constructor effects.
    auto& state = *image_registry_;
    state.image = new (singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x10u, sizeof(FontRegistryImageStorage)})) FontRegistryImageStorage{};
    // AC36BB identity precedes AC36C1 sentinel allocation and publication.
    state.image->native_vtable_00 = 0x00d5ca84u;
    auto* head = new (singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x0cu, sizeof(FontRegistryImageNode)})) FontRegistryImageNode;
    state.pending_node = head;
    head->next = head; // AC3400: only next/previous, no sentinel payload store.
    head->previous = head;
    state.image->head_08 = head;
    state.image->count_0c = 0;
    state.pending_node = nullptr;
    state.phase = ImageRegistry::Phase::building;
}
NativeFontResources& NativeFontResourceOwners::adopt_image_before_load_00ac3910(
    std::unique_ptr<NativeFontResources>& font) {
    if (!image_registry_ || image_registry_->phase != ImageRegistry::Phase::building ||
        !font || !font->image_ || font->phase_ != NativeFontResources::Phase::preparing)
        throw std::logic_error("native registry append requires a constructed, unloaded Font image");
    for (const auto& current : fonts_)
        if (&current->descriptor() == &font->descriptor())
            throw std::logic_error("Font descriptor already has canonical image custody");
    auto& owned = *font;
    fonts_.push_back(std::move(font)); // C++ custody BEFORE node allocation/failure.
    auto& state = *image_registry_;
    auto* head = state.image->head_08;
    auto* node = new (singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x0cu, sizeof(FontRegistryImageNode)})) FontRegistryImageNode;
    state.pending_node = node;
    node->next = head; // AC3490 operand/value stores, then AC37F0 size guard.
    node->previous = head->previous;
    node->font = owned.image_;
    if (0x3fffffffu - state.image->count_0c < 1u)
        throw std::length_error("list<T> too long"); // Qualified Source CRT transport.
    ++state.image->count_0c;
    head->previous = node; // AC3BB5, before the previous-next store at AC3BCC.
    node->previous->next = node;
    state.pending_node = nullptr;
    return owned;
}
void NativeFontResourceOwners::complete_image_registry() {
    if (!image_registry_ || image_registry_->phase != ImageRegistry::Phase::building ||
        image_registry_->pending_node)
        throw std::logic_error("native Font registry completion requires one completed build");
    for (const auto& current : fonts_)
        if (!current->image_ || !current->live())
            throw std::logic_error("native Font registry cannot publish an incomplete Font");
    image_registry_->phase = ImageRegistry::Phase::ready;
}
void* NativeFontResourceOwners::native_registry_image() const noexcept {
    return image_registry_ && image_registry_->phase == ImageRegistry::Phase::ready
        ? image_registry_->image : nullptr;
}
NativeFontResources* NativeFontResourceOwners::find_native_font_00ac3570(const void* incoming) const {
    if (!native_registry_image() || !incoming)
        throw std::logic_error("native Font name lookup requires a ready same-owner registry and name header");
    auto* registry = image_registry_->image;
    auto* node = registry->head_08->next;
    for (;;) {
        if (node == registry->head_08) return nullptr;
        // The node value IS the same constructed Font, never a descriptor cast.
        auto* image = static_cast<NativeFontResources::Image*>(node->font);
        std::uint32_t incoming_length;
        std::memcpy(&incoming_length, incoming, sizeof(incoming_length));
        if (incoming_length == image->name_0c.length()) {
            bool equal = incoming_length == 0;
            if (!equal) {
                const char* incoming_data;
                std::memcpy(&incoming_data, static_cast<const std::byte*>(incoming) + 4, sizeof(incoming_data));
                equal = _stricmp(incoming_data, image->name_0c.data()) == 0;
            }
            if (equal) return &resolve_image(node->font); // Identity -> custody only.
        }
        node = node->next;
    }
}
void NativeFontResourceOwners::retire_image_registry_00ac36f0() {
    if (!image_registry_) {
        if (!fonts_.empty()) throw std::logic_error("native registry retirement has no image authority");
        return; // Fresh application that never entered its registry getter/load.
    }
    auto& state = *image_registry_;
    if (state.phase != ImageRegistry::Phase::ready || state.pending_node)
        throw std::logic_error("native Font registry cannot replay incomplete or completed retirement");
    state.phase = ImageRegistry::Phase::retiring;
    auto* registry = state.image;
    registry->native_vtable_00 = 0x00d5ca84u;
    for (auto* node = registry->head_08->next; node != registry->head_08; node = node->next)
        // The resource owner already combines AD53A0 and its caller's outer
        // Font free. Do NOT free/decrement that same image a second time here.
        resolve_image(node->font).destroy_00ad53a0_fragment();
    auto* head = registry->head_08;
    auto* node = head->next;
    head->next = head;
    head->previous = head;
    registry->count_0c = 0;
    while (node != head) {
        auto* next = node->next;
        singleton_lifetime_free(node);
        node = next;
    }
    singleton_lifetime_free(head);
    registry->head_08 = nullptr;
    // Borrowed Source profile has no fake F8BF44 publication to clear.
    registry->native_vtable_00 = 0x00ce3818u;
    singleton_lifetime_free(registry);
    state.image = nullptr;
    state.phase = ImageRegistry::Phase::retired;
}
} // namespace bsp
