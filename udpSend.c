#include <linux/module.h>
#include <linux/init.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <net/sock.h>

#define SERVERPORT 5555
static struct socket *clientsocket=NULL;

static int __init client_init( void )
{
  int len;
  char buf[64];
  struct msghdr msg;
  struct iovec iov;
  mm_segment_t oldfs;
  struct sockaddr_in to;
  printk(KERN_ERR "sendthread initialized\n");
  if( sock_create( PF_INET,SOCK_DGRAM,IPPROTO_UDP,&clientsocket)<0 ){
    printk( KERN_ERR "server: Error creating clientsocket.n" );
    return -EIO;
   }
  memset(&to,0, sizeof(to));
  to.sin_family = AF_INET;
  to.sin_addr.s_addr = in_aton( "127.0.0.1" );  
      /* destination address */
  to.sin_port = htons( (unsigned short)
      SERVERPORT );
  memset(&msg,0,sizeof(msg));
  msg.msg_name = &to;
  msg.msg_namelen = sizeof(to);
  memcpy( buf, "hallo from kernel space", 24 );
  iov.iov_base = buf;
  iov.iov_len  = 24;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_iov    = &iov;
  msg.msg_iovlen = 1;
  // msg.msg_flags    = MSG_NOSIGNAL;
  printk(KERN_ERR " vor send\n");
  oldfs = get_fs();
  set_fs( KERNEL_DS );

	int i = 0;
	while ( i < 10){
		i++;
  		len = sock_sendmsg( clientsocket, &msg, 24 );
	}
//  len = sock_sendmsg( clientsocket, &msg, 24 );
  //len = sock_sendmsg( clientsocket, &msg, 24 );
  //len = sock_sendmsg( clientsocket, &msg, 24 );
  set_fs( oldfs );
  printk( KERN_ERR "sock_sendmsg returned: %d\n", len);
  return 0;
}

static void __exit client_exit( void )
{
  if( clientsocket )
    sock_release( clientsocket );
}

 module_init( client_init );
 module_exit( client_exit );
 MODULE_LICENSE("GPL");
