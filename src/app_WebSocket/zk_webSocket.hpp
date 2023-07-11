/*
 * @description:
 * @version:
 * @Author: zwy
 * @Date: 2023-04-02 11:16:05
 * @LastEditors: zwy
 * @LastEditTime: 2023-04-12 16:30:13
 */
#ifndef _ZK_WEB_WEBSOCKET_HPP
#define _ZK_WEB_WEBSOCKET_HPP
#include "client_ws.hpp"
#include "server_ws.hpp"

#include <map>
#include <future>
#include <chrono>
#include "../TrtLib/common/json.hpp"

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

        void startUp(const std::string &end_point);

        void stop();

        void setIP(const std::string &ip);

        void setPort(unsigned short port);

        void sendMessage(const std::string &message);

        Json::Value receiveMessage();

        void sendHeartbeat();

        void sendOriginal(Json::Value &data, Json::Value &nameMap);

        std::chrono::high_resolution_clock::time_point getWsTime();

        bool wsIsRunning() { return this->m_is_running; }

    private:
        WsServer m_server;
        std::string m_ip;
        unsigned short m_port;
        std::thread m_server_thread;
        bool m_is_running;
        Json::Value m_recv_message;
        std::chrono::high_resolution_clock::time_point m_ws_time;
    };
}

#endif