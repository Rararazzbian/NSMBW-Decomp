#include <game/bases/d_enemy_boss.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_mat_clr.hpp>
#include <game/mLib/m_3d/anm_tex_pat.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/bases/d_cc.hpp>

STATIC_ASSERT(sizeof(m3d::mdl_c) == 0x40);
STATIC_ASSERT(sizeof(m3d::anmChr_c) == 0x38);
STATIC_ASSERT(sizeof(m3d::anmMatClr_c) == 0x30);
