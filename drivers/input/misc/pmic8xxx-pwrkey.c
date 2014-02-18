/* Copyright (c) 2010-2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/log2.h>

#include <linux/mfd/pm8xxx/core.h>
#include <linux/input/pmic8xxx-pwrkey.h>

#ifdef CONFIG_SW_RESET
#include <mach/board-msm8660.h>
#include <linux/mfd/pm8xxx/core.h>
#endif

/* define to enable reboot on very long key hold */
#define VERY_LONG_HOLD_REBOOT

#ifdef VERY_LONG_HOLD_REBOOT
#include <mach/system.h>
#include <linux/mfd/pm8xxx/misc.h>
//#include <linux/mfd/pm8xxx/pm8921.h>
#endif

#ifdef CONFIG_PM_DEEPSLEEP
#include <linux/suspend.h>
#include <linux/wakelock.h>
#endif

#undef CONFIG_MACH_KTTECH

#ifdef CONFIG_MACH_KTTECH
#include <linux/wakelock.h>
struct work_struct kttech_pwrkey_work;
struct wake_lock kttech_pwrkey_wakelock;
#endif

#define PON_CNTL_1 0x1C
#define PON_CNTL_PULL_UP BIT(7)
#define PON_CNTL_TRIG_DELAY_MASK (0x7)

/**
 * struct pmic8xxx_pwrkey - pmic8xxx pwrkey information
 * @key_press_irq: key press irq number
 * @pdata: platform data
 */
struct pmic8xxx_pwrkey {
	struct input_dev *pwr;
	int key_press_irq;
	int key_release_irq;
	bool press;
	struct device *dev;
	const struct pm8xxx_pwrkey_platform_data *pdata;
#ifdef CONFIG_PM_DEEPSLEEP
	struct hrtimer longPress_timer;
	int expired;
	struct wake_lock wake_lock;
#endif
#ifdef VERY_LONG_HOLD_REBOOT
	struct hrtimer very_longPress_timer;
#endif
};

#ifdef CONFIG_PM_DEEPSLEEP
static enum hrtimer_restart longPress_timer_callback(struct hrtimer *timer)
{
	struct pmic8xxx_pwrkey *pwrkey  =
		container_of(timer, struct pmic8xxx_pwrkey, longPress_timer);

	pwrkey->expired = 1;
	input_report_key(pwrkey->pwr, KEY_POWER, 1);
	input_sync(pwrkey->pwr);

	return HRTIMER_NORESTART;

}
#endif

#ifdef VERY_LONG_HOLD_REBOOT
extern void pwroff_machine(void);
static enum hrtimer_restart very_longPress_timer_callback(struct hrtimer *timer)
{
	pr_info("Power key held down for 7 seconds; REBOOTING.");
	pwroff_machine();

	while (1) {
		pr_info("Power key held down for 7 seconds; waiting for REBOOT.");
	}

	return HRTIMER_NORESTART;
}
#endif

static void dump_reg(struct pmic8xxx_pwrkey *pwrkey, bool from)
{
	int press, release;

	pr_info("%s: from = %s\n", __func__, from ? "press" : "release");
	press = pm8xxx_read_irq_stat(pwrkey->dev->parent,
				     pwrkey->key_press_irq);
	release = pm8xxx_read_irq_stat(pwrkey->dev->parent,
				       pwrkey->key_release_irq);
	pr_info("%s: press RT = %d, release RT = %d\n", __func__,
		press, release);
}

// paiksun...
#ifdef CONFIG_SW_RESET
static struct platform_device	*pwrkey_pm_chip;
static int pwrkey_irq;
extern int pm8058_read_irq_stat(const struct device *dev, int irq);

int pm8058_get_pwrkey_status(void)
{
	return pm8xxx_read_irq_stat(pwrkey_pm_chip->dev.parent, pwrkey_irq);
}
#endif

#ifdef CONFIG_MACH_KTTECH
static void kttech_pwrkey_work_f(struct work_struct *work)
{
   wake_lock_timeout(&kttech_pwrkey_wakelock, 2*HZ);
   printk(KERN_CRIT "%s: Power Key Pressed\n", 	__func__ );
}
#endif

