#include "../src/TrtLib/common/ilogger.hpp"
#include "../src/TrtLib/builder/trt_builder.hpp"
#include "../src/app_yolo/yolo.hpp"
#include <numeric>
#include <cmath>
#include "common.hpp"

const std::string engine_file = "../workspace/yolov5s.engine";
const std::string onnx_file = "../workspace/yolov5s.onnx";
#define RANGE 1.6
//static const char *label_string[] = {"PointerInstrument", "IndicatorLight", "LiquidCrystalInstrument"};

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

cv::Mat ImagePretreatment(cv::Mat image)
{
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
    cv::Mat kernel = cv::Mat::ones(5, 5, CV_32F) / 25;
    cv::filter2D(gray, gray, -1, kernel);
    cv::Canny(gray, gray, 50, 150, 3);
    cv::Mat Matrix = cv::Mat::ones(3, 3, CV_8U);
    cv::morphologyEx(gray, gray, cv::MORPH_CLOSE, Matrix);

    return gray;
}

void findEllipse(cv::Mat img, int x, int y, int r, double &X, double &Y, double &MA, double &ma, double &angle)
{
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(img, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    int height = img.rows;
    int width = img.cols;

    X = 0;
    Y = 0;
    MA = 0;
    ma = 0;
    angle = 0;

    for (int i = 0; i < contours.size(); i++)
    {
        if (contours[i].size() > 5)
        {
            cv::RotatedRect ellipse = cv::fitEllipse(contours[i]);
            double MA0 = std::max(ellipse.size.width, ellipse.size.height);
            double ma0 = std::min(ellipse.size.width, ellipse.size.height);
            double X0 = ellipse.center.x;
            double Y0 = ellipse.center.y;
            double distance = std::sqrt(std::pow(X0 - x, 2) + std::pow(Y0 - y, 2));

            if (ma0 < std::min(width, height) && MA0 < std::max(width, height) && distance < r / 2 && ma0 > ma && MA0 > MA)
            {
                X = X0;
                Y = Y0;
                MA = MA0;
                ma = ma0;
                angle = ellipse.angle;
            }
        }
    }
}

std::vector<cv::Point2f> findVertex(cv::Mat &img_copy, double &X, double &Y, double &MA, double &ma, double &angle)
{
    std::vector<cv::Point2f> points;
    cv::Mat img1 = cv::Mat::zeros(img_copy.size(), CV_8UC1);
    cv::ellipse(img1, cv::Point2d(X, Y), cv::Size(MA / 2, ma / 2), angle, 0, 360, cv::Scalar(255), 2);
    cv::Mat img2 = cv::Mat::zeros(img_copy.size(), CV_8UC1);
    cv::line(img2, cv::Point2d(X - cos(angle) * ma, Y + sin(angle) * ma),
             cv::Point2d(X + cos(angle) * ma, Y - sin(angle) * ma), cv::Scalar(255), 1);
    cv::line(img2, cv::Point2d(X + sin(angle) * MA, Y + cos(angle) * MA),
             cv::Point2d(X - sin(angle) * MA, Y - cos(angle) * MA), cv::Scalar(255), 1);
    for (int i = 0; i < img_copy.rows; i++)
    {
        for (int j = 0; j < img_copy.cols; j++)
        {
            if (img1.at<uchar>(i, j) > 0 && img2.at<uchar>(i, j) > 0)
            {
                points.push_back(cv::Point2f(j, i));
            }
        }
    }

    std::vector<cv::Point2f> point;
    int n = points[0].x;
    for (int i = 0; i < points.size(); i++)
    {
        if (abs(points[i].x - n) > 2)
        {
            point.push_back(points[i]);
            n = points[i].x;
        }
    }
    point.push_back(points[0]);
    cv::Mat img3 = cv::Mat::zeros(img_copy.size(), CV_8UC1);
    cv::ellipse(img3, cv::Point2d(X, Y), cv::Size(MA / 2, ma / 2), angle, 0, 360, cv::Scalar(255), -1);
    for (int i = 0; i < img_copy.rows; i++)
    {
        for (int j = 0; j < img_copy.cols; j++)
        {
            if (img3.at<uchar>(i, j) == 0)
            {
                img_copy.at<uchar>(i, j) = 255;
            }
        }
    }

    // 对交点按照 x 坐标排序
    std::vector<cv::Point2f> order;

    order.push_back(point[std::min_element(point.begin(), point.end(), [](const cv::Point2f &a, const cv::Point2f &b)
                                           { return a.y < b.y; }) -
                          point.begin()]);
    order.push_back(point[std::max_element(point.begin(), point.end(), [](const cv::Point2f &a, const cv::Point2f &b)
                                           { return a.y < b.y; }) -
                          point.begin()]);
    order.push_back(point[std::min_element(point.begin(), point.end(), [](const cv::Point2f &a, const cv::Point2f &b)
                                           { return a.x < b.x; }) -
                          point.begin()]);
    order.push_back(point[std::max_element(point.begin(), point.end(), [](const cv::Point2f &a, const cv::Point2f &b)
                                           { return a.x < b.x; }) -
                          point.begin()]);
    return order;
}

cv::Mat perspective_transformation(cv::Mat img_copy, std::vector<cv::Point2f> point)
{
    int w = std::min(img_copy.size().height, img_copy.size().width);
    cv::Point2f pts1[] = {cv::Point2f(point[0].x, point[0].y),
                          cv::Point2f(point[1].x, point[1].y),
                          cv::Point2f(point[2].x, point[2].y),
                          cv::Point2f(point[3].x, point[3].y)};
    cv::Point2f pts2[] = {cv::Point2f(w / 2, 0), cv::Point2f(w / 2, w),
                          cv::Point2f(0, w / 2), cv::Point2f(w, w / 2)};
    cv::Mat M = cv::getPerspectiveTransform(pts1, pts2);
    cv::Mat dst;
    cv::warpPerspective(img_copy, dst, M, cv::Size(w, w));
    return dst;
}

void getData(std::map<std::string, cv::Mat> class_image)
{
    if (!class_image.empty())
    {
        for (auto it = class_image.begin(); it != class_image.end(); ++it)
        {
            if (it->first.compare("PointerInstrument") == 0) // 指针式仪表
            {
                cv::Mat image = it->second;

                // 预处理
                cv::Mat gray = ImagePretreatment(image);
                cv::imwrite("../workspace/test_instrument/77_gray.JPG", gray);

                // 寻找圆框
                double X, Y, MA, ma, angle;
                int x = gray.cols / 2, y = gray.rows / 2;
                findEllipse(gray, x, y, 200, X, Y, MA, ma, angle);

                // 透视变换纠正拍摄角度
                std::vector<cv::Point2f> dst_pts;
                dst_pts.push_back(cv::Point2f(0, 0));
                dst_pts.push_back(cv::Point2f(image.cols, 0));
                dst_pts.push_back(cv::Point2f(image.cols, image.rows));
                dst_pts.push_back(cv::Point2f(0, image.rows));

                cv::Mat Vertex = gray.clone();
                std::vector<cv::Point2f> src_pts = findVertex(Vertex, X, Y, MA, ma, angle);
                cv::imwrite("../workspace/test_instrument/77_Vertex.JPG", Vertex);

                std::vector<cv::Vec2f> lines;
                std::vector<cv::Point> lines2Point;
                cv::HoughLines(gray, lines, 2, CV_PI / 180, 400, 0, 0);
                double point_angle;
                for (size_t i = 0; i < lines.size(); i = +2)
                {
                    float rho1 = lines[i][0], theta1 = lines[i][1];
                    float rho2 = lines[i++][0], theta2 = lines[i++][1];
                    float theta = (theta1 + theta2) / 2;
                    float rho = (rho1 + rho2) / 2;

                    cv::Point pt1, pt2;
                    double a = cos(theta), b = sin(theta);
                    double x0 = a * rho, y0 = b * rho;
                    pt1.x = cvRound(x0 + 1000 * (-b));
                    pt1.y = cvRound(y0 + 1000 * (a));
                    pt2.x = cvRound(x0 - 1000 * (-b));
                    pt2.y = cvRound(y0 - 1000 * (a));

                    point_angle = theta * 180 / CV_PI;

                    std::cout << "theta: " << theta << " angle: " << point_angle
                              << " rho: " << rho << std::endl;

                    cv::line(image, pt1, pt2, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
                    lines2Point.push_back(pt1);
                    lines2Point.push_back(pt2);
                }
                cv::imwrite("../workspace/test_instrument/77_Hough.JPG", image);

                // 判断读数
                float true_angle, data;
                if (0 < point_angle < 180) // 第一、二象限
                {
                    true_angle = 180 - point_angle + 45;
                    data = true_angle / 270 * RANGE;
                }
                else if (-45 < point_angle < 0)
                {
                    true_angle = 180 + abs(point_angle) + 45;
                    data = true_angle / 270 * RANGE;
                }
                else if (-180 < point_angle < -135)
                {
                    true_angle = 180 + point_angle - 45;
                    data = true_angle / 270 * RANGE;
                }
                INFO("true data ====>: %.2f", data);
            }
            else if (it->first.compare("IndicatorLight") == 0) // 指示灯仪表
            {
            }
            else if (it->first.compare("LiquidCrystalInstrument") == 0) // 液晶显示仪表
            {
            }
            else
            {
                return;
            }
        }
    }
}

int main(int argc, char const *argv[])
{
    cv::Mat image = cv::imread("../workspace/car.jpg");
    cv::Mat image_clone = image.clone();
    std::map<std::string, cv::Mat> crop_result; // key: 类别 -> value: 识别结果框
    std::shared_ptr<InferInstance> yolo;
    yolo.reset(new InferInstance());
    if (!yolo->startup())
    {
        yolo.reset();
        exit(1);
    }
    Yolo::BoxArray boxarray;
    yolo->inference(image, boxarray);
    for (auto &box : boxarray)
    {

        //    cv::Scalar color(0, 255, 0);
        cv::rectangle(image, cv::Point(box.left, box.top), cv::Point(box.right, box.bottom), colors[box.class_label], 3);

        auto name = cocolabels[box.class_label];
        auto caption = cv::format("%s %.2f", name, box.confidence);
        int text_width = cv::getTextSize(caption, 0, 1, 2, nullptr).width + 10;

        // 可视化结果
        cv::rectangle(image, cv::Point(box.left - 3, box.top - 33), cv::Point(box.left + text_width, box.top), colors[box.class_label], -1);
        cv::putText(image, caption, cv::Point(box.left, box.top - 5), 0, 1, cv::Scalar::all(0), 2, 16);

        // 取出识别结果
        cv::Rect roi(box.left, box.top, box.right - box.left, box.bottom - box.top);
        cv::Mat crop = image_clone(roi);
        crop_result.insert(std::make_pair(std::string(name), crop));
        //getData(crop_result);
    }
    cv::imwrite("../workspace/result.JPG", image);
    return 0;
}
