'''
description: 
version: 
Author: zwy
Date: 2023-04-04 11:18:33
LastEditors: zwy
LastEditTime: 2023-04-04 11:19:56
'''
import os
path="/home/zwy/CWorkspace/zkView_yolo/yolov5/InstrumentDataset/image/val"

#获取该目录下所有文件，存入列表中
fileList=os.listdir(path)

n=0
for i in fileList:
    
    #设置旧文件名（就是路径+文件名）
    oldname=path+ os.sep + fileList[n]   # os.sep添加系统分隔符
    
    #设置新文件名
    newname=path + os.sep + fileList[n].split(".jpg")[0]+'.JPG'
    
    os.rename(oldname,newname)   #用os模块中的rename方法对文件改名
    print(oldname,'======>',newname)
    
    n+=1