/*
 * @description:
 * @version:
 * @Author: zwy
 * @Date: 2023-04-02 11:16:05
 * @LastEditors: zwy
 * @LastEditTime: 2023-04-02 14:11:38
 */
#ifndef _ZK_WEB_WEBSOCKET_HPP
#define _ZK_WEB_WEBSOCKET_HPP
#include "client_ws.hpp"
#include "server_ws.hpp"

#include <future>

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;

namespace zwy
{
    class zkWebSocketServer
    {

    public:
        zkWebSocketServer() = delete;

        zkWebSocketServer(std::string ip, unsigned short port);

        ~zkWebSocketServer();

        void start(const std::string &end_point);

        void stop();

        void setIP(const std::string &ip);

        void setPort(unsigned short port);

        void sendMessage(const std::string &message);

        // void receiveMessage();

    private:
        WsServer m_server;
        std::string m_ip;
        unsigned short m_port;
        std::thread m_server_thread;
        bool m_is_running;
    };
}

#endif