project(
    'Satan-OS', 'c',
    version: '0.1',
    meson_version: '>=1.4.0',
    default_options: ['warning_level=3', 'optimization=2']
)

xxd = find_program('xxd')

#
# Serial Loader
#
serial_loader_sources = files(
  'src/serial_loader/boot.S',
  'src/serial_loader/loader.c',
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

#
# Userland
#
userland_lib_sources = files(
  'src/userland/bin/_start.c',
  'src/userland/lib/dirent.c',
  'src/userland/lib/stdio.c',
  'src/userland/lib/stdlib.c',
  'src/userland/lib/string.c',
  'src/userland/lib/usr_syscalls.c',
  'src/userland/lib/usr_syscalls.S',

  'src/arm_runtime/eabi_rt.c',
)
userland_include_directories = include_directories(
  'src/userland/include'
)
userland_program_link_script = files(
  'src/userland/bin/linker.ld'
)

# try to reduce userland elf size as much as possible
userland_c_args = [
  # allows section garbage collection to be effective
  '-ffunction-sections',  # one section per function
  '-fdata-sections',      # one section per variable
]
userland_link_args = [
  '-T', userland_program_link_script[0].full_path(),
  '-Wl,--gc-sections,--strip-all', # garbage collect unused sections
  '-Wl,--nmagic'                   # turn off page alignment of sections 
]

userland_programs_src = [
  'src/userland/bin/init.c',
  'src/userland/bin/hello.c',
  'src/userland/bin/lucifer.c',
]
userland_programs_payloads = []
userland_programs = []

foreach program_src: userland_programs_src
  program_name = program_src.split('/')[-1].replace('.c', '')
  elf_target = executable(
    program_name + '.elf',
    sources: files(program_src) + userland_lib_sources,
    link_args: userland_link_args,
    c_args : userland_c_args,
    install: false,
    include_directories: userland_include_directories
  )
  userland_programs += elf_target
  elf_payload_c = custom_target(
    program_name + '_payload_c',
    input: elf_target,
    output: program_name + '_payload.c',
    command: [xxd, '-i', '@INPUT@', '@OUTPUT@'] 
  )
  userland_programs_payloads += elf_payload_c
endforeach

# Root fs: create a fat32 rootfs populated with programs
blank_img = custom_target(
  'blank_sd',
  depend_files: [meson.project_source_root() + '/tools/create_blank_fs.sh'],
  output: 'sd.blank.img',
  command: ['bash', meson.project_source_root() + '/tools/create_blank_fs.sh', '@OUTPUT@'],
)

custom_target(
  'rootfs',
  input: [blank_img] + userland_programs,
  depend_files: [meson.project_source_root() + '/tools/create_root_fs.sh'],
  output: 'sd.rootfs.img',
  command: ['bash', meson.project_source_root() + '/tools/create_root_fs.sh', '@OUTPUT@', '@INPUT@'],
  build_by_default: true
)

# Kernel

# sources
kernel_sources = files(
  # MUST BE THE FIRST: TODO edit the linker script to ensure this is the start
  'src/kernel/hardware/boot.S',

  'src/kernel/banner.c',
  'src/kernel/utils.c',
  'src/kernel/elf_loader/elf_loader.c',

  'src/kernel/hardware/cpu.S',
  'src/kernel/hardware/gpio.c',
  'src/kernel/hardware/interupts.c',
  'src/kernel/hardware/interupts.S',
  'src/kernel/hardware/mini_uart.c',
  'src/kernel/hardware/mmio.S',
  'src/kernel/hardware/mmu.c',
  'src/kernel/hardware/mmu.S',
  'src/kernel/hardware/sd_host/sd_host.c',
  'src/kernel/hardware/watchdog.c',

  'src/kernel/kernel.c',
  'src/kernel/lib/str.c',
  'src/kernel/lib/str.h',
  'src/kernel/lib/fnv_hash.c',
  'src/kernel/lib/hashmap.c',
  'src/kernel/memory/bitfield.c',
  'src/kernel/memory/section_allocator.c',
  'src/kernel/memory/memory_allocator.c',
  'src/kernel/memory/translation_table_allocator.c',
  'src/kernel/scheduler/scheduler.c',
  'src/kernel/scheduler/scheduler.S',
  'src/kernel/syscalls.c',
  'src/kernel/vfs/inode.c',
  'src/kernel/vfs/fs_utils.c',
  'src/kernel/vfs/vfs.c',
  'src/kernel/vfs/driver_registry.c',
  'src/kernel/vfs/dev/block_device_file_ops.c',
  'src/kernel/vfs/dev/sdcard.c',
  'src/kernel/vfs/dev/tty.c',
  'src/kernel/vfs/dev/ramdisk.c',
  'src/kernel/vfs/dev/zero.c',
  'src/kernel/vfs/filesystems/ramfs/ramfs.c',
  'src/kernel/vfs/filesystems/fat32/fat32_fs.c',

  'src/arm_runtime/eabi_rt.c',
)
kernel_linker_script = files(
  'src/kernel/linker.ld'
)

kernel_elf = executable(
  'kernel.elf',
  sources: kernel_sources + userland_programs_payloads,
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
