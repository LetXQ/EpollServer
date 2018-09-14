#ifndef EPOLL_CLIENT_H_
#define EPOLL_CLIENT_H_
#include "../include/common_def.h"

class EpollClient
{
public:
    EpollClient(int32_t port, const char* ip);
    ~EpollClient();
    int32_t Run();
    void AddCmd(uint8_t type, CmdBase* p_cmd);
    const TcpClient* GetClntPtr() const;
private:
    int32_t ConnectToServer();
    int32_t SendDataToServer();
    int32_t ReadDataFromServer();
    bool DelEpoll();
    void Disconnect();
    int32_t DoCmd(Packet& packet);
private:
    int32_t m_i_epollfd = 0;
    int32_t m_i_port = 0;
    int32_t m_user_id = 0;
    int32_t m_sockfd = 0;
    TcpClient* m_p_clnt = nullptr;
    char m_ip[100];
    CmdPtrMap m_cmd_map;
};

#endif // !EPOLL_CLIENT_H_
