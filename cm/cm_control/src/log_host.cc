
#include "log/Client.hh"
#include "log/Message.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <list>

using namespace cm::log;

class Logger {
public:
  Logger() {}
  Logger(unsigned id, unsigned sw, const char* path) : _id(id), _sw(sw), _path(path)
  {
    _open(time(0));
  }
  ~Logger() {}
public:
  unsigned id() const { return _id; }
  unsigned sw() const { return _sw; }
public:
  void record(Message& m) {
    time_t t = time(0);
    struct tm tm_v;
    localtime_r(&t,&tm_v);
    if (tm_v.tm_year != _tm.tm_year ||
	tm_v.tm_mon  != _tm.tm_mon  ||
	tm_v.tm_mday != _tm.tm_mday) {
      pclose(_f);
      _open(t);
    }
    m.set_header(t);
    fwrite(&m,sizeof(m),1,_f);
  }
private:
  void _open(time_t t)
  {
    localtime_r(&t,&_tm);

    char buff[128];
    sprintf(buff,"%s/cm%03d-%d.%04d%02d%02d.gz",
	    _path, _id, _sw,
	    _tm.tm_year+1900, _tm.tm_mon+1, _tm.tm_mday);

    struct stat s;
    int nexists = stat(buff,&s);
    int i=0;
    while(!nexists) {
      i++;
      sprintf(buff,"%s/cm%03d-%d.%04d%02d%02d.%d.gz",
	      _path, _id, _sw,
	      _tm.tm_year+1900, _tm.tm_mon+1, _tm.tm_mday, i);
      nexists = stat(buff,&s);
    }
    char pnam[256];
    sprintf(pnam,"gzip > %s",buff);
    _f = popen(pnam,"w");
    if (!_f)
      printf("Error opening file %s\n",buff);
    else
      printf("Opened pipe %s\n",pnam);
  }
private:
  unsigned    _id, _sw;
  const char* _path;
  FILE*       _f;
  struct tm   _tm;
};

int main(int argc, char** argv)
{
  const char* path = ".";

  extern char* optarg;
  int c;
  while ( (c=getopt( argc, argv, "p:")) != EOF ) {
    switch(c) {
    case 'p':
      path = optarg;
      break;
    default:
      printf("Usage: %s [-p <path>]\n",argv[0]);
      exit(1);
    }
  }

  std::list<Logger*> loggers;
  Client client;
  while(1) {
    Message& m = client.receive();

    Logger* l=0;
    std::list<Logger*>::iterator it=loggers.begin();
    do {
      if (it==loggers.end()) {
	l = new Logger(m.id(),m.sw(),path);
	loggers.push_back(l);
	break;
      }
      if ((*it)->id() == m.id() && (*it)->sw() == m.sw()) {
	l = *it;
	break;
      }
      it++;
    } while(1);
    l->record(m);
  }
}
