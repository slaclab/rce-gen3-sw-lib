
#include "control/CmFwUploadTask.hh"

#include "console/UploadCmd.hh"

#include "xcf/Manager.hh"

#include "debug/Print.hh"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>

static const unsigned CM_FW_UPLOAD_PORT = 1201;


CmFwUploadTask::CmFwUploadTask()
{
}

CmFwUploadTask::~CmFwUploadTask()
{
}

void CmFwUploadTask::run()
{
  int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in servaddr, cliaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_port        = htons(CM_FW_UPLOAD_PORT);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  int error = ::bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr));
  if (error) {
    service::debug::printv("*** error binding: %s", strerror(errno));
  }
  error = ::listen(sockfd, 5);
  if (error) {
    service::debug::printv("*** error listening: %s", strerror(errno));
  }

  while (true) {
    service::debug::printv("CmFwUploadTask::wait for connection");

    socklen_t addrlen = sizeof(cliaddr);
    int connfd = ::accept(sockfd, (sockaddr*)&cliaddr, &addrlen);
    service::debug::printv("CmFwUploadTask connected from 0x%x port %d\n",
                           ntohl(cliaddr.sin_addr.s_addr), ntohs(cliaddr.sin_port));

    //    int rcvsize = 16*1024*1024; // 16MB limit
    //    int err = ::setsockopt(connfd, SOL_SOCKET, SO_RCVBUF,
    //			         (char*)&rcvsize, (int)sizeof(rcvsize));
    //    if (err)
    //      service::debug::printv("Error %d (%s) setting socket %d receive size\n",
    //		                   err, strerror(errno), connfd);

    UploadCmd cmd;
    int bytes = ::read(connfd, &cmd, sizeof(cmd));
    if (bytes == sizeof(cmd)) {
      // Byte swap
      { unsigned* d = (unsigned*)&cmd;
      for(unsigned k=0; k<sizeof(cmd)>>2; k++,d++)
	*d = ntohl(*d);
      }

      service::debug::printv("CmFwUploadTask receiving file %x of len %x",
                             cmd.filenum(),cmd.length());

      char* img_data = new char[cmd.length()+1];
      if (img_data) {
	int remaining = cmd.length();
	int offset = 0;
	while( remaining > 0 ) {
	  int readsz = (remaining < 1024*1024) ? remaining : 1024*1024;
	  bytes = ::read(connfd, img_data + offset, readsz);
	  if (bytes < 0) {
	    service::debug::printv("CmFwUploadTask load error %x/%x size %x img_data %p",
                                   offset,cmd.length(),bytes,img_data);
	    service::debug::printv("%s", strerror(errno));
	    break;
	  }
	  offset += bytes;
	  remaining -= bytes;
	}
	if (remaining==0) {
	  img_data[cmd.length()] = 0;
	  cm::xcf::Manager* mgr = new cm::xcf::Manager;
	  mgr->execute(img_data,cmd.length());
	  delete mgr;
	}
	delete[] img_data;
      }
      else
	service::debug::printv("CmFwUploadTask error allocating");
    }
    else
      service::debug::printv("CmFwUploadTask read error size %x : %s",
                             bytes,strerror(errno));

    service::debug::printv("connection from 0x%x port %d closed\n",
                           ntohl(cliaddr.sin_addr.s_addr),
                           ntohs(cliaddr.sin_port));
    ::close(connfd);
  }
  ::close(sockfd);
}
