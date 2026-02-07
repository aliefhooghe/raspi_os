


def istr(x: int):
    return f"0x{x:08X}u"

while True:
    try:
        line = input()
        if line in [ 'quit', 'exit', 'q']:
            break
        if ':' in line:
            start, end = map(int, line.split(':'))

            if start > end:
                start, end = end, start

            n = 0
            for i in range(start, end + 1):
                n |= (1 << i)
            print(istr(n))
        else:
            n = int(line)
            print(istr(1 << n))
    except:
        pass
