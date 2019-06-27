/* 
 * morse code driver
 * CMPT433 2017 Fall
 * Group: Greenseer
 */
#include <linux/module.h>
#include <linux/miscdevice.h>	
#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/kfifo.h>
#include <linux/leds.h>
//#error Are we building morse code driver?

#define DEVICE_NAME  "morse-code"
#define DEFAULT_DOT_TIME 200
#define FIFO_SIZE 256	// Must be a power of 2.

/******************************************************
 * Morse Code Encoding
 ******************************************************/
static unsigned short morsecode_codes[] = {
		0xB800,	// A 1011 1
		0xEA80,	// B 1110 1010 1
		0xEBA0,	// C 1110 1011 101
		0xEA00,	// D 1110 101
		0x8000,	// E 1
		0xAE80,	// F 1010 1110 1
		0xEE80,	// G 1110 1110 1
		0xAA00,	// H 1010 101
		0xA000,	// I 101
		0xBBB8,	// J 1011 1011 1011 1
		0xEB80,	// K 1110 1011 1
		0xBA80,	// L 1011 1010 1
		0xEE00,	// M 1110 111
		0xE800,	// N 1110 1
		0xEEE0,	// O 1110 1110 111
		0xBBA0,	// P 1011 1011 101
		0xEEB8,	// Q 1110 1110 1011 1
		0xBA00,	// R 1011 101
		0xA800,	// S 1010 1
		0xE000,	// T 111
		0xAE00,	// U 1010 111
		0xAB80,	// V 1010 1011 1
		0xBB80,	// W 1011 1011 1
		0xEAE0,	// X 1110 1010 111
		0xEBB8,	// Y 1110 1011 1011 1
		0xEEA0	// Z 1110 1110 101
};

/******************************************************
 * Parameter
// Declare the variable as a parameter.
//   S_IRUGO makes it's /sys/module node readable.
//   # cat /sys/module/morsecode_driver/parameters/dottime
 ******************************************************/

static unsigned int dottime = DEFAULT_DOT_TIME;

module_param(dottime, uint, S_IRUGO);
MODULE_PARM_DESC(dottime, " dottime for morse code led blinking");

/**************************************************************
 * FIFO Support
// Info on the interface:
//    https://www.kernel.org/doc/htmldocs/kernel-api/kfifo.html#idp10765104
// Good example:
//    http://lxr.free-electrons.com/source/samples/kfifo/bytestream-example.c
 *************************************************************/

//static DECLARE_KFIFO(morsecode_fifo, char, FIFO_SIZE);

//dynamic fifo
static struct kfifo morsecode_fifo;

static int put_to_fifo(char ch)
{
	int ret;
	int i;
	if(kfifo_is_full(&morsecode_fifo)){
		struct kfifo tmp_fifo;
				
		ret = kfifo_alloc(&tmp_fifo, kfifo_size(&morsecode_fifo),GFP_KERNEL);
		if(ret){
			printk(KERN_ERR "morsecode_driver: error kfifo_alloc tmp_fifo\n");
		}
		for(i=0;i<kfifo_size(&morsecode_fifo);i++){
			char c;
			ret=kfifo_get(&morsecode_fifo, &c);
			if (!ret) {
				printk(KERN_ERR "morsecode_driver: error kfifo_get\n");
			}
			ret=kfifo_put(&tmp_fifo, c);
			if (!ret) {
				printk(KERN_ERR "morsecode_driver: error kfifo_put\n");
			}
		}
		kfifo_reset(&morsecode_fifo);
		//kfifo_free(morsecode_fifo);
		ret = kfifo_alloc(&morsecode_fifo, 2*kfifo_size(&tmp_fifo),GFP_KERNEL);
		if(ret){
			printk(KERN_ERR "morsecode_driver: error kfifo_alloc morsecode_fifo realloc\n");
		}
		for(i=0;i<kfifo_size(&tmp_fifo);i++){
			char c;
			ret=kfifo_get(&tmp_fifo, &c);
			if (!ret) {
				printk(KERN_ERR "morsecode_driver: error kfifo_get\n");
			}
			ret=kfifo_put(&morsecode_fifo, c);
			if (!ret) {
				printk(KERN_ERR "morsecode_driver: error kfifo_put\n");
			}
		}
		kfifo_free(&tmp_fifo);
	}
	
	ret = kfifo_put(&morsecode_fifo, ch);
	if (!ret) {
		printk(KERN_ERR "morsecode_driver: error kfifo_put\n");
	}
	return ret;
}

/******************************************************
 * LED
 ******************************************************/
DEFINE_LED_TRIGGER(morsecode_ledtrig);

static void morsecode_led_on(int count)
{
	led_trigger_event(morsecode_ledtrig, LED_FULL);
	msleep(dottime*count);
}

