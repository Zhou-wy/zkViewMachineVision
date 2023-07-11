/*
 * @description:
 * @version:
 * @Author: zwy
 * @Date: 2023-04-01 17:31:27
 * @LastEditors: zwy
 * @LastEditTime: 2023-04-12 16:30:21
 */
#include <thread>

#include <map>
#include <future>
#include <chrono>

#include "../src/app_WebSocket/zk_webSocket.hpp"
#include "../src/TrtLib/common/ilogger.hpp"
#include "../src/TrtLib/common/json.hpp"

bool checkHeart(zwy::zkWebSocketServer &server)
{
  while (server.wsIsRunning())
  {
    auto now = std::chrono::system_clock::now();
    auto lastHeartbeatTime = std::chrono::time_point_cast<std::chrono::milliseconds>(server.getWsTime());
    auto currentTime = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto elapsedTime = currentTime - lastHeartbeatTime;
    // 以下为间隔15S
    if (elapsedTime.count() > (15 * 1000))
    {
      INFOW("connection timed out");
      return false;
    }
  }
}

int main(int argc, char const *argv[])
{
  INFO("=======test begin======");
  iLogger::set_log_level(iLogger::LogLevel::Debug);
  try
  {
    zwy::zkWebSocketServer server("192.168.0.113", 8765);

    server.startUp("zkSource");

    INFO("WebSocket server is running.");

    std::thread th([&server]()
                   {
        if(!checkHeart(server)) 
        {
          
        } 
      });

    // detach the thread so that it can continue running in the background
    th.join();
  }
  catch (const std::exception &e)
  {
    INFOW("Exception caught: %s", e.what());
    return 1;
  }
  return 0;
}
