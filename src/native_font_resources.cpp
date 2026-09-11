#include "bsp/native_font_resources.hpp"
#include <cstddef>
#include <cstring>
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
std::int32_t NativeFontResources::signed_height_14() const noexcept {
    std::int16_t height;
    std::memcpy(&height, &data_->scaled_height, sizeof(height));
    return height;
}
const FontGlyphData& NativeFontResources::glyph(std::uint16_t key) const noexcept {
    return select_font_glyph_00ad4480(*data_, key);
}
void NativeFontResources::destroy_00ad53a0_fragment() {
    if (phase_ == Phase::destroyed) return;
    if (phase_ != Phase::live)
        throw std::logic_error("native font destruction requires a live non-reentrant owner");
    phase_ = Phase::destroying;
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
    if (!font || font->phase_ != NativeFontResources::Phase::live)
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
} // namespace bsp
