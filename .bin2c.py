import sys

name = sys.argv[1]

print(f"""
#include <cstddef>

const char {name}[] = {{""")
read_bytes = 0
while True:
    data = sys.stdin.buffer.read(16)
    read_bytes += len(data)
    if len(data):
        line = ", ".join([f"0x{b:02x}" for b in data])
        print(f"    {line},")
    if len(data) < 16:
        break

print(f"""}};
const size_t {name}_size = {read_bytes};""")
