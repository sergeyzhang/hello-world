#include "../../include/data_transmitter/DataTransmitter.h"

void sigchld_handler(int s) {
  // waitpid() might overwrite errno, so we save and restore it:
  int saved_errno = errno;
#ifdef __linux__
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
#endif
  errno = saved_errno;
}

void* get_in_addr(struct sockaddr* sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int DataTransmitter::run() const {
  std::cout << "INFO: transmission starts ... \n";
  int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
  sockfd = -10;
  new_fd = -10;
#ifndef __linux__
  SOCKET listen_socket = INVALID_SOCKET;
  SOCKET client_socket = INVALID_SOCKET;
  char recv_buf[512];
#else
  struct sockaddr_storage their_addr;  // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  char presentation[INET6_ADDRSTRLEN];
#endif

  struct addrinfo hints, *servinfo, *ptr;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;  // use my IP
  const char* PORT = "3490";
  int ret = getaddrinfo(nullptr, PORT, &hints, &servinfo);
  if (ret != 0) {
    std::cout << "ERROR: getaddrinfo() failed.\n";
#ifndef __linux__
    WSACleanup();
#endif
    return -1;
  }
  // loop through all the results and bind to the first we can
  int time_out = 1;
  for (ptr = servinfo; ptr != nullptr; ptr = ptr->ai_next) {
#ifndef __linux__
    listen_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (listen_socket == INVALID_SOCKET) {
      std::cout << "ERROR: socket() failed.\n";
      WSACleanup();
      return -1;
    }
    // Connect to server.
    ret = bind(listen_socket, servinfo->ai_addr, (int)servinfo->ai_addrlen);
    if (ret == SOCKET_ERROR) {
      std::cout << "ERROR: bind() failed.\n";
      freeaddrinfo(servinfo);
      closesocket(listen_socket);
      WSACleanup();
      return -1;
    }
#else
    sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (sockfd == -1) {
      std::cout << "INFO: socket() is generating sockfd.\n";
      continue;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &time_out, sizeof(int)) ==
        -1) {
      std::cout << "ERROR: setsockopt() failed.\n";
      return -1;
    }
    if (bind(sockfd, ptr->ai_addr, ptr->ai_addrlen) == -1) {
      close(sockfd);
      std::cout << "INFO: bind. \n";
      continue;
    }
#endif
    break;
  }
  freeaddrinfo(servinfo);  // all done with this structure
  if (!ptr) {
    std::cout << "ERROR: client failed to bind. \n";
    return -1;
  }
#ifndef __linux__
  if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
    std::cout << "ERROR: listen() failed.\n";
    closesocket(listen_socket);
    WSACleanup();
    return -1;
  }
#else
  if (listen(sockfd, SOMAXCONN) == -1) {
    std::cout << "ERROR: listen() failed.\n";
    return -1;
  }
  sa.sa_handler = sigchld_handler;  // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    std::cout << "ERROR: sigaction() failed.\n";
    return -1;
  }
#endif
  const char* send_buf = "hello from client";
  int flag = 0;   // 1 for connection; 0 for test
  while (flag) {  // main accept() loop
#ifndef __linux__
    client_socket = accept(sockfd, nullptr, nullptr);
    if (client_socket == INVALID_SOCKET) {
      std::cout << "ERROR: accept() failed.\n";
      closesocket(listen_socket);
      WSACleanup();
      return -1;
    }
    ret = recv(client_socket, recv_buf, sizeof(recv_buf) / sizeof(recv_buf[0]),
               0);
    if (ret > 0) {
      std::cout << "INFO: " << ret << " bytes received.\n";
    }
    ret = send(client_socket, recv_buf, sizeof(recv_buf) / sizeof(recv_buf[0]),
               0);
    std::cout << "INFO: " << ret << " bytes sent.\n";
    if (ret == SOCKET_ERROR) {
      std::cout << "ERROR: send() failed.\n";
      closesocket(client_socket);
      WSACleanup();
      return -1;
    } else if (ret == 0) {
      std::cout << "INFO: connection closing...\n";
    } else {
      std::cout << "ERROR: recv() failed.\n";
      closesocket(client_socket);
      WSACleanup();
      return -1;
    }
    flag = 0;  // send only once
#else
    std::cout << "INFO: wating for accept a client socket.\n";
    sin_size = sizeof(their_addr);
    new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
    std::cout << "INFO: accept " << new_fd << "\n";
    if (new_fd == -1) {
      std::cout << "INFO: accept\n";
      continue;
    }
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr),
              presentation, sizeof(presentation));
    std::cout << "INFO: get connection from " << presentation << "\n";
    if (!fork()) {    // this is the child process
      close(sockfd);  // child doesn't need the listener
      if (send(new_fd, send_buf, int(strlen(send_buf)), 0) == -1) {
        std::cout << "ERROR: send() failed.\n";
      }
      close(new_fd);
      exit(0);
    }
    close(new_fd);  // parent doesn't need this
    flag = 0;
#endif
  }
#ifndef __linux__
  if (shutdown(client_socket, SD_SEND) == SOCKET_ERROR) {
    std::cout << "ERROR: shutdown() failed.\n";
    closesocket(client_socket);
    WSACleanup();
    return -1;
  }
  closesocket(client_socket);
  WSACleanup();
#endif
  return 0;
}
