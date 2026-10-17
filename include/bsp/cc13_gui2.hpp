#pragma once
// cc13_gui2: the GUI Text geometry update and its two glyph-layout builders,
// the callees that kept cc13_gui1's text setters (00ABAED0, 00ABB000, 00AB6AB0)
// from being wired. Packet cc13-gui2 (2026-10-17), docs/CC13_GUI2.md.
//
// Every routine works on the actual native Text object (raw address, Win32
// layout) and reaches each callee through a named hook invoked at the binary's
// call point with the binary's arguments. The three routines call each other
// directly, as the image does. Names are hypotheses, not recovered symbols.
// Reconstructed and build-tested only; nothing is wired, fixture- or
// game-validated.
//
// x87: these routines run on the main thread, which runs at 24-bit precision
// (control word 007Fh after device creation, docs/CC13_FPU_PRECISION.md), so
// each x87 arithmetic op is written as its own float rounding in the image's
// order (fix cc13-fix12). The FISTP of the container width keeps the binary's
// control-word sequence (inline asm) on the float-rounded 960 product.
//
// SEH: 00ABA8D0's unwind states only release the copied wide string and the
// parameter-name temporaries when a callee throws; the hooks are not expected
// to throw and no unwind path is reproduced (as in cc13_gui1).

#include "bsp/cc13_gui1.hpp"

#include <cstdint>

namespace bsp::cc13_gui2 {

using cc13_gui1::NativeStringHeader;
using cc13_gui1::NativeWideStringHeader;

struct GuiTextGeometryHooks : cc13_gui1::NativePoolHooks {
    // --- geometry and streams -------------------------------------------
    // 00B74640 BSP_GeneratedModel_GetGeometry, __thiscall(model; unused) RET 4
    // -> model+180h.
    virtual void* generated_model_get_geometry_00b74640(void* model, std::uint32_t unused) = 0;
    // 00B73260 BSP_Geometry_GetVertexStream, __thiscall(geometry; index) RET 4.
    virtual void* geometry_get_vertex_stream_00b73260(void* geometry, std::uint32_t index) = 0;
    // 00B732C0 BSP_MeshObject_GetElement, __thiscall(geometry; index) RET 4 ->
    // draw section.
    virtual void* mesh_get_element_00b732c0(void* geometry, std::uint32_t index) = 0;
    // 00B73BB0 BSP_Geometry_SetVertexStream, __thiscall(geometry; index, stream) RET 8.
    virtual void geometry_set_vertex_stream_00b73bb0(void* geometry, std::uint32_t index, void* stream) = 0;
    // 00B73B70 BSP_Geometry_SetIndexStream, __thiscall(geometry; index object) RET 4.
    virtual void geometry_set_index_stream_00b73b70(void* geometry, void* index_stream) = 0;
    // vertex_stream->vtable[+10h](count, 0, 0): the result is not read by
    // either builder. vertex_stream->vtable[+14h](): the closing call.
    virtual void vertex_stream_vslot10(void* stream, std::uint32_t count, std::uint32_t zero_a,
        std::uint32_t zero_b) = 0;
    virtual void vertex_stream_vslot14(void* stream) = 0;
    // index_stream->vtable[+0Ch](count, 0, 0) -> the index write pointer;
    // index_stream->vtable[+10h](): the closing call.
    virtual std::uint16_t* index_stream_vslot0c(void* stream, std::uint32_t count, std::uint32_t zero_a,
        std::uint32_t zero_b) = 0;
    virtual void index_stream_vslot10(void* stream) = 0;
    // 004768D0 BSP_LogicalVertexStream_ReadPosition, __thiscall(stream; float
    // out[3], index) RET 8 -> out (the result is not read).
    virtual float* logical_vertex_stream_read_position_004768d0(void* stream, float* out,
        std::uint32_t index) = 0;
    // 00B865A0 BSP_DrawSection_RebuildVertexLayout, __thiscall(section; geometry) RET 4.
    virtual void draw_section_rebuild_vertex_layout_00b865a0(void* section, void* geometry) = 0;

