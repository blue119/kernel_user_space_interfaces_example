#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#include <linux/genetlink.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/mngt.h>

int main() {
	struct nl_handle *handle;
	struct nl_msg *msg;
	struct myhdr {
		char mychar[20];        
	} *hdr;
	int id;

	handle = nl_handle_alloc();
	if (handle == NULL)
	    goto open_failure;

	if (genl_connect(handle) != 0)
	    goto open_failure;

	id = genl_ctrl_resolve(handle, "CONTROL_EXMPL"); 
	if(id < 0) {
		perror("genl_ctrl_resolve\n");
		goto open_failure;
	}
	else
		printf("id %i\n", id);

	
	msg = nlmsg_alloc();
	memset(msg, 0, 16);
	int pid = getpid();
	int seq = 1;

	hdr = genlmsg_put(msg, pid, seq, id, sizeof(struct myhdr), NLM_F_REQUEST, 1, 1);

	memcpy(hdr->mychar, "hello world", strlen("hello world") + 1);

	int ret = nl_send(handle, msg);
	printf("message sent %i\n", ret);
	nlmsg_free(msg);

	return 0;

open_failure:
	if (handle) {
		nl_close(handle);
		nl_handle_destroy(handle);
	}
	printf("Erreur open_failure\n");
	return 0;

}




