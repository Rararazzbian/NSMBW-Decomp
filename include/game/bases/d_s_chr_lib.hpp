#pragma once
#include <game/mLib/m_3d.hpp>

/// @unofficial PROPOSED new header (not landed). `dsChrLib::bindAnimToNode` is
/// used twice by daWmKinokoBase_c::createModel(), binding each of the two
/// animation objects to a single named node ("kinoko" and "trunk", read
/// directly out of `original/d_basesNP.rel`'s .data at 0x458B0+0x78/+0x80).
/// The mangled target symbol is
/// `bindAnimToNode__8dsChrLibFPQ23m3d6bmdl_cPQ23m3d8anmChr_cPCcQ44nw4r3g3d9AnmObjChr10BindOption`,
/// also found (already implemented, not just declared) in the DOL's own
/// symbol table at 0x800DFA80 (`bin/dtk/wiimj2d_symbols.txt`), so this class
/// is real and shared between the DOL and this REL -- only its header is
/// missing from this project.
class dsChrLib {
public:
    static void bindAnimToNode(m3d::bmdl_c *mdl, m3d::anmChr_c *anmChr, const char *nodeName,
                                nw4r::g3d::AnmObjChr::BindOption bindOption);
};
