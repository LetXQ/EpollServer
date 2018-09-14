#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H
#include "../include/common_def.h"

class EpollServer;
class TcpClient
{
public:
    TcpClient(int32_t uid, int32_t sockfd, int32_t epollfd, EpollServer* server);

    int32_t SendPacket(Packet& packet);
    int32_t ReadPacket(Packet& packet);

    void AddWriteToEpoll();
    void DelWriteFromEpoll();

    int32_t DoWrite();
    int32_t DoRead(int32_t& total_size);
    void DoClose();

    void ClearWritePackets();
    void ResetSendPos(int32_t last_read_id);
    void Reconnect(int32_t new_fd);
    void Clear();
    int32_t GetClntSock() const;
    int32_t GetBrokenTime() const;
    int32_t GetUserID() const;
    EpollServer* GetServerPtr();
private:
    int32_t SetNonBlock();
    int32_t ParsePacket();
public:
    PacketList m_write_packets;
    PacketList m_read_packets;

private:
    bool m_b_broken = false;
    int32_t m_i_sockfd = -1;
    int32_t m_i_broken_time = 0;
    uint32_t m_i_send_id = 0;
    int32_t m_i_uid = 0;
    int32_t m_i_epollfd = 0;

    EpollServer* m_p_server = nullptr;

    PacketList m_old_packets;
    PacketList::iterator m_write_iter;
    int32_t m_i_write_packet_pos = 0;
    int32_t m_i_read_packet_size = 0;
    int32_t m_i_read_packet_pos = 0;

    uint8_t m_read_buff[MAX_BUFF_SIZE];
    int32_t m_read_buff_pos = 0;
};

#endif // TCP_CLIENT_H
