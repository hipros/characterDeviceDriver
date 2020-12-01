#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>

#define CHR_DEV_NAME "BufferedMem"
#define CHR_DEV_MAJOR 275

static int N = 0;
static int M = 0;
struct kfifo data_buf, temp_buf;
ssize_t cur_len;

module_param(N, int, 0);
MODULE_PARM_DESC(N, "Write size");

module_param(M, int, 0);
MODULE_PARM_DESC(M, "Read size");

int chr_open(struct inode *inode, struct file *filep)
{
	int number = MINOR(inode->i_rdev);
	printk("Virtual Character Device Open : Minor Number is %d\n", number);
	return 0;
}
int chr_release(struct inode *inode, struct file *filep)
{
	printk("Virtual Character Device Release\n");
	return 0;
}

long chr_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	BUG_ON(cmd > -1 && cmd < 2);
	int i = 0, ret = 0, loop_time = 0;
	static int karg = 0;
	unsigned int kcmd = 0;
	char temp_c = 0;

	karg = (int)arg; 
	kcmd = cmd;

	switch (kcmd) {
		case 0: 
			printk("Device driver N change %d to %d\n", N, karg);

			printk("temp buffer allocate\n");
			kfifo_alloc(&temp_buf, N, GFP_KERNEL);

			printk("move past data buffer to temp buffer\n");
			while(kfifo_len(&data_buf)) {
				kfifo_out(&data_buf, &temp_c, sizeof(temp_c));
				kfifo_in(&temp_buf, &temp_c, sizeof(temp_c));
			}

			printk("data buffer free & alloc\n");
			kfifo_free(&data_buf);
			kfifo_alloc(&data_buf, karg, GFP_KERNEL);
			printk("data buffer size: %d\n", kfifo_size(&data_buf));
			printk("data buffer len: %d\n", kfifo_len(&data_buf));

			printk("remove element from temp buffer\n");
			loop_time = (int)kfifo_len(&temp_buf) - (int)(karg);
			printk("loop time: %d\n", loop_time);			
			for(i = 0; i < loop_time; i++) {
				ret = kfifo_out(&temp_buf, &temp_c, sizeof(temp_c));
				printk("remove character: %c\n", temp_c);
			}
			
			printk("temp buffer len: %d\n", kfifo_len(&temp_buf));
			printk("move element temp buffer to data buffer\n");
			while(kfifo_len(&temp_buf)) {
				printk("while loop in\n");
				kfifo_out(&temp_buf, &temp_c, sizeof(temp_c));
				printk("temp character from temp buf: %c\n", temp_c);
				printk("data buffer address: %p\n", &data_buf);
				printk("temp character address: %p\n", &temp_c);
				kfifo_in(&data_buf, &temp_c, sizeof(temp_c));
				printk("data buffer insert\n");
			}

			printk("free temp buffer");
			kfifo_free(&temp_buf);

			N = karg; 

			break;
		case 1: 
			printk("Device driver M change %d to %d\n", M, karg);
			M = karg; 
			break;
	}
	return 0;
}

ssize_t chr_write(struct file *filep, const char * buf, size_t count, loff_t *f_pos)
{
	int ret = 0, ind = 0;
	char cbuf[128] = {0,}, dummy = 0;
	size_t len = 0;

	len = count;
	ret = copy_from_user(cbuf, buf, len);

	while(ind < len-1){
		while(kfifo_len(&data_buf) >= N){
			kfifo_out(&data_buf, &dummy, sizeof(dummy));
			printk("remove by cycle: %c\n", dummy);
			printk("fifo len: %d\n N: %d\n", kfifo_len(&data_buf), N);
			break;
		}
		kfifo_in(&data_buf, &cbuf[ind], sizeof(cbuf[ind]));
		ind++;
	}

	printk("write string to character device driver: %s\n", cbuf);

	return count;
}

ssize_t chr_read(struct file *filep, char * buf, size_t count, loff_t *f_pos)
{
	int ret = 0, ind = 0;
	char cbuf[128]={0,};

	if(*f_pos != 0) return 0;

	while(ind < M && !kfifo_is_empty(&data_buf)) {
		ret = kfifo_out(&data_buf, &cbuf[ind], sizeof(cbuf[ind]));
		ind++;
		(*f_pos)++;		
	}
	copy_to_user(buf, cbuf, ind);
	if(ind == 0){
		printk("kernel beffer is empty\n");
	} else {
		printk("read from character device driver: %s\n", cbuf);
	}
	return ind;
}

struct file_operations chr_fops = {
	.owner = THIS_MODULE,
	.write = chr_write,
	.read = chr_read,
	.open = chr_open,
	.release = chr_release,
	.unlocked_ioctl = chr_ioctl
};

int chr_init(void)
{
	int registeration, ret;
	printk("Registeration Character Device to Kernel\n");
	registeration = register_chrdev(CHR_DEV_MAJOR, CHR_DEV_NAME, &chr_fops);
	if(registeration < 0) return registeration;
	printk("MAJOR NUMBER: %d\n", registeration);
	ret = kfifo_alloc(&data_buf, N, GFP_KERNEL);
	return 0;
}

void chr_exit(void)
{
	printk("Unregisteration Character Device to Kernel\n");
	unregister_chrdev(CHR_DEV_MAJOR, CHR_DEV_NAME);
	kfifo_free(&data_buf);
}

module_init(chr_init);
module_exit(chr_exit);

MODULE_LICENSE("GPL");