static irqreturn_t pwrkey_press_irq(int irq, void *_pwrkey)
{
	struct pmic8xxx_pwrkey *pwrkey = _pwrkey;

#ifdef VERY_LONG_HOLD_REBOOT
	struct timespec uptime;

	do_posix_clock_monotonic_gettime(&uptime);

	if (uptime.tv_sec > 50 && pwrkey->press == false) {
		hrtimer_start(&pwrkey->very_longPress_timer,
				ktime_set(7, 0), HRTIMER_MODE_REL);
	}
#endif

	pr_info("%s: Enter %s\n", __func__, __func__);
	dump_reg(pwrkey, true);

#ifdef CONFIG_PM_DEEPSLEEP

	if (get_deepsleep_mode() && pwrkey->press == false) {
		pwrkey->expired = 0;
		hrtimer_start(&pwrkey->longPress_timer,
					ktime_set(2, 0), HRTIMER_MODE_REL);
		wake_lock_timeout(&pwrkey->wake_lock, 2*HZ+5);
	} else
#endif
	{

	if (pwrkey->press == true) {
		//dump_reg(pwrkey, true);
		pwrkey->press = false;
		pr_info("%s: Relese-before-press\n",__func__);
		pr_info("%s: Leave %s\n", __func__, __func__);
		return IRQ_HANDLED;
	} else {
		pwrkey->press = true;
	}


#ifdef CONFIG_MACH_KTTECH
	if (!work_pending(&kttech_pwrkey_work))
		schedule_work(&kttech_pwrkey_work);    
#endif

    printk(KERN_CRIT "%s: Power Key Pressed\n", 	__func__ );

	input_report_key(pwrkey->pwr, KEY_POWER, 1);
	input_sync(pwrkey->pwr);

	}
	pr_info("%s: Leave %s\n", __func__, __func__);
	return IRQ_HANDLED;
}

static irqreturn_t pwrkey_release_irq(int irq, void *_pwrkey)
{
	struct pmic8xxx_pwrkey *pwrkey = _pwrkey;

	pr_info("%s: Enter %s\n", __func__, __func__);
	dump_reg(pwrkey, false);

#ifdef VERY_LONG_HOLD_REBOOT
	if (pwrkey->press == true)
	hrtimer_cancel(&pwrkey->very_longPress_timer);
#endif

#ifdef CONFIG_PM_DEEPSLEEP
	if (get_deepsleep_mode() && pwrkey->press == true) {
		hrtimer_cancel(&pwrkey->longPress_timer);
		if (pwrkey->expired == 1) {
			pwrkey->expired = 0;

	if (pwrkey->press == false) {
		pr_info("%s: Relese-before-press\n",__func__);
		input_report_key(pwrkey->pwr, KEY_POWER, 1);
		input_sync(pwrkey->pwr);
		pwrkey->press = true;
	} else {
		pwrkey->press = false;
	}

			input_report_key(pwrkey->pwr, KEY_POWER, 0);
			input_sync(pwrkey->pwr);
		}
		wake_lock_timeout(&pwrkey->wake_lock, 5);
	} else
#endif
	{

	if (pwrkey->press == false) {
		//dump_reg(pwrkey, false);
		pr_info("%s: Relese-before-press\n",__func__);
		input_report_key(pwrkey->pwr, KEY_POWER, 1);
		input_sync(pwrkey->pwr);
		pwrkey->press = true;
	} else {
		pwrkey->press = false;
	}
 
   printk(KERN_CRIT "%s: Power Key Release\n", 	__func__ );
   
	input_report_key(pwrkey->pwr, KEY_POWER, 0);
	input_sync(pwrkey->pwr);

	}
	pr_info("%s: Leave %s\n", __func__, __func__);
	return IRQ_HANDLED;
}

#ifdef CONFIG_PM_SLEEP
static int pmic8xxx_pwrkey_suspend(struct device *dev)
{
	struct pmic8xxx_pwrkey *pwrkey = dev_get_drvdata(dev);

	if (device_may_wakeup(dev)) {
		enable_irq_wake(pwrkey->key_press_irq);
                enable_irq_wake(pwrkey->key_release_irq);
        }

	return 0;
}

