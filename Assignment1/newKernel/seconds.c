/*
Linux distribution info (lsb_release -a)
No LSB modules are available.
Distributor ID:	Ubuntu
Description:	Ubuntu 25.10
Release:	25.10
Codename:	questing

Kernel version (uname -r)
6.17.0-8-generic

*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

//including jiffies and HZ for assignment 1
#include <linux/jiffies.h>

#define BUFFER_SIZE 128

#define PROC_NAME "seconds"
//#define MESSAGE "Number of elapsed seconds:\n"

//track the start time of the module 
static unsigned long start_jiffies;


int proc_init(void);
ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos);
void proc_exit(void);

/**
 * Function prototypes
 */

static const struct proc_ops my_proc_ops = {
        .proc_read = proc_read,
};

/* 
This function is called when the module is loaded.
when insmod is called:
*/
int proc_init(void)
{

        start_jiffies = jiffies;
        proc_create(PROC_NAME, 0, NULL, &my_proc_ops);

        printk(KERN_INFO "/proc/%s created\n", PROC_NAME);

	return 0;
}

/* This function is called when the module is removed. */
void proc_exit(void) {

        // removes the /proc/seconds
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
 */
ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
        int rv = 0;
        char buffer[BUFFER_SIZE];
        static int completed = 0;

        if (completed) {
                completed = 0;
                return 0;
        }

        completed = 1;

        unsigned long time_elapsed = (jiffies - start_jiffies) / HZ;
        rv = snprintf(buffer, BUFFER_SIZE, "Seconds elapsed: %lu\n", time_elapsed);

        // copies the contents of buffer to userspace usr_buf
        copy_to_user(usr_buf, buffer, rv);

        return rv;
}


/* Macros for registering module entry and exit points. */
module_init( proc_init );
module_exit( proc_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Seconds Elapsed for a module");
MODULE_AUTHOR("SGG");

