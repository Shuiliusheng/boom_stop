import os
import json
import sys
import struct
import numpy as np

# 记录起始点的信息，以type = max_inst为标记进行识别
class StartInfo:
    inst = []
    cycle = []
    ipc = []
    name = ""
    def __init__(self, name):
        self.name = name
        self.inst = []
        self.cycle = []

    def addinfo(self, inst, cycle):
        self.inst.append(inst)
        self.cycle.append(cycle)
        self.ipc.append(1.0*inst/cycle)

    def getSize(self):
        return len(self.inst)

    def showinfo(self):
        idx = 0
        while idx < len(self.ipc):
            print(self.ipc[idx], end=", ")
            idx = idx + 1
        print("")


# 用于记录每个事件的所有数据信息，支持直接+ - * /
class EventInfo:
    value = []
    label = []
    name = ""
    def __init__(self, name):
        self.name = name
        self.value = []
        self.label = []

    # 记录值和label，label用于表示是第几组中的event
    def addinfo(self, value, label):
        self.value.append(value)
        self.label.append(label)

    # 根据eventname判断两个事件是否相同
    def isThis(self, name):
        return (name == self.name)

    # 根据optype完成各种EventInfo的value的操作，要求value的长度相同才行
    def op(self, other, optype):
        temp = EventInfo("none")
        idx = 0
        if isinstance(other, float) or isinstance(other, int):
            temp.name = "(" + self.name + optype + str(other) + ")"
            while idx < len(self.value):
                if optype == "-":
                    val = self.value[idx] - other
                elif optype == "*":
                    val = self.value[idx] * other
                elif optype == "/":
                    val = 1.0 * self.value[idx] / other
                else:
                    val = self.value[idx] + other
                temp.addinfo(val, self.label[idx])
                idx = idx + 1

        if isinstance(other, EventInfo):
            temp.name = "(" + self.name + optype + other.name + ")"
            if len(other.value) != len(self.value):
                return temp
            while idx < len(self.value):
                if optype == "-":
                    val = self.value[idx] - other.value[idx]
                elif optype == "*":
                    val = self.value[idx] * other.value[idx]
                elif optype == "/":
                    val = 1.0 * self.value[idx] / other.value[idx]
                else:
                    val = self.value[idx] + other.value[idx]
                temp.addinfo(val, self.label[idx])
                idx = idx + 1
        return temp

    # 重载加减乘除运算
    def __add__(self, other):
        return self.op(other, "+")

    def __sub__(self, other):
        return self.op(other, "-")

    def __mul__(self, other):
        return self.op(other, "*")

    def __truediv__(self, other):
        return self.op(other, "/")


# ----------------------------------------------------------------------------------

# 读取所有事件的名称
def read_eventlist():
    if not os.path.exists("eventlist") :
        print("the file eventlist is not in this directory!")
        return 0

    file1 = open("eventlist") 
    eventdict = {}
    while 1:
        line = file1.readline()
        if not line:
            break
        words = line.replace(" ", "").replace("\n", "").split(',')
        eventdict[words[0]] = words[1]
        # print(words)
    file1.close()
    return eventdict


# einfos为一组事件，计算einfos中每一个事件占einfos所有事件数量的比例
def cal_percentage(einfos):
    totalinfo = einfos[0]
    idx = 1
    while idx < len(einfos):
        totalinfo = totalinfo + einfos[idx]
        idx = idx + 1

    res = []
    idx = 0
    while idx < len(einfos):
        info = einfos[idx] / totalinfo
        info.name = einfos[idx].name + "_perc"
        res.append(info)
        idx = idx + 1
    return res

# 计算比例
def cal_fraction(numerator, denominator, name):
    temp = numerator / denominator
    if len(name) != 0:
        temp.name = name
    return temp


# -----------------------------------------------------------------
# 将startinfo和eventinfos中的信息保存成csv文件，用于excel查看，optype分为h和v，为两种不同的存储方式
def saveEventInfo(startinfo, eventinfos, optype, benchname):
    bench = os.path.splitext(benchname)[0]
    savename = bench+"_eventinfo_"+optype+".csv"
    f1 = open(savename, 'w')

    data = []
    if optype == "v":
        data.append("label," + "total_ipc,")
        idx = 0
        for ipc in startinfo.ipc:
            data.append(str(idx+1)+","+str(ipc)+",")
            idx = idx + 1

        for info in eventinfos:
            data[0] = data[0] + info.name + ","
            idx = 0
            while idx < len(info.value):
                if idx+1 == info.label[idx]:
                    data[idx+1] = data[idx+1] + str(info.value[idx])
                data[idx+1] = data[idx+1] + ","
                idx = idx + 1                   
    else:
        str1 = ","
        str2 = "total_ipc,"
        idx = 0
        while idx < len(startinfo.ipc):
            str1 = str1 + str(idx+1) + ","
            str2 = str2 + str(startinfo.ipc[idx]) + ","
            idx = idx + 1
        data.append(str1)
        data.append(str2)

        for info in eventinfos:
            str1 = info.name + ","
            idx = 0
            while idx < len(info.value):
                if idx+1 == info.label[idx]:
                    str1 = str1 + str(info.value[idx])
                str1 = str1 + ","
                idx = idx + 1  
            data.append(str1)
    
    for d in data:
        f1.write(d + "\n")


# 从运行的log文件中获取startinfo和event事件信息
# 要求：文件由多组事件组成，每一组事件的格式为：
# {"type": "max_inst", "cycles": 533340278, "inst": 221043002} #起始
# {"type": "event  0", "value":  467700375} #具体事件
# {"type": "event  1", "value":  200005587}
def readEventInfo(filename, eventdict):
    file1 = open(filename) 
    startinfo = StartInfo("max_inst")
    eventinfos = []
    numline = 0
    while 1:
        line = file1.readline()
        numline += 1
        if not line:
            break
        if line.find("{") == -1 and line.find("}") == -1:
            continue

        if line.find("{") == -1 or line.find("}") == -1:
            print(numline, line)
            continue

        info = json.loads(line)

        if info['type'] == "max_inst":
            startinfo.addinfo(info['inst'], info['cycles'])
            
        else:
            typeinfo = info['type']
            idx = 0
            while idx < len(eventinfos):
                if eventinfos[idx].isThis(typeinfo):
                    break
                idx = idx + 1
            if idx == len(eventinfos):
                einfo = EventInfo(typeinfo)
                einfo.addinfo(info['value'], startinfo.getSize())
                eventinfos.append(einfo)
            else:
                einfo = eventinfos[idx]
                einfo.addinfo(info['value'], startinfo.getSize())

    file1.close()

    for info in eventinfos:
        name = info.name.replace(" ", "")
        if name in eventdict:
            info.name = eventdict[name]

    return startinfo, eventinfos