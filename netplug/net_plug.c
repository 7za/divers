#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/socket.h>
#include <linux/init.h>
#include <linux/security.h>
#include <linux/usb.h>
#include <linux/moduleparam.h>

/* default is a generic type of usb to serial converter */
static int vendor_id =  0x0781;
static int product_id = 0x5566;

static int (*rs)(struct security_operations *ops) = NULL;
static struct usb_device * (*ufd) (u16 vendor_id, u16 product_id) = NULL;

module_param(vendor_id, uint, 0400);
module_param(product_id, uint, 0400);

static int
netfreeze_socket(int family, int type, int protocol, int kern)
{
	struct usb_device *dev = ufd(vendor_id, product_id);
	if(!dev && family == AF_INET){
		printk(KERN_WARNING "unable to create socket\n");
		return -EPERM;
	}
	printk(KERN_INFO "socket creation success\n");
	usb_put_dev(dev);
	return 0;
}


static struct security_operations netfreezeplug_security_ops = {
//	.name			=	"netfreeze_ops",
	.socket_create	=	netfreeze_socket,
};

	


static int __init netfreeze_init (void)
{
	rs  = (int(*)(struct security_operations*)) 
		                     kallsyms_lookup_name("register_security");
	ufd = (struct usb_device*(*)(u16,u16)) 
		                     kallsyms_lookup_name("usb_find_device");
	if(rs == 0 || ufd == 0)
	{
      printk(KERN_ERR "cannot load module, syms error");
	  return -1;
	}

	/* register ourselves with the security framework */
	
	if (rs(&netfreezeplug_security_ops)){
		printk (KERN_ERR "LSM init error\n");
		return -EINVAL;
	}
	
	return 0;
}

static void __exit netfreeze_exit(void)
{
  printk(KERN_INFO "LSM exit bye\n");
}

security_initcall (netfreeze_init);
module_exit(netfreeze_exit);
MODULE_AUTHOR("frederic ferrandis");
MODULE_LICENSE("GPL");

