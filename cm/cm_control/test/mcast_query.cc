/***
 *
 * qi3 - generate IGMP queries
 *
 * Usage:	qi3
 *
 * File:	qi3.c
 * Date:	28/4/2000
 * Auth:	wilbertdg@hetnet.nl
 *
 ***/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

typedef u_int32_t n_long;
#include <netinet/ip.h>

#define ALL_SYSTEMS				"224.0.0.1"
#define ALL_ROUTERS				"224.0.0.2"
#define ALL_IGMPV3_ROUTERS		"224.0.0.22"

#define	NUM_SOURCES		50

#define	MRT		100				/* max response time */
#define	RBV		2				/* robustness variable */
#define	QQI		125				/* querier query interval */

/*
 * ---------------------------------------------------------------------------- 
 *
 * protoypes
 *
 * ---------------------------------------------------------------------------- 
 */

void catch_sig(int);
unsigned short in_cksum(unsigned short*, int);
void print_igmp(char*, int);
void igmpv1_query(int, char*, struct in_addr);
void igmpv2_query(int, char*, struct in_addr, char*);
void igmpv3_query(int, char*, struct in_addr, char*, struct in_addr*, int);

/*
 * ---------------------------------------------------------------------------- 
 *
 * igmp structs
 *
 * ---------------------------------------------------------------------------- 
 */

struct igmpq {
	u_char	iq_type;	
	u_char	iq_mrt;	
	u_short	iq_cs;
	struct in_addr iq_group;
};

struct igmpq_v3 {
	u_char	iq_type;	
	u_char	iq_mrt;	
	u_short	iq_cs;
	struct in_addr iq_group;
	u_char	iq_rsv;
	u_char	iq_qqi;
	u_short	iq_nsrcs;
	struct in_addr iq_srcs[NUM_SOURCES];
};

int sockfd;
struct in_addr sources[NUM_SOURCES];

/*
 * ---------------------------------------------------------------------------- 
 *
 * main() 
 *
 * ---------------------------------------------------------------------------- 
 */
int
main(int argc, char** argv)
{
	int n;
	char buf[256];
	struct in_addr all_systems;
	char version;
	char group[20];
	char ra[4];

	/*
	 * initialze
	 */
	inet_aton(ALL_SYSTEMS,&all_systems);
	for (n=0; n<NUM_SOURCES; n++) {
	  inet_aton("139.63.1.23",&sources[n]);
	  sources[n].s_addr = htonl(ntohl(sources[n].s_addr)+n);
	}

	/*
	 * create raw socket
	 */
	sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_IGMP);
	if (sockfd < 0) {
		perror("Failed to create raw socket. Try again as root.");
		exit(0);
	}
	/*
	int result = ::bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr));
	if (result < 0)
	  printf("Error binding RemoteConsole socket: %s\n", strerror(result));
	else {
	  sockaddr_in name;
	  socklen_t name_len=sizeof(name);
	  getsockname(_sockfd,(sockaddr*)&name,&name_len); 
	  printf("Socket %d bound to %lu.%lu.%lu.%lu/%d (%d)\n",
		 _sockfd,
		 (name.sin_addr.s_addr>>24)>> 0,(name.sin_addr.s_addr<< 8)>>24,
		 (name.sin_addr.s_addr<<16)>>24,(name.sin_addr.s_addr<<24)>>24,
		 name.sin_port,(int)name_len);
	}
	*/

	/* 
	 * set router alert 
	 */
	ra[0] = 148;
	ra[1] = 4;
	ra[2] = 0;
	ra[3] = 0;
	if (setsockopt(sockfd, IPPROTO_IP, IP_OPTIONS, ra, 4) == 0)
		printf("router-alert option set\n");
	else
		printf("failed to set router-alert option\n");
	
	signal(SIGINT, catch_sig);

	/*
	 * send out queries
	 */
	while (1) {
		printf("version: ");
		scanf("%s", group);
		version = group[0];
		printf("version = %c\n", version);
		switch (version) {
		case 'q':
			goto quit;
		case '1':
			igmpv1_query(sockfd, buf, all_systems);
			break;
		case '2': 
			printf("group: ");
			scanf("%s", group);
			printf("group = %s\n", group);
			igmpv2_query(sockfd, buf, all_systems, group);
			break;
		case '3':
			printf("group: ");
			scanf("%s", group);
			printf("group = %s\n", group);
			printf("number: ");
			scanf("%s", buf);
			n = atoi(buf);
			printf("number = %d\n", n);
			if (n >= 0 && n <= NUM_SOURCES) {
				igmpv3_query(sockfd, buf, all_systems, group, sources, n);
				printf("igmpv3 query transmitted\n"); 
				print_igmp(buf, 12);
			}
			else
				printf("wrong number [1..%d]\n", NUM_SOURCES);
			break;
		default:
			printf("wrong version [1..3]\n");
		}	
	}

