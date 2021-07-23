# GPIO controller drivers test

After running `make` command, there will be three modules:

* fake-gpio-chip.ko
* fake-gpio-ins.ko

The first module is a fake gpiochip driver, for people not having such MCP chip.
The goal of the second module is just to create a platform device that matches
our fake-gpio-chip module.

## Fake gpiochip driver

Prior to testing the fake gpio driver, one should load the following modules:

```bash
# insmod fake-gpio-chip.ko
# insmod fake-gpio-ins.ko
```

For testing purpose, one can list available gpiochip on the system

```bash
# ls -l /sys/class/gpio/gpiochip*
[...]
lrwxrwxrwx    1 root     root             0 Aug 13 17:30 /sys/class/gpio/gpiochip448 -> ../../devices/platform/fake-gpio-chip.0/gpio/gpiochip448
[...]
```

Additionally, `udevadm` may print more informations:

```bash
# udevadm info /sys/class/gpio/gpiochip448
P: /devices/platform/fake-gpio-chip.0/gpio/gpiochip448
E: DEVPATH=/devices/platform/fake-gpio-chip.0/gpio/gpiochip448
E: SUBSYSTEM=gpio
```

Parameters defined in the drivers may be checked as below:

```bash
# cat /sys/class/gpio/gpiochip496/
base       device/    label      ngpio      power/     subsystem/ uevent
# cat /sys/class/gpio/gpiochip496/ngpio 
16
# cat /sys/class/gpio/gpiochip496/base 
448
# cat /sys/class/gpio/gpiochip496/label 
fake-gpio-chip
```

Feel free to improve this driver, adding for example debug message when one
exports GPIOs, change their directions or their values.


