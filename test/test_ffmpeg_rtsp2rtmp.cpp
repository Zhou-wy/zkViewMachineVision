/*
 * @description:
 * @version:
 * @Author: zwy
 * @Date: 2023-04-03 13:11:17
 * @LastEditors: zwy
 * @LastEditTime: 2023-04-03 13:40:05
 */
#include <iostream>
#include "../src/VideoPusher/rtsp2flv.hpp"
#include "../src/VideoPusher/clipp.h"

using namespace clipp;

int main(int argc, char *argv[])
{
    std::string cameraID = "rtsp://admin:admin123@192.168.0.212:554/cam/realmonitor?channel=1&subtype=0";
    int fps = 30, width = 1920, height = 1080, bitrate = 300000;
    std::string h264profile = "high444";
    std::string outputServer = "rtmp://192.168.0.113:1935/myapp/mystream";
    bool dump_log = false;

    auto cli = ((option("-c", "--camera") & value("camera", cameraID)) % "camera ID (default: 0)",
                (option("-o", "--output") & value("output", outputServer)) % "output RTMP server (default: rtmp://localhost/live/stream)",
                (option("-f", "--fps") & value("fps", fps)) % "frames-per-second (default: 30)",
                (option("-w", "--width") & value("width", width)) % "video width (default: 800)",
                (option("-h", "--height") & value("height", height)) % "video height (default: 640)",
                (option("-b", "--bitrate") & value("bitrate", bitrate)) % "stream bitrate in kb/s (default: 300000)",
                (option("-p", "--profile") & value("profile", h264profile)) % "H264 codec profile (baseline | high | high10 | high422 | high444 | main) (default: high444)",
                (option("-l", "--log") & value("log", dump_log)) % "print debug output (default: false)");

    if (!parse(argc, argv, cli))
    {
        std::cout << make_man_page(cli, argv[0]) << std::endl;
        return 1;
    }

    // if (dump_log)
    // {
    //     av_log_set_level(AV_LOG_DEBUG);
    // }

    stream_video(width, height, fps, cameraID, bitrate, h264profile, outputServer);

    return 0;
}