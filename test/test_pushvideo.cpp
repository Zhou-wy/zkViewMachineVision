// tensorRT include
#include <NvInfer.h>
#include <NvInferRuntime.h>

// cuda include
#include <cuda_runtime.h>

// system include
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <functional>
#include <unistd.h>
#include <opencv2/opencv.hpp>

#include "src/TrtLib/common/ilogger.hpp"
#include "src/TrtLib/common/json.hpp"
#include "src/TrtLib/builder/trt_builder.hpp"
#include "src/app_yolo/yolo.hpp"
#include "src/VideoPusher/rtsp2flv.hpp"
#include "src/loadConfig.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

const std::string engine_file = "../onnx/zkView.engine";
const std::string onnx_file = "../onnx/zkView.onnx";

// 用于同步的互斥锁和条件变量
std::mutex mtx;
std::condition_variable cv_producer, cv_consumer;
// 图像队列
std::queue<cv::Mat> img_queue;

// static const char *cocolabels[] = {
//     "person", "bicycle", "car", "motorcycle", "airplane",
//     "bus", "train", "truck", "boat", "traffic light", "fire hydrant",
//     "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse",
//     "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack",
//     "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis",
//     "snowboard", "sports ball", "kite", "baseball bat", "baseball glove",
//     "skateboard", "surfboard", "tennis racket", "bottle", "wine glass",
//     "cup", "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich",
//     "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake",
//     "chair", "couch", "potted plant", "bed", "dining table", "toilet", "tv",
//     "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave",
//     "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase",
//     "scissors", "teddy bear", "hair drier", "toothbrush"};

static const char *cocolabels[] = {"PointerInstrument", "IndicatorLight", "LiquidCrystalInstrument"};

class InferInstance
{
public:
    bool startup()
    {
        yoloIns = get_infer(Yolo::Type::V5);
        return yoloIns != nullptr;
    }

    bool inference(const cv::Mat &image_input, Yolo::BoxArray &boxarray)
    {

        if (yoloIns == nullptr)
        {
            INFOE("Not Initialize.");
            return false;
        }

        if (image_input.empty())
        {
            INFOE("Image is empty.");
            return false;
        }
        boxarray = yoloIns->commit(image_input).get();
        return true;
    }

private:
    std::shared_ptr<Yolo::Infer> get_infer(Yolo::Type type)
    {
        if (!iLogger::exists(engine_file))
        {
            TRT::compile(
                TRT::Mode::FP32,
                10,
                onnx_file,
                engine_file);
        }
        else
        {
            INFOW("%s has been created!", engine_file.c_str());
        }
        return Yolo::create_infer(engine_file, type, 0, 0.25, 0.45);
    }
    std::shared_ptr<Yolo::Infer> yoloIns;
};

// 生产者线程函数
void YoloInference(std::string in_video_url)
{
    cv::Mat image;
    std::shared_ptr<InferInstance> yolo;
    yolo.reset(new InferInstance());
    if (!yolo->startup())
    {
        yolo.reset();
        exit(1);
    }
    srand((unsigned)time(NULL));
    INFO("opencv version: %s", CV_VERSION);
    cv::VideoCapture cap = cv::VideoCapture(in_video_url);

    if (!cap.isOpened())
    {
        INFOE("Error opening video stream or file");
        return;
    }
    Yolo::BoxArray boxarray;
    while (cap.read(image))
    {
        try
        {
            yolo->inference(image, boxarray);
            // auto t = cv::getTickCount();
            for (auto &box : boxarray)
            {

                cv::Scalar color(0, 255, 0);
                cv::rectangle(image, cv::Point(box.left, box.top), cv::Point(box.right, box.bottom), color, 3);

                auto name = cocolabels[box.class_label];
                auto caption = cv::format("%s %.2f", name, box.confidence);
                int text_width = cv::getTextSize(caption, 0, 1, 2, nullptr).width + 10;

                cv::rectangle(image, cv::Point(box.left - 3, box.top - 33), cv::Point(box.left + text_width, box.top), color, -1);
                cv::putText(image, caption, cv::Point(box.left, box.top - 5), 0, 1, cv::Scalar::all(0), 2, 16);
            }
            // auto d = cv::getTickCount();
            // // double spendTime = (d - t) * 1000 / cv::getTickFrequency();
            // double fps = cv::getTickFrequency() / (d - t);

            // auto fpsString = cv::format("%s %.2f", "FPS:", fps);
            // cv::putText(image, fpsString, cv::Point(5, 20), 0, 1, cv::Scalar::all(0), 2, 16); // 字体颜色
            INFOD("yolo Inference done!");
            // 加锁队列
            std::unique_lock<std::mutex> lock(mtx);

            // 队列满，等待消费者消费
            cv_producer.wait(lock, []()
                             { bool is_full = img_queue.size() < 3000;
            if (!is_full) {
                INFO("Producer is waiting...");
            }
            return is_full; });
            // 图像加入队列
            img_queue.push(image);

            // 通知消费者
            cv_consumer.notify_one();
            INFOD("yolo: the size of image queue : %d ", img_queue.size());
        }
        catch (const std::exception &ex)
        {
            INFOE("Error occurred in producer_thread: %s", ex.what());
        }
    }
}

