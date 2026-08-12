#pragma once
#include <nw4r/g3d.h>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/fanm.hpp>

namespace m3d {
    class anmChrBlend_c : public banm_c {
    public:
        virtual banm_c::anmType_e getType() const { return banm_c::TYPE_ANM_CHR; };

        bool create(nw4r::g3d::ResMdl mdl, int count, mAllocator_c *allocator, size_t *objSize);

        /// @brief Convenience overload, mirroring the ones m3d::mdl_c and
        /// m3d::anmChr_c carry.
        /// @details Callers reach @ref create through this wrapper, which is what
        /// places their argument temporaries in the inline-expansion pass rather
        /// than the direct-call pass. daEnDfpakkun_c::createMdl does not match
        /// without it.
        bool create(nw4r::g3d::ResMdl mdl, int count, mAllocator_c *allocator) {
            return create(mdl, count, allocator, nullptr);
        }

        void attach(int idx, nw4r::g3d::AnmObjChrRes *chrRes, float weight);
        void attach(int idx, m3d::anmChr_c *anmChrRes, float weight);
        void detach(int idx);
    };
}
