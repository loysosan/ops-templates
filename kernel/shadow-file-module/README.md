# Shadow File Kernel Module

This is a Linux Kernel Module (LKM) designed to hide files and directories from being listed. It intercepts the `getdents64` syscall using `ftrace` to filter out any directory entry whose name contains a predefined "magic word".

## Description

The primary purpose of this module is to serve as an educational example of syscall hooking in the Linux kernel. When loaded, any file or directory with `"magicword"` in its name will not appear in directory listings (e.g., from the `ls` command).

## How It Works

1.  **Syscall Hooking with ftrace**: The module uses `ftrace`, a powerful kernel tracing framework, to intercept calls to the `getdents64` syscall. This is the syscall responsible for reading directory entries.

2.  **Dynamic Symbol Resolution**: To find the address of the syscall handler (`__x64_sys_getdents64`), the module temporarily registers a `kprobe`. This is a modern and robust way to find kernel symbol addresses, especially since `kallsyms_lookup_name` is no longer exported for modules in recent kernels.

3.  **Filtering Logic**:
    *   When a hooked `getdents64` call occurs, the original syscall is executed first.
    *   The results (the directory entries) are copied from user space into a kernel buffer.
    *   The module iterates through the entries in the kernel buffer.
    *   If an entry's name contains the `MAGIC_WORD`, it is removed from the buffer by modifying the `d_reclen` (record length) of the *previous* entry to make it span over the entry being hidden. This effectively makes the kernel "skip" the hidden entry when reading the directory structure.
    *   The modified, clean buffer is then copied back to user space.

## How to Build and Use

### Prerequisites

You must have the Linux kernel headers installed for your currently running kernel.
*   On Debian/Ubuntu: `sudo apt-get install linux-headers-$(uname -r)`
*   On Fedora/CentOS: `sudo dnf install kernel-devel`

### Compilation

Navigate to the module's source directory and run `make`:
```bash
make
```
This will produce the kernel object file `main.ko`.

### Usage

1.  **Load the module**:
    ```bash
    sudo insmod main.ko
    ```

2.  **Test it**:
    Create a file or directory with the magic word in its name.
    ```bash
    touch a_test_file.txt
    touch a_magicword_file.txt
    ls -la
    ```
    The `a_magicword_file.txt` should not be visible in the output of `ls`. You can check the kernel log for hiding messages: `dmesg | grep "Hiding file"`.

3.  **Unload the module**:
    ```bash
    sudo rmmod main
    ```
    After unloading, the hidden files will become visible again.

## Disclaimer

**Warning:** This code runs in kernel space. Bugs can cause kernel panics and system instability. Use this for educational purposes only, preferably on a virtual machine.