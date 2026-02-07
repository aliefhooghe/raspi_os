import serial
import argparse
import time

class SerialBootProtocol:
    # while not init
    #   => INIT (1 bytes)
    #   <= INIT_ACK (1bytes)
    #
    # => SIZE (4 bytes)
    # for byte in bytes:
    #    => byte
    #    <= byte
    # <= END (1 bytes)

    INIT = 0xFA
    INIT_ACK = 0xFB
    END = 0xFC
    END_ACK = 0xFD

def serial_send(port: serial.Serial, data: bytes):
    datasize = len(data)

    print('⏳ wait for device ...')
    while True:

        init_ack = port.read(1)

        if len(init_ack) == 0:
            # reached timeout
            port.write(bytes([SerialBootProtocol.INIT]))
        elif init_ack == bytes([SerialBootProtocol.INIT_ACK]):
            print('✅ transfert initiated')
            break
        elif init_ack == bytes([SerialBootProtocol.INIT]):
            port.write(bytes([SerialBootProtocol.INIT]))


    print(f'🚀 sending {datasize} bytes...')
    port.write(datasize.to_bytes(4, byteorder='little'))

    start_time = time.time()
    bytes_per_sec = 0.0
    remaining = ""

    for idx, byte in enumerate(data):
        # Compute stats
        progression = int(80.0 * float(idx)/float(datasize))

        print(
            f'\r⏳ \033[31;1m' + progression * '█',
            end=' ' * (80 - progression) + f'\033[0m {idx+1}/{datasize} - {bytes_per_sec} b/s. {remaining} s',
            flush=True)

        port.write(bytes([byte]))
        ack = port.read(1)

        if ack != bytes([byte]):
            print(f'\n❌ fatal ack error {ack} instead of {bytes([byte])}')
            raise Exception('ack error')

        total_sent = idx + 1
        time_spent = time.time() - start_time
        bytes_per_sec = int(total_sent / time_spent)
        remaining_sec = (datasize - idx) / bytes_per_sec
        remaining = f"remaining: {remaining_sec // 60}m {int(remaining_sec % 60)}s       "


    print(f'\n💾 {datasize} where sent. Wait for final ack...')
    transfert_ack = port.read(1)
    if transfert_ack == bytes([SerialBootProtocol.END]):
        print(f'🎉 received final ack !')
        port.write(bytes([SerialBootProtocol.END_ACK]))
        port.flush()
        print(f'✅ data transfert was succesfull.')
    else:
        print(f'❌ final ack receipt failure' + str(transfert_ack))


def main():

    parser = argparse.ArgumentParser(
        description='Serial tool.'
    )
    parser.add_argument(
        '--device',
        type=str,
        required=True,
        help="Serial device."
    )
    parser.add_argument(
        '--data',
        type=str,
        required=True,
        help="File to sent."
    )
    args = parser.parse_args()

    print(f'👹 Satan Serial Loader 👹')
    port = serial.Serial(
        port=args.device,
        baudrate=115200,
        timeout=0.5)
    with open(args.data, 'rb') as file:
        file_content = file.read()
        serial_send(port, data=file_content)

if __name__ == '__main__':
    main()
