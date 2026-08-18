import capstone

with open('scratch/gemini_round7/sinit.bin', 'rb') as f:
    code = f.read()

cs = capstone.Cs(capstone.CS_ARCH_PPC, capstone.CS_MODE_32 | capstone.CS_MODE_BIG_ENDIAN)

print("Disassembly of __sinit_\\m_pad_cpp (0x8016F7B0):")
for ins in cs.disasm(code, 0x8016F7B0):
    print(f"/* {ins.address:08X} */  {ins.bytes.hex(' '):12}  {ins.mnemonic:8} {ins.op_str}")
