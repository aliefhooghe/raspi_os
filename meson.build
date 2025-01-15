project(
    'Satan-OS', 'c',
    meson_version: '>=1.4.0',
    default_options: ['warning_level=3', 'optimization=2']
)

fs = import('fs')


kernel_sources = files(
  'src/kernel/boot.S',
  'src/kernel/kernel.c'
)
kernel_linker_script = files(
  'src/kernel/linker.ld'
)

kernel_elf = executable(
  'kernel.elf',
  sources: kernel_sources,
  link_args: ['-T', kernel_linker_script[0].full_path()],
  install: false
)

kernel_img = custom_target(
  'kernel.img',
  input: kernel_elf,
  output: 'kernel.img',
  command: ['llvm-objcopy', '@INPUT@', '-O', 'binary', '@OUTPUT@'],
  build_by_default: true
)
