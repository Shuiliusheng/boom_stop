### event log processing
1. 用途
    - 整理程序运行生成的log文件
    - 提供分析event事件之间的相关性的一些简单方法
2. 文件介绍
    - event.py: 包含读取和保存log文件中的事件信息EventInfo，提供了直接进行操作的方法，例如不同事件数值之间的加减乘除
        - class StartInfo: 用于记录事件组开始的信息，要求log中的格式为：{"type": "max_inst", "cycles": 533340278, "inst": 221043002}
        - class EventInfo: 用于记录不同事件的所有信息，包括名称和所有value，要求log中格式为：{"type": "event  0", "value":  467700375}
        - read_eventlist()
            - 用于从当前目录的eventlist文件中，获取事件号和事件名的对应关系
            - 返回值为一个字典eventdict（eventn: eventname）
        - readEventInfo(filename, eventdict)
            - 用于从文件中获取所有事件的信息，将其存放到不同的EventInfo对象中，并且根据eventdict设置对应的事件名
            - 返回值为startinfo, eventinfos：前者为每个事件开始第一句话的信息（max_inst），后者为所有事件的信息
        - saveEventInfo(startinfo, eventinfos, optype, benchname)
            - 将startinfo和eventinfos中的信息存储到csv文件中
            - optype："h"/"v"，代表两种存放的格式，互为转置
        - cal_fraction(numerator, denominator, name)
            - numerator为分子的事件EventInfo对象，denominator为分母事件EventInfo对象，name为结果EventInfo对象内部记录的名称
            - 结果返回一个EventInfo对象，对象的value元素为分子/分母的结果
        - cal_percentage(einfos)
            - einfos为一组EventInfo事件对象，该函数用于计算每个事件在该组所有事件的值的占比情况
            - 返回值为一组EventInfo事件对象
    - analysis.py: 提供对不同的EventInfo进行相关性分析的一些方法
        - class CorInfo：相关性信息的类，重载了比较操作，主要记录相关值和两个事件的名字
        - cal_correlation(info1, info2)
            - 计算EventInfo对象info1和info2之间的相关性
            - 返回值为CorInfo对象
        - find_max_cor(baseinfo, infos, topnum, min_step, max_times)
            - 计算baseinfo和一组事件infos的相关性，并且从中通过组合infos来选择相关性最大的组合事件
            - topnum：表示选择相关性最高的topnum个事件组合
            - min_step：表示刚这一轮和上一轮相关性最高值的增值的最小幅度，如果低于该幅度，则停止寻找
            - max_times：表示最多寻找多少轮
            - 返回值为一组CorInfo对象
        - saveCorInfo(corinfos, benchname)
            - 将corinfos保存到文件中
    - process.py: 主文件，用于直接执行
        - 主要在process_infos函数中组合eventinfo对象，并且计算一些相关性
        ```python
            # 读取事件名称
            eventdict = read_eventlist()
            # 从log文件中获取信息
            startinfo, eventinfos = readEventInfo(sys.argv[1], eventdict)
            # 计算一些相关性，同时增加一些事件
            corinfos = process_infos(startinfo, eventinfos)
            # 保存event信息到csv文件中
            saveEventInfo(startinfo, eventinfos, "h", sys.argv[1])
            # 保存相关性信息到csv文件中
            saveCorInfo(corinfos, sys.argv[1])
        ```