/*
 * @description:
 * @version:
 * @Author: zwy
 * @Date: 2023-07-06 10:57:10
 * @LastEditors: zwy
 * @LastEditTime: 2023-07-11 17:31:35
 */

#include <string>

#include "../src/TrtLib/common/ilogger.hpp"
#include "../src/TrtLib/builder/trt_builder.hpp"
#include "../src/yolov8/yolov8_seg.hpp"
#include "common.hpp"

class YOLOv8SegInstance
{
private:
    std::string m_engine_file;
    std::string m_onnx_file;
    std::shared_ptr<YOLOv8Seg::SegInfer> SegIns;

    std::shared_ptr<YOLOv8Seg::SegInfer> get_infer(YOLOv8Seg::Task task)
    {
        if (!iLogger::exists(m_engine_file))
        {
            TRT::compile(
                TRT::Mode::FP32,
                10,
                m_onnx_file,
                m_engine_file);
        }
        else
        {
            INFOW("%s has been created!", m_engine_file.c_str());
        }
        return YOLOv8Seg::create_seg_infer(m_engine_file, task, 0, 0.6, 0.6);
    }

public:
    YOLOv8SegInstance(const std::string &onnx_file, const std::string &engine_file);
    ~YOLOv8SegInstance();

    bool startup()
    {
        SegIns = get_infer(YOLOv8Seg::Task::seg);
        return SegIns != nullptr;
    }

    bool inference(const cv::Mat &image_input, YOLOv8Seg::BoxSeg &boxarray)
    {

        if (SegIns == nullptr)
        {
            INFOE("Not Initialize.");
            return false;
        }

        if (image_input.empty())
        {
            INFOE("Image is empty.");
            return false;
        }
        boxarray = SegIns->commit(image_input).get();
        return true;
    }
};

YOLOv8SegInstance::YOLOv8SegInstance(const std::string &onnx_file, const std::string &engine_file) : m_onnx_file(onnx_file), m_engine_file(engine_file)
{
    std::cout << "         " << std::endl;
    std::cout << " ____        __  __      ____       ____                __    __  _____       __         _____        " << std::endl;
    std::cout << "/\\  _`\\     /\\ \\/\\ \\    /\\  _`\\    /\\  _`\\             /\\ \\  /\\ \\/\\  __`\\    /\\ \\       /\\  __`\\      " << std::endl;
    std::cout << "\\ \\,\\L\\_\\   \\ \\ \\ \\ \\   \\ \\ \\L\\_\\  \\ \\ \\L\\ \\           \\ `\\`\\\\/'/\\ \\ \\/\\ \\   \\ \\ \\      \\ \\ \\/\\ \\     " << std::endl;
    std::cout << " \\/_\\__ \\    \\ \\ \\ \\ \\   \\ \\  _\\L   \\ \\ ,__/            `\\ `\\ /'  \\ \\ \\ \\ \\   \\ \\ \\  __  \\ \\ \\ \\ \\    " << std::endl;
    std::cout << "   /\\ \\L\\ \\   \\ \\ \\_\\ \\   \\ \\ \\L\\ \\  \\ \\ \\/               `\\ \\ \\   \\ \\ \\_\\ \\   \\ \\ \\L\\ \\  \\ \\ \\_\\ \\   " << std::endl;
    std::cout << "   \\ `\\____\\   \\ \\_____\\   \\ \\____/   \\ \\_\\                 \\ \\_\\   \\ \\_____\\   \\ \\____/   \\ \\_____\\  " << std::endl;
    std::cout << "    \\/_____/    \\/_____/    \\/___/     \\/_/                  \\/_/    \\/_____/    \\/___/     \\/_____/  " << std::endl;
    std::cout << "         " << std::endl;
}

YOLOv8SegInstance::~YOLOv8SegInstance()
{
}

int main(int argc, char const *argv[])
{
    std::string onnx = "../workspace/yolov8/yolov8s-seg.transd.onnx";
    std::string engine = "../workspace/yolov8/yolov8s-seg.transd.engine";
    cv::Mat image = cv::imread("../workspace/car.jpg");

    std::shared_ptr<YOLOv8SegInstance> seg = std::make_shared<YOLOv8SegInstance>(onnx, engine);
    if (!seg->startup())
    {
        seg.reset();
        exit(1);
    }

    YOLOv8Seg::BoxSeg boxarray;
    seg->inference(image, boxarray);

    for (auto &obj : boxarray)
    {
        INFO("rect: %.2f, %.2f, %.2f, %.2f, confi: %.2f, name: %s", obj.left, obj.top, obj.right, obj.bottom, obj.confidence, cocolabels[obj.class_label]);
        cv::rectangle(image, cv::Point(obj.left, obj.top), cv::Point(obj.right, obj.bottom), colors[obj.class_label], 3);

        auto name = cocolabels[obj.class_label];
        auto caption = cv::format("%s %.2f", name, obj.confidence);
        int text_width = cv::getTextSize(caption, 0, 1, 2, nullptr).width + 10;

        // 可视化结果
        cv::rectangle(image, cv::Point(obj.left - 3, obj.top - 33), cv::Point(obj.left + text_width, obj.top), colors[obj.class_label], -1);
        cv::putText(image, caption, cv::Point(obj.left, obj.top - 5), 0, 1, cv::Scalar::all(0), 2, 16);
        if (obj.seg)
        {
            cv::Mat img_clone = image.clone();
            cv::Mat mask(obj.seg->height, obj.seg->width, CV_8U, obj.seg->data);
            img_clone(cv::Rect(obj.left, obj.top, obj.right - obj.left, obj.bottom - obj.top)).setTo(colors[obj.class_label], mask);
            cv::addWeighted(image, 0.5, img_clone, 0.5, 1, image);

        }
    }
    cv::imwrite("../workspace/car_result.jpg", image);

    return 0;
}
