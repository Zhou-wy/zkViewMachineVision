<!--
 * @description: 
 * @version: 
 * @Author: zwy
 * @Date: 2023-04-03 16:06:43
 * @LastEditors: zwy
 * @LastEditTime: 2023-04-06 10:35:52
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


## 三、编译源码

### 3.1 下载源码

```shell
git clone https://github.com/Zwyywz/zkViewMachineVision.git
```



### 3.2 搭建推流服务器

- 下载`nginx`和`nginx-http-flv-module`
  `nginx`和`nginx-http-flv-module`源码下载命令：

```shell
wget http://nginx.org/download/nginx-1.22.1.tar.gz # nginx 源码
tar -zvxf nginx-1.22.1.tar.gz # 解压
cd nginx-1.22.1
git clone https://github.com/winshining/nginx-http-flv-module.git # nginx-http-flv-module 源码
```

- 编译`nginx `源码

```shell
./configure --add-module=./nginx-http-flv-module
make -j8
sudo make install
```

编译完成后， `nginx` 安装路径在`/usr/local/nginx/sbin/nginx`

更改nginx配置文件，参考`nginx-http-flv-module`给出的[配置文件](https://github.com/winshining/nginx-http-flv-module/blob/master/README.CN.md)

编辑`/usr/local/nginx/conf/nginx.conf`文件:

```txt
http {
    include       mime.types;
    default_type  application/octet-stream;

    keepalive_timeout  65;

    server {
        listen       80;

        location / {
            root   /var/www;
            index  index.html index.htm;
        }

        error_page   500 502 503 504  /50x.html;
        location = /50x.html {
            root   html;
        }

        location /live {
            flv_live on; #打开 HTTP 播放 FLV 直播流功能
            chunked_transfer_encoding on; #支持 'Transfer-Encoding: chunked' 方式回复

            add_header 'Access-Control-Allow-Origin' '*'; #添加额外的 HTTP 头
            add_header 'Access-Control-Allow-Credentials' 'true'; #添加额外的 HTTP 头
        }

        location /hls {
            types {
                application/vnd.apple.mpegurl m3u8;
                video/mp2t ts;
            }

            root /tmp;
            add_header 'Cache-Control' 'no-cache';
        }

        location /dash {
            root /tmp;
            add_header 'Cache-Control' 'no-cache';
        }

        location /stat {
            #推流播放和录制统计数据的配置

            rtmp_stat all;
            rtmp_stat_stylesheet stat.xsl;
        }

        location /stat.xsl {
            root /var/www/rtmp; #指定 stat.xsl 的位置
        }

        #如果需要 JSON 风格的 stat, 不用指定 stat.xsl
        #但是需要指定一个新的配置项 rtmp_stat_format

        #location /stat {
        #    rtmp_stat all;
        #    rtmp_stat_format json;
        #}

        location /control {
            rtmp_control all; #rtmp 控制模块的配置
        }
    }
}

rtmp_auto_push on;
rtmp_auto_push_reconnect 1s;
rtmp_socket_dir /tmp;

rtmp {
    out_queue           4096;
    out_cork            8;
    max_streams         128;
    timeout             15s;
    drop_idle_publisher 15s;

    log_interval 5s; #log 模块在 access.log 中记录日志的间隔时间，对调试非常有用
    log_size     1m; #log 模块用来记录日志的缓冲区大小

    server {
        listen 1935;
        server_name www.test.*; #用于虚拟主机名后缀通配

        application myapp {
            live on;
            gop_cache on; #打开 GOP 缓存，减少首屏等待时间
        }

        application hls {
            live on;
            hls on;
            hls_path /tmp/hls;
        }

        application dash {
            live on;
            dash on;
            dash_path /tmp/dash;
        }
    }

    server {
        listen 1935;
        server_name *.test.com; #用于虚拟主机名前缀通配

        application myapp {
            live on;
            gop_cache on; #打开 GOP 缓存，减少首屏等待时间
        }
    }

    server {
        listen 1935;
        server_name www.test.com; #用于虚拟主机名完全匹配

        application myapp {
            live on;
            gop_cache on; #打开 GOP 缓存，减少首屏等待时间
        }
    }
}
```

**特别注意： 需要关闭防火墙，或者开启1935端口**

```shell
sudo ufw allow 1935 
# or
sudo ufw disable
```

启动nginx服务

```shell
ps -ef | grep nginx #看一下是否有nginx 启动
# 如果有，则杀掉
kill 0000(上面输出的进程号)  
sudo /usr/local/nginx/sbin/nginx -c /usr/local/nginx/conf/nginx.conf # 启动nginx
```

如果没有任何输出，则表示正常启动，如果不放心，可用`ps -ef | grep nginx`查看



### 3.3 更改源码

只需更改项目根目录下 `main.cpp`中相关配置：

```c++
// demo 不用改
const std::string engine_file = "../workspace/yolov5s.engine";
const std::string onnx_file = "../workspace/yolov5s.onnx";

// 在main函数中

//改成自己的流媒体地址
std::string in_url = "rtsp://admin:admin123@192.168.0.212:554/cam/realmonitor?channel=1&subtype=0";
// 改成自己的IP， port 和 myapp/mystream 要与nginx配置保持一致
std::string out_url = "rtmp://192.168.0.113:1935/myapp/mystream";
// 视频的长宽和源视频fps, bitrate设置为3000000，可较为清晰推流1080P视频
int fps = 30, width = 1920, height = 1080, bitrate = 3000000;
```





### 3.4 更改CMakeLists.txt

只需更改项目根目录中`CMakeLists.txt`中关于第三方库的路径：

```cmake
set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc) # nvcc 编译器路径
set(CUDA_HOME /usr/local/cuda) # cuda路径
set(TRT_HOME /home/zwy/TensorRT-7.2.3.4) # TensorRT 路径
set(FFMPEG_PATH /usr/local/ffmpeg) # ffmpeg 路径
```

然后即可编译源码：

```shell
mkdir build && cd build
cmake ..
make -j8
```

![](doc/img/1.png)

![](doc/img/2.png)

生成的可执行文件在 项目根目录 `bin`下

![](doc/img/3.png)

## 四、搭配zkview平台，完成前端展示

具体使用可见 [zkview 教程](http://zkview.com/docs/start/#_1-%E8%BE%85%E5%8A%A9%E5%B7%A5%E5%85%B7)

**这里补充一点非常重要的事： rtmp 和 http-flv 转换关系**

```shell
rtmp://m_ip:m_port/m_app/m_stream
# 转换成 http-flv
http://m_ip/live?port=m_port&app=m_app&stream=m_stream
```



```shell
# 例如RTMP流地址为：
rtmp://192.168.0.113:1935/myapp/mystream
# 则其对应的flv地址是：
http://192.168.0.113/live?port=1935&app=myapp&stream=mystream
```



简单点来说既是，在画布中拖入`FLV格式`组件即可:

![](doc/img/4.png)

![](doc/img/5.png)

**实时预览**
![](doc/img/6.png)