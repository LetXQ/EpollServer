#ifndef COMMON_DEF_H_
#define COMMON_DEF_H_
#include <cstdint>
#include <string.h>
#include <cstring>
#include <sys/time.h>
#include <vector>
#include <memory>
#include <map>

class TcpClient;
using TcpClientPtr = std::shared_ptr<TcpClient>;
class CmdBase;
using CmdBasePtr = std::shared_ptr<CmdBase>;

using ClientPtrMap = std::map<int32_t, TcpClientPtr>;
using CmdPtrMap = std::map<int8_t, CmdBasePtr>;

constexpr int32_t MAX_SOCKFD_NUM = 9999;
constexpr int32_t BACKLOG = 10;
constexpr int32_t BROKEN_WAIT_TIME = 30;
constexpr int32_t MAX_BUFF_SIZE = 2048;

struct Packet
{
    Packet() {}
    Packet(uint8_t _type, uint32_t _id, const std::string& _data, int32_t _time = 0)
        : type(_type)
        , send_id(_id)
        , data(_data)
        , send_time(_time)
    {}
    uint8_t type = 0;
    uint32_t send_id = 0;
    int32_t send_time = 0;
    std::string data = "";
};
using PacketList = std::vector<Packet>;

int32_t GetNowSec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

#define SAVE_DELETE(x) \
       if (x) { \
            delete x; \
            x = nullptr; \
    }

#endif // !COMMON_DEF_H_
