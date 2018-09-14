#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H

#include "cmd_def.h"
#include "tcp_client.h"
#include "common_def.h"
#include "common_notice.h"

class LoginHandler : public CmdBase
{
public:
    int32_t DoCmd(TcpClient* p_clnt, const Packet& packet) override;

    static int32_t SendLogin(TcpClient* p_clnt, int32_t uid, int32_t last_read_id);

    static int32_t ExecLogin(TcpClient* p_clnt, int32_t uid, int32_t last_read_id);
};

class InfoHandler : public CmdBase
{
public:
    int32_t DoCmd(TcpClient* p_clnt, const Packet& packet) override;

    static int32_t SendInfo(TcpClient* p_clnt, int32_t from_uid, int32_t last_read_id);

    static int32_t ExecInfo(TcpClient* p_clnt, int32_t from_uid, std::string& info);
};

class TickHandler : public CmdBase
{
public:
    int32_t DoCmd(TcpClient* p_clnt, const Packet& packet) override;

    static int32_t SendTick(TcpClient* p_clnt , int32_t last_read_id);

    static int32_t ExecTick(TcpClient* p_clnt, int32_t last_read_id);
};

class ForwardHandler : public CmdBase
{
public:
    int32_t DoCmd(TcpClient* p_clnt, const Packet& packet) override;

    static int32_t SendForward(TcpClient* p_clnt , int32_t id, std::string& data);

    static int32_t ExecForward(TcpClient* p_clnt, int32_t id, std::string& data);
};
#endif // CMD_HANDLER_H
