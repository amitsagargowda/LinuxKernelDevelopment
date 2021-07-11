#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/module.h>
#include <linux/version.h>

/* Module information */
MODULE_AUTHOR( "Test" );
MODULE_DESCRIPTION( "test serial driver" );
MODULE_LICENSE("GPL");

#define DELAY_TIME		HZ * 2	/* 2 seconds per character */
#define TEST_DATA_CHARACTER	't'

#define TEST_SERIAL_MAJOR	240	/* experimental range */
#define TEST_SERIAL_MINORS	1	/* only have one minor */
#define UART_NR			1	/* only use one port */

#define TEST_SERIAL_NAME	"ttytest"

#define MY_NAME			TEST_SERIAL_NAME

static struct timer_list *timer;

static void test_stop_tx(struct uart_port *port)
{
}

static void test_stop_rx(struct uart_port *port)
{
}

static void test_enable_ms(struct uart_port *port)
{
}

static void test_tx_chars(struct uart_port *port)
{
	struct circ_buf *xmit = &port->state->xmit;
	int count;

	if (port->x_char) {
		pr_debug("wrote %2x", port->x_char);
		port->icount.tx++;
		port->x_char = 0;
		return;
	}
	if (uart_circ_empty(xmit) || uart_tx_stopped(port)) {
		test_stop_tx(port);
		return;
	}

	count = port->fifosize >> 1;
	do {
		pr_debug("wrote %2x", xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;
		if (uart_circ_empty(xmit))
			break;
	} while (--count > 0);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	if (uart_circ_empty(xmit))
		test_stop_tx(port);
}

static void test_start_tx(struct uart_port *port)
{
}

static void test_timer(unsigned long data)
{
	struct uart_port *port;
	struct tty_struct *tty;
	struct tty_port *tty_port;


	port = (struct uart_port *)data;
	if (!port)
		return;
	if (!port->state)
		return;
	tty = port->state->port.tty;
	if (!tty)
		return;

	tty_port = tty->port;

	/* add one character to the tty port */
	/* this doesn't actually push the data through unless tty->low_latency is set */
	tty_insert_flip_char(tty_port, TEST_DATA_CHARACTER, 0);

	tty_flip_buffer_push(tty_port);

	/* resubmit the timer again */
	timer->expires = jiffies + DELAY_TIME;
	add_timer(timer);

	/* see if we have any data to transmit */
	test_tx_chars(port);
}

static unsigned int test_tx_empty(struct uart_port *port)
{
	return 0;
}

static unsigned int test_get_mctrl(struct uart_port *port)
{
	return 0;
}

static void test_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
}

static void test_break_ctl(struct uart_port *port, int break_state)
{
}

static void test_set_termios(struct uart_port *port,
			     struct ktermios *new, struct ktermios *old)
{
	int baud, quot, cflag = new->c_cflag;
	/* get the byte size */
	switch (cflag & CSIZE) {
	case CS5:
		printk(KERN_DEBUG " - data bits = 5\n");
		break;
	case CS6:
		printk(KERN_DEBUG " - data bits = 6\n");
		break;
	case CS7:
		printk(KERN_DEBUG " - data bits = 7\n");
		break;
	default: // CS8
		printk(KERN_DEBUG " - data bits = 8\n");
		break;
	}

	/* determine the parity */
	if (cflag & PARENB)
		if (cflag & PARODD)
			pr_debug(" - parity = odd\n");
		else
			pr_debug(" - parity = even\n");
	else
		pr_debug(" - parity = none\n");

	/* figure out the stop bits requested */
	if (cflag & CSTOPB)
		pr_debug(" - stop bits = 2\n");
	else
		pr_debug(" - stop bits = 1\n");

	/* figure out the flow control settings */
	if (cflag & CRTSCTS)
		pr_debug(" - RTS/CTS is enabled\n");
	else
		pr_debug(" - RTS/CTS is disabled\n");

	/* Set baud rate */
        baud = uart_get_baud_rate(port, new, old, 0, port->uartclk/16);
        quot = uart_get_divisor(port, baud);
	
	//UART_PUT_DIV_LO(port, (quot & 0xff));
	//UART_PUT_DIV_HI(port, ((quot & 0xf00) >> 8));
}

static int test_startup(struct uart_port *port)
{
	/* this is the first time this port is opened */
	/* do any hardware initialization needed here */

	/* create our timer and submit it */
	if (!timer) {
		timer = kmalloc(sizeof(*timer), GFP_KERNEL);
		if (!timer)
			return -ENOMEM;
	}
    #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
        timer->data = (unsigned long)port;
        timer->function = test_timer;
    #else
        timer_setup(timer, (void *) test_timer, (unsigned long) port);
        timer->function = (void *) test_timer;
    #endif
	timer->expires = jiffies + DELAY_TIME;
	add_timer(timer);
	return 0;
}

static void test_shutdown(struct uart_port *port)
{
	/* The port is being closed by the last user. */
	/* Do any hardware specific stuff here */

	/* shut down our timer */
	del_timer(timer);
}

static const char *test_type(struct uart_port *port)
{
	return "testtty";
}

static void test_release_port(struct uart_port *port)
{

}

static int test_request_port(struct uart_port *port)
{
	return 0;
}

static void test_config_port(struct uart_port *port, int flags)
{
}

static int test_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	return 0;
}

static struct uart_ops test_ops = {
	.tx_empty	= test_tx_empty,
	.set_mctrl	= test_set_mctrl,
	.get_mctrl	= test_get_mctrl,
	.stop_tx	= test_stop_tx,
	.start_tx	= test_start_tx,
	.stop_rx	= test_stop_rx,
	.enable_ms	= test_enable_ms,
	.break_ctl	= test_break_ctl,
	.startup	= test_startup,
	.shutdown	= test_shutdown,
	.set_termios	= test_set_termios,
	.type		= test_type,
	.release_port	= test_release_port,
	.request_port	= test_request_port,
	.config_port	= test_config_port,
	.verify_port	= test_verify_port,
};

static struct uart_port test_port = {
	.ops		= &test_ops,
};

static struct uart_driver test_reg = {
	.owner		= THIS_MODULE,
	.driver_name	= TEST_SERIAL_NAME,
	.dev_name	= TEST_SERIAL_NAME,
	.major		= TEST_SERIAL_MAJOR,
	.minor		= TEST_SERIAL_MINORS,
	.nr		= UART_NR,
};

static int __init test_init(void)
{
	int result;

	printk(KERN_INFO "test serial driver loaded\n");

	result = uart_register_driver(&test_reg);
	if (result)
		return result;

	result = uart_add_one_port(&test_reg, &test_port);
	if (result)
		uart_unregister_driver(&test_reg);

	return result;
}

module_init(test_init);