quit:
	close(sockfd);
	return(0);
}

/*
 * ---------------------------------------------------------------------------- 
 *
 * catch_sig 
 *
 * ---------------------------------------------------------------------------- 
 */
void catch_sig(int sig_num)
{
	printf("\nterminated.\n");
	close(sockfd);
	exit(0);
}

/*
 * ---------------------------------------------------------------------------- 
 *
 * in_cksum 
 *
 * ---------------------------------------------------------------------------- 
 */
unsigned short in_cksum(unsigned short *addr, int len)
{
	int nleft = len;
	int sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}
	if (nleft == 1) {
		*(u_char*)(&answer) = *(u_char*)w;
		sum += answer;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	answer = ~sum;
	return (answer);
}

/*
 * ---------------------------------------------------------------------------- 
 *
 * print_igmp 
 *
 * ---------------------------------------------------------------------------- 
 */
void print_igmp(char* p,int n)
{
	struct igmpq_v3 *igh = (struct igmpq_v3*) p;
	int i;

	printf("\ttype  = %02x\n", igh->iq_type);
	printf("\tmrt   = %d\n", igh->iq_mrt);
	printf("\tcs    = %d\n", igh->iq_cs);
	printf("\tgroup = %s\n", inet_ntoa(igh->iq_group));
	if (n >= 12) {
		printf("num    = %d\n", igh->iq_nsrcs);
		for (i=0; i<ntohs(igh->iq_nsrcs); i++)
			printf("\t\t%s\n", inet_ntoa(igh->iq_srcs[i]));
	}
}

/*
 * ---------------------------------------------------------------------------- 
 *
 * igmpv1 query
 *
 * ---------------------------------------------------------------------------- 
 */
void igmpv1_query(int s,char* buf,struct in_addr dst)
{
	struct igmpq *igh;
	struct sockaddr_in sa;

	igh = (struct igmpq*) buf;
	igh->iq_type = 17;		/* query */
	igh->iq_mrt = 0;
	igh->iq_cs = 0;
	igh->iq_group.s_addr = 0;
	igh->iq_cs = in_cksum((u_short*)igh, sizeof(*igh));
	sa.sin_addr.s_addr = dst.s_addr;
	sendto(sockfd, buf, sizeof(*igh), 0, (struct sockaddr*)&sa, sizeof(sa));
}
 
/*
 * ---------------------------------------------------------------------------- 
 *
 * igmpv2 query
 *
 * ---------------------------------------------------------------------------- 
 */
void igmpv2_query(int s, char *buf, struct in_addr dst, char *group)
{
	struct igmpq *igh;
	struct sockaddr_in sa;

	igh = (struct igmpq*) buf;
	igh->iq_type = 17;		/* query */
	igh->iq_mrt = MRT;
	igh->iq_cs = 0;
	igh->iq_group.s_addr = inet_addr(group);
	igh->iq_cs = in_cksum((u_short*)igh, sizeof(*igh));
	sa.sin_addr.s_addr = dst.s_addr;
	sendto(sockfd, buf, sizeof(*igh), 0, (struct sockaddr*)&sa, sizeof(sa));
}

/*
 * ---------------------------------------------------------------------------- 
 *
 * igmpv3 query
 *
 * ---------------------------------------------------------------------------- 
 */
void igmpv3_query(int s,char *buf,struct in_addr dst,char *group,struct in_addr *srcs,int num)
{
	struct igmpq_v3 *igh;
	struct sockaddr_in sa;
	int i, size;

	igh = (struct igmpq_v3*) buf;
	igh->iq_type = 17;		/* query */
	igh->iq_mrt = MRT;
	igh->iq_cs = 0;
	igh->iq_group.s_addr = inet_addr(group);
	igh->iq_rsv = RBV;
	igh->iq_qqi = QQI;
	igh->iq_nsrcs = htons(num);
	for (i=0; i<num; i++)
		igh->iq_srcs[i].s_addr = srcs[i].s_addr;
	size = sizeof(*igh) + (num-1) * sizeof(struct in_addr); 
	printf("size = %d\n", size);
	igh->iq_cs = in_cksum((u_short*)igh, size);
	sa.sin_addr.s_addr = dst.s_addr;
	sendto(sockfd, buf, size, 0, (struct sockaddr*)&sa, sizeof(sa));
}

/*
struct igmpq_v3 {
	u_char	iq_type;	
	u_char	iq_mrt;	
	u_short	iq_cs;
	struct in_addr iq_group;
	u_char	iq_rsv;
	u_char	iq_qqi;
	u_short	iq_nsrc;
	struct in_addr iq_srcs[1];
};
*/

 
