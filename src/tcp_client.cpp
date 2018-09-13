#include <fcntl.h>
#include <cassert>
#include <sys/epoll.h>

#include "../include/tcp_client.h"
#include "../include/common_notice.h"
#include "../include/cmd_def.h"
#include "../include/bytebuffer.h"

TcpClient::TcpClient(int32_t uid, int32_t sockfd, int32_t epollfd, void *server)
    : m_i_uid(uid)
    , m_i_sockfd(sockfd)
    , m_i_epollfd(epollfd)
    , m_p_server(server)
{
    assert(0 == SetNonBlock());
    m_write_iter = m_write_packets.end();
    memset(m_read_buff, 0, MAX_BUFF_SIZE);
}

int32_t TcpClient::SendPacket(Packet &packet)
{
    if(m_b_broken || GetNowSec() - m_i_broken_time > BROKEN_WAIT_TIME)
        return ERR_CLNT_IS_BROKEN;

    if (CMD_TYPE_TICK != packet.type)
    {
        packet.send_id = ++m_i_send_id;
    }

    ByteBuffer head;
    uint16_t packet_size = packet.data.length();
    head << packet.type << packet.send_id << packet_size;
    std::string send_head(head.contents(), head.size());
    packet.data = send_head + packet.data;

    m_write_packets.push_back(packet);
    if (m_write_iter == m_write_packets.end())
        --m_write_iter;

    this->AddWriteToEpoll();
    return 0;
}

int32_t TcpClient::ReadPacket(Packet &packet)
{
    if (m_read_packets.empty())
        return ERR_READPACKET_EMPTY;
    packet = *m_read_packets.begin();
    m_read_packets.pop_front();
    return 0;
}

void TcpClient::AddWriteToEpoll()
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLERR;
    ev.data.ptr = this;
    epoll_ctl(m_i_epollfd, EPOLL_CTL_ADD, m_i_sockfd, &ev);
}

void TcpClient::DelWriteFromEpoll()
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLERR;
    ev.data.ptr = this;
    epoll_ctl(m_i_epollfd, EPOLL_CTL_MOD, m_i_sockfd, &ev);
}

int32_t TcpClient::DoWrite()
{
    if (m_write_packets.empty())
        return 0;

    bool b_write = false;
    while (m_write_iter != m_write_packets.end()) {
        int32_t want_size = m_write_iter->data.length() - m_i_write_packet_pos;
        int32_t write_size = send(m_i_sockfd, m_write_iter->data.c_str() +m_i_write_packet_pos, want_size, MSG_NOSIGNAL);
        if (write_size < 0)
        {
            this->DoClose();
            return ERR_CLNT_IS_CLOSED;
        }
        else if (0 == write_size)
        {
            if (!b_write)
            {
                this->DoClose();
                return ERR_CLNT_IS_CLOSED;
            }
            break;
        }
        else
        {
            b_write = true;
            if (write_size == want_size)
            {
                m_i_write_packet_pos = 0;
                (*m_write_iter).send_time = GetNowSec();
                m_old_packets.push_back(*m_write_iter);
                ++m_write_iter;
                m_write_packets.pop_front();
            }
            else
            {
                m_i_write_packet_pos += write_size;
                break;
            }
        }
    }

    if (m_write_iter == m_write_packets.end())
        this->DelWriteFromEpoll();
    return 0;
}

int32_t TcpClient::DoRead(int32_t& total_size)
{
    int32_t _total_size = 0;
    int32_t b_read = false;
    int32_t read_size = 0;

    do{
        read_size = recv(m_i_sockfd, m_read_buff + m_read_buff_pos, MAX_BUFF_SIZE - m_read_buff_pos, 0);

        if (read_size > 0)
        {
            b_read = true;
        }
        else {
           break;
        }

        _total_size += read_size;
        m_read_buff_pos += read_size;
        int32_t num = this->ParsePacket();
    }while(read_size != 0);

    if (!b_read)
    {
        this->DoClose();
        return ERR_NO_DATA_TO_READ;
    }
    total_size = _total_size;
    return 0;
}

void TcpClient::DoClose()
{
    m_b_broken = true;
    m_i_broken_time = GetNowSec();
    close(m_sockfd);
    m_i_sockfd = 0;
}

void TcpClient::ClearWritePackets()
{
    m_write_packets.clear();
    m_old_packets.clear();
    // todo
}

void TcpClient::ResetSendPos(int32_t last_read_id)
{
    if (m_old_packets.empty())
        return;

    auto iter = m_old_packets.begin();
    auto last_iter = m_old_packets.end();
    int32_t now = GetNowSec();

    do{
        --last_iter;
        if ((*last_iter).send_id >= last_read_id && now - BROKEN_WAIT_TIME < (*last_iter).send_time)
        {
            m_write_packets.push_front(*last_iter);
        }
        else
        {
            break;
        }
    }while(iter != last_iter);
    m_old_packets.clear();
    m_write_iter = m_write_packets.begin();
    m_i_write_packet_pos = 0;
}

void TcpClient::Reconnect(int32_t new_fd)
{
    m_i_sockfd = new_fd;
    m_b_broken = false;
}

void TcpClient::Clear()
{
    m_read_packets.clear();
    m_write_packets.clear();
    m_i_sockfd = 0;
}

int32_t TcpClient::GetClntSock() const
{
    return m_i_sockfd;
}

int32_t TcpClient::GetBrokenTime() const
{
    return m_i_broken_time;
}

int32_t TcpClient::GetUserID() const
{
    return m_i_uid;
}

int32_t TcpClient::SetNonBlock()
{
    if (m_i_sockfd <= 0)
        return ERR_SOCKFD_INVALID;
    int ret = fcntl(m_i_sockfd, F_SETFL, O_NONBLOCK);
    return ret;
}

int32_t TcpClient::ParsePacket()
{
    int32_t ret = 0, read_pos = 0;
    while (m_read_buff_pos - read_pos >= 7) {
        uint8_t type = m_read_buff[read_pos];
        uint32_t send_id = 0;
        memcpy(&send_id, m_read_buff + read_pos + sizeof(uint8_t), sizeof(uint32_t));
        uint16_t packet_len = 0;
        memcpy(&packet_len, m_read_buff + read_pos + sizeof(uint8_t) + sizeof(uint32_t), sizeof(uint16_t));

        std::string data((char*)m_read_buff + read_pos + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint16_t), packet_len);
        Packet new_packet(type, send_id, data);
        m_read_packets.push_back(new_packet);
        ++ret;
        read_pos +=  sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint16_t) + packet_len;
    }

    if (read_pos != 0)
    {
        memcpy(m_read_buff, m_read_buff+read_pos, m_read_buff_pos - read_pos);
        m_read_buff_pos = m_read_buff_pos - read_pos;
    }
    return ret;
}
