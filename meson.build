project(
    'Satan-OS', 'c',
    meson_version: '>=1.4.0',
    default_options: ['warning_level=3', 'optimization=2']
)

fs = import('fs')

# Common sources

common_sources = files(
  'src/common/hardware/cpu.h',
  'src/common/hardware/cpu.S',
  'src/common/hardware/gpio.h',
  'src/common/hardware/mini_uart.c',
  'src/common/hardware/mini_uart.h',
  'src/common/hardware/mmio.h',
  'src/common/hardware/mmio.S'
)
common_include_dir = include_directories(
  'src/common/'
)

# Serial Loader

serial_loader_sources = files(
  'src/serial_loader/boot.S',
  'src/serial_loader/loader.c'
)
serial_loader_linker_script = files(
  'src/serial_loader/linker.ld'
)

serial_loader_elf = executable(
  'serial_loader.elf',
  sources: serial_loader_sources + common_sources,
  link_args: ['-T', serial_loader_linker_script[0].full_path()],
  install: false,
  include_directories: common_include_dir
)

serial_loader_img = custom_target(
  'serial_loader.img',
  input: serial_loader_elf,
  output: 'serial_loader.img',
  command: ['llvm-objcopy', '@INPUT@', '-O', 'binary', '@OUTPUT@'],
  build_by_default: true
)

# Kernel

kernel_sources = files(
  'src/kernel/boot.S',
  'src/kernel/kernel.c'
)
kernel_linker_script = files(
  'src/kernel/linker.ld'
)

kernel_elf = executable(
  'kernel.elf',
  sources: kernel_sources + common_sources,
  link_args: ['-T', kernel_linker_script[0].full_path()],
  install: false,
  include_directories: common_include_dir
)

kernel_img = custom_target(
  'kernel.img',
  input: kernel_elf,
  output: 'kernel.img',
  command: ['llvm-objcopy', '@INPUT@', '-O', 'binary', '@OUTPUT@'],
  build_by_default: true
)
