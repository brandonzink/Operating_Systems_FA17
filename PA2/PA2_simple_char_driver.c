#include<linux/init.h>
#include<linux/module.h>

#include<linux/fs.h>
#include<asm/uaccess.h>
#include<linux/slab.h>

#define BUFFER_SIZE 1024

static char device_buffer[BUFFER_SIZE];

int open = 0;
int close = 0;
int tempHolder = 0;

ssize_t simple_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset){

	int numberRead;
	int toRead = BUFFER_SIZE - *offset;

	// return if we make it to the end of the file based on buffer size and current offset
	if (toRead == 0){
		printk(KERN_ALERT "END OF FILE");
		return toRead;
	}
	
	// the number read by subtracting the return value from copy to user
	numberRead = toRead - copy_to_user(buffer, device_buffer + *offset, toRead);
	printk(KERN_ALERT "READING %d bytes (simple_char_driver_read)\n", numberRead);

	// recalculate the value of offset from pointer
	*offset += numberRead;

	return numberRead;
}



ssize_t simple_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset){
	
	int numberWrite;
	int numberWritten;
	int numberLeft = BUFFER_SIZE - *offset - tempHolder;

	// double check to make sure there is space left
	if(numberLeft > length){
		numberWrite = length; 
	}
	else{
		numberWrite = numberLeft;
	}

	// the number written calculated by subtracting non-written from return value from copy from uer
	numberWritten = numberWrite - copy_from_user(device_buffer + *offset + tempHolder, buffer, numberWrite);
	
	// if it's full
	if(numberWritten == 0){
		printk(KERN_ALERT "NO SPACE LEFT =(\n");
	}
	else{
		// update the pointer offset and the global tempHolder
		*offset += numberWritten;
		tempHolder += numberWritten;

		printk(KERN_ALERT "WRITING %d bytes (simple_char_driver_write)\n", numberWritten);
	}
	return numberWritten;
}


int simple_char_driver_open (struct inode *pinode, struct file *pfile){

	//if it opens
	open++;
	printk(KERN_ALERT "OPENING for the %d time (simple_char_driver_open)\n", open);
	return 0;
}


int simple_char_driver_close (struct inode *pinode, struct file *pfile){

	//if it closes
	close++;
	printk(KERN_ALERT "CLOSING for the %d time\n", close);
	return 0;
}

loff_t simple_char_driver_seek(struct file *pfile, loff_t offset, int whence) { //return loff_t
    loff_t pos = 0;
    switch(whence) { //might be &pos
        case 0 : //set
            pos = offset;
            break;
        case 1 : //current
            pos = pfile->f_pos + offset;
            break;
        case 2 : //end
            pos = BUFFER_SIZE - offset;
            break;
    }
    if(pos < 0) //max and min for position
        pos = 0;
    if(pos > BUFFER_SIZE)
        pos = BUFFER_SIZE;
    pfile->f_pos = pos;
    return pos; //return pos;
}

struct file_operations simple_char_driver_file_operations = {
	.owner   = THIS_MODULE,
	.read    = simple_char_driver_read,
	.write   = simple_char_driver_write,
	.open    = simple_char_driver_open,
	.release = simple_char_driver_close,
	.llseek = simple_char_driver_seek
};

static int simple_char_driver_init(void)
{
	//the init was called, log it
	printk(KERN_ALERT "INITIALIZING (simple_char_driver_init)\n");
	//register it
	register_chrdev(240, "simple_driver", &simple_char_driver_file_operations); //might be pointer
	//free memory
	kmalloc(device_buffer, GFP_KERNEL);
	return 0;
}

// Return type must be void to avoid warning in compilation
static void simple_char_driver_exit(void)
{
	//print to the log file that the exit function is called
	printk(KERN_ALERT "EXITING Simple Character Driver\n");
	//unregister the device 
	unregister_chrdev(240, "simple_driver");
	//free the memory
	kfree(&device_buffer);
}

module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);