    // --- font, glyphs, materials ------------------------------------------
    // 00AD4480 BSP_Font_SelectGlyph, __thiscall(font; code unit zero-extended
    // to a DWORD) RET 4 -> glyph record.
    virtual void* font_select_glyph_00ad4480(void* font, std::uint32_t code_unit) = 0;
    // 00B189F0 BSP_Material_SetTextureSlot, __thiscall(material; slot, resource) RET 8.
    virtual void material_set_texture_slot_00b189f0(void* material, std::uint32_t slot, void* resource) = 0;
    // 00B19210 BSP_Material_SetShaderAndClearParameters, __thiscall(material; shader) RET 4.
    virtual void material_set_shader_00b19210(void* material, void* shader) = 0;
    // 00B18B00 / 00B18AA0 / 00B18B20 BSP_Material_RegisterFloat2/Float4/Float
    // Parameter, __thiscall(material; const NativeString* name, const void*
    // source) RET 8.
    virtual void material_register_float2_00b18b00(void* material, const NativeStringHeader* name,
        const void* source) = 0;
    virtual void material_register_float4_00b18aa0(void* material, const NativeStringHeader* name,
        const void* source) = 0;
    virtual void material_register_float_00b18b20(void* material, const NativeStringHeader* name,
        const void* source) = 0;
    // 00B179F0 BSP_Material_GetDiffuseColor, __thiscall(material; unused) RET 4
    // -> material+38h (four floats).
    virtual float* material_get_diffuse_color_00b179f0(void* material, std::uint32_t unused) = 0;
    // 00F8BE54: the image's zero block passed as the shadow's cBlendFactor source.
    virtual const void* zero_block_00f8be54() = 0;

    // --- text context -------------------------------------------------------
    // 00AB98F0 BSP_TextContext_WriteGlyphQuad, __thiscall(text; ten DWORDs) RET 28h:
    // glyph, position, vertex stream, index pointer, position, position, quad
    // index, font height, first vertex, code unit. Both builders pass the same
    // float[3] for the three position slots.
    virtual void text_write_glyph_quad_00ab98f0(void* text, void* glyph, float* position_2,
        void* vertex_stream, std::uint16_t* indices, float* position_5, float* position_6,
        std::uint32_t quad_index, std::uint32_t font_height, std::uint32_t first_vertex,
        std::uint32_t code_unit) = 0;
    // 00AB80C0 BSP_TextContext_ClearGlyphChildren, __thiscall(text) RET.
    virtual void text_clear_glyph_children_00ab80c0(void* text) = 0;
    // 00AB8530 BSP_TextContext_EnsureDrawSections, __thiscall(text) RET.
    virtual void text_ensure_draw_sections_00ab8530(void* text) = 0;
    // 00AB8400 BSP_TextContext_CreateGlyphBuffers, __thiscall(text; capacity,
    // geometry) RET 8.
    virtual void text_create_glyph_buffers_00ab8400(void* text, std::uint32_t capacity, void* geometry) = 0;
    // 00AB8CE0 BSP_TextContext_EnsureFontShader, __thiscall(text) RET -> AL.
    virtual bool text_ensure_font_shader_00ab8ce0(void* text) = 0;
    // 00AA9F10 BSP_UIContext_RegisterClipParameters, __thiscall(text; material) RET 4.
    virtual void ui_context_register_clip_parameters_00aa9f10(void* text, void* material) = 0;
    // text->vtable[+50h](text+50h).
    virtual void text_vslot50(void* text, void* text_plus_50) = 0;
    // shadow model (text+188h)->vtable[+2Ch](const float scale[3]).
    virtual void model_vslot2c(void* model, const float* scale) = 0;
    // 00B6E680 BSP_Node_SetParent, __thiscall(node; parent) RET 4.
    virtual void node_set_parent_00b6e680(void* node, void* parent) = 0;
    // 00B6D890 BSP_Node_PropagateRootRegistration, __thiscall(node; root) RET 4.
    virtual void node_propagate_root_registration_00b6d890(void* node, void* root) = 0;

