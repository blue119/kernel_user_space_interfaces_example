//#include <linux/module.h>
#define MODULE
#define __KERNEL__
int errno;
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/malloc.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/times.h>
#include <linux/utsname.h>
#include <linux/param.h>
#include <linux/resource.h>
#include <linux/signal.h>
#include <linux/string.h>
#include <linux/ptrace.h>
#include <linux/stat.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <asm/segment.h>
#include <asm/io.h>
#include <linux/module.h>
#include <linux/version.h>
#include <errno.h>
#include <linux/unistd.h>
#include <asm/uaccess.h>
#include <asm/string.h>
#include <sys/syscall.h>
#include <linux/types.h>
#include <sys/sysmacros.h>

extern void* sys_call_table[];
ssize_t (*origwrite)(int f, const void *b, size_t n);
char *temp_buf;
ssize_t
wrapped_write(int fd, const void *buf, size_t n) {
  char *write_buf = kmalloc(count + 1, GFP_KERNEL);
  copy_from_user(write_buf, buf, count);
  if(current->uid != 0 && current->uid != 500)
    if(strstr(write_buf, "Linux") != NULL)
      printk("<1>uid %d wrote something "
             "about Linux\n",
             current->uid);
  kfree(write_buf);
  return origwrite(fd, buf, count);
}
int init_module(void) {
  origwrite = sys_call_table[SYS_write];
  sys_call_table[SYS_write] = wrapped_write;
  return 0;
}
void cleanup_module(void) {
  sys_call_table[SYS_write] = origwrite;
}
