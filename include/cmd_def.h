#ifndef CMD_DEF_H_
#define CMD_DEF_H_
#include <iostream>
#include <memory>
#include "../include/common_def.h"

enum cmd_type_t
{
    CMD_TYPE_NULL = -1,
    CMD_TYPE_LOGIN = 1,
    CMD_TYPE_TICK = 2,
    CMD_TYPE_FORWARD = 3,
    CMD_TYPE_INFO = 4,
    CMD_TYPE_LOGOUT = 5,
};

class TcpClient;
class CmdBase
{
public:
    virtual int32_t DoCmd(TcpClient* p_clnt, const Packet& packet) = 0;
};
#endif // CMD_DEF_H_!
