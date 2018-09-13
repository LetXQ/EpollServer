#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>

#include "../include/epoll_server.h"
#include "../include/common_notice.h"
#include "../include/tcp_client.h"
#include "../include/cmd_def.h"

EpollServer::EpollServer()
{

}

EpollServer::~EpollServer()
{

}

int32_t EpollServer::InitServer(int32_t port, const char* ip)
{
    m_i_epollfd = epoll_create(MAX_SOCKFD_NUM);
    int32_t ret = fcntl(m_i_epollfd, F_SETFL, O_NONBLOCK);

    if (-1 == ret)
        return ERR_FCNTL_FAILED;

    m_i_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (-1 == m_i_sockfd)
        return ERR_SOCKET_FAILED;

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (!ip)
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    int32_t i_reuse_addr = 1;
    ret = setsockopt(m_i_sockfd, SOL_SOCKET, SO_REUSEADDR, &i_reuse_addr, sizeof(i_reuse_addr));
    if (-1 == ret)
        return ERR_SETSOCKOPT_FAILED;

    ret = bind(m_i_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (-1 == ret)
        return ERR_BIND_FAILED;

    ret = listen(m_i_sockfd, BACKLOG);
    if (-1 == ret)
        return ERR_LISTEN_FAILED;

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLERR;
    ev.data.ptr = &m_i_sockfd;
    epoll_ctl(m_i_epollfd, EPOLL_CTL_ADD, m_i_sockfd, &ev);
}

void EpollServer::Run()
{
    while (true) {
        struct epoll_event evs[MAX_SOCKFD_NUM];
        int32_t nfds = epoll_wait(m_i_epollfd, evs, MAX_SOCKFD_NUM, 10000);
        for (int32_t i = 0; i < nfds; ++i)
        {
            this->ExecEvent(evs[i]);
        }
    }
}

void EpollServer::AddCmd(int32_t type, CmdBasePtr p_cmd)
{
    m_p_cmd_map[type] = p_cmd;
}

int32_t EpollServer::ClntLogin(TcpClient *p_clnt, int32_t user_id,  int32_t last_read_id)
{
    if (!p_clnt)
        return ERR_NULLPTR;

    auto iter = m_p_client_map.find(user_id);
    if (iter != m_p_client_map.end())
    {
        return this->Reconnect(iter->second, p_clnt, last_read_id);
    }
    else
    {
        m_p_client_map[user_id] = p_clnt;
        return 0;
    }
}

void EpollServer::Forward(TcpClient *p_clnt,  int32_t user_id,  const std::__cxx11::string &data)
{
    if (!p_clnt)
        return;

    auto iter = m_p_client_map.find(user_id);
    if (iter != m_p_client_map.end())
    {

    }
    else
    {
    }
}

int32_t EpollServer::SetNonBlock()
{
    if (m_i_sockfd <= 0)
        return ERR_SOCKFD_INVALID;
    int ret = fcntl(m_i_sockfd, F_SETFL, O_NONBLOCK);
    return ret;
}

void EpollServer::ExecEvent(epoll_event &ev)
{
    if (ev.data.ptr == &(m_i_sockfd))
    {
        this->ExecNewClient();
    }
    else
    {
        if (EPOLLERR & ev.events)
        {
            this->ExecErr(static_cast<TcpClient*>(ev.data.ptr));
        }
        else if (EPOLLIN & ev.events)
        {
            this->ExecRead(static_cast<TcpClient*>(ev.data.ptr));
        }
        else if (EPOLLOUT & ev.events)
        {
            this->ExecWrite(static_cast<TcpClient*>(ev.data.ptr));
        }
        else
        {
            std::cout << "ExecEvent: unknown trigger!\n";
        }
    }
}

void EpollServer::DelClntFromEpoll(TcpClient *p_clnt)
{
    if (!p_clnt)
        return;
    struct epoll_event ev;
    ev.data.ptr = p_clnt;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLERR;
    epoll_ctl(m_i_epollfd, EPOLL_CTL_DEL, p_clnt->GetClntSock(), &ev);
}

void EpollServer::AddClntToEpoll(TcpClient *p_clnt)
{
    if (!p_clnt)
        return;

    struct epoll_event ev;
    ev.events = EPOLLET | EPOLLOUT;
    ev.data.ptr = p_clnt;
    epoll_ctl(m_i_epollfd, EPOLL_CTL_ADD, p_clnt->GetClntSock(), &ev);

    if (!p_clnt->m_write_packets.empty())
    {
        p_clnt->AddWriteToEpoll();
    }
}

void EpollServer::ExecNewClient()
{
    struct sockaddr_in clnt_addr;
    socklen_t len = static_cast<socklen_t>(sizeof(clnt_addr));
    int32_t clnt_fd = accept(m_i_sockfd, (struct sockaddr*)&clnt_addr, &len);
    if (clnt_fd < 0)
    {
        std::cout << "AcceptNewClient fd: [" << clnt_fd << "] Error\n";
    }
    else
    {
        TcpClient* p_clnt = new TcpClient(0, clnt_fd, m_i_epollfd, this);
        if (p_clnt)
        {
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLOUT |EPOLLERR;
            ev.data.ptr = p_clnt;
            epoll_ctl(m_i_epollfd, EPOLL_CTL_ADD, clnt_fd, &ev);
        }
    }
}

void EpollServer::ExecErr(TcpClient* p_clnt)
{
    if (!p_clnt)
        return;
    this->DelClntFromEpoll(p_clnt);
    p_clnt->DoClose();

    // 客户端一直不登录情况下，发生错误
    if (0 == p_clnt->GetUserID())
    {
        SAVE_DELETE(p_clnt);
    }
}

void EpollServer::ExecRead(TcpClient* p_clnt)
{
    if (!p_clnt)
        return;

    int32_t tmp = 0, ret = 0;
    ret = p_clnt->DoRead(tmp);
    if (ret != 0)
    {
        return;
    }

    if (!p_clnt->m_read_packets.empty())
    {
        int ret = 0;
        for (auto& elem : m_read_packets)
        {
            ret = this->ExecCmd(p_clnt, elem);
            if (ret != 0)
                break;
        }
        p_clnt->m_read_packets.clear();
    }
}

void EpollServer::ExecWrite(TcpClient* p_clnt)
{
    if (!p_clnt)
        return;
    p_clnt->DoWrite();
}

int32_t EpollServer::ExecCmd(TcpClient *p_clnt, Packet &packet)
{
    if (!p_clnt)
        return;

    auto iter = m_p_cmd_map.find(packet.type);
    if (iter != m_p_cmd_map.end())
    {
        return iter->second->DoCmd(p_clnt, packet);
    }
    else
    {
        return ERR_CMD_NOT_FOUND;
    }
}

int32_t EpollServer::Reconnect(TcpClient *p_old, TcpClient *p_new, int32_t last_read_id)
{
    if (!p_old || !p_new)
        return ERR_NULLPTR;
    if (p_old == p_new)
        return 0;

    this->DelClntFromEpoll(p_old);
    p_old->Reconnect(p_new->GetClntSock());
    SAVE_DELETE(p_new);
    int32_t now_time = GetNowSec();
    if (now_time - p_old->GetBrokenTime() > BROKEN_WAIT_TIME)
    {
        p_old->ClearWritePackets();
    }
    else
    {
        p_old->ResetSendPos(last_read_id);
    }
    return 0;
}
