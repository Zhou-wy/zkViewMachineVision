/*
 * @description: 
 * @version: 
 * @Author: zwy
 * @Date: 2023-04-04 12:48:08
 * @LastEditors: zwy
 * @LastEditTime: 2023-04-04 12:56:55
 */
#include <fstream>
#include <iostream>
#include "../src/TrtLib/common/json.hpp"

int main()
{
    // 创建Json::Value对象
    Json::Value root;

    // 创建Json::CharReader对象
    Json::CharReaderBuilder builder;

    // 从文件中读取JSON数据
    std::ifstream json_file("config.json");
    std::string errors;
    if (Json::parseFromStream(builder, json_file, &root, &errors))
    {
        // 解析成功，可以对root对象进行操作
        Json::Value base = root["base"];
        std::string workspace_dir = base["workspace"].asString();
        std::cout << "workspace_dir: " << workspace_dir << std::endl;
    }
    else
    {
        // 解析失败，输出错误信息
        std::cout << "failed to parse JSON: " << errors << std::endl;
    }

    return 0;
}
