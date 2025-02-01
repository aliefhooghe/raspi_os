

table = {
    'FIR stack': (0x00000000, 0x00001000),
    'IRQ stack': (0x00001000, 0x00002000),
    'SVC stack': (0x00002000, 0x00003000),
    'KERNEL'   : (0x00008000, 0x00010000),
    'HEAP'     : (0x00080000, 0x00800000)
}

# total mem
def colorize(code: int, input: str):
    return f'\033[{30 + code%6};1m{input}\033[0m'

def sizeof_fmt(num, suffix="B"):
    for unit in ("", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi"):
        if abs(num) < 1024.0:
            return f"{num:3.1f}{unit}{suffix}"
        num /= 1024.0
    return f"{num:.1f}Yi{suffix}"

def ll(step: int):
    start = 0x0

    size = max([i[1] for i in table.values()])

    line_pos = 0
    line_size = 64
    print("sizes:")
    idx = 1

    for name, (a, b) in table.items():
        sz = sizeof_fmt(b-a)
        name = colorize(idx, f'{name:12}')
        print(f"{name} | {b-a:08x} = {sz:8}| {a:08x} -> {b:08x}")
        idx = idx + 1
    sz = sizeof_fmt(size)
    print(f"\nTOTAL        | {sz:8}")

    print()

    for offset in range(start, size, step):
        mem_pos = int(offset + step/2)
        color = 0
        idx = 1
        for name, interval in table.items():
            if mem_pos > interval[0] and mem_pos < interval[1]:
                color = idx
                break
            idx = idx + 1

        print(colorize(color, '█'), end='')

        line_pos = line_pos + 1
        if line_pos == line_size:
            print()
            line_pos = 0




ll(step=0x1000)
# print(sizeof_fmt(0x8000))
