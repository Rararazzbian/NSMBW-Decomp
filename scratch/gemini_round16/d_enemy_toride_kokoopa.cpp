#include <game/bases/d_enemy_toride_kokoopa.hpp>
#include <game/bases/d_actor_manager.hpp>
#include <game/bases/d_score_manager.hpp>
#include <game/framework/f_manager.hpp>
#include <game/sLib/s_lib.hpp>

void dEnTorideKokoopa_c::calcKokoopaMdl() {}
void dEnTorideKokoopa_c::calcShellMdl() {}
void dEnTorideKokoopa_c::drawKokoopa() {}
void dEnTorideKokoopa_c::drawShell() {}
void dEnTorideKokoopa_c::setKokoopaCc() {}
void dEnTorideKokoopa_c::setShellCc() {}
void dEnTorideKokoopa_c::moveAdjust_HIO() {}
void dEnTorideKokoopa_c::calcCcData() {}

float dEnTorideKokoopa_c::getDrawScale() {
    return 1.0f;
}

void dEnTorideKokoopa_c::calcRootJntPos() {
    mMdlKokoopa.getNodeWorldMtxMultVecZero(mRootJntIdx, mRootJntPos);
    mRootJntPos.z = 0.0f;
}

void dEnTorideKokoopa_c::calcShellJntPos() {
    mMdlShell.getNodeWorldMtxMultVecZero(mShellJntIdx, mShellJntPos);
    mShellJntPos.z = 0.0f;
}
