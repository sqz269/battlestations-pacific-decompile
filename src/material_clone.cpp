#include "bsp/material_clone.hpp"
#include <new>
#include <utility>

namespace bsp {
HRESULT clone_material_00b18b60(const MaterialCloneState& source,
    std::shared_ptr<MaterialCloneState>& output) {
    if (source.byte10d && source.pointer0c
        && (source.owner0c.get() != source.pointer0c || !source.owner0c.use_count()))
        return E_INVALIDARG;

    try {
        auto clone = std::make_shared<MaterialCloneState>();
        clone->word08 = source.word08;
        clone->pointer0c = source.pointer0c;
        clone->byte10d = source.byte10d;
        if (clone->byte10d && clone->pointer0c)
            clone->owner0c = source.owner0c;

        // Native signed-count loop copies every slot below the high-water
        // count, including nulls. The existing typed slot API bounds it to9
        // and preserves each logical texture's retained identity.
        const auto& textures = source.textures.textures();
        for (std::uint32_t index = 0; index < textures.size(); ++index)
            set_material_texture_00b189f0(clone->textures, index, textures[index]);

        //00b18c79 forces the flag on before REP MOVSD copies17DWORDs.
        // The existing setter has the same order and preserves all float bits.
        clone->lighting.set_lighting_record_00b179d0(0, source.lighting.values());
        clone->effect = source.effect;
        clone->word104 = source.word104;
        clone->word108 = source.word108;
        // word100 remains its fresh-constructor zero, regardless of source.
        output = std::move(clone);
        return S_OK;
    } catch (const std::bad_alloc&) {
        return E_OUTOFMEMORY;
    }
}
}
