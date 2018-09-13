#ifndef EPOLL_SERVER_H_
#define EPOLL_SERVER_H_
#include <sys/epoll.h>

#include "../include/common_def.h"

class EpollServer
{
public:
    EpollServer();
    ~EpollServer();

    int32_t InitServer(int32_t port = 8888, const char* ip = nullptr);
    void Run();
    void AddCmd(int32_t type, CmdBasePtr p_cmd);
    int32_t ClntLogin(TcpClient* p_clnt, int32_t user_id,  int32_t last_read_id);
    void Forward(TcpClient*p_clnt,  int32_t user_id,  const std::string& data);
private:
    int32_t SetNonBlock();

    void ExecEvent(struct epoll_event& ev);
    void DelClntFromEpoll(TcpClient* p_clnt);
    void AddClntToEpoll(TcpClient* p_clnt);

    void ExecNewClient();
    void ExecErr(TcpClient* p_clnt);
    void ExecRead(TcpClient* p_clnt);
    void ExecWrite(TcpClient* p_clnt);

    int32_t ExecCmd(TcpClient* p_clnt, Packet& packet);
    int32_t Reconnect(TcpClient* p_old, TcpClient* p_new, int32_t last_read_id);
private:
    int32_t m_i_epollfd = -1;
    int32_t m_i_sockfd = -1;
    CmdPtrMap m_p_cmd_map;
    ClientPtrMap m_p_client_map;
};

#endif // !EPOLL_SERVER_H_
