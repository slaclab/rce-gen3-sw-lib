/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP BDI Emulator File Server Driver.
**
**  Implementation of PGP BDI Emulator File Server driver.
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - March 09, 2017
**
**  Revision History:
**	None.
**
** --
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/stat.h>

#include "pgp/Pgp.h"
#include "BdiEm_Test.h"

#define BUFLEN   1472
#define HDRLEN   128  // max bytes in Xaui PPI header
#define PSLEEP   11   // 11-~211Mbyte/s, 12-~223Mbyte/s, 20-~311Mbyte/s
    
uint32_t _psleep = PSLEEP; // number of consecutive datagram posts before sleeping

char *_buf;

/*
** ++
**
**
** --
*/

static void timespec_diff(struct timespec *start, 
                          struct timespec *stop,
                          struct timespec *result)
  {
  
  if ((stop->tv_nsec - start->tv_nsec) < 0) 
    {
    result->tv_sec = stop->tv_sec - start->tv_sec - 1;
    result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    }
  else
  {
  result->tv_sec = stop->tv_sec - start->tv_sec;
  result->tv_nsec = stop->tv_nsec - start->tv_nsec;
  }

  return;
  }

/*
** ++
**
**
** --
*/
  
static int diep(char *s)
  {
  
  perror(s);
  return -1;
  }

/*
** ++
**
**
** --
*/

static int transmit(int s, struct sockaddr_in* si_other, int slen, char *filename, uint32_t link, uint32_t vc)
  {
  
  int size,total = 0;
  int reads = 0;
  int sleep = 0;
  struct stat st;
  struct timespec start_tv;   
  struct timespec end_tv;   
  struct timespec diff_tv;
  double tput,duration;
    
  FILE *fd = fopen(filename,"rb");
  if(!fd)
    {
    printf("%s: file open failure %s\n",__func__,filename);
    diep("fopen()");
    return 0;
    }

  fstat(fileno(fd),&st);

  printf("Fetch file %s %d bytes link %d\n",filename,(int)st.st_size,link);

  Pgp_Frame *frame = (Pgp_Frame*)_buf;
  Pgp_Header *pgp  = &frame->hdr;
  
  bzero(_buf,BUFLEN);

  clock_gettime(CLOCK_REALTIME,&start_tv);       
  
  while((size = fread(frame->payload+(HDRLEN-sizeof(UdpPacket)),1,BUFLEN-HDRLEN,fd)))
    {
    /* Encode a PGP header */
    pgp->ctl   = 0;
    pgp->fuser = 0;
    pgp->luser = link; // use luser field to encode link
    pgp->dst   = vc;
    pgp->size  = size;
    
    if(!total)
      pgp->fuser = PGP_SOF;
    
    if((total+size) == st.st_size)
      pgp->ctl = PGP_EOF;
      
    if (sendto(s, _buf, size+HDRLEN, 0, (const struct sockaddr*)si_other, slen)==-1)
      return diep("sendto()");
    
    total+=size;
    reads++;
    if(++sleep > _psleep)
      {
      usleep(1);
      sleep = 0;
      }
  
    bzero(_buf,BUFLEN);
    }

  clock_gettime(CLOCK_REALTIME,&end_tv);       

  timespec_diff(&start_tv,&end_tv,&diff_tv);

  printf("Sent %d file bytes in %d blocks to link %d vc %d\n",total,reads,link,vc);

  if(total)
    {
    printf("Transmit time %u sec %u nsec\n",
                  (int)(diff_tv.tv_sec), 
                  (int)(diff_tv.tv_nsec));
    tput = (total/1024/1024);
    duration = (diff_tv.tv_sec*1000000000)+diff_tv.tv_nsec;
    tput = tput/(0.000000001*duration);
    printf("Throughput %.3f MB/s %.3f Mbps\n",tput,tput*8);
    }
    
  fclose(fd);

  return total;
  }

/*
** ++
**
**
** --
*/
 
int main(int argc, char **argv)
  {
  int len;
  struct sockaddr_in si_me, si_other;
  uint32_t s, slen=sizeof(si_other);

  _buf = malloc(BUFLEN);

  if(argc >= 2)
    _psleep = strtoul(argv[1],0,0);

  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    return diep("socket");

  memset((char *) &si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(BDIEM_UDP_PORT);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s, (const struct sockaddr*)&si_me, sizeof(si_me))==-1)
      return diep("bind");

  while(1)
    {
    bzero(_buf,BUFLEN);

    printf("----------------------------------------------------------------\n");
    printf("Waiting for file request...\n");
    len = recvfrom(s, _buf, sizeof(BDIEM_Header), MSG_WAITALL, (struct sockaddr *)&si_other, &slen);
    if(len==-1)
      return diep("recvfrom()");
    printf("Received %d byte packet from %s:%d\n", 
           len, inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
    BDIEM_Header *hdr = (BDIEM_Header*)_buf;
    if(hdr->magic != BDIEM_UDP_MAGIC)
      continue;

    transmit(s, &si_other, slen, hdr->filename, hdr->link, hdr->vc);
    }

  close(s);  
  free(_buf);

  return 0;
  }
