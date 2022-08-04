#!/usr/bin/env python3


import os
import json
import sys
import struct
import numpy as np

from event import *
from analysis import *

#-------------------------------------------------------------------------------------
def process_infos(startinfo, eventinfos, eventsets):

    # 处理一些原始数据，计算一些比例，占比等参数
    eventinfos.append(cal_fraction(eventsets['insts'], eventsets['cycles'], "user_ipc"))
    eventinfos.append(cal_fraction(eventsets['icache_access'] - eventsets['icache_hit'], eventsets['icache_access'], "icache_miss_rate"))
    eventinfos.append(cal_fraction(eventsets['icache_to_L2'], eventsets['icache_access'], "icache_to_l2_rate"))
    eventinfos.append(cal_fraction(eventsets['itlb_access'] - eventsets['itlb_hit'], eventsets['itlb_access'], "itlb_miss_rate"))
    eventinfos.append(cal_fraction(eventsets['itlb_to_ptw'], eventsets['itlb_access'], "itlb_to_ptw_rate"))

    eventinfos.append(cal_fraction(eventsets['dcache_nack_num'], eventsets['dcache_access'], "dcache_nack_rate"))
    eventinfos.append(cal_fraction(eventsets['dcache_to_L2'], eventsets['dcache_access'], "dcache_to_l2_rate"))
    eventinfos.append(cal_fraction(eventsets['dtlb_miss_num'], eventsets['dtlb_access'], "dtlb_miss_rate"))
    eventinfos.append(cal_fraction(eventsets['dtlb_to_ptw'], eventsets['dtlb_access'], "dtlb_to_ptw_rate"))

    eventinfos.append(cal_fraction(eventsets['brmask_stall'], eventsets['cycles'], "brmask_stall_cycle_rate"))
    eventinfos.append(cal_fraction(eventsets['dis_rob_stall'], eventsets['cycles'], "dis_rob_stall_cycle_rate"))
    eventinfos.append(cal_fraction(eventsets['spec_miss_issuop'], eventsets['iss_uop_num'], "misspec_issuop_rate"))
    eventinfos.append(cal_fraction(eventsets['rollback_cycles'], eventsets['cycles'], "rollback_cycles_rate"))

    eventinfos.append(cal_fraction(eventinfos[58] + eventinfos[59] + eventinfos[60] + eventinfos[61], eventinfos[1], "com_excpt_rate"))
    eventinfos.extend(cal_percentage(eventinfos[58:62]))  # exception rate

    eventinfos.append(cal_fraction((eventinfos[43] + eventinfos[44]) * 1000, eventinfos[1], "exe_misp_MPKI"))
    tinfos = []
    tinfos.append(eventsets['exe_misp_br'])   # exe mis br
    tinfos.append(eventsets['exe_misp_ret'])   # exe mis ret
    tinfos.append(eventsets['exe_misp_jalrcall'])   # exe mis jalrcall
    temp = eventsets['exe_misp_jalr'] - eventsets['exe_misp_ret'] - eventsets['exe_misp_jalrcall']
    temp.name = "exe_misp_jalr_else"
    tinfos.append(temp)   # exe mis else jalr
    eventinfos.extend(cal_percentage(tinfos[0:4]))  # exe misp rate

    eventinfos.append(cal_fraction((eventinfos[53] + eventinfos[54]) * 1000, eventinfos[1], "com_misp_MPKI"))
    tinfos1 = []
    tinfos1.append(eventinfos[53])   # com mis br
    tinfos1.append(eventinfos[55])   # com mis ret
    tinfos1.append(eventinfos[56])   # com mis jalrcall
    temp = eventinfos[54] - eventinfos[55] - eventinfos[56]
    temp.name = "com_misp_jalr_else"
    tinfos1.append(temp)   # exe mis else jalr
    eventinfos.extend(cal_percentage(tinfos1[0:4])) # com misp rate

    eventinfos.extend(cal_percentage(eventinfos[8:12]))   # npc source percentage
    eventinfos.extend(cal_percentage(eventinfos[12:15]))  # fb out percentage
    eventinfos.extend(cal_percentage(eventinfos[15:18]))  # dec out percentage
    eventinfos.extend(cal_percentage(eventinfos[20:23]))  # dis out percentage
    eventinfos.extend(cal_percentage(eventinfos[27:30]))  # iss out percentage
    eventinfos.extend(cal_percentage(eventinfos[31:33]))  # exe load store percentage
    eventinfos.extend(cal_percentage(eventinfos[47:49]))  # com load store percentage

    eventinfos.append(cal_fraction(eventsets['com_misp_br'] + eventsets['com_misp_jalr'], eventsets['com_is_br'] + eventsets['com_is_jalr'], "com_mispred_rate"))
    

    # 计算每个事件和ipc之间的相关性
    cor_res = []
    user_ipc = cal_fraction(eventinfos[1], eventinfos[0], "user_ipc")
    for infos in eventinfos:
        cor_res.append(cal_correlation(user_ipc, infos))

    cor_res = sorted(cor_res, reverse=True)

    # 挑选一些事件，通过find_max_cor找到相关性最大的一些组合排列
    tempinfos = []
    tempinfos.append(eventinfos[2]-eventinfos[3])
    tempinfos.append(eventinfos[5]-eventinfos[6])
    # tempinfos.append(eventinfos[8])
    tempinfos.append(eventinfos[9])
    tempinfos.append(eventinfos[10])
    tempinfos.append(eventinfos[11])
    tempinfos.append(eventinfos[12])
    tempinfos.append(eventinfos[18])
    tempinfos.append(eventinfos[19])
    tempinfos.append(eventinfos[30])
    tempinfos.append(eventinfos[34])
    tempinfos.append(eventinfos[37])
    tempinfos.append(eventinfos[43])
    tempinfos.append(eventinfos[44])
    tempinfos.append(eventinfos[45])
    tempinfos.append(eventinfos[46])
    tempinfos.append(eventinfos[61])
    tempinfos.append(eventinfos[62])

    # find_max_cor(baseinfo, infos, topnum, min_step, max_times)
    corinfos = find_max_cor(user_ipc, tempinfos, 10, 0.02, 10)
    # for cor in corinfos:
    #     cor.show()
    
    return corinfos



#----------------------------------------------------------------------------------#

if len(sys.argv) < 2: 
    print("parameters are not enough.\n ./process.py logfile")
    exit()

if not os.path.exists(sys.argv[1]):
    print(sys.argv[1]+" is not exist.")
    exit()

# 读取事件名称
eventdict = read_eventlist()
# 从log文件中获取信息
startinfo, eventinfos, eventsets = readEventInfo(sys.argv[1], eventdict)
# 计算一些相关性，同时增加一些事件
corinfos = process_infos(startinfo, eventinfos, eventsets)
# 保存event信息到csv文件中
saveEventInfo(startinfo, eventinfos, "h", sys.argv[1])
# 保存相关性信息到csv文件中
saveCorInfo(corinfos, sys.argv[1])