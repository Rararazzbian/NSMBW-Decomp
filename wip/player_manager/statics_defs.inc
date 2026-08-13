// Out-of-line storage definitions for daPyMng_c's static members.
//
// NO AUTHORING BATCH OWNS THESE, and batch B8 correctly flagged that several
// batches' functions cannot assemble address-exact until they exist. They are
// the lead's to place, and their ORDER is the whole point: definition order in
// the .cpp controls emission order within each section, and the target anchors
// several adjacent .bss arrays off m_playerID's own relocation -- reaching
// mPlayerEntry as 0x40(r31) and so on, including arrays a given function never
// reads. MWCC only shares one base register across several objects once it can
// see all their definitions in a single TU, which is why standalone drafts of
// getNumInGame, getCoinAll, initYoshiPriority, setYoshiPriority,
// isCreateBalloon and checkCorrectCreateInfo all show register-allocation
// differences that are NOT source defects.
//
// .bss and .sbss are separate sections, so each preserves the relative order of
// its own objects independently; writing the .bss group first and the .sbss
// group second satisfies both at once.
//
// Section placement is automatic, not chosen: MWCC routes objects of 8 bytes or
// fewer to .sbss and larger ones to .bss. The int[4] arrays are 0x10 and land
// in .bss; the scalars, m_star_time/m_star_count (s16[4] = 8) and m_yoshiColor
// (u8[4] = 4) land in .sbss. If any of them comes out in the wrong section the
// bound is wrong, so check rather than assume.

// --- .bss, in target address order 0x80355110 .. 0x803551D0 ----------------
int daPyMng_c::m_playerID[4];          // 0x80355110  <- the base anchor
int daPyMng_c::m_yoshiID[4];           // 0x80355120
int daPyMng_c::mCourseInList[4];       // 0x80355130
int daPyMng_c::m_yoshiFruit[4];        // 0x80355140
int daPyMng_c::mPlayerEntry[4];        // 0x80355150  = m_playerID + 0x40
PLAYER_TYPE_e daPyMng_c::mPlayerType[4];    // 0x80355160  = + 0x50
PLAYER_POWERUP_e daPyMng_c::mPlayerMode[4]; // 0x80355170  = + 0x60
u32 daPyMng_c::mCreateItem[4];         // 0x80355180  = + 0x70
int daPyMng_c::mRest[4];               // 0x80355190
int daPyMng_c::mCoin[4];               // 0x803551A0  indexed by PLAYER_TYPE_e
int daPyMng_c::m_quakeTimer[4];        // 0x803551B0  = + 0xA0
int daPyMng_c::m_quakeEffectFlag[4];   // 0x803551C0  = + 0xB0

// The four embedded managers. Each is preceded in .bss by a 0xC
// __register_global_object destructor-chain node that the compiler emits, and
// __sinit constructs them in exactly this order -- so this order is load
// bearing twice over.
//
// The 4-byte gaps before mDemoManager and mAttention are NOT alignment of these
// classes and need no action: MWCC aligns a .bss object to 8 when its SIZE is a
// multiple of 8, regardless of the type's own alignment. 0x98 and 0x58 are;
// 0x5C and 0xC5C are not. See HANDOFF.
daPyDemoMng_c daPyMng_c::mDemoManager;  // 0x803551E0, 0x98
dMultiMng_c   daPyMng_c::mMultiManager; // 0x80355284, 0x5C
dAttention_c  daPyMng_c::mAttention;    // 0x803552F0, 0x58
dPyEffectMng_c daPyMng_c::mEffectMng;   // 0x80355354, 0xC5C  (ends 0x80355FB0)

// --- .sbss, in target address order 0x80429F80 .. 0x80429FD0 ---------------
int daPyMng_c::mNum;                 // 0x80429F80
u32 daPyMng_c::mCtrlPlrNo;           // 0x80429F84
u8  daPyMng_c::mActPlayerInfo;       // 0x80429F88  (1 byte, then a 3-byte pad)
u8  daPyMng_c::m_yoshiColor[4];      // 0x80429F8C  indexed raw, unscaled
s16 daPyMng_c::m_star_time[4];       // 0x80429F90
s16 daPyMng_c::m_star_count[4];      // 0x80429F98
int daPyMng_c::mScore;               // 0x80429FA0
PLAYER_POWERUP_e daPyMng_c::mKinopioMode; // 0x80429FA4
int daPyMng_c::mTimeUpPlayerNum;     // 0x80429FA8
int daPyMng_c::mAllBalloon;          // 0x80429FAC
int daPyMng_c::mPauseEnableInfo;     // 0x80429FB0
u32 daPyMng_c::mPauseDisable;        // 0x80429FB4
u32 daPyMng_c::mStopTimerInfo;       // 0x80429FB8
u32 daPyMng_c::mStopTimerInfoOld;    // 0x80429FBC
int daPyMng_c::mQuakeTrigger;        // 0x80429FC0
int daPyMng_c::mBgmState;            // 0x80429FC4
int daPyMng_c::mBonusNoCap;          // 0x80429FC8
int daPyMng_c::mKinopioCarryCount;   // 0x80429FCC

// 0x80429FD0 is the unnamed 1-byte file-scope flag (dtk calls it lbl_80429FD0),
// read and written only by setHipAttackQuake. It is BATCH B7's to name and
// define, and it is the last object in the unit's .sbss -- if it is missing the
// section is 8 bytes short and four of five binaries fail.

// --- NOT class statics, but they are in our .bss and nobody flagged them ---
//
// Three ints at 0x80355FB0, immediately after mEffectMng ends. dtk labels the
// whole 0x10 there as `gap_08_80355FB0_bss` padding, and the bounds derivation
// accepted that -- but batch B7 found setHipAttackQuake actually READS AND
// WRITES them: a lazy one-shot 3-entry sound-id table.
//
// This is the "an unclaimed object is a finding, not noise" trap for the second
// time on this project, and it is the same shape that failed the previous
// unit's first link. Being 0xC of a 0x10 gap, it is invisible to the bound --
// the section still ends at 0x80355FC0 either way -- so no bounds check would
// ever have caught it. Only a function actually touching it does.
//
// They belong INSIDE setHipAttackQuake as a function-local static, which is why
// they are not defined here. Listed so the assembly step confirms they exist.