    // --- strings ------------------------------------------------------------
    // 00F8BE50: the image's empty UTF-16 string, read when text+0F0h is null.
    virtual const std::uint16_t* empty_wide_00f8be50() = 0;
    // 004C53E0 BSP_WideString_Resize, __thiscall(string; length, preserve) RET 8.
    virtual void wide_string_resize_004c53e0(NativeWideStringHeader* self, std::uint32_t length,
        std::uint32_t preserve) = 0;
    // 004C8DD0 BSP_WideString_CopyConstruct, __thiscall(uninitialised dst; src) RET 4.
    virtual void wide_string_copy_construct_004c8dd0(NativeWideStringHeader* dst,
        const NativeWideStringHeader* src) = 0;
    // 004C5E20 BSP_WideString_Assign, __thiscall(dst; src) RET 4 -> dst.
    virtual NativeWideStringHeader* wide_string_assign_004c5e20(NativeWideStringHeader* dst,
        const NativeWideStringHeader* src) = 0;
    // [00F8BC4C] the locale table; 00A9EC30 BSP_TextContext_UppercaseUtf16InPlace,
    // __thiscall(table; wide string*) RET 4.
    virtual void* locale_table_00f8bc4c() = 0;
    virtual void uppercase_utf16_in_place_00a9ec30(void* table, NativeWideStringHeader* text) = 0;
    // 00C03A39 __wcsicmp (CRT, cdecl).
    virtual int wcsicmp_00c03a39(const std::uint16_t* left, const std::uint16_t* right) = 0;
    // 0041E870 BSP_NativeString_Assign, __thiscall(uninitialised out; const char*) RET 4.
    virtual void native_string_assign_0041e870(NativeStringHeader* out, const char* text) = 0;
};

// 00AB9FD0 BSP_TextContext_BuildSingleLineGlyphGeometry, __thiscall(ECX = text;
// const wide* source (never read), draw section) RET 8.
void text_build_single_line_geometry_00ab9fd0(GuiTextGeometryHooks& host, void* text,
    const NativeWideStringHeader* source, void* section);
// 00ABA270 BSP_TextContext_BuildGlyphGeometry (the wrapped builder),
// __thiscall(ECX = text; const wide* source, draw section) RET 8.
void text_build_wrapped_geometry_00aba270(GuiTextGeometryHooks& host, void* text,
    const NativeWideStringHeader* source, void* section);
// 00ABA8D0 BSP_TextContext_UpdateUtf16Geometry, __thiscall(ECX = text; const
// wide* source) RET 4, SEH 00CB8130. The target of cc13_gui1's
// GuiTextHooks::update_utf16_geometry_00aba8d0.
void text_update_utf16_geometry_00aba8d0(GuiTextGeometryHooks& host, void* text,
    const NativeWideStringHeader* source);

// ---------------------------------------------------------------------------
// GUI wiring blockers, segment 77
// ---------------------------------------------------------------------------
struct GuiWidgetAlphaHooks {
    virtual ~GuiWidgetAlphaHooks() = default;
    // 00B74650 BSP_GeneratedModel_HasGeometry, __thiscall(model) RET -> EAX
    // (model+180h != 0).
    virtual std::uint32_t generated_model_has_geometry_00b74650(void* model) = 0;
    // 00B74640 BSP_GeneratedModel_GetGeometry, __thiscall(model; unused) RET 4.
    virtual void* generated_model_get_geometry_00b74640(void* model, std::uint32_t unused) = 0;
    // 00B72B40 BSP_MeshObject_GetCount58, __thiscall(geometry) RET -> geometry+58h.
    virtual std::uint32_t mesh_get_count58_00b72b40(void* geometry) = 0;
    // 00B732C0 BSP_MeshObject_GetElement, __thiscall(geometry; index) RET 4.
    virtual void* mesh_get_element_00b732c0(void* geometry, std::uint32_t index) = 0;
    // 00B179F0 BSP_Material_GetDiffuseColor, __thiscall(material; unused) RET 4.
    virtual float* material_get_diffuse_color_00b179f0(void* material, std::uint32_t unused) = 0;
    // 00BF6713, the CRT invalid-parameter report of the checked list iterator.
    virtual void invalid_parameter_00bf6713() = 0;
};

// 00AA8610 BSP_GuiWidget_SetAlphaRecursive, __thiscall(ECX = widget; float
// alpha) RET 4.
void gui_widget_set_alpha_recursive_00aa8610(GuiWidgetAlphaHooks& host, void* widget, float alpha);

// The 8-byte {tag, payload} variant the property visitor takes by value. Tags
// seen in 00AAA710: 0 string (key and fallback a C string, field a
// NativeString*), 2 float (field a pointer, fallback the float bits), 3 bool
// (field a pointer, fallback a byte), 5 float3*, 6 float2*, 8 float4*.
struct NativeVariantPair {
    std::uint32_t tag;
    std::uint32_t payload;
};
// The child-entry block 00AAA710 builds on its stack (S+34h): 250 pairs whose
// tags are preset to -1 (payloads left as they were), then the count at
// +7D0h, zeroed before visitor->vtable[+18h] fills the block.
struct NativeVariantChildBlock {
    NativeVariantPair entries[250];
    std::int32_t count;
};
static_assert(sizeof(NativeVariantPair) == 8 && sizeof(NativeVariantChildBlock) == 0x7D4);

struct GuiWidgetReadPropertiesHooks : cc13_gui1::GuiWidgetHooks, cc13_gui1::NativePoolHooks {
    // visitor->vtable[+0Ch](key, field, fallback), three pairs by value (RET 18h).
    virtual void visitor_vslot0c(void* visitor, NativeVariantPair key, NativeVariantPair field,
        NativeVariantPair fallback) = 0;
    // visitor->vtable[+18h](NativeVariantChildBlock*): list the child entries.
    virtual void visitor_vslot18(void* visitor, NativeVariantChildBlock* block) = 0;
    // visitor->vtable[+4h](entry by value) / vtable[+8h](): enter / leave a child.
    virtual void visitor_vslot04(void* visitor, NativeVariantPair entry) = 0;
    virtual void visitor_vslot08(void* visitor) = 0;
    // The two function-local static colours and their guard DWORDs (bit 0):
    // [00F8BCE0] white {1,1,1,1} guarded by [00F8BCF0], [00F8BCCC] {0,0,0,1}
    // guarded by [00F8BCDC].
    virtual std::uint32_t* white_guard_00f8bcf0() = 0;
    virtual float* white_00f8bce0() = 0;
    virtual std::uint32_t* black_guard_00f8bcdc() = 0;
    virtual float* black_00f8bccc() = 0;
    // [0109CF04] the platform object; its +0Dh byte is the widescreen flag.
    virtual void* platform_0109cf04() = 0;
    // 00BF7FBF __stricmp (CRT, cdecl).
    virtual int stricmp_00bf7fbf(const char* left, const char* right) = 0;
    // 00425850 BSP_NativeString_EqualsCStringInsensitive, __thiscall(string;
    // const char*) RET 4 -> AL.
    virtual bool native_string_equals_insensitive_00425850(const NativeStringHeader* self,
        const char* text) = 0;
    // 0041DD40 BSP_NativeString_Resize, __thiscall(string; length, preserve) RET 8.
    virtual void native_string_resize_0041dd40(NativeStringHeader* self, std::uint32_t length,
        std::uint32_t preserve) = 0;
    // 00BF7680 memcpy (CRT).
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t size) = 0;
    // 00AA2490 BSP_GuiLayout_WidgetTypeForKey, __fastcall(ECX = const
    // NativeString*) -> type tag (0 for none).
    virtual std::uint32_t widget_type_for_key_00aa2490(const NativeStringHeader* key) = 0;
    // 00B75030 BSP_GeneratedModel_Construct is the inherited
    // GuiWidgetHooks::generated_model_construct_00b75030 (NativeString name);
    // 00AAA710 passes its NativeString temporary.
    // widget->vtable[+5Ch]() -> type id; widget->vtable[+34h](DWORD of a byte);
    // child->vtable[+18h](visitor); child->vtable[+78h]().
    virtual std::uint32_t widget_vslot5c(void* widget) = 0;
    virtual void widget_vslot34_dword(void* widget, std::uint32_t value) = 0;
    virtual void widget_vslot18(void* widget, void* visitor) = 0;
    virtual void widget_vslot78(void* widget) = 0;
};

