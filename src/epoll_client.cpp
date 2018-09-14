#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cassert>
#include <stdio.h>

#include "../include/epoll_client.h"
#include "../include/tcp_client.h"
#include "../include/common_notice.h"
#include "../include/cmd_def.h"
#include "../include/cmd_handler.h"

EpollClient::EpollClient(int32_t port, const char *ip)
    : m_i_port(port)
{
    memset(m_ip, 0, sizeof(m_ip));
    strncpy(m_ip, ip, strlen(ip));
    m_i_epollfd = epoll_create(MAX_SOCKFD_NUM);
    int32_t ret = fcntl(m_i_epollfd, F_SETFL, O_NONBLOCK);
    assert(ret != -1);
}

EpollClient::~EpollClient()
{
    this->Disconnect();
}

int32_t EpollClient::Run()
{
    int32_t ret = this->ConnectToServer();
    if (ret != 0)
    {
        std::cout << "EpollClient: Run Failed ret [" << ret << "]\n";
        return ret;
    }

    struct epoll_event ev;
    ev.data.fd = m_sockfd;
    ev.events = EPOLLOUT | EPOLLERR | EPOLLHUP;
    epoll_ctl(m_i_epollfd, EPOLL_CTL_ADD, m_sockfd, &ev);

    while (true) {
        struct epoll_event evs[MAX_SOCKFD_NUM];
        char buffer[MAX_BUFF_SIZE];
        memset(buffer, 0, MAX_BUFF_SIZE);

        int32_t nfds = epoll_wait(m_i_epollfd, evs, MAX_SOCKFD_NUM, 100);
        for (int32_t i = 0; i < nfds; ++i)
        {
            if (evs[i].events & EPOLLOUT)
            {
                ret = this->SendDataToServer();
                if (ret == 0)
                {
                    struct epoll_event i_ev;
                    i_ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
                    i_ev.data.fd = evs[i].data.fd;
                    epoll_ctl(m_i_epollfd, EPOLL_CTL_MOD, i_ev.data.fd, &i_ev);
                }
                else
                {
                    printf("EpollClient Disconnect-----1---ret[%d]\n", ret) ;
                    this->DelEpoll();
                    this->Disconnect();
                }
            }
            else if(evs[i].events & EPOLLIN)
            {
                int32_t len = this->ReadDataFromServer();
                if (len <= 0)
                {
                    this->DelEpoll();
                    this->Disconnect();
                }
                else
                {
                    struct epoll_event i_ev;
                    i_ev.events = EPOLLOUT | EPOLLERR | EPOLLHUP;
                    i_ev.data.fd = evs[i].data.fd;
                    epoll_ctl(m_i_epollfd, EPOLL_CTL_MOD, i_ev.data.fd, &i_ev);
                }
            }
            else
            {
                this->DelEpoll();
                this->Disconnect();
            }
        }
    }
}

void EpollClient::AddCmd(uint8_t type, CmdBase *p_cmd)
{
    m_cmd_map[type] = p_cmd;
}

TcpClient *EpollClient::GetClntPtr()
{
    printf("GetClntPtr m_p_clnt:  [%p] this[%p]\n", m_p_clnt, this) ;
    return m_p_clnt;
}

int32_t EpollClient::ConnectToServer()
{
    printf("ConnectToServer start m_p_clnt:  [%p] this[%p]\n", m_p_clnt, this) ;
    if (m_p_clnt)
    {
        this->Disconnect();
    }

    m_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (m_sockfd < 0)
        return ERR_SOCKET_FAILED;

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(m_i_port);
    inet_pton(AF_INET, m_ip, &serv_addr.sin_addr);

    int32_t ireuseaddr = 1;
    int32_t ret = setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &ireuseaddr, sizeof(ireuseaddr));
    if (-1 == ret)
        return ERR_SETSOCKOPT_FAILED;
    ret = connect(m_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (-1 == ret)
        return ERR_CONNECT_FAILED;
    m_p_clnt = new TcpClient(m_user_id, m_sockfd, m_i_epollfd, nullptr);
    printf("ConnectToServer end m_p_clnt:  [%p] this[%p]\n", m_p_clnt, this) ;
    return 0;
}

int32_t EpollClient::SendDataToServer()
{
    if (m_p_clnt)
    {
        return m_p_clnt->DoWrite();
    }
    return ERR_NULLPTR;
}

int32_t EpollClient::ReadDataFromServer()
{
    if (!m_p_clnt)
        return 0;

    int32_t total_size = 0;
    int32_t ret = m_p_clnt->DoRead(total_size);
    if (ret != 0)
        return 0;

    Packet packet;
    while (0 == m_p_clnt->ReadPacket(packet)) {
        this->DoCmd(packet);
    }
    m_p_clnt->m_read_packets.clear();
    return total_size;
}

bool EpollClient::DelEpoll()
{
    if (!m_p_clnt || m_p_clnt->GetClntSock() <= 0)
        return true;

    bool b_ret = false;
    struct epoll_event ev;
    ev.data.fd = m_p_clnt->GetClntSock();
    ev.events = 0;
    if (0 == epoll_ctl(m_i_epollfd, EPOLL_CTL_DEL, ev.data.fd, &ev))
        b_ret = true;
    else
    {
        // to do
    }
    return b_ret;
}

void EpollClient::Disconnect()
{
    if (m_p_clnt)
    {
        printf("Disconnect m_p_clnt:  [%p] this[%p]\n", m_p_clnt, this) ;
        m_p_clnt->DoClose();
        m_p_clnt->Clear();
        SAVE_DELETE(m_p_clnt);
    }
}

int32_t EpollClient::DoCmd(Packet &packet)
{
    if (!m_p_clnt)
        return ERR_NULLPTR;

    auto iter = m_cmd_map.find(packet.type);
    if (iter != m_cmd_map.end())
    {
        return iter->second->DoCmd(m_p_clnt, packet);
    }
    else
    {
        return ERR_CMD_NOT_FOUND;
    }
}
