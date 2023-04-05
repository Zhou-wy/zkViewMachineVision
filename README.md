<!--
 * @description: 
 * @version: 
 * @Author: zwy
 * @Date: 2023-04-03 16:06:43
 * @LastEditors: zwy
 * @LastEditTime: 2023-04-05 15:56:02
-->
# 这是一个针对zkview平台上嵌入机器视觉算法的框架

## 一、简介

这是一个针对zkview工业互联平台组态软件所设计的机器视觉库，目前所嵌入的算法以YOLO为主，主要包括了YOLOv3、YOLOv5、YOLOx。用以完成目标检测任务。其识别任务的基本流程是：

摄像头输入 -> 目标检测算法逐帧检测 -> ffmpeg逐帧推流成FLV格式视频流 ->前端展示视频

1、摄像头输入设置包括但不限于： 本地摄像头、流媒体摄像头（支持RTSP、RTMP、HTTP-flv等格式摄像头）、各种格式视频（.mp4、.mov、.avi等）

2、目标检测算法：本项目开发一套基于TensorRT库高性能YOLO推理框架，使用cuda实现了对于输入图片的批量并行预处理、实现了模型的Int8量化、实现了消费者-生产者模型、将推理速度由70+ fps 提升到 150+ fps

3、ffmpeg视频推流：目前实现了将检测图片由H264编码器编码后推流成FLV格式视频流

4、前端展示页面：zkview 专业的工业互联网WEB组态软件，共享资源，海量组件，高效开发前端页面

![main](doc/img/main.gif)

- 🖥️ [zkview](https://zkview.com/)：专业的工业互联网WEB组态软件

- 🚀 [Yolo](https://github.com/ultralytics/yolov5)：Real-Time Object Detection

- 🌐 [TensorRT](https://developer.nvidia.com/tensorrt)：用于高性能深度学习推理的 SDK

- ▶️  [ffmpeg](https://ffmpeg.org/)：快速的视频和音频转换工具，从实时的视频源中抓取流媒体文件

- 🖼️ [OpenCV](https://opencv.org/)：跨平台的计算机视觉库

## 二、搭建开发环境

### 2.1 [安装TensorRT](doc/installTensorRT.md)

### 2.2 [安装OpenCV](doc/installOpenCV.md)

### 2.3 [安装FFmpeg](doc/installFFmpeg.md)