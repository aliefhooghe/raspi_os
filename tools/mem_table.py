

def colorize(name: str, msg: str):
    COLOR_BASE = 0x10
    COLOR_COUNT = 216
    # hash = int(hashlib.sha1(name.encode('utf-8')).hexdigest(), 16)
    color = COLOR_BASE + (hash(name) % COLOR_COUNT)
    return f'\033[38;5;{color}m{msg}\033[0m'

table = {
    'FIR stack'    : (0x00000000, 0x00001000),
    'IRQ stack'    : (0x00001000, 0x00002000),
    'SVC stack'    : (0x00002000, 0x00003000),
    'KERNEL'       : (0x00008000, 0x00020000),
    'KERNEL HEAP'  : (0x00100000, 0x00800000),

    'DYN SECTIONS' : (0x00800000, 0x04800000)
}

def sizeof_fmt(num: float, suffix: str ="B") -> str:
    for unit in ("", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi"):
        if abs(num) < 1024.0:
            return f"{num:3.1f}{unit}{suffix}"
        num /= 1024.0
    return f"{num:.1f}Yi{suffix}"

def ll(step: int, line_size: int):
    start = 0x0

    size = max([i[1] for i in table.values()])

    line_pos = 0



    print("+-----------------+--------------------+----------------------+")
    print(f"\033[1m| section         | size               | position             |\033[0m")
    print("+-----------------+--------------------+----------------------+")
    for name, (a, b) in table.items():
        sz = sizeof_fmt(b-a)
        name = colorize(name, f'◘ {name:13}')
        print(f"| {name} | {b-a:08x} = {sz:8}| {a:08x} -> {b:08x} |")
    sz = sizeof_fmt(size)
    print("+-----------------+--------------------+----------------------+")
    print(f"| TOTAL           | {sz:8}           |")
    print("+-----------------+--------------------+")

    print()
    print('cell size:', sizeof_fmt(step))
    print('line size:', sizeof_fmt(line_size * step))
    print()

    print(' ', sizeof_fmt(4 * step))
    print('|---' * int(line_size / 4))

    for offset in range(start, size, step):
        mem_pos = int(offset + step/2)
        curent_name = None
        for name, interval in table.items():
            if mem_pos > interval[0] and mem_pos < interval[1]:
                curent_name = name
                break

        if curent_name is None:
            print('░', end='')
        else:
            print(colorize(curent_name, '◘'), end='')

        line_pos = line_pos + 1
        if line_pos == line_size:
            print(' ', sizeof_fmt(offset + step))
            line_pos = 0




ll(step=0x1000, line_size=256)
# print(sizeof_fmt(0x8000))
