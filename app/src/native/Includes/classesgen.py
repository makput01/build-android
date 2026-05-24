with open("classes.dex", "rb") as f:
    data = f.read()

with open("classesdex.h", "w") as f:
    f.write("#include <cstddef>\n#include <cstdint>\n")
    f.write("inline const uint8_t com_id9909_dex[] = {")
    for i, b in enumerate(data):
        b ^= i * 10101011011
        b &= 0xFF
        if i % 12 == 0:
            f.write("\n\t")
        f.write(f"0x{b:02x}, ")
    f.write("\n};\n")
    f.write(f"inline const size_t com_id9909_dex_len = sizeof(com_id9909_dex);\n")
