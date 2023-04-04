/*
 * @description:
 * @version:
 * @Author: zwy
 * @Date: 2023-04-01 17:31:27
 * @LastEditors: zwy
 * @LastEditTime: 2023-04-02 14:21:42
 */
#include <thread>
#include <chrono>
#include <map>
#include <future>

#include "../src/app_WebSocket/zk_webSocket.hpp"
#include "../src/TrtLib/common/ilogger.hpp"

int main(int argc, char const *argv[])
{
  INFO("=======test begin======");
  iLogger::set_log_level(iLogger::LogLevel::Debug);
  // zwy::zkWebSocketServer server("localhost", 8765);
  // server.setOnMessageHandler([](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::Message> message)
  //                            { std::cout << "Received message: " << message->string() << std::endl; });
  try
  {
    zwy::zkWebSocketServer server("localhost", 8765);
    // server.setOnMessageHandler([](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::Message> message)
    //                            { std::cout << "Received message: " << message->string() << std::endl; });
    server.start("zkSource");
    std::cout << "WebSocket server is running." << std::endl;
    std::string input;
    do
    {
      std::cout << "Enter a message to send (or 'quit' to exit): ";
      std::getline(std::cin, input);
      if (input != "quit")
      {
        server.sendMessage(input);
      }
    } while (input != "quit");
    server.stop();
    std::cout << "WebSocket server is stopped." << std::endl;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Exception caught: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
