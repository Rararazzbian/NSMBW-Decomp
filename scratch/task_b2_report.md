# Round 5, Task B2: dInfo_c + 0xafc

## Result

pad11 starts at 0x3f0. The byte at dInfo_c + 0xafc is byte 0x70c within
pad11[0x712]. There are five bytes after the named field.

The proposed name is mEffectStopOverride. This describes only the observed
use: daPyMng_c::isEffectStop tests the byte, and non-zero prevents effects
from being considered stopped. No broader behavior is inferred.

## Exact arithmetic

The preceding members, in declaration order, have these sizes. dCyuukan_c is
0x34 bytes under MWCC. The three bytes of compiler
alignment after bool m_6c are included explicitly.

    pad1                              0x008  -> 0x008
    mCyuukan                          0x034  -> 0x03c
    mCurrentCourseWorld               0x004  -> 0x040
    mCurrentCourseNo                  0x004  -> 0x044
    mCurrentCourseNode                0x004  -> 0x048
    pad2                              0x00c  -> 0x054
    m_54                              0x004  -> 0x058
    pad3                              0x008  -> 0x060
    m_60                              0x004  -> 0x064
    m_64                              0x004  -> 0x068
    m_68                              0x004  -> 0x06c
    m_6c                              0x001  -> 0x06d
    pad5                              0x02c  -> 0x099
    alignment before m_9c             0x003  -> 0x09c
    m_9c                              0x004  -> 0x0a0
    pad6                              0x2e4  -> 0x384
    mCharIDs[4]                       0x010  -> 0x394
    mIsWorldSelect                    0x001  -> 0x395
    pad7                              0x01e  -> 0x3b3
    mClearCyuukan                     0x001  -> 0x3b4
    mDisplayCourseWorld               0x004  -> 0x3b8
    mDisplayCourseNum                 0x004  -> 0x3bc
    mTotalCollectionCoin              0x004  -> 0x3c0
    mSaveFileNumber                   0x004  -> 0x3c4
    mPlayerNum                        0x004  -> 0x3c8
    mScissorIndex                     0x004  -> 0x3cc
    mPlayNumber                       0x004  -> 0x3d0
    mTextBoxMessageGroup              0x004  -> 0x3d4
    mTextBoxMessageID                 0x004  -> 0x3d8
    pad9                              0x001  -> 0x3d9
    mExtensionAttached                0x001  -> 0x3da
    m_3da                             0x001  -> 0x3db
    pad10                             0x001  -> 0x3dc
    mScissorPane                      0x004  -> 0x3e0
    mScissorDrawInfo                  0x004  -> 0x3e4
    mCourseSelectPageNum              0x004  -> 0x3e8
    mCourseSelectIndexInPage          0x004  -> 0x3ec

    pad11 start = 0x3f0

The pointer members are 4 bytes in the 32-bit Wii ABI. The hand-sum above
places them at 0x3e0 and 0x3e4, but existing comments place them at 0x3dc and
0x3e0. This is a four-byte contradiction: MWCC reports dCyuukan_c as 0x34,
while the existing d_info layout uses 0x30 bytes for this region.

    N = 0xafc - 0x3f0
      = 0x70c

    M = 0x712 - 0x70c - 0x001
      = 0x005

    0x70c + 0x001 + 0x005 = 0x712
    0x3f0 + 0x712 = 0xb02
    sizeof(dInfo_c) = 0xb02 + (4 * 0x16) + 0x004 = 0xb5e

The hand-summed expected class size is 0xb5e. MWCC instead reports 0xb5c for
the current class, so this report cannot prove that the hand-summed region is
the actual compiled region.

## Before

    int mCourseSelectPageNum;
    int mCourseSelectIndexInPage;
    u8 pad11[0x712];
    bool mFukidashiActionPerformed[4][0x16];
    u32 pad12;

## Proposed replacement

This follows the existing pad4 precedent where hidden fields were split out
and named. The comment states only the available evidence. It is a proposal,
not a safe header change while the four-byte layout contradiction remains.

    int mCourseSelectPageNum;
    int mCourseSelectIndexInPage;
    u8 pad_before_mEffectStopOverride[0x70c];
    /// @brief Byte tested by daPyMng_c::isEffectStop at dInfo_c + 0xafc.
    /// A non-zero value prevents effects from being considered stopped.
    /// @unofficial
    u8 mEffectStopOverride;
    u8 pad_after_mEffectStopOverride[0x5];
    bool mFukidashiActionPerformed[4][0x16];
    u32 pad12;

    static_assert(sizeof(dInfo_c) == 0xb5e, "dInfo_c size changed");
    static_assert(offsetof(dInfo_c, mEffectStopOverride) == 0xafc,
                  "mEffectStopOverride offset changed");

If this MWCC configuration does not support static_assert or offsetof for
this class, retain equivalent checks in a compiler-supported layout probe.

## Probe status

scratch/task_b2_probe.cpp compiled successfully with MWCC and the include
paths and flags from build.ninja. The probe statically confirmed
sizeof(dCyuukan_c) == 0x34 and sizeof(dInfo_c) == 0xb5c. This conflicts with
the hand-summed 0xb5e layout above. An attempted offsetof
constant expression used as an array bound was rejected by MWCC, so the probe
did not independently expose the numeric field offset. No ninja, configure.py,
progress.py, or land.py command was run.

## Offset impact

Because the MWCC size probe conflicts with the hand-summed layout, this field
cannot be safely placed based on the current evidence. Keep the raw cast as
the fallback and do not apply the proposed replacement yet. If the discrepancy
is resolved, the replacement is non-perturbing when 0x70c + 1 + 5 equals
0x712. It is
offset-perturbing, and catastrophic for all following members, if the total
is wrong. Until the header change is accepted and validated by project layout
checks, the raw cast remains the safe fallback.
