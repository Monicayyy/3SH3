/**
 * hello.c
 *
 * Kernel module that communicates with /proc file system.
 * 
 * Samkith jain September 6, 2024
 * Comp Sci 3SH3, Fall 2024
 * Reference:  hello.c Operating System Concepts - 10th Edition
 * Copyright John Wiley & Sons - 2018
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h> 
#include <linux/uaccess.h> //what kernel code uses to safely copy data to a user-space buffer

#define BUFFER_SIZE 128 //message will fit in a small local buffer

#define PROC_NAME "hello" //process name is hello
#define MESSAGE "Hello World\n"

/**
 * Function prototypes
 */
ssize_t proc_read(struct file *file, char *buf, size_t count, loff_t *pos);

/*
"when someone reads /proc/hello, call function proc_read()"
*/
static const struct proc_ops my_proc_ops = {
        .proc_read = proc_read,
};

/* This function is called when the module is loaded. */
int proc_init(void)
{

        // creates the /proc/hello entry when run sudo insmod hello_newKernel.ko
        // the following function call is a wrapper for
        // proc_create_data() passing NULL as the last argument
        proc_create(PROC_NAME, 0, NULL, &my_proc_ops); //creates the process hello and attaches the read handler (proc_read) to it

        printk(KERN_INFO "/proc/%s created\n", PROC_NAME);

	return 0;
}

/* 
This function is called when the module is removed. 
sudo rmmod hello_newKernel
*/
void proc_exit(void) {

        // removes the /proc/hello entry
        remove_proc_entry(PROC_NAME, NULL);

        printk( KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

/**
 * This function is called each time the /proc/hello is read.
 * 
 * This function is called repeatedly until it returns 0, so
 * there must be logic that ensures it ultimately returns 0
 * once it has collected the data that is to go into the 
 * corresponding /proc file.
 *
 * params:
 *
 * file:
 * buf: buffer in user space
 * count:
 * pos:

 when run cat /proc/hello
 */
ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
        int rv = 0;
        char buffer[BUFFER_SIZE];
        static int completed = 0;

		//if end of file, tells cat there is no more data. 
        if (completed) {
                completed = 0;
                return 0;
        }

        completed = 1;

        rv = sprintf(buffer, "Hello World\n"); //builds the string in a kernel buffer

        // copies the contents of buffer to userspace usr_buf
        copy_to_user(usr_buf, buffer, rv);

        return rv; //returns the number of bytes copied
}


/* Macros for registering module entry and exit points. */
module_init( proc_init );
module_exit( proc_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hello Module");
MODULE_AUTHOR("SGG");

