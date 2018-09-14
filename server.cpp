#include <iostream>
#include "include/epoll_server.h"
#include "include/cmd_handler.h"
#include "include/cmd_def.h"

int main(int argc, char* argv[])
{
    EpollServer epoll_serv;
    int ret = epoll_serv.InitServer();
    if (ret != 0)
    {
        std::cout << "EpollServer Failed ret: [" << ret << "]\n";
        exit(-1);
    }
    CmdBase* p_login = new LoginHandler;
    epoll_serv.AddCmd(CMD_TYPE_LOGIN, p_login);
    epoll_serv.Run();
    return 0;
}
