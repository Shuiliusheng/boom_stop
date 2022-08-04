#!/usr/bin/env python3


import os
import sys
import csv
import os
import sys
import math
import numpy as np
import matplotlib as mpl
import numpy as np

import matplotlib as mpl
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
from matplotlib.ticker import MultipleLocator
import matplotlib.ticker as ticker
from matplotlib.backends.backend_pdf import PdfPages
from pyparsing import alphanums
mpl.rcParams['font.sans-serif'] = ['Times New Roman']
mpl.rcParams['font.serif'] = ['Times New Roman']
mpl.rcParams['axes.unicode_minus'] = False

class TotalData:
    filename = ""
    name = ""
    totalvalue = []
    maxvalue = []
    minvalue = []
    
    def __init__(self, filename):
        self.filename = filename
        self.name = filename.split("_eventinfo")[0]
        self.totalvalue = []
        self.maxvalue = []
        self.minvalue = []
    
    def addinfo(self, words):
        if len(words[0]) < 2 or len(words) != 4:
            return 
        self.totalvalue.append(float(words[1]))
        self.maxvalue.append(float(words[2]))
        self.minvalue.append(float(words[3]))


def drawBar(values, names, xlabels, ylabel):
    fig, ax = plt.subplots()
    ccolors = plt.get_cmap('RdYlBu')(np.linspace(0.85, 0.15, len(values)))
    barw = 0.25
    step = barw*(len(values)+1)
    xticks = np.arange(0, step * len(xlabels), step)
    idx = 0
    while idx < len(values):
        ax.bar(xticks + idx*barw, values[idx], width=barw, label=names[idx], color=ccolors[idx])
        idx = idx + 1

    ax.set_ylabel(ylabel)
    ax.legend()
    print(xticks)
    print(xlabels)
    print(len(xticks))
    print(len(xlabels))
    ax.set_xticks(xticks + 0.5*barw*len(values))
    ax.set_xticklabels(xlabels, rotation=45, ha='right', fontsize=8)
    return fig

def getKeyData(filedatas, keyidx):
    totalvalues = []
    minvalues = []
    maxvalues = []
    idx = 0
    while idx < len(filedatas):
        print(filedatas[idx].totalvalue)
        totalvalues.append(filedatas[idx].totalvalue[keyidx])
        minvalues.append(filedatas[idx].minvalue[keyidx])
        maxvalues.append(filedatas[idx].maxvalue[keyidx])
        idx = idx + 1
    return [totalvalues, maxvalues, minvalues]

def getNames(filedatas):
    names = []
    for fdata in filedatas:
        names.append(fdata.name)
    return names

def drawKeyData(filedatas, keyname):
    values = getKeyData(filedatas, keyname)
    xlabel = getNames(filedatas)
    fig = drawBar(values, ["total", "max", "min"], xlabel, keyname)
    pp = PdfPages("resname.pdf")
    pp.savefig(fig, bbox_inches='tight', dpi=200)
    pp.close()


filedatas = []
def readdata(pathname, filename):
    f = open(pathname, 'r')
    reader = csv.reader(f)
    temp = TotalData(filename)
    for row in reader:
        temp.addinfo(row)
    f.close()
    filedatas.append(temp)

def getKeyIdxs(pathname, keyidxs):
    idx = 0
    f = open(pathname, 'r')
    reader = csv.reader(f)
    for row in reader:
        keyidxs[row[0]]  = idx - 1
        idx = idx + 1
    f.close()

def getfilenames(dirname):
    pathnames = []
    filenames = []
    for root, ds, fs in os.walk(dirname):
        for f in fs:
            if f.endswith('.csv') and ("total" in f):
                pathnames.append(os.path.join(root, f))
                filenames.append(f)
    return pathnames, filenames
                


if len(sys.argv) < 2: 
    print("parameters are not enough.\n ./process.py dirname")
    exit()

if not os.path.exists(sys.argv[1]):
    print(sys.argv[1]+" is not exist.")
    exit()

pathnames, filenames = getfilenames(sys.argv[1])
idx = 0
while idx < 3:
    f = open(pathnames[idx], 'r')
    reader = csv.reader(f)
    temp = TotalData(filenames[idx])
    for row in reader:
        temp.addinfo(row)
    f.close()
    filedatas.append(temp)
    idx = idx + 1

idx = 0
while idx < 3:
    print(filedatas[idx].totalvalue)
    print("")
    idx = idx + 1

keyidxs = {}
getKeyIdxs(pathnames[0], keyidxs)
# drawKeyData(filedatas, keyidxs["user_ipc"])
