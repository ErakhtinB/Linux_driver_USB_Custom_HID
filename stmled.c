#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/uaccess.h>
 
#define MIN(a,b) (((a) <= (b)) ? (a) : (b))

#define STMLED_VID	1155
#define STMLED_PID	22352
#define BULK_EP_OUT 	0x01 //0x1
#define BULK_EP_IN 	0x81
#define MAX_PKT_SIZE 	512
#define MSG_TIMEOUT_MS	5000
 
static struct usb_device *device;
static struct usb_class_driver class;
static unsigned char bulk_buf[MAX_PKT_SIZE];
 
static int stmled_open(struct inode *i, struct file *f)
{
    return 0;
}
static int stmled_close(struct inode *i, struct file *f)
{
    return 0;
}
static ssize_t stmled_read(struct file *f, char __user *buf, size_t cnt, loff_t *off)
{
    int retval;
    int read_cnt;
 
    /* Read the data from the bulk endpoint */
    retval = usb_bulk_msg(device, usb_rcvbulkpipe(device, BULK_EP_IN),
            bulk_buf, MAX_PKT_SIZE, &read_cnt, MSG_TIMEOUT_MS);
    if (retval)
    {
        printk(KERN_ERR "Bulk message returned %d\n", retval);
        return retval;
    }
    if (copy_to_user(buf, bulk_buf, MIN(cnt, read_cnt)))
    {
        return -EFAULT;
    }
 
    return MIN(cnt, read_cnt);
}
static ssize_t stmled_write(struct file *f, const char __user *buf, size_t cnt, loff_t *off)
{
    int retval;
    int wrote_cnt = MIN(cnt, MAX_PKT_SIZE);
 
    if (copy_from_user(bulk_buf, buf, MIN(cnt, MAX_PKT_SIZE)))
    {
        return -EFAULT;
    }
 
    /* Write the data into the bulk endpoint */
    retval = usb_bulk_msg(device, usb_sndbulkpipe(device, BULK_EP_OUT),
            bulk_buf, MIN(cnt, MAX_PKT_SIZE), &wrote_cnt, MSG_TIMEOUT_MS);
    if (retval)
    {
        printk(KERN_ERR "Bulk message returned %d\n", retval);
        return retval;
    }
 
    return wrote_cnt;
}
 
static struct file_operations fops =
{
    .open = stmled_open,
    .release = stmled_close,
    .read = stmled_read,
    .write = stmled_write,
};
 
static int stmled_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    int retval;
 
    device = interface_to_usbdev(interface);
 
    class.name = "usb/stmled%d";
    class.fops = &fops;
    if ((retval = usb_register_dev(interface, &class)) < 0)
    {
        /* Something prevented us from registering this driver */
        pr_err("Not able to get a minor for this device.");
    }
    else
    {
        printk(KERN_INFO "Minor obtained: %d\n", interface->minor);
    }
 
    return retval;
}
 
static void stmled_disconnect(struct usb_interface *interface)
{
    usb_deregister_dev(interface, &class);
}
 
/* Table of devices that work with this driver */
static struct usb_device_id stmled_table[] =
{
    { USB_DEVICE(STMLED_VID, STMLED_PID) },
    {} /* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, stmled_table);
 
static struct usb_driver stmled_driver =
{
    .name = "stmled_driver",
    .probe = stmled_probe,
    .disconnect = stmled_disconnect,
    .id_table = stmled_table,
};
 
static int __init stmled_init(void)
{
    int result;
 
    /* Register this driver with the USB subsystem */
    if ((result = usb_register(&stmled_driver)))
    {
        pr_err("usb_register failed. Error number %d", result);
    }
    return result;
}
 
static void __exit stmled_exit(void)
{
    /* Deregister this driver with the USB subsystem */
    usb_deregister(&stmled_driver);
}
 
module_init(stmled_init);
module_exit(stmled_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Boris Erakhti <boris.erakhtin@gmail.com>");
MODULE_DESCRIPTION("USB STMLED Device Driver");

