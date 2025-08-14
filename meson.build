project(
    'Satan-OS', 'c',
    version: '0.1',
    meson_version: '>=1.4.0',
    default_options: ['warning_level=3', 'optimization=2']
)


xxd = find_program('xxd')


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

# Userland
userland_lib_sources = files(
  'src/userland/bin/_start.c',
  'src/userland/lib/stdio.c',
  'src/userland/lib/stdlib.c',
  'src/userland/lib/string.c',
  'src/userland/lib/usr_syscalls.c',
  'src/userland/lib/usr_syscalls.S'
)

userland_include_directories = include_directories(
  'src/userland/include'
)

userland_program_link_script = files(
  'src/userland/bin/linker.ld'
)

hello_elf = executable(
  'hello.elf',
  sources: files('src/userland/bin/hello.c') + userland_lib_sources,
  link_args: ['-T', userland_program_link_script[0].full_path()],
  install: false,
  include_directories: userland_include_directories
)

# Kernel

# resources
hello_payload_c = custom_target(
  'hello_payload_c',
  input: hello_elf,
  output: 'hello_elf_payload.c',
  command: [xxd, '-i', '@INPUT@', '@OUTPUT@'] 
)

# sources
kernel_sources = files(
  # MUST BE THE FIRST: TODO edit the linker script to ensure this is the start
  'src/kernel/hardware/boot.S',

  'src/kernel/banner.c',
  'src/kernel/utils.h',
  'src/kernel/utils.c',
  'src/kernel/elf_loader/elf_format.h',
  'src/kernel/elf_loader/elf_loader.h',
  'src/kernel/elf_loader/elf_loader.c',
  'src/kernel/hardware/cpu.h',
  'src/kernel/hardware/cpu.S',
  'src/kernel/hardware/gpio.h',
  'src/kernel/hardware/interupts.c',
  'src/kernel/hardware/interupts.h',
  'src/kernel/hardware/interupts.S',
  'src/kernel/hardware/io_registers.h',
  'src/kernel/hardware/mini_uart.c',
  'src/kernel/hardware/mini_uart.h',
  'src/kernel/hardware/mmio.h',
  'src/kernel/hardware/mmio.S',
  'src/kernel/hardware/mmu.c',
  'src/kernel/hardware/mmu.h',
  'src/kernel/hardware/mmu.S',
  'src/kernel/hardware/watchdog.c',
  'src/kernel/hardware/watchdog.h',
  'src/kernel/kernel.c',
  'src/kernel/kernel.h',
  'src/kernel/kernel_types.h',
  'src/kernel/lib/eabi_rt.c',
  'src/kernel/lib/str.c',
  'src/kernel/lib/str.h',
  'src/kernel/memory/bitfield.h',
  'src/kernel/memory/bitfield.c',
  'src/kernel/memory/section_allocator.h',
  'src/kernel/memory/section_allocator.c',
  'src/kernel/memory/memory_allocator.h',
  'src/kernel/memory/memory_allocator.c',
  'src/kernel/memory/translation_table_allocator.h',
  'src/kernel/memory/translation_table_allocator.c',
  'src/kernel/scheduler/scheduler.c',
  'src/kernel/scheduler/scheduler.h',
  'src/kernel/scheduler/scheduler.S',
  'src/kernel/scheduler/task_context.h',
  'src/kernel/syscalls.c',
  'src/kernel/syscalls.h',
  'src/kernel/usermode/usermode.c',
  'src/kernel/usermode/usermode.h',
  'src/kernel/usermode/usr_syscalls.c',
  'src/kernel/usermode/usr_syscalls.h',
  'src/kernel/usermode/usr_syscalls.S',
  'src/kernel/usermode/libc/dirent.c',
  'src/kernel/usermode/libc/dirent.h',
  'src/kernel/usermode/libc/stdio.c',
  'src/kernel/usermode/libc/stdio.h',
  'src/kernel/usermode/libc/stdlib.c',
  'src/kernel/usermode/libc/stdlib.h',
  'src/kernel/usermode/libc/string.c',
  'src/kernel/usermode/libc/string.h',
  'src/kernel/vfs/vfs.h',
  'src/kernel/vfs/vfs.c',
  'src/kernel/vfs/dentry.h',
  'src/kernel/vfs/device_ops.h',
  'src/kernel/vfs/inode.h',
  'src/kernel/vfs/super_block.h',
  'src/kernel/vfs/driver_registry.h',
  'src/kernel/vfs/driver_registry.c',
  'src/kernel/vfs/dev/tty.h',
  'src/kernel/vfs/dev/tty.c',
  'src/kernel/vfs/dev/zero.h',
  'src/kernel/vfs/dev/zero.c',
  'src/kernel/vfs/filesystems/ramfs.h',
  'src/kernel/vfs/filesystems/ramfs.c',
)
kernel_linker_script = files(
  'src/kernel/linker.ld'
)

kernel_elf = executable(
  'kernel.elf',
  sources: kernel_sources + hello_payload_c,
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
