#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <chrono>

#include "libmigrateclient.h"

int main() {
  int sock;

  struct sockaddr_in addr;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("sock error");
  }

  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(12345);

  if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    perror("bind error");
  }

  SendDescriptor(15000, sock);

  char ch;

  std::cin >> ch;
}
