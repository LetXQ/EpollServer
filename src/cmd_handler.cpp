#include "../include/cmd_handler.h"
#include "../include/bytebuffer.h"
#include "../include/epoll_server.h"

int32_t LoginHandler::DoCmd(TcpClient *p_clnt, const Packet &packet)
{
    if (!p_clnt)
        return ERR_NULLPTR;
    ByteBuffer buff;
    buff.append(packet.data);
    int32_t clnt_id = 0, last_read_id = 0;
    buff >> clnt_id >> last_read_id;
    return LoginHandler::ExecLogin(p_clnt, clnt_id, last_read_id);
}

int32_t LoginHandler::SendLogin(TcpClient *p_clnt, int32_t uid, int32_t last_read_id)
{
    if (!p_clnt)
        return ERR_NULLPTR;
    ByteBuffer buff;
    buff << uid << last_read_id;
    std::string data(buff.contents(), buff.size());
    Packet packet(0, CMD_TYPE_LOGIN, data);
    return p_clnt->SendPacket(packet);
}

int32_t LoginHandler::ExecLogin(TcpClient *p_clnt, int32_t uid, int32_t last_read_id)
{
    if (!p_clnt)
        return ERR_NULLPTR;

    auto p_server = p_clnt->GetServerPtr();
    if (!p_server)
        return ERR_NULLPTR;
    return p_server->ClntLogin(p_clnt, uid, last_read_id);
}

int32_t InfoHandler::DoCmd(TcpClient *p_clnt, const Packet &packet)
{
    if (!p_clnt)
        return ERR_NULLPTR;
    ByteBuffer buff;
    buff.append(packet.data);
    int32_t from_uid = 0;
    buff >> from_uid;
    std::string info(buff.remain_contents(), buff.size());
    return InfoHandler::ExecInfo(p_clnt, from_uid, info);
}

int32_t InfoHandler::SendInfo(TcpClient *p_clnt, int32_t from_uid, int32_t last_read_id)
{
    if (!p_clnt)
        return ERR_NULLPTR;
    ByteBuffer buff;
    buff << from_uid;
    std::string data(buff.contents(), buff.size());
    Packet packet(0, CMD_TYPE_INFO, data);
    return p_clnt->SendPacket(packet);
}

int32_t InfoHandler::ExecInfo(TcpClient *p_clnt, int32_t from_uid, std::string& info)
{
    if (!p_clnt)
        return ERR_NULLPTR;
}

int32_t TickHandler::DoCmd(TcpClient *p_clnt, const Packet &packet)
{
    if (!p_clnt)
        return ERR_NULLPTR;
    ByteBuffer buff;
    buff.append(packet.data);
    int32_t last_read_id = 0;
    buff >> last_read_id;
    return TickHandler::ExecTick(p_clnt, last_read_id);
}

int32_t TickHandler::SendTick(TcpClient *p_clnt,  int32_t last_read_id)
{
    if (!p_clnt)
        return ERR_NULLPTR;
    ByteBuffer buff;
    buff << last_read_id;
    std::string data(buff.contents(), buff.size());
    Packet packet(0, CMD_TYPE_TICK, data);
    return p_clnt->SendPacket(packet);
}

int32_t TickHandler::ExecTick(TcpClient *p_clnt, int32_t last_read_id)
{
    if (!p_clnt)
        return ERR_NULLPTR;
    p_clnt->ResetSendPos(last_read_id + 1);
    return 0;
}

int32_t ForwardHandler::DoCmd(TcpClient *p_clnt, const Packet &packet)
{
    if (!p_clnt)
        return ERR_NULLPTR;
    ByteBuffer buff;
    buff.append(packet.data);
    int32_t id = 0;
    std::string data;
    buff >> id >> data;
    return ForwardHandler::ExecForward(p_clnt, id, data);
}

int32_t ForwardHandler::SendForward(TcpClient *p_clnt, int32_t id,  std::string& data)
{
    if (!p_clnt)
        return ERR_NULLPTR;
    ByteBuffer buff;
    buff << id << data;
    std::string send_data(buff.contents(), buff.size());
    Packet packet(0, CMD_TYPE_FORWARD, send_data);
    return p_clnt->SendPacket(packet);
}

int32_t ForwardHandler::ExecForward(TcpClient *p_clnt, int32_t id,  std::string& data)
{
    if (!p_clnt)
        return ERR_NULLPTR;
    auto p_server = p_clnt->GetServerPtr();
    p_server->Forward(p_clnt, id, data);
    return 0;
}