// 消费者线程函数
void video2flv(double width, double height, int fps, int bitrate, std::string codec_profile, std::string out_url)
{

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    av_register_all();
#endif
    avformat_network_init();

    const char *output = out_url.c_str();
    // std::vector<uint8_t> imgbuf(height * width * 3 + 16);
    // cv::Mat image(height, width, CV_8UC3, imgbuf.data(), width * 3);

    AVFormatContext *ofmt_ctx = nullptr;
    const AVCodec *out_codec = nullptr;
    AVStream *out_stream = nullptr;
    AVCodecContext *out_codec_ctx = nullptr;

    initialize_avformat_context(ofmt_ctx, "flv");
    initialize_io_context(ofmt_ctx, output);

    out_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    out_stream = avformat_new_stream(ofmt_ctx, out_codec);
    out_codec_ctx = avcodec_alloc_context3(out_codec);

    set_codec_params(ofmt_ctx, out_codec_ctx, width, height, fps, bitrate);
    initialize_codec_stream(out_stream, out_codec_ctx, out_codec, codec_profile);

    out_stream->codecpar->extradata = out_codec_ctx->extradata;
    out_stream->codecpar->extradata_size = out_codec_ctx->extradata_size;

    av_dump_format(ofmt_ctx, 0, output, 1);

    auto *swsctx = initialize_sample_scaler(out_codec_ctx, width, height);
    auto *frame = allocate_frame_buffer(out_codec_ctx, width, height);

    int cur_size;
    uint8_t *cur_ptr;

    int ret = avformat_write_header(ofmt_ctx, nullptr);
    if (ret < 0)
    {
        INFOE("Could not write header!");
        exit(1);
    }

    bool end_of_stream = false;
    INFO("begin to flv url: %s", output);
    while (true)
    {
        try
        {
            // 加锁队列
            std::unique_lock<std::mutex> lock(mtx);
            INFOD("ffmpeg: the size of image queue : %d ", img_queue.size());
            // 队列空，等待生产者生产
            cv_consumer.wait(lock, []()
                             { return !img_queue.empty(); });

            // 取出队首图像
            cv::Mat image = img_queue.front();
            img_queue.pop();

            // 解锁队列
            lock.unlock();
            cv_producer.notify_one();

            // 消费图像consumeImage(image);
            cv::resize(image, image, cv::Size(width, height));
            const int stride[] = {static_cast<int>(image.step[0])};
            sws_scale(swsctx, &image.data, stride, 0, image.rows, frame->data, frame->linesize);
            frame->pts += av_rescale_q(1, out_codec_ctx->time_base, out_stream->time_base);
            write_frame(out_codec_ctx, ofmt_ctx, frame);
        }
        catch (const std::exception &ex)
        {
            INFOE("Error occurred in consumer_thread: %s", ex.what());
        }
    }
    av_write_trailer(ofmt_ctx);

    av_frame_free(&frame);
    avcodec_close(out_codec_ctx);
    avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
}

int main(int argc, char const *argv[])
{
    std::string in_url = "rtsp://admin:admin123@192.168.0.212:554/cam/realmonitor?channel=1&subtype=0";
    int fps = 30, width = 1920, height = 1080, bitrate = 3000000;
    std::string h264profile = "high444";
    std::string out_url = "rtmp://192.168.0.113:1935/myapp/mystream";
    iLogger::set_log_level(iLogger::LogLevel::Info);

    // 创建生产者和消费者线程
    std::thread producer(YoloInference, in_url);
    std::thread consumer(video2flv, width, height, fps, bitrate, h264profile, out_url);

    // 等待线程结束
    producer.join();
    consumer.join();
    return 0;
}
