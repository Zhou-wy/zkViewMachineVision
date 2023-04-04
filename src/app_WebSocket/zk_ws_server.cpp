#include "zk_webSocket.hpp"

#include "../TrtLib/common/ilogger.hpp"
namespace zwy
{

    zkWebSocketServer::zkWebSocketServer(std::string ip, unsigned short port) : m_ip(ip), m_port(port), m_is_running(false), m_server()
    {
    }
    zkWebSocketServer ::~zkWebSocketServer(){};
    void zkWebSocketServer::start(const std::string &end_point)
    {
        if (this->m_is_running)
        {
            INFOE("WebSocket server is already running.");
            return;
        }
        this->m_server.config.port = m_port;
        this->m_server.config.address = m_ip;
        auto &echo = this->m_server.endpoint["^/" + end_point + "$"];

        echo.on_message = [](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message)
        {
            INFOD("Received message: %s", in_message->string());
        };

        this->m_server_thread = std::thread([this]()
                                            { m_server.start(); });

        this->m_is_running = true;
    }

    void zkWebSocketServer::stop()
    {
        if (!this->m_is_running)
        {
            INFOE("WebSocket server is not running.");
            return;
        }
        this->m_server.stop();
        this->m_server_thread.join();
        this->m_is_running = false;
    }

    void zkWebSocketServer::setIP(const std::string &ip)
    {
        this->m_ip = ip;
    }
    void zkWebSocketServer::setPort(unsigned short port)
    {
        this->m_port = port;
    }

    void zkWebSocketServer::sendMessage(const std::string &message)
    {
        for (auto &connection : this->m_server.get_connections())
        {
            connection->send(message);
        }
    }
    // void zkWebSocketServer::receiveMessage()
    // {
    //     this->m_server.on_message = [](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message)
    //     {
    //         std::cout << "Received message: " << message->string() << std::endl;
    //     };
    // }
}
