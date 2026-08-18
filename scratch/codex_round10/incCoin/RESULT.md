# incCoin result

## Final source

```cpp
void daPyMng_c::incCoin(int plrNo) {
    daPyMng_c::changeItemKinopioPlrNo(plrNo);
    dMultiMng_c::mspInstance->incCoin(plrNo);
    if (daPyMng_c::getCoinAll() < scCoinMax) {
        mCoin[mPlayerType[plrNo]]++;
    } else {
        int single = 1;
        int entry = daPyMng_c::getEntryNum();
        int value = entry ^ 1;
        int half = value >> 1;
        value = (value & entry) - half;
        if ((unsigned)value >> 31) {
            single = 0;
            dBgParameter_c *bgParam = dBgParameter_c::ms_Instance_p;
            mVec2_c pos(bgParam->xStart() + bgParam->xSize() / 2, bgParam->yStart() - bgParam->ySize() / 2);
            int remote = 0;
            for (int i = 0; i < 4; i++) {
                if (mPlayerEntry[i]) remote |= dAudio::getRemotePlayer(i);
            }
            dAudio::SoundEffectID_t(SE_SYS_100COIN_ONE_UP).playMapSound(pos, remote);
            dAudio::SoundEffectID_t(SE_SYS_100COIN_ONE_UP_RC).playMapSound(pos, remote);
        }
        bool restFlag = single;
        for (int i = 0; i < 4; i++) {
            if (mPlayerEntry[i]) {
                dAcPy_c *p = daPyMng_c::getPlayer(i);
                if (p != nullptr && !p->isStatus(4)) {
                    dScoreMng_c::m_instance->fn_800e25a0(8, i, single);
                    continue;
                }
            }
            daPyMng_c::addRest(i, 1, restFlag);
        }
        for (int i = 0; i < 4; i++) mCoin[mPlayerType[i]] = 0;
    }
}
```

## Measurement

Whole-TU compile via `harness.py`, followed by address extraction and `diff_fn`. Baseline was 126 draft instructions versus 130 target. The final source emits 130 instructions, reaching the target `0x208` bytes.

Result: **near miss, not MATCH**. The four-instruction size gap is closed. Remaining differences are the outer branch layout, register allocation in the branchless entry-count materialization, and unlinked `.bss` relocation naming (`SYM0` versus `m_playerID`). Normal comparison forms stayed at 126 instructions. Reversing the outer blocks reached 130 with broader register differences.

## Confidence

Medium. The source-shape mechanism is strongly supported because it closes the exact four-instruction gap and preserves the target branchless arithmetic structure. Byte-exactness is not established.
