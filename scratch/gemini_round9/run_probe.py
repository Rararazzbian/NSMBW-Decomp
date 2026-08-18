import os
import sys
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
MWCC = ROOT / 'compilers' / 'Wii' / '1.1' / 'mwcceppc.exe'

PROBE_CPP = ROOT / 'scratch' / 'gemini_round9' / 'probe.cpp'
PROBE_O = ROOT / 'scratch' / 'gemini_round9' / 'probe.o'

cpp_code = """
#include <types.h>
#include <game/bases/d_a_en_blockmain.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>

class dPanelObjList_c {
public:
    dPanelObjList_c();
    ~dPanelObjList_c();
    u32 m_00;
    u32 m_04;
    u16 m_08;
    u8 m_0a;
    u8 m_0b;
    f32 m_0c;
    f32 m_10;
    f32 m_14;
    f32 m_18;
    u16 m_1c;
    u8 m_1e;
    u8 m_1f;
};

class daEnCoinMain_c : public daEnBlockMain_c {
public:
    virtual ~daEnCoinMain_c();
    virtual void setWaterSpeed();
    virtual void beginFunsui();
    virtual void endFunsui();

    dPanelObjList_c mPanelObjList; // 0x698
    u8 pad_6B8[0x728 - 0x6B8];
    u32 m_728;
    u32 m_72c;
    u32 m_730;
    u32 m_734;
    u32 m_738;
    u32 m_73c;
    u32 m_740;
    u16 m_744;
    u16 m_746;
    u16 m_748;
    u8 m_74a;
    u8 pad_74b;
    dHeapAllocator_c mAllocator; // 0x74C
    m3d::bmdl_c *mpBmdl; // 0x768
    m3d::mdl_c mMdl1; // 0x76C
    m3d::mdl_c mMdl2; // 0x7AC
    m3d::mdl_c mMdl3; // 0x7EC
    m3d::anmChr_c mAnmChr1; // 0x82C
    m3d::anmChr_c mAnmChr2; // 0x864
    m3d::anmTexSrt_c mTexSrt; // 0x89C
};

template<size_t S> struct Sizer;
Sizer<sizeof(m3d::anmTexSrt_c)> s_texsrt;
Sizer<sizeof(daEnCoinMain_c)> s_coinmain;
"""

PROBE_CPP.write_text(cpp_code)

cmd = [
    str(MWCC), '-c', '-proc', 'gekko', '-fp', 'hard', '-O4', '-inline', 'noauto',
    '-Cpp_exceptions', 'off', '-enum', 'int', '-RTTI', 'off', '-ipa', 'file',
    '-enc', 'SJIS', '-DREVOLUTION', '-I-',
    '-i', 'include', '-i', 'include/lib', '-i', 'include/lib/MSL', '-i', 'include/lib/MSL/internal',
    '-i', 'include/lib/revolution/BTE/include', '-i', 'include/lib/revolution/BTE/stack/include',
    '-i', 'include/lib/revolution/BTE/stack/btm', '-i', 'include/lib/revolution/BTE/bta/include',
    '-i', 'include/lib/revolution/BTE/bta/sys', '-i', 'include/lib/revolution/BTE/gki/common',
    '-i', 'include/lib/revolution/BTE/gki/platform',
    str(PROBE_CPP), '-o', str(PROBE_O)
]

p = subprocess.run(cmd, cwd=str(ROOT), capture_output=True, text=True)
print("STDOUT:", p.stdout)
