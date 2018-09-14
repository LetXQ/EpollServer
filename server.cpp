#include <iostream>
#include "include/epoll_server.h"

int main(int argc, char* argv[])
{
    EpollServer epoll_serv;
    int ret = epoll_serv.InitServer();
    if (ret != 0)
    {
        std::cout << "EpollServer Failed ret: [" << ret << "]\n";
        exit(-1);
    }
    epoll_serv.Run();
    return 0;
}
