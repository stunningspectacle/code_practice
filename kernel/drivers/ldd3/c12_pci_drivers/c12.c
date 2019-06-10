#include <linux/module.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static struct proc_dir_entry *proc_file;

static int c12_show(struct seq_file *m, void *data)
{
	struct pci_dev *dev = NULL;
	u16 vendor, device;
	u32 addr0, addr1;
	u8 int_line, int_pin;

	dev = pci_get_device(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, dev);
	while (dev) {
		vendor = device = 0;
		pci_read_config_word(dev, PCI_VENDOR_ID, &vendor);
		pci_read_config_word(dev, PCI_DEVICE_ID, &device);

		addr0 = addr1 = 0;
		pci_read_config_dword(dev, PCI_BASE_ADDRESS_0, &addr0);
		pci_read_config_dword(dev, PCI_BASE_ADDRESS_1, &addr1);

		int_line = int_pin = 0;
		pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &int_line);
		pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &int_pin);
		seq_printf(m, "vendor(0x%x), device(0x%x): \n"
				"\taddr0(0x%x), addr1(0x%x), line(0x%x), pin(0x%x)\n",
				vendor, device, addr0, addr1, int_line, int_pin);
		dev = pci_get_device(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, dev);
	}

	return 0;
}

#define DEFINE_SHOW_ATTRIBUTE1(__name) \
static int __name ## _open(struct inode *inode, struct file *filp) \
{ \
	return single_open(filp, __name ## _show, inode->i_private);\
} \
static struct file_operations __name ## _fops = { \
	.owner = THIS_MODULE, \
	.open = __name ## _open, \
	.release = single_release, \
	.read = seq_read, \
	.llseek = seq_lseek, \
};

DEFINE_SHOW_ATTRIBUTE1(c12);

static int __init c12_init(void)
{
	pr_info("%s is called\n", __func__);

	proc_file = proc_create("c12", 0666, NULL, &c12_fops);
	if (!proc_file)
		pr_err("Failed to create proc dir entry\n");

	return 0;
}

static void __exit c12_exit(void)
{
	pr_info("%s is called\n", __func__);
	if (proc_file)
		proc_remove(proc_file);
}

module_init(c12_init);
module_exit(c12_exit);
MODULE_LICENSE("GPL");
