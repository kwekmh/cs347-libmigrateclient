#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <cstdlib>

#include "libmigrateclient.h"

bool SendDescriptor(int service_identifier, int fd) {
  int sock;
  struct sockaddr_un addr;

  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("SendDescriptor socket");
    return false;
  }

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, MIGRATION_CLIENT_SOCKET_PATH, strlen(MIGRATION_CLIENT_SOCKET_PATH) + 1);

  if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    perror("SendDescriptor() connect");
    return false;
  }

  std::stringstream msgstream;

  msgstream << "SOCKET " << service_identifier;

  std::string msg = msgstream.str();

  msgstream.str("");
  msgstream.clear();

  msgstream << msg.length() << " " << msg;

  msg = msgstream.str();

  if (send(sock, msg.c_str(), msg.length(), 0) < 0) {
    perror("SendDescriptor() send");
    return false;
  } else {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Sending descriptor " << fd << std::endl;
    struct msghdr msghdr;
    char nothing = '!';
    struct iovec nothing_ptr;
    struct cmsghdr *cmsghdr;

    //char buf[SOCKET_BUFFER_MAX_SIZE];
    struct {
      struct cmsghdr h;
      int fd[1];
    } buf;

    nothing_ptr.iov_base = &nothing;
    nothing_ptr.iov_len = 1;
    msghdr.msg_name = NULL;
    msghdr.msg_namelen = 0;
    msghdr.msg_iov = &nothing_ptr;
    msghdr.msg_iovlen = 1;
    msghdr.msg_flags = 0;
    msghdr.msg_control = &buf;
    msghdr.msg_controllen = sizeof(struct cmsghdr) + sizeof(int);
    cmsghdr = CMSG_FIRSTHDR(&msghdr);
    cmsghdr->cmsg_len = msghdr.msg_controllen;
    cmsghdr->cmsg_level = SOL_SOCKET;
    cmsghdr->cmsg_type = SCM_RIGHTS;

    ((int *) CMSG_DATA(cmsghdr))[0] = fd;

    if (sendmsg(sock, &msghdr, 0) < 0) {
      perror("SendDescriptor() sendmsg");
      return false;
    } else {
      std::cout << "Descriptor sent" << std::endl;
      return true;
    }
  }
}
