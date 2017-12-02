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

MigrateClient * InitMigrateClient(int service_identifier) {
  MigrateClient *migrate_client = new MigrateClient();

  int sock;
  struct sockaddr_un addr;

  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("SendDescriptor() socket");
    return NULL;
  }

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, MIGRATION_CLIENT_SOCKET_PATH, strlen(MIGRATION_CLIENT_SOCKET_PATH) + 1);

  if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    perror("SendDescriptor() connect");
    return NULL;
  }

  migrate_client->local_sock = sock;
  migrate_client->service_identifier = service_identifier;

  SendRegistration(migrate_client);

  pthread_t migrate_client_pthread;

  pthread_create(&migrate_client_pthread, NULL, HandleMigrateClient, (void *) migrate_client);

  return migrate_client;
}

void * HandleMigrateClient(void *c) {
  std::cout << "Starting Migration Client Service" << std::endl;
  MigrateClient *migrate_client = (MigrateClient *) c;

  char buf[MSG_BUFFER_SIZE];

  int in_bytes;

  while (1) {
    in_bytes = recv(migrate_client->local_sock, buf, MSG_BUFFER_SIZE, 0);
    if (in_bytes < 0) {
      perror("HandleMigrateClient() recv");
      // TODO: Clean up
      pthread_exit(NULL);
    } else if (in_bytes == 0) {
      // TODO: Clean up
      pthread_exit(NULL);
    }

    std::cout << "LOCALMSG: " << std::string(buf, in_bytes) << std::endl;

    int i = 0;

    while (i < in_bytes) {
      std::stringstream msg_size_ss;
      for (; i < in_bytes; i++) {
        if (buf[i] != ' ') {
          msg_size_ss << buf[i];
        } else {
          break;
        }
      }

      i++;

      std::string msg_size_str = msg_size_ss.str();
      int msg_size = std::stoi(msg_size_ss.str());
      if (msg_size == 6 and strncmp(buf + i, "SOCKET", 6) == 0) {
        i += 6;

        //Create socket
        int new_sock = socket(AF_INET, SOCK_STREAM, 0);
        migrate_client->remote_sock = new_sock;
        SendDescriptor(migrate_client, new_sock);
      }
    }
  }
}

bool SendDescriptor(MigrateClient *migrate_client, int fd) {
  std::stringstream msgstream;

  msgstream << "SOCKET " << migrate_client->service_identifier;

  std::string msg = msgstream.str();

  msgstream.str("");
  msgstream.clear();

  msgstream << msg.length() << " " << msg;

  msg = msgstream.str();

  if (send(migrate_client->local_sock, msg.c_str(), msg.length(), 0) < 0) {
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

    if (sendmsg(migrate_client->local_sock, &msghdr, 0) < 0) {
      perror("SendDescriptor() sendmsg");
      return false;
    } else {
      std::cout << "Descriptor sent" << std::endl;
      return true;
    }
  }
}

void SendRegistration(MigrateClient *migrate_client) {
  std::cout << "Registration client with Migration Client Service" << std::endl;
  std::stringstream msgstream;

  msgstream << "REG " << migrate_client->service_identifier;

  std::string msg = msgstream.str();

  msgstream.str("");
  msgstream.clear();

  msgstream << msg.length() << " " << msg;

  msg = msgstream.str();

  if (send(migrate_client->local_sock, msg.c_str(), msg.length(), 0) < 0) {
    perror("SendRegistration() send");
  }
}