static int pmic8xxx_pwrkey_resume(struct device *dev)
{
	struct pmic8xxx_pwrkey *pwrkey = dev_get_drvdata(dev);

	if (device_may_wakeup(dev)) {
		disable_irq_wake(pwrkey->key_press_irq);
		disable_irq_wake(pwrkey->key_release_irq);
	}

#ifdef CONFIG_MACH_KTTECH
   wake_lock_timeout(&kttech_pwrkey_wakelock, 2*HZ);
   printk(KERN_CRIT "%s: Power Key Resume\n", 	__func__ );
#endif

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(pm8xxx_pwr_key_pm_ops,
		pmic8xxx_pwrkey_suspend, pmic8xxx_pwrkey_resume);

static int __devinit pmic8xxx_pwrkey_probe(struct platform_device *pdev)
{
	struct input_dev *pwr;
	int key_release_irq = platform_get_irq(pdev, 0);
	int key_press_irq = platform_get_irq(pdev, 1);
	int err;
	unsigned int delay;
	u8 pon_cntl;
	struct pmic8xxx_pwrkey *pwrkey;
	const struct pm8xxx_pwrkey_platform_data *pdata =
					dev_get_platdata(&pdev->dev);

// paiksun...
#ifdef CONFIG_SW_RESET
	pwrkey_pm_chip = pdev;
	pwrkey_irq = key_press_irq;
#endif

	if (!pdata) {
		dev_err(&pdev->dev, "power key platform data not supplied\n");
		return -EINVAL;
	}

	/* Valid range of pwr key trigger delay is 1/64 sec to 2 seconds. */
	if (pdata->kpd_trigger_delay_us > USEC_PER_SEC * 2 ||
		pdata->kpd_trigger_delay_us < USEC_PER_SEC / 64) {
		dev_err(&pdev->dev, "invalid power key trigger delay\n");
		return -EINVAL;
	}

	pwrkey = kzalloc(sizeof(*pwrkey), GFP_KERNEL);
	if (!pwrkey)
		return -ENOMEM;

	pwrkey->pdata = pdata;

	pwr = input_allocate_device();
	if (!pwr) {
		dev_dbg(&pdev->dev, "Can't allocate power button\n");
		err = -ENOMEM;
		goto free_pwrkey;
	}

	input_set_capability(pwr, EV_KEY, KEY_POWER);

	pwr->name = "pmic8xxx_pwrkey";
	pwr->phys = "pmic8xxx_pwrkey/input0";
	pwr->dev.parent = &pdev->dev;

	delay = (pdata->kpd_trigger_delay_us << 6) / USEC_PER_SEC;
	delay = ilog2(delay);

	err = pm8xxx_readb(pdev->dev.parent, PON_CNTL_1, &pon_cntl);
	if (err < 0) {
		dev_err(&pdev->dev, "failed reading PON_CNTL_1 err=%d\n", err);
		goto free_input_dev;
	}

	pon_cntl &= ~PON_CNTL_TRIG_DELAY_MASK;
	pon_cntl |= (delay & PON_CNTL_TRIG_DELAY_MASK);
	if (pdata->pull_up)
		pon_cntl |= PON_CNTL_PULL_UP;
	else
		pon_cntl &= ~PON_CNTL_PULL_UP;

	err = pm8xxx_writeb(pdev->dev.parent, PON_CNTL_1, pon_cntl);
	if (err < 0) {
		dev_err(&pdev->dev, "failed writing PON_CNTL_1 err=%d\n", err);
		goto free_input_dev;
	}

	err = input_register_device(pwr);
	if (err) {
		dev_dbg(&pdev->dev, "Can't register power key: %d\n", err);
		goto free_input_dev;
	}

	pwrkey->key_press_irq = key_press_irq;
        pwrkey->key_release_irq = key_release_irq;
	pwrkey->pwr = pwr;
	pwrkey->press = false;
	pwrkey->dev = &pdev->dev;

#ifdef CONFIG_MACH_KTTECH
   wake_lock_init(&kttech_pwrkey_wakelock, WAKE_LOCK_SUSPEND, "kttech_pwrkey");
   INIT_WORK(&kttech_pwrkey_work, kttech_pwrkey_work_f);
#endif

	platform_set_drvdata(pdev, pwrkey);

	err = request_any_context_irq(key_press_irq, pwrkey_press_irq,
		IRQF_TRIGGER_RISING, "pmic8xxx_pwrkey_press", pwrkey);
	if (err < 0) {
		dev_dbg(&pdev->dev, "Can't get %d IRQ for pwrkey: %d\n",
				 key_press_irq, err);
		goto unreg_input_dev;
	}

	err = request_any_context_irq(key_release_irq, pwrkey_release_irq,
		 IRQF_TRIGGER_RISING, "pmic8xxx_pwrkey_release", pwrkey);
	if (err < 0) {
		dev_dbg(&pdev->dev, "Can't get %d IRQ for pwrkey: %d\n",
				 key_release_irq, err);

		goto free_press_irq;
	}

#ifdef VERY_LONG_HOLD_REBOOT
	hrtimer_init(&(pwrkey->very_longPress_timer),
			CLOCK_MONOTONIC,
			HRTIMER_MODE_REL);

	(pwrkey->very_longPress_timer).function =
		very_longPress_timer_callback;
#endif

#ifdef CONFIG_PM_DEEPSLEEP

	hrtimer_init(&(pwrkey->longPress_timer),
			CLOCK_MONOTONIC,
			HRTIMER_MODE_REL);

	(pwrkey->longPress_timer).function = longPress_timer_callback;
	wake_lock_init(&pwrkey->wake_lock, WAKE_LOCK_SUSPEND, "pwrkey");
#endif

	device_init_wakeup(&pdev->dev, pdata->wakeup);

	return 0;

free_press_irq:
	free_irq(key_press_irq, NULL);
unreg_input_dev:
	platform_set_drvdata(pdev, NULL);
	input_unregister_device(pwr);
	pwr = NULL;
free_input_dev:
	input_free_device(pwr);
free_pwrkey:
	kfree(pwrkey);
	return err;
}

static int __devexit pmic8xxx_pwrkey_remove(struct platform_device *pdev)
{
	struct pmic8xxx_pwrkey *pwrkey = platform_get_drvdata(pdev);
	int key_release_irq = platform_get_irq(pdev, 0);
	int key_press_irq = platform_get_irq(pdev, 1);
#ifdef CONFIG_PM_DEEPSLEEP
	wake_lock_destroy(&pwrkey->wake_lock);
#endif
	device_init_wakeup(&pdev->dev, 0);

	free_irq(key_press_irq, pwrkey);
	free_irq(key_release_irq, pwrkey);
	input_unregister_device(pwrkey->pwr);
	platform_set_drvdata(pdev, NULL);
#ifdef CONFIG_MACH_KTTECH
   cancel_work_sync(&kttech_pwrkey_work);
   wake_lock_destroy(&kttech_pwrkey_wakelock);
#endif
	kfree(pwrkey);

	return 0;
}

static struct platform_driver pmic8xxx_pwrkey_driver = {
	.probe		= pmic8xxx_pwrkey_probe,
	.remove		= __devexit_p(pmic8xxx_pwrkey_remove),
	.driver		= {
		.name	= PM8XXX_PWRKEY_DEV_NAME,
		.owner	= THIS_MODULE,
		.pm	= &pm8xxx_pwr_key_pm_ops,
	},
};

static int __init pmic8xxx_pwrkey_init(void)
{
	return platform_driver_register(&pmic8xxx_pwrkey_driver);
}
module_init(pmic8xxx_pwrkey_init);

static void __exit pmic8xxx_pwrkey_exit(void)
{
	platform_driver_unregister(&pmic8xxx_pwrkey_driver);
}
module_exit(pmic8xxx_pwrkey_exit);

MODULE_ALIAS("platform:pmic8xxx_pwrkey");
MODULE_DESCRIPTION("PMIC8XXX Power Key driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Trilok Soni <tsoni@codeaurora.org>");
