/*
 * @description:
 * @version:
 * @Author: zwy
 * @Date: 2023-04-03 12:44:00
 * @LastEditors: zwy
 * @LastEditTime: 2023-04-12 16:56:12
 */

#include <thread>
#include <chrono>

#include "zk_webSocket.hpp"
#include "../TrtLib/common/ilogger.hpp"

namespace zwy
{

    zkWebSocketServer::zkWebSocketServer(std::string ip, unsigned short port) : m_ip(ip), m_port(port), m_is_running(false), m_server(), m_ws_time(std::chrono::high_resolution_clock::now())
    {
    }
    zkWebSocketServer ::~zkWebSocketServer(){};
    void zkWebSocketServer::startUp(const std::string &end_point)
    {
        if (this->m_is_running)
        {
            INFOE("WebSocket server is already running.");
            return;
        }
        this->m_server.config.port = m_port;
        this->m_server.config.address = m_ip;
        auto &echo = this->m_server.endpoint["^/" + end_point + "$"];

        echo.on_message = [this](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message)
        {
            std::string input_str = in_message->string();
            INFO("recv: %s", input_str.c_str());
            Json::CharReaderBuilder builder;
            JSONCPP_STRING errs;
            std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
            bool success = reader->parse(input_str.c_str(), input_str.c_str() + input_str.size(), &m_recv_message, &errs);
            if (!success)
            {
                INFOE("Failed to parse JSON string: ");
                return 1;
            }
            if (this->m_recv_message["name"] == "HEART")
            {
                this->m_ws_time = std::chrono::system_clock::now();
                Json::Value message;
                message["name"] = "HEART";
                std::string response = Json::FastWriter().write(message);
                this->sendMessage(response);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            else if (this->m_recv_message["name"] == "ORIGINAL")
            {
                Json::Value data_test;
                data_test["t1"] = 1.34;
                data_test["t2"] = 2.12;
                data_test["t3"] = 3.14;

                Json::Value nameMap_test;
                nameMap_test["t1"] = "测试1";
                nameMap_test["t2"] = "测试2";
                nameMap_test["t3"] = "测试3";
                this->sendOriginal(data_test, nameMap_test);
            }
            else if (this->m_recv_message["name"] == "REGISTER")
            {
                // std::string receive_json = msg->get_payload();
                // Json::Value json = Json::Value(Json::Reader().parse(receive_json));
                // Json::Value data = json["data"];
                // register_[hdl] = Json::FastWriter().write(data);
                // Json::Value rdata;
                // for (Json::Value::iterator itr = data.begin(); itr != data.end(); itr++)
                // {
                //     rdata[(*itr)["name"].asString()] = data[(*itr)["name"].asString()];
                // }
                // Json::Value message;
                // message["name"] = "DATA";
                // message["data"] = rdata;
                // string response = Json::FastWriter().write(message);
                // server.send(hdl, response, websocketpp::frame::opcode::text);
                // cout << "收到用户注册测点" << endl;
            }
            else if (this->m_recv_message["name"] == "USER")
            {
                std::cout << "收到用户发送数据" << std::endl;
            }
        };

        this->m_server_thread = std::thread([this]()
                                            { m_server.start(); });

        this->m_is_running = true;
        // this->sendHeartbeat();
    }
    std::chrono::high_resolution_clock::time_point zkWebSocketServer::getWsTime()
    {
        return this->m_ws_time;
    }
    void zkWebSocketServer::sendOriginal(Json::Value &data, Json::Value &nameMap)
    {
        Json::Value root_data, root_nameMap;
        root_data["name"] = "ORIGINALDATA";
        root_data["data"] = data;


        root_nameMap["name"] = "NAMEMAP";
        root_nameMap["data"] = nameMap;

        Json::StreamWriterBuilder data_builder;
        std::string data_str = Json::writeString(data_builder, root_data);

        Json::StreamWriterBuilder nameMap_builder;
        std::string nameMap_str = Json::writeString(nameMap_builder, root_nameMap);

        INFO("sendOriginal data: %s", data_str.c_str());
        INFO("sendOriginal nameMap: %s", nameMap_str.c_str());
        this->sendMessage(data_str);
        this->sendMessage(nameMap_str);
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
            INFO("send: %s", message.c_str());
            connection->send(message);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    Json::Value zkWebSocketServer::receiveMessage()
    {
        return this->m_recv_message;
    }

    void zkWebSocketServer::sendHeartbeat()
    {
        std::thread([this]()
                    {
                        while (this->m_is_running)
                        {
                            auto now = std::chrono::system_clock::now();
                            auto lastHeartbeatTime = std::chrono::time_point_cast<std::chrono::milliseconds>(this->m_ws_time);
                            auto currentTime = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
                            auto elapsedTime = currentTime - lastHeartbeatTime;
                            // 以下为间隔10S
                            if (elapsedTime.count() > (15 * 1000))
                            {
                                INFOW("%s connection timed out", this->m_ip.c_str());
                            }
                        } })
            .detach();
    }
}
