#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/slab.h>

MODULE_AUTHOR("Ryuichi Ueda");
MODULE_AUTHOR("Nagasaka Takumi");
MODULE_DESCRIPTION("driver for LED contol");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;
static volatile u32 *gpio_base = NULL;

static ssize_t question(struct file* filp, const char __user *buf, size_t count, loff_t* pos){
	printk(KERN_INFO "led_write is called\n");
	char *str = kmalloc(sizeof(count+1)*sizeof(char), GFP_KERNEL);
	memset(str, '\0', count+1);
	if(copy_from_user(str, buf, count)){
		kfree(str);
		return -EFAULT;
	}
	if(strcmp(str, "Q1\n") == 0){
		printk(KERN_INFO "Q1: The quick brown ??? jumps over the lazy dog.\n   The words in [?]\nA: cat\nB: fox\n");
		gpio_base[10] = 1 << 24;
		gpio_base[10] = 1 << 25;
		gpio_base[7] = 1 << 24;
	}
	else if(strcmp(str, "Q2\n") == 0){
		printk(KERN_INFO "Q2: All your base are belong to ???.\n   The words in [?]\nA: cat\nB: us\n");
		gpio_base[10] = 1 << 24;
		gpio_base[10] = 1 << 25;
		gpio_base[7] = 1 << 24;
	}
	else if(strcmp(str, "Q3\n") == 0){
		printk(KERN_INFO "Q3: It's raining ???s and dogs.\n   The words in [?]\nA: cat\nB: girl\n");
		gpio_base[10] = 1 << 24;
		gpio_base[10] = 1 << 25;
		gpio_base[7] = 1 << 25;
	}
	kfree(str);
	return count;
}

static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.write = question
};

static int __init init_mod(void){
	int retval;
	gpio_base = ioremap_nocache(0x3f200000, 0xA0);

	const u32 led1 = 25;
	const u32 index1 = led1/10;
	const u32 shift1 = (led1%10)*3;
	const u32 mask1 = ~(0x7 << shift1);
	gpio_base[index1] = (gpio_base[index1] & mask1) | (0x1 << shift1);


	const u32 led2 = 24;
	const u32 index2 = led2/10;
	const u32 shift2 = (led2%10)*3;
	const u32 mask2 = ~(0x7 << shift2);
	gpio_base[index2] = (gpio_base[index2] & mask2) | (0x1 << shift2);

	retval = alloc_chrdev_region(&dev, 0, 1, "myled");
	if(retval < 0){
		printk(KERN_ERR "alloc_chrdev_region failed.\n");
		return retval;
	}
	printk(KERN_INFO "%s is loaded. major:%d\n", __FILE__, MAJOR(dev));

	cdev_init(&cdv, &led_fops);
	retval = cdev_add(&cdv, dev, 1);
	if(retval < 0){
		printk(KERN_ERR "cdv_add failed. major:%d, minor:%d", MAJOR(dev),MINOR(dev));
		return retval;
	}
	cls = class_create(THIS_MODULE, "myled");
	if(IS_ERR(cls)){
		printk(KERN_ERR "class_create failed.");
		return PTR_ERR(cls);
	}
	device_create(cls, NULL, dev, NULL, "myled%d", MINOR(dev));
	return 0;
}

static void __exit cleanup_mod(void){
	device_destroy(cls, dev);
	class_destroy(cls);
	cdev_del(&cdv);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unoaded. major:%d\n", __FILE__, MAJOR(dev));
}

module_init(init_mod);
module_exit(cleanup_mod);
