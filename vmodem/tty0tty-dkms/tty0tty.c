#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/version.h>

#define TTY0TTY_MINORS 2

struct tty0tty {
    struct tty_port port;
};

static struct tty_driver *tty0tty_driver;
static struct tty0tty tty0tty_ports[TTY0TTY_MINORS];

static int tty0tty_open(struct tty_struct *tty, struct file *file)
{
    struct tty0tty *t = &tty0tty_ports[tty->index];
    return tty_port_open(&t->port, tty, file);
}

static void tty0tty_close(struct tty_struct *tty, struct file *file)
{
    struct tty0tty *t = &tty0tty_ports[tty->index];
    tty_port_close(&t->port, tty, file);
}

static int tty0tty_write(struct tty_struct *tty, const unsigned char *buf, int count)
{
    struct tty0tty *t = &tty0tty_ports[tty->index ^ 1];
    tty_insert_flip_string(&t->port, buf, count);
    tty_flip_buffer_push(&t->port);
    return count;
}

static unsigned int tty0tty_write_room(struct tty_struct *tty)
{
    return 65536;
}

static const struct tty_operations tty0tty_ops = {
    .open = tty0tty_open,
    .close = tty0tty_close,
    .write = tty0tty_write,
    .write_room = tty0tty_write_room,
};

static int __init tty0tty_init(void)
{
    int i, ret;

    tty0tty_driver = tty_alloc_driver(TTY0TTY_MINORS,
            TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV);
    if (IS_ERR(tty0tty_driver))
        return PTR_ERR(tty0tty_driver);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,14,0)
    tty0tty_driver->owner = THIS_MODULE;
#endif
    tty0tty_driver->driver_name = "tty0tty";
    tty0tty_driver->name = "tnt";
    tty0tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
    tty0tty_driver->init_termios = tty_std_termios;
    tty0tty_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
    tty_set_operations(tty0tty_driver, &tty0tty_ops);

    for (i = 0; i < TTY0TTY_MINORS; i++)
        tty_port_init(&tty0tty_ports[i].port);

    ret = tty_register_driver(tty0tty_driver);
    if (ret) {
        for (i = 0; i < TTY0TTY_MINORS; i++)
            tty_port_destroy(&tty0tty_ports[i].port);
        put_tty_driver(tty0tty_driver);
        return ret;
    }

    for (i = 0; i < TTY0TTY_MINORS; i++)
        tty_port_link_device(&tty0tty_ports[i].port, tty0tty_driver, i);

    return 0;
}

static void __exit tty0tty_exit(void)
{
    int i;

    for (i = 0; i < TTY0TTY_MINORS; i++) {
        tty_port_unlink_device(&tty0tty_ports[i].port, tty0tty_driver, i);
        tty_port_destroy(&tty0tty_ports[i].port);
    }

    tty_unregister_driver(tty0tty_driver);
    put_tty_driver(tty0tty_driver);
}

module_init(tty0tty_init);
module_exit(tty0tty_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MacintoshPi");
MODULE_DESCRIPTION("Virtual serial loopback driver");
