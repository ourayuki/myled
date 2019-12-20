#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/slab.h>

MODULE_AUTHOR("Nagasaka Takumi");
MODULE_DESCRIPTION("driver for LED contol");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;
static volatile u32 *gpio_base = NULL;

//static ssize_t led_write(struct file* filp, const char* buf, size_t count, loff_t* pos){
//	printk(KERN_INFO "led_write is called\n");
//	return 1;
//}

// changed
static ssize_t led_write(struct file* filp, char __user *buf, size_t count, loff_t* pos){
	char *newbuf = kmalloc(sizeof(char) * count, GFP_KERNEL);
	//int buflen = 0;
	//buflen = strlen(buf);
	//count = buflen;
	char command[sizeof(char)*count];
	if(copy_from_user(newbuf, buf, sizeof(char)*count)){
		kfree(newbuf);
		return -EFAULT;
	}
	sscanf(newbuf, "%s\n", command);
	if(strcmp(command, "0") == 0){
		gpio_base[10] = 1 << 25;
	}
	else if(strcmp(command, "1") == 0){
		gpio_base[7] = 1 << 25;
	}
	else if(strcmp(command, "2") == 0){
		gpio_base[10] = 1 << 24;
	}
	else if(strcmp(command, "3") == 0){
		gpio_base[7] = 1 << 24;
	}
	printk(KERN_INFO "receive %s\n", command);

	if(strcmp(command, "q1") == 0){
		printk(KERN_INFO "test message\n");
	}
	kfree(newbuf);
	return 1;
}

static ssize_t sushi_read(struct file* filp, char* buf, size_t count, loff_t* pos){
	int size = 0;
	char sushi[] = {0xF0, 0x9F, 0x8D, 0xA3, 0x0A};
	if(copy_to_user(buf+size, (const char *)sushi, sizeof(sushi))){
		printk(KERN_INFO "sushi : copy_to_user failed\n");
		return -EFAULT;
	}
	size += sizeof(sushi);
	return size;
};

static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.write = led_write,
	.read = sushi_read
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
