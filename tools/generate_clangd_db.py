
import os
import json
import argparse

def main():

    parser = argparse.ArgumentParser(
        description='Generate json commands file from makefile.'
    )
    parser.add_argument(
        '--makefile',
        type=str,
        required=True,
        help="Path to the Makefile to be parsed."
    )
    parser.add_argument(
        "--commands",
        type=str,
        required=True,
        help="Path to the output commands file."
    )
    args = parser.parse_args()
    commands = []

    c_ext = '.c'
    o_ext = '.o'

    with open(args.makefile, 'r') as makefile:
        for line in makefile.readlines():
            line = line.strip()

            if line.startswith('clang') and c_ext in line and o_ext in line:
                directory = os.getcwd()
                file = None
                output = None

                for token in line.split(' '):
                    if token.endswith(c_ext):
                        file = token
                    elif token.endswith(o_ext):
                        output = token

                commands.append(
                    {
                        'directory': "/home/aliefhooghe/programmes/raspi_os",
                        'command': line,
                        'file': file,
                        'output': output
                    }
                )

    with open(args.commands, 'w+') as commands_file:
        json.dump(commands, commands_file, indent=4)

if __name__ == '__main__':
    main()