static void morsecode_led_off(int count)
{
	led_trigger_event(morsecode_ledtrig, LED_OFF);
	msleep(dottime*count);
}

static void led_register(void)
{
	// Setup the trigger's name:
	led_trigger_register_simple("morse-code", &morsecode_ledtrig);
}

static void led_unregister(void)
{
	// Cleanup
	led_trigger_unregister_simple(morsecode_ledtrig);
}

static int flash_morsecode(unsigned short code)
{
	int i;
	int one_count = 0;  //how many 1s have seen
	int zero_flag = 0;  //is previous bit 0?
	for(i=15;i>=0;i--){			
		if(code&(1<<i)){ //get the i-th bit
			one_count++;
			zero_flag = 0;
		}else{
			if(zero_flag==1){ //if two consecutive 0 
				break;
			}
			zero_flag=1;
			morsecode_led_on(one_count);
			if(one_count==1){
				put_to_fifo('.');
			}else if(one_count==3){
				put_to_fifo('-');
			}
			one_count=0;
			morsecode_led_off(1);
		}
	}
	return 0;
}

/******************************************************
 * File Operation Callbacks
 ******************************************************/

static ssize_t morsecode_read(struct file *file,char *buff,size_t count, loff_t *ppos)
{
	int num_bytes_read=0;
	//printk(KERN_INFO "morsecode_drv:morsecode_read(), buff size %d, f_pos %d\n",(int) count, (int) *ppos);

	if (kfifo_to_user(&morsecode_fifo, buff, count, &num_bytes_read)) {
		return -EFAULT;
	}
	*ppos += num_bytes_read;
	return num_bytes_read;
	
}

static ssize_t morsecode_write(struct file *file,const char *buff, size_t count, loff_t *ppos)
{
	int buff_idx = 0;
	int ini_flag = 0;
	int space_flag = 0;

	//printk(KERN_INFO "morsecode_drv: In morsecode_write()\n");

	for (buff_idx = 0; buff_idx < count; buff_idx++) {
		char ch;
		int i;
		unsigned short morsecode;
		
		// Get the character
		if (copy_from_user(&ch, &buff[buff_idx], sizeof(ch))) {
			return -EFAULT;
		}

		if( (ch>='A' && ch<='Z') || (ch>='a' && ch<='z') ){
			if(ini_flag && space_flag){ // first letter in a new word
				morsecode_led_off(6);
				put_to_fifo(' ');
				put_to_fifo(' ');
				put_to_fifo(' ');			
				space_flag=0;				
			}else if(ini_flag && (!space_flag)){ //a letter in word
				morsecode_led_off(2);
				put_to_fifo(' ');
			}else if(!ini_flag){ //first letter for the message
				ini_flag=1;
			}

			i=ch-'A';

			if(ch>='a' && ch<='z'){
				i=ch-'a';
			}

			morsecode = morsecode_codes[i];
			flash_morsecode(morsecode);
		}else if(ch == ' ') {
			if(ini_flag){
				space_flag=1;
			}
		}		
	}

	put_to_fifo('\n');

	*ppos += count;

	// Return # bytes actually written.
	return count;
}

/******************************************************
 * Misc support
 ******************************************************/
struct file_operations my_fops = {
	.owner    =  THIS_MODULE,
	.read     =  morsecode_read,
	.write    =  morsecode_write,
};

static struct miscdevice my_miscdevice = {
		.minor    = MISC_DYNAMIC_MINOR,         // Let the system assign one.
		.name     = DEVICE_NAME,                // /dev/.... file.
		.fops     = &my_fops                    // Callback functions.
};

/******************************************************
 * Driver initialization and exit:
 ******************************************************/
static int __init morsecode_driver_init(void)
{
	int ret;
	printk(KERN_INFO "----> My morse code driver init()\n");
	
	//register as a misc driver, led trigger and init kfifo
	ret = kfifo_alloc(&morsecode_fifo, FIFO_SIZE, GFP_KERNEL);
	if(ret){
		printk(KERN_ERR "morsecode_driver: error kfifo_alloc\n");
	}

	ret = misc_register(&my_miscdevice);
	led_register();
	//INIT_KFIFO(morsecode_fifo);
		
	return ret;
}
static void __exit morsecode_driver_exit(void)
{
	printk(KERN_INFO "<---- My morse code driver exit().\n");
	
	//unregister misc driver and led trigger
	misc_deregister(&my_miscdevice);
	kfifo_free(&morsecode_fifo);
	led_unregister();
}


module_init(morsecode_driver_init);
module_exit(morsecode_driver_exit);

MODULE_AUTHOR("Baihui Zhang & Chenxia Dun");
MODULE_DESCRIPTION("A morse code driver");
MODULE_LICENSE("GPL");// Important to leave as GPL.
