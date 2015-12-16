 #include <netlink/netlink.h>
 #include <netlink/genl/genl.h>
 #include <netlink/genl/ctrl.h>
#include <linux/errno.h>

enum {
	DOC_EXMPL_A_UNSPEC,
	DOC_EXMPL_A_MSG,
        __DOC_EXMPL_A_MAX,
};
#define DOC_EXMPL_A_MAX (__DOC_EXMPL_A_MAX - 1)

enum {
	DOC_EXMPL_C_UNSPEC,
	DOC_EXMPL_C_ECHO,
	__DOC_EXMPL_C_MAX,
};
#define DOC_EXMPL_C_MAX (__DOC_EXMPL_C_MAX - 1)

#define VERSION_NR 1



static int recv_msg(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlattr *attrs[DOC_EXMPL_A_MAX+1];

	/* in theory we could use genlmsg_parse
 	 * but on my system it does not work ->
 	 * we use nla_parse */
//	genlmsg_parse(nlh, 0, attrs, DOC_EXMPL_A_MAX, NULL);
	struct genlmsghdr *ghdr = nlmsg_data(nlh);
	if (nla_parse(attrs, DOC_EXMPL_A_MAX, genlmsg_attrdata(ghdr, 0),
			 genlmsg_attrlen(ghdr, 0), NULL) < 0) {
		printf("couldn't parse attributes\n");
		return -1;
	}

	/* the data is in the attribute DOC_EXMPL_A_MSG */
	if (attrs[DOC_EXMPL_A_MSG]) {
		char *value = nla_get_string(attrs[DOC_EXMPL_A_MSG]);
		printf("message received: %s\n", value);
	}
	else {
		printf("error receiving message\n");
	}

	return 0;
}

int main() {
	struct nl_handle *sock;
	struct nl_msg *msg;
	int family;

	/* Allocate a new netlink socket */
	sock = nl_handle_alloc();
	if(sock == NULL) {
		printf("couldn't allocate socket\n");
		goto out;
	}

	/* Connect to the generic netlink socket in the kernel */
	if (genl_connect(sock) < 0) {
		printf("error while connecting\n");
		goto out;
	}

	/* obtain the family id for the CONTROL_EXMPL family */
	family = genl_ctrl_resolve(sock, "CONTROL_EXMPL");
	if (family < 0) {
		printf("error obtaining family\n");
		goto out;
	}

	/* allocate a new message */
	msg = nlmsg_alloc();

	/* make the header */
	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family, 0, NLM_F_REQUEST, DOC_EXMPL_C_ECHO, VERSION_NR);
	
	/* add the data */
	nla_put_string(msg, DOC_EXMPL_A_MSG, "hello from userspace");

	/* send the message */
	if (nl_send_auto_complete(sock, msg) < 0) {
		printf("error sending message\n");
		goto out;
	}

	/* free the message */
	nlmsg_free(msg);

	/* set the callback function to receive answers to recv_msg */
	if (nl_socket_modify_cb(sock, NL_CB_MSG_IN, NL_CB_CUSTOM, recv_msg, NULL) < 0) {
		printf("error setting callback function\n");
		
		goto out;
	}

	/* receive the answer */
	nl_recvmsgs_default(sock); 

out:	
	nl_handle_destroy(sock);

	return 0;
}


