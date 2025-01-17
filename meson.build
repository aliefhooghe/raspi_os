project(
    'Satan-OS', 'c',
    meson_version: '>=1.4.0',
    default_options: ['warning_level=3', 'optimization=2']
)


# Serial Loader

serial_loader_sources = files(
  'src/serial_loader/boot.S',
  'src/serial_loader/loader.c',
  'src/serial_loader/lib.h',
  'src/serial_loader/lib.c',
  'src/serial_loader/lib.S',
)
serial_loader_linker_script = files(
  'src/serial_loader/linker.ld'
)

serial_loader_elf = executable(
  'serial_loader.elf',
  sources: serial_loader_sources,
  link_args: ['-T', serial_loader_linker_script[0].full_path()],
  install: false,
  include_directories: include_directories('src/serial_loader')
)

custom_target(
  'serial_loader.img',
  input: serial_loader_elf,
  output: 'serial_loader.img',
  command: ['llvm-objcopy', '@INPUT@', '-O', 'binary', '@OUTPUT@'],
  build_by_default: true
)

# Kernel

kernel_sources = files(
  'src/kernel/boot.S',
  'src/kernel/kernel.c',
  'src/kernel/hardware/cpu.h',
  'src/kernel/hardware/cpu.S',
  'src/kernel/hardware/gpio.h',
  'src/kernel/hardware/mini_uart.c',
  'src/kernel/hardware/mini_uart.h',
  'src/kernel/hardware/mmio.h',
  'src/kernel/hardware/mmio.S',
  'src/kernel/hardware/watchdog.c',
  'src/kernel/hardware/watchdog.h'
)
kernel_linker_script = files(
  'src/kernel/linker.ld'
)

kernel_elf = executable(
  'kernel.elf',
  sources: kernel_sources,
  link_args: ['-T', kernel_linker_script[0].full_path()],
  install: false,
  include_directories: include_directories('src/kernel')
)

custom_target(
  'kernel.img',
  input: kernel_elf,
  output: 'kernel.img',
  command: ['llvm-objcopy', '@INPUT@', '-O', 'binary', '@OUTPUT@'],
  build_by_default: true
)
