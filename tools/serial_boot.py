import serial
import argparse


class SerialBootProtocol:
    # => MAGIC (1 bytes)
    # => SIZE (4 bytes)
    # for byte in bytes:
    #    => byte
    #    <= byte
    # <= MAGIC (1 bytes)
    MAGIC = 0x42

def serial_send(port: serial.Serial, data: bytes):
    datasize = len(data)

    print('📡 initiate serial transfert.')
    port.write(bytes([SerialBootProtocol.MAGIC]))

    print(f'🚀 sending {datasize} bytes...')
    port.write(datasize.to_bytes(4, byteorder='little'))

    for idx, byte in enumerate(data):
        progression = int(100.0 * float(idx)/float(datasize))

        print('\r⏳ [' + progression * '=', end=f'] ({idx+1}/{datasize})', )

        port.write(bytes([byte]))
        ack = port.read(1)

        if ack != bytes([byte]):
            print(f'\n❌ fatal ack error {ack} instead of {bytes([byte])}')
            raise Exception('ack error')

    print(f'\n💾 {datasize} where sent. Wait for final ack...')
    transfert_ack = port.read(1)
    if transfert_ack == bytes([SerialBootProtocol.MAGIC]):
        print(f'🎉 received final ack !')
        print(f'✅ data transfert was succesfull.')
    else:
        print(f'❌ final ack receipt failure')



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
    port = serial.Serial(port=args.device, baudrate=115200)
    with open(args.data, 'rb') as file:
        file_content = file.read()
        serial_send(port, data=file_content)

if __name__ == '__main__':
    main()