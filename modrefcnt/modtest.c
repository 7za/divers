/*!
 * \author frederic ferrandis
 * \brief  little example to test hw timer
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>


static  int   __init resget_init(void);
static  void  __exit resget_exit(void);

#define LBRIDGEN_DRIVER_INITIALIZER         \
            {   .lbclass        = (void*)0, \
                .device         = (void*)0  \
            }

static struct lbridgen_driver {
    struct cdev  lbcdev;
    dev_t lbdnum;
    struct class *lbclass;
    struct device *device;

}   lbridgen_driver =   LBRIDGEN_DRIVER_INITIALIZER; 




static int lb_release(struct inode *inode, struct file *file)
{
    printk("release device\n");
    printk("release device %d\n", module_refcount(THIS_MODULE));
    module_put(THIS_MODULE);

    printk("release device %d\n", module_refcount(THIS_MODULE) );
    return 0;
}


/*!
 * \brief this function is called after a cat on /dev/file
 * after first cat, timer is initialized and irq is requested
 * after second cat, timer is enabled
 */
static int
lb_open(struct inode *inode, struct file *file)
{
    int ret;
    static int count = 0;

    printk("opening device %d\n", module_refcount(THIS_MODULE));
    __module_get(THIS_MODULE);

    printk("opening device %d\n", module_refcount(THIS_MODULE));
        

    return 0;
}


static ssize_t
lb_read(struct file *file, char __user *buff, size_t len, loff_t *ptr)
{
    return 0;
}


static struct file_operations lb_fops = {
    .owner      =   THIS_MODULE,
    .open       =   lb_open,
    .read       =   lb_read, /* just to disable message "operation not supported during cat*/
    .release    =   lb_release,
};



static int __init 
resget_create_char_device(void)
{
    int ret = -ENOMEM;
    /* create/register a range of char device number */
    printk("device %d\n", module_refcount(THIS_MODULE));
    ret = alloc_chrdev_region(&lbridgen_driver.lbdnum, 0, 1, "chrdev_timer_test");
    if(unlikely(ret != 0)){
        printk(KERN_ERR "unable to allocate char_device region\n");
        return ret;
    }

    /* create struct class which will be populate in /sys/class */ 
    lbridgen_driver.lbclass = class_create(THIS_MODULE, "timers_test_bdic");
    if(IS_ERR(lbridgen_driver.lbclass)){
        printk(KERN_ERR "unable to allocate new class\n");
        ret = -ENOMEM;
        goto chrdev_class_create_err;
    }

    /* initialize char device with struct file_operations  */
    /* this function never fails */
    /* then add char device module ownership*/
    cdev_init( &lbridgen_driver.lbcdev, &lb_fops);
    lbridgen_driver.lbcdev.owner = THIS_MODULE;

    /* connect char device with previous allocated range of char device number
     * a call of kobj_map, which seems to never failed is the return value of 
     * the function, but read retcode for safety purpose 
     */
    ret = cdev_add(&lbridgen_driver.lbcdev, lbridgen_driver.lbdnum, 1);
    if(unlikely(ret != 0)){
        printk( KERN_ERR "unable to add char device in system\n"); 
        goto chrdev_chardev_add_error;
    }

    /* register in sysfs our chardev in class previously created */ 
    lbridgen_driver.device = device_create(lbridgen_driver.lbclass, 
                                           0, 
                                           lbridgen_driver.lbdnum, 
                                           NULL, 
                                           "timer_test_bdic");

    if(IS_ERR(lbridgen_driver.device)){
        printk(KERN_ERR "unable to add device in class\n");
        goto chrdev_device_create_err;
    }

    return 0; 

/* 
 * error handler ... I know that Thales dislike goto, but it's quite simple 
 * in that case
 */
chrdev_device_create_err:
    cdev_del(&lbridgen_driver.lbcdev);
chrdev_chardev_add_error:
    class_destroy(lbridgen_driver.lbclass);
chrdev_class_create_err:
    unregister_chrdev_region(lbridgen_driver.lbdnum, 1);

    return ret;
}



static int __init resget_init(void)
{

    int ret = resget_create_char_device();
    if(ret){
        return ret;
    }

    printk("module insertion success\n");
    //timer_prepare((unsigned long)&tl);
    return 0;
}



static void __exit resget_exit(void)
{
    
    cdev_del(&lbridgen_driver.lbcdev);

    if(lbridgen_driver.lbclass){
        device_destroy(lbridgen_driver.lbclass, lbridgen_driver.lbdnum);
        class_destroy(lbridgen_driver.lbclass);
    }
}



module_init(resget_init);
module_exit(resget_exit);
MODULE_AUTHOR("frederic ferrandis");
MODULE_LICENSE("GPL");

