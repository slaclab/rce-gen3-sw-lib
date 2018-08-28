
#include "cm_shell/ShellCommon.hh"
#include "libtelnet.h"
#include "cm_svc/Print.hh"

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <stdarg.h>
#include <termios.h>

#include <list>
using std::list;

static const telnet_telopt_t my_telopts[] = {
  { TELNET_TELOPT_ECHO,      TELNET_WILL,           0 },
  { TELNET_TELOPT_LINEMODE,            0, TELNET_DO   },
  //  { TELNET_TELOPT_NAOCRD,    TELNET_WILL, 0           },
  { -1, 0, 0 }
};

void ttySetCharNoEcho(bool set) {
    static struct termios org_mode;
    static struct termios mode;
    static bool saved = false;

    if (set && !saved) {
        tcgetattr(0, &mode);
        org_mode = mode;
        saved = true;
        mode.c_iflag &= ~IXON;
        mode.c_lflag &= ~ICANON;
        mode.c_lflag &= ~ECHO;
        mode.c_cc[VMIN] = 1;
        tcsetattr(0, TCSANOW, &mode);
    } else if (saved) {
        tcsetattr(0, TCSANOW, &org_mode);
    }
}

namespace cm {
  namespace shell {
    class Fd {
    public:
      Fd(int fd) : _fd(fd) {}
      virtual ~Fd() { close(_fd); }
      virtual Fd* process() = 0;
      int fd() const { return _fd; }
    private:
      int _fd;
    };

    class ClientFd : public Fd,
                     public AbsConsole {
    public:
      ClientFd(int, Command&);
      void push(const char*,int);
    public: // Fd interface
      Fd*  process();
    public: // AbsConsole interface
      void printv(const char* msgformat, ...);
      void reserve(unsigned) {}
    private:
      static void telnet_eh(telnet_t *telnet, 
                            telnet_event_t *event, 
                            void *user_data);
      Command&  _cmd;
      telnet_t* _telnet;
      char      _buff[4096];
      int       _len;
    };

    class AcceptFd : public Fd {
    public:
      AcceptFd(int port, Command& cmd) :
        Fd(socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)),
        _cmd(cmd) 
      {
        if (fd()<0) return;

        int optval=1;
        setsockopt(fd(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
          
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        addr.sin_addr.s_addr = htonl( INADDR_ANY );

        if (bind(fd(), (struct sockaddr*)&addr, sizeof(addr))<0)
          perror("cm::shell listening socket bind failed");

        listen(fd(),5);
      }
      Fd* process() {
        int newfd;
        struct sockaddr addr;
        socklen_t len = sizeof(addr);

        newfd = accept( fd(), &addr, &len);
        if (newfd >= 0)
          return new ClientFd(newfd,_cmd);
        return this;
      }
    private:
      Command& _cmd;
    };
  }
};

using namespace cm::shell;

ClientFd::ClientFd(int nfd, Command& cmd) : Fd(nfd), _cmd(cmd), _len(0) {
  int optval=1;
  setsockopt( fd(), SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval) );

  _telnet = telnet_init(my_telopts, telnet_eh, 0, this);
  
  for (int i = 0; my_telopts[i].telopt >= 0; i++) {
    if (my_telopts[i].him > 0)
      telnet_negotiate(_telnet, my_telopts[i].him, my_telopts[i].telopt);
    if (my_telopts[i].us > 0)
      telnet_negotiate(_telnet, my_telopts[i].us, my_telopts[i].telopt);
  }

  cmd.open(this);
}

void ClientFd::push(const char* buffer, int sz)
{
// write(fd(), buffer,sz);
  telnet_send(_telnet, buffer, sz);

#define MY_CR    0x0d
#define MY_BKSP  0x08

  for(int i=0; i<sz; i++) {
    switch(buffer[i]) {
    case MY_CR:
      { char* argv[16];
        int k=0;
        char* p;
        _buff[_len] = 0;
        while( (argv[k] = strtok_r( k==0 ? _buff : NULL, " ", &p) ) )
          k++;
        
        _cmd.process(k,argv,this);
        _len = 0;
      } break;
    case MY_BKSP:
      if (_len>0) _len--;
      break;
    case 0:
      break;
    default:
      _buff[_len++] = buffer[i];
      break;
    }
  }
}

Fd* ClientFd::process() {
  char buf[1600];
  int  len;

  len = read(fd(), buf, sizeof(buf)-1);

  if (len == 0) {
//    printf("Empty read for client\n");
    return NULL;
  } else if (len < 0) {
    perror("ClientFd::process");
    return NULL;
  } else {
    telnet_recv(_telnet, buf, len);
  }

  return this;
}

void ClientFd::printv(const char* msgformat, ...)
{
  va_list ap; 
  va_start(ap, msgformat); 
  telnet_vprintf(_telnet, msgformat, ap);
  va_end(ap); 
}

void ClientFd::telnet_eh(telnet_t* telnet,
                         telnet_event_t* event,
                         void* user_data)
{
  ClientFd* client = (ClientFd*)user_data;
  switch(event->type) {
  case TELNET_EV_DATA:
    client->push(event->data.buffer, event->data.size);
    break;
  case TELNET_EV_SEND:
    { int status=0;
      while( (status = write(client->fd(), event->data.buffer, event->data.size)) == -1 &&
             errno == EINTR);
      if (status == -1) { close(client->fd()); } 
    }
    break;
  case TELNET_EV_ERROR:
    fprintf(stderr, "TELNET error: %s", event->error.msg);
    break;
  case TELNET_EV_WILL: /* subnegotiate */
    //    printf("telnet_eh: TELNET_EV_WILL %d\n",*p);
    //    break;
  default:
    //    printf("telnet_eh: ev.type %d  ev.neg.telopt %d\n",
    //           event->type, event->neg.telopt);
    break;
  }
}

void* cm::shell::initialize( void* arg )
{
  Command& cmd = *reinterpret_cast<Command*>(arg);

  list<Fd*> conns;

  //
  //  Create a listening socket for accepting connections
  //
  conns.push_back(new AcceptFd(30000,cmd));  
  while(1) {
    struct pollfd fds[32];
    int nfds=0;

    for(list<Fd*>::iterator it=conns.begin(); it!=conns.end(); it++,nfds++) {
      fds[nfds].fd = (*it)->fd();
      fds[nfds].events = POLLIN;
      fds[nfds].revents = 0;
    }

    int v = poll(fds, nfds, -1);
    if (v > 0) {
      nfds=0;
      for(list<Fd*>::iterator it=conns.begin(); it!=conns.end(); it++,nfds++) {
        if (fds[nfds].revents & (POLLERR|POLLHUP)) {
          conns.remove(*it);
          break;
        }
        if (fds[nfds].revents & POLLIN) {
          Fd* fd = (*it)->process();
          if (fd==*it) continue;
          if (fd==0) conns.remove(*it);
          else       conns.push_back(fd);
          break;
        }
      }
    }
    else if (v < 0) {
    }
  }
  return 0;
}