// 00AAA710 BSP_GuiWidget_ReadPropertiesAndChildren, __thiscall(ECX = widget;
// visitor) RET 4, SEH 00CB7571, frame 7F8h.
void gui_widget_read_properties_and_children_00aaa710(GuiWidgetReadPropertiesHooks& host, void* widget,
    void* visitor);

// ---------------------------------------------------------------------------
// Small GUI-band blockers (each a hook in an existing frontend/terrain host)
// ---------------------------------------------------------------------------
struct InputBackendHooks {
    virtual ~InputBackendHooks() = default;
    // device->vtable[+28h]() -> AL.
    virtual bool device_vslot28(void* device) = 0;
    // 00BF6713, the checked vector iterator's invalid-parameter report.
    virtual void invalid_parameter_00bf6713() = 0;
};
// 00A90F90 BSP_InputBackend_AnyDeviceHasInput, __thiscall(ECX = backend) RET
// -> AL. Three device-pointer vectors at +6Ch, +90h, +0B4h.
bool input_backend_any_device_has_input_00a90f90(InputBackendHooks& host, void* backend);

// 00AA0F30, a bare RET (no stack arguments, ECX unused).
void gui_trivial_noop_00aa0f30() noexcept;
// 00A9AC50, __thiscall(ECX = listbox) RET -> listbox+0FCh (the row block).
void* gui_listbox_row_block_00a9ac50(void* listbox) noexcept;
// 00AB1060, no arguments, RET -> the DWORD at [00F8BDD8] (the widget kind
// token 0068DB40 compares); the caller passes that global's address.
std::uint32_t gui_widget_kind_token_00ab1060(const std::uint32_t* global_00f8bdd8) noexcept;
// 00AD5760 BSP_FoliageSystem_GetBushShadowAllowed, __thiscall(ECX = system)
// RET -> AL = system+10h.
std::uint8_t foliage_get_bush_shadow_allowed_00ad5760(const void* system) noexcept;
// 00ADA0C0 BSP_Landscape_TickTerrainNoOp, one stack DWORD, RET 4, no effect.
void landscape_tick_terrain_noop_00ada0c0(void* self, std::uint32_t argument) noexcept;

}  // namespace bsp::cc13_gui2
