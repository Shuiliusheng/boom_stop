import os
import json
import sys
import struct
import numpy as np

from event import *

class CorInfo:
    name1 = ""
    name2 = ""
    cor = 0.0
    indices = []
    info2 = EventInfo("none")
    def __init__(self, name1, name2, cor):
        self.name1 = name1
        self.name2 = name2
        self.cor = cor
        self.indices = []
        self.info2 = EventInfo("none")

    def show(self):
        print(self.name1 + " & " + self.name2 + " corv :", self.cor)

    def __eq__(self, other):
        return abs(self.cor) == abs(other.cor)

    def __le__(self, other):
        return abs(self.cor) < abs(other.cor)

    def __gt__(self, other):
        return abs(self.cor) > abs(other.cor)

# -----------------------------------------------------------------------------------
# 计算info1和info2的相关性
def cal_correlation(info1, info2):
    if not(np.any(info1.value)) or not(np.any(info2.value)):
        return CorInfo(info1.name, info2.name, 0.0)

    ab = np.array([info1.value, info2.value])
    cor_info = np.corrcoef(ab)
    res = CorInfo(info1.name, info2.name, cor_info[0,1])
    return res

# -----------------------------------------------------------------------------------
# find_max_cor的辅助函数，用于从cor中选择最高的num个，如果没有这么多，则全选
def get_top_cor(cor_res, num):
    t = sorted(cor_res, reverse=True)
    if num > len(t):
        num = len(t)
    res = []
    idx = 0
    while idx < num:
        res.append(t[idx])
        idx = idx + 1
    return res

# find_max_cor的辅助函数，用于清除组合相同的cors
def remove_same_arr(cors):
    idx = 1
    res = []
    cors[0].indices = sorted(cors[0].indices)
    res.append(cors[0])
    while idx < len(cors):
        is_diff = True
        idx1 = 0
        cors[idx].indices = sorted(cors[idx].indices)
        while idx1 < len(res):
            if len(res[idx1].indices) == len(cors[idx].indices) and np.sum(cors[idx].info2.value[1:15]) == np.sum(res[idx1].info2.value[1:15]):
                is_diff = False
                break
            idx1 = idx1 + 1
        if is_diff:
            res.append(cors[idx])
        idx = idx + 1
    return res


# 从infos一组事件中通过+/-的方式进行组合，得到和baseinfo的cor最高的结果
# topnum：选择所有可能的组合中最相关的topnum个
# min_step：当此轮的结果的最相关值提升的小于min_step，则停止
# max_times：最多筛选多少轮
def find_max_cor(baseinfo, infos, topnum, min_step, max_times):
    cor1 = []
    res = []
    idx = 0
    while idx < len(infos):
        t = cal_correlation(baseinfo, infos[idx])
        t.indices.append(idx)
        t.info2 = infos[idx]
        cor1.append(t)
        idx = idx + 1
    
    print("-------- start find max correlation ----------")
    c = 0
    last_top_cor = 0.0
    while c < max_times:
        cor1 = remove_same_arr(cor1)
        tcors = get_top_cor(cor1, topnum)
        res.extend(tcors[0:int(len(tcors)/5)])
        print("-------- running " + str(c) + ", max cor: " + str(abs(tcors[0].cor)))
        if abs(abs(last_top_cor) - abs(tcors[0].cor)) < min_step:
            break
        last_top_cor = tcors[0].cor
        cor1 = []
        idx = 0
        while idx < len(tcors):
            idx1 = 0
            cor1.append(tcors[idx])
            while idx1 < len(infos):
                if tcors[idx].indices.count(idx1) != 0:
                    idx1 = idx1 + 1
                    continue
                tinfo = tcors[idx].info2 + infos[idx1]
                t = cal_correlation(baseinfo, tinfo)
                if tcors[idx] < t:
                    t.indices.extend(tcors[idx].indices)
                    t.indices.append(idx1)
                    t.info2 = tinfo
                    cor1.append(t)

                tinfo1 = tcors[idx].info2 - infos[idx1]
                t1 = cal_correlation(baseinfo, tinfo1)
                if tcors[idx] < t1:
                    t1.indices.extend(tcors[idx].indices)
                    t1.indices.append(idx1)
                    t1.info2 = tinfo1
                    cor1.append(t1)
                idx1 = idx1 + 1
            idx = idx + 1
        c = c+1

    res.extend(get_top_cor(remove_same_arr(cor1), topnum))
    return res


# 将corinfos信息保存到csv文件中，便于查看
def saveCorInfo(corinfos, benchname):
    bench = os.path.splitext(benchname)[0]
    savename = bench+"_corinfo_"+".csv"
    f1 = open(savename, 'w')

    infos = sorted(corinfos, reverse=True)
    data = []
    data.append("event1,event2,correlation")
    for info in infos:
        data.append(info.name1+","+info.name2+","+str(info.cor))
    
    for d in data:
        f1.write(d + "\n")