/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP BDI Emulator Client Driver.
**
**  Implementation of PGP BDI Emulator Client driver.
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
#include <sys/signal.h>

#include "pgp/Pgp.h"
#include "BdiEm_Test.h"

#define BUFLEN   8192

#define DATA_PORT 9931

#define NUM_DGRAMS 81737

#define NUM_BYTES 119988000

#define DEFAULT_RCVBUF (1024 * 4192)

#define CHECK_SEQ

//#define WRITE_FILE

char *_buf;

uint32_t trg     = 0;
uint32_t seq_err = 0;
uint32_t _seq    = 0;

uint32_t total = 0;
uint32_t npack = 0;

FILE *fd;

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

void my_handler(int s)
  {
  
  printf("\nRx: EXIT AT %d bytes %d datagrams trg %d seqerr %d\n",total,npack,++trg,seq_err);
  exit(1); 
  }


/*
** ++
**
**
** --
*/
 
int main(int argc, char **argv)
  {  
  int start = 0;
  
  struct timespec start_tv;   
  struct timespec end_tv;   
  struct timespec diff_tv;
  double tput,duration;

#ifdef WRITE_FILE
  char filestem[256];
#endif
  
  int len;
  struct sockaddr_in si_me, si_other;
  uint32_t s, slen=sizeof(si_other);

  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);

  _buf = malloc(BUFLEN);

  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    return diep("socket");

  int ssize = DEFAULT_RCVBUF;
  int result = setsockopt(s,SOL_SOCKET,SO_RCVBUF,&ssize,sizeof(ssize));
  if(result)
    return diep("setsockopt");

  memset((char *) &si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(DATA_PORT);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s, (const struct sockaddr*)&si_me, sizeof(si_me))==-1)
      return diep("bind");
  
#ifdef WRITE_FILE
  if(argc >= 2)
    {
    sprintf(filestem,"%s_%d.bin",argv[1],trg);
    fd = fopen(filestem,"wb");
    }
#endif  

  while(1)
    {
    len = recvfrom(s, _buf, BUFLEN, MSG_WAITALL, (struct sockaddr *)&si_other, &slen);
    if(len==-1)
      return diep("recvfrom()");
    
    //printf("Received %d byte packet from %s:%d\n", 
    //       len, inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));

#ifdef WRITE_FILE
    if(argc >= 2)
      fwrite(_buf,len,1,fd);
#endif
    
#ifdef CHECK_SEQ 
    uint32_t *seq = (uint32_t*)_buf;
    if(seq[1] != _seq)
      {
      printf("SEQUENCE ERROR: GOT 0x%x EXP 0x%x DROPPED %d TOTAL %d\n",seq[1],_seq,seq[1] - _seq,seq_err);
      if(seq[1] > _seq)
        {
        seq_err += (seq[1] - _seq);
        npack += (seq[1] - _seq);
        }
      _seq = seq[1];
      }
#endif

    ++_seq; 
    ++npack;
    total+=len;
      
    if(!start)
      clock_gettime(CLOCK_REALTIME,&start_tv);       
    start = 1;
    
    if(total == NUM_BYTES)
      {
      if(npack > NUM_DGRAMS)
        printf("NPACK EXCEEDS EXPECTED MAX %d\n",npack);
        
      clock_gettime(CLOCK_REALTIME,&end_tv);       

      timespec_diff(&start_tv,&end_tv,&diff_tv);

      printf("Rx: %d bytes %d datagrams trg %d seqerr %d in %u sec %u nsec\n",total,npack,trg,seq_err,
                    (int)(diff_tv.tv_sec), 
                    (int)(diff_tv.tv_nsec));
      ++trg;

#ifdef WRITE_FILE
      if(argc >= 2)
        {
        fclose(fd);
        sprintf(filestem,"%s_%d.bin",argv[1],trg);
        fd = fopen(filestem,"wb");
        }
#endif
            
      tput = (total/1024/1024);
      duration = (diff_tv.tv_sec*1000000000)+diff_tv.tv_nsec;
      tput = tput/(0.000000001*duration);
      printf("Throughput: %.3f MB/s %.3f Mbps\n",tput,tput*8);
      
      total = 0;
      npack = npack - NUM_DGRAMS;
      start = 0;
      }
    else if(total > NUM_BYTES)
      printf("DROPPED FRAMES AT %d bytes\n",total);
    }
  return 0;
  }
