#/usr/bin/python3  #! check again why this should be here


# compile with: " gcc -fPIC -shared -o $NAME_OF_FILE$.so $NAME_OF_FILE$.c "
# any changes to the C-File need to be recompiled

#! for passing c-type strings: (bytes("HALLO\n", 'utf-8'))

import threading
import subprocess #TODO ???? do i need this
import sys
import os
import paramiko #ssh
import ctypes
from numpy.ctypeslib import ndpointer

import time
from datetime import datetime

import tkinter as tk            #TODO: import needed things only. othrwise namespace is flooded.
from tkinter.filedialog import askdirectory
from tkinter import messagebox
from tkinter import TRUE, FALSE
from tkinter import RIGHT, LEFT, CENTER
from tkinter import font





##-------------------------------------------------------------------
##--- C-Flag Dictionaries/Lists  ------------------------------------
##------------------------------------------------------------------- 

modeDict = {
    "Stream"    : " -mode stream",
    "Triggered" : " -mode trig",
}

odrDict = {
    "0,781 Hz"  : " -odr 0",
    "1,563 Hz"  : " -odr 1",
    "3,125 Hz"  : " -odr 2",
    "6,25 Hz"   : " -odr 3",
    "12,5 Hz"   : " -odr 4",
    "25 Hz"     : " -odr 5",
    "50 Hz"     : " -odr 6",
    "100 Hz"    : " -odr 7",
    "200 Hz"    : " -odr 8",
    "400 Hz"    : " -odr 9",
    "800 Hz"    : " -odr 10",
    "1600 Hz"   : " -odr 11",
    "3200 Hz"   : " -odr 12",
    "6400 Hz"   : " -odr 13",
    "12800 Hz"  : " -odr 14",
    "25600 Hz"  : " -odr 15",
}

resolutionDict = {
    #"8-Bit"  : " -res 8",   # 8-Bit Resolution not implemented. In case it is needed later, i left it in.
    "16-Bit" : " -res 16",
}

readModeDict = {
    "Synchronous Read"                      : " -read sync0",
    #"Synchronous Read (Hardware Interrupt)" : " -read sync1",  # Not Implemented. In case it is needed later i left it in.
    "Asynchronous Read"                     : " -read async",
}

gRangeDict = {
    "2 g"   : " -g 2",
    "4 g"   : " -g 4",
    "8 g"   : " -g 8",
    "16 g"  : " -g 16",
}

triggerModeDict = {
    "offset"    : " -trig offset",
    "fixed"     : " -trig fixed",
}

edgeDetectDict = {
    "positive" : " -edge pos",
    "negative" : " -edge neg",
    "both"     : " -edge both",
}

triggerLogicDict = {
    "AND"      : " -logic 0",
    "OR"       : " -logic 1",
}

triggerBitmaskDict = {
    "X"        : " -axes x",
    "Y"        : " -axes y",
    "XY"       : " -axes xy",
    "Z"        : " -axes z",
    "XZ"       : " -axes xz",
    "YZ"       : " -axes yz",
    "XYZ"      : " -axes xyz",
}

##-------------------------------------------------------------------
##--- Global Variables  ---------------------------------------------
##------------------------------------------------------------------- 

# Init Config
MODE_INDEX      = 0
ODR_INDEX       = 1
RES_INDEX       = 2
READ_MODE_INDEX = 3
GRANGE_INDEX    = 4

# Trigger Config
TRIG_MODE_INDEX     = 0
EDGE_INDEX          = 1
TIME_BEFORE_INDEX   = 2
TIME_AFTER_INDEX    = 3
X_THRES_INDEX       = 4
Y_THRES_INDEX       = 5
Z_THRES_INDEX       = 6
LOGIC_INDEX         = 7
BITMASK_INDEX       = 8

# Axes
X_INDEX = 0
Y_INDEX = 1
Z_INDEX = 2


runThreads      = False
firstTrigCycle  = True
triggerCount    = 0

piSSH           = paramiko.SSHClient()

tcp = ""



##-------------------------------------------------------------------
##--- Function Definitions  -----------------------------------------
##-------------------------------------------------------------------


def kx132(initConfig, triggerConfig, outputPathStr, outputNameStr):
    global tcp
    global runThreads

    runThreads = True

    ssh_init_kx132(initConfig, triggerConfig)

    # tcp_c_lib_path = "C:\\Users\\User\\Documents\\HTW\\Bachelorarbeit\\Source\\test\\tcp_test\\tcp_multi_win.so"
    tcp_c_lib_path = ".\\tcp_multi_win.so"
    tcp = ctypes.CDLL(tcp_c_lib_path)


    # Need to specify return type of c-funtion to pointer
    tcp.tcp_single_read.restype         = ndpointer(dtype=ctypes.c_int16, shape=(3,))
    tcp.tcp_read_uint32.restype         = ndpointer(dtype=ctypes.c_uint32, shape=(1,))

    tcp.tcp_init()

    readThread = threading.Thread(target=tcp_read_data, args=(initConfig[MODE_INDEX], outputPathStr, outputNameStr))
    # sendThread = threading.Thread(target=tcp_send)

    # this should enable to kill the threads, if keyboard interrupt kills main
    readThread.daemon = True
    # sendThread.daemon = True

    readThread.start()
    # sendThread.start()

    readThread.join()
    # sendThread.join()

    tcp.tcp_close()

    print("All TCP threads terminated. Program finished.\n")


def getOffsetThresholdFlags(thresholdList):
    negBoundary = 0 #uint16_t
    posBoundary = 65536  #uint16_t
    for i in range (len(thresholdList)):
        if not( negBoundary <= thresholdList[i] <= posBoundary):
            print(f"[warning] Threshold #{i+1} was set out of boundary [{negBoundary} - {posBoundary}]. It will be set to +32000.")
            thresholdList[i] = 32000

    return f' -xO {thresholdList[X_INDEX]} -yO {thresholdList[Y_INDEX]} -zO {thresholdList[Z_INDEX]}' 

def getFixedThresholdFlags(thresholdList):
    negBoundary = -32768 #int16_t
    posBoundary = 32767  #int16_t
    for i in range (len(thresholdList)):
        if not( negBoundary <= thresholdList[i] <= posBoundary):
            print(f"[warning] Threshold #{i+1} was set out of boundary [{negBoundary} - {posBoundary}]. It will be set to +32000.")
            thresholdList[i] = 32000

    return f' -xF {thresholdList[X_INDEX]} -yF {thresholdList[Y_INDEX]} -zF {thresholdList[Z_INDEX]}'    

def getTimeFlags(timeList):
    return f' -t1 {timeList[0]} -t2 {timeList[1]}'  

def getInitFlags(initConfig):
    mode            = modeDict              [initConfig[MODE_INDEX]]
    odr             = odrDict               [initConfig[ODR_INDEX]]
    reso            = resolutionDict        [initConfig[RES_INDEX]]
    read            = readModeDict          [initConfig[READ_MODE_INDEX]]
    g               = gRangeDict            [initConfig[GRANGE_INDEX]]

    initString = mode + odr + reso + read + g

    return initString

def getTriggerFlags(triggerConfig):
    trig            = triggerModeDict       [triggerConfig[TRIG_MODE_INDEX]]
    edge            = edgeDetectDict        [triggerConfig[EDGE_INDEX]]
    logic           = triggerLogicDict      [triggerConfig[LOGIC_INDEX]]
    bitmask         = triggerBitmaskDict    [triggerConfig[BITMASK_INDEX]]
    
    timeList        = []
    timeList.append(main.timeBeforeTrig.get())
    timeList.append(main.timeAfterTrig.get())
    time            = getTimeFlags(timeList)

    thresholdList   = []
    thresholdList.append(triggerConfig[X_THRES_INDEX])
    thresholdList.append(triggerConfig[Y_THRES_INDEX])
    thresholdList.append(triggerConfig[Z_THRES_INDEX])

    if(triggerConfig[TRIG_MODE_INDEX] == 'offset'):
        thres = getOffsetThresholdFlags(thresholdList)
    elif(triggerConfig[TRIG_MODE_INDEX] == 'fixed'):
        thres = getFixedThresholdFlags(thresholdList)

    triggerString = trig + edge + logic + bitmask + time + thres

    return triggerString

def ssh_init_kx132(initConfig, triggerConfig):
    
    initString      = getInitFlags(initConfig)
    triggerString   = getTriggerFlags(triggerConfig)

    flags = initString + triggerString
    cmd = "sudo /home/pi/Documents/kx132/build/./kx132" + flags
    print(cmd)
    print()


    piSSH.load_system_host_keys()
    piSSH.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    piSSH.connect('100.200.150.42', username='pi', password='pipo')

    stdin, stdout, stderr = piSSH.exec_command(cmd) #TODO (stderr)? clear that here

    # stdin.write('<?php echo "Hello!"; sleep(2); ?>')
    # stdin.write('Hallo') 

    stdin.channel.shutdown_write()

    # print(f'STDOUT: {stdout.read().decode("utf8")}')
    # print(f'STDERR: {stderr.read().decode("utf8")}')

    
def ssh_close_kx132():
    global piSSH
    piSSH.close()

def tcp_quit():
    global tcp

    exitCmd = bytes("exit", 'utf-8')
    tcp.tcp_send(exitCmd)
    # tcp.tcp_close()


def tcp_read_data(modeConfig, outputPath, outputName):
    global tcp
    global runThreads
    global firstTrigCycle
    global triggerCount

    xyzData         = []
    formattedData   = []

    dateTimeStr     = datetime.now().strftime("_%d_%m_%Y__%H_%M_%S")

    filepath = outputPath + "/" + outputName + dateTimeStr + ".txt"

    modeStr    = main.switchModeVar.get()
    odrStr     = main.switchODRVar.get()
    gRangeStr  = main.gRangeVar.get()

    indexStr   = "index"
    xStr       = "x"
    yStr       = "y"
    zStr       = "z"



    if(modeConfig == 'Stream'):

        with open(filepath, 'w') as file:

            timestamp   = time.time()
            dateTimeStr = str(datetime.fromtimestamp(timestamp))

            file.write(dateTimeStr)
            file.write('\n')
            file.write(f'Modus:             {modeStr}\n')
            file.write(f'Frequenz:          {odrStr}\n')
            file.write(f'Empfindlichkeit:   {gRangeStr}\n')
            file.write('\n')
            file.write(f'{indexStr:>6},{xStr:>6},{yStr:>6},{zStr:>6}\n')

            count = 1

            while(runThreads):
                arrayOut = tcp.tcp_single_read()
                # print(f'{arrayOut}     #{count}')

                file.write(f'{count:>6},{arrayOut[X_INDEX]:>6},{arrayOut[Y_INDEX]:>6},{arrayOut[Z_INDEX]:>6}\n')

                count += 1

        tcp_quit()


    elif(modeConfig == 'Triggered'):
        normalizedData  = []
        bufferSize      = []
        formattedData   = []
    
        with open(filepath, 'w') as file:

            while(runThreads):

                normalizedData = []

                arrayOut = tcp.tcp_single_read()

                if(runThreads):
                    triggerCount += 1
                    main.toggleTriggerSignal()
                

                xNormalized = arrayOut[X_INDEX]
                yNormalized = arrayOut[Y_INDEX]
                zNormalized = arrayOut[Z_INDEX]

                normalizedData.append(xNormalized)
                normalizedData.append(yNormalized)
                normalizedData.append(zNormalized)


                if(firstTrigCycle):
                    main.setNormalizedData(normalizedData)
                    firstTrigCycle = False

                bufferSize = tcp.tcp_read_uint32()
                bufferSize = int(bufferSize[0])

                triggerIndex = tcp.tcp_read_uint32()
                triggerIndex = int(triggerIndex[0])


                timestamp       = time.time()
                dateTimeStr     = str(datetime.fromtimestamp(timestamp))

                trigModeStr     = main.trigModeVar.get()
                edgeDetectStr   = main.edgeDetVar.get()
                logicStr        = main.trigLogicVar.get()
                bitmaskStr      = main.trigBitmaskVar.get()
                
                xThres          = main.xThresEntry.get()
                yThres          = main.yThresEntry.get()
                zThres          = main.zThresEntry.get()
                
                timeBefore      = main.timeBeforeEntry.get()
                timeAfter       = main.timeAfterEntry.get()

                if(runThreads):

                    file.write(f'START_OF_BLOCK_{triggerCount}\n')
                    file.write(f'\n')
                    file.write(dateTimeStr)
                    file.write('\n')
                    file.write(f'Modus:                     {modeStr}\n')
                    file.write(f'Frequenz:                  {odrStr}\n')
                    file.write(f'Empfindlichkeit:           {gRangeStr}\n')
                    file.write('\n')
                    file.write(f'Trigger-Modus:             {trigModeStr}\n')
                    file.write(f'Flankenerkennung:          {edgeDetectStr}\n')
                    file.write(f'Logik:                     {logicStr}\n')
                    file.write(f'Bitmaske:                  {bitmaskStr}\n')
                    file.write('\n')
                    file.write(f'Zeit vor Trigger  (ms): {timeBefore}\n')
                    file.write(f'Zeit nach Trigger (ms): {timeAfter}\n')
                    file.write('\n')
                    file.write(f'X_Normalized:  {xNormalized:>6}\n')
                    file.write(f'Y_Normalized:  {yNormalized:>6}\n')
                    file.write(f'Z_Normalized:  {zNormalized:>6}\n')
                    file.write('\n')
                    file.write(f'X_Threshold:   {xThres:>6}\n')
                    file.write(f'Y_Threshold:   {yThres:>6}\n')
                    file.write(f'Z_Threshold:   {zThres:>6}\n')
                    file.write('\n')
                    file.write(f'Anzahl Samples:    {bufferSize}\n')
                    file.write(f'Trigger bei Index: {triggerIndex}\n')
                    file.write('\n')
                    file.write('\n')
                    file.write(f'{indexStr:>6},{xStr:>6},{yStr:>6},{zStr:>6}\n')

                    for i in range (bufferSize):
                        
                        arrayOut = tcp.tcp_single_read()

                        xData = arrayOut[X_INDEX]
                        yData = arrayOut[Y_INDEX]
                        zData = arrayOut[Z_INDEX]


                        # print(f'#{i:>5}#   {xData:>6},{yData:>6},{zData:>6}')
                        
                        file.write(f'{(i+1):>6},{xData:>6},{yData:>6},{zData:>6}\n')

                    file.write(f'\n')
                    file.write(f'END_OF_BLOCK_{triggerCount}\n')
                    file.write(f'\n\n\n')

                    main.toggleTriggerSignal()

    return



def tcp_send():
    global tcp
    global runThreads

    while(runThreads):
        string      = (input("Enter Command [max 256 chars]: "))[:256]
        stringBytes = bytes(string, 'utf-8') # acts as a ctype string pointer

        tcp.tcp_send(stringBytes)
        
        if(string[:4] == "exit"):
            runThreads = False
        exit()

def tcp_send_from_button(string):
    global tcp

    stringBytes = bytes(string, 'utf-8') # acts as a ctype string pointer
    
    tcp.tcp_send(stringBytes)
        

##-------------------------------------------------------------------
##--- TkInter  ---------------------------------------------------------
##-------------------------------------------------------------------
class mainWindow:
    def __init__(self, root):
        global triggerCount

        root.geometry('1100x620')
        root.title('KX132 Accelerometer GUI')

        ##-----------------------------------------------------------
        ##  FRAMES
        ##-----------------------------------------------------------
        self.InitFrame   = tk.Frame(root, width=1050, height=120, highlightthickness= 1, highlightbackground="black")
        self.TrigFrame   = tk.Frame(root, width=1050, height=200, highlightthickness= 1, highlightbackground="black")
        self.ButtonFrame = tk.Frame(root, width=1050, height=120, highlightthickness= 1, highlightbackground="black")
        self.PathFrame   = tk.Frame(root, width=1050, height=80, highlightthickness= 1, highlightbackground="black")

        self.InitFrame.grid_propagate   (FALSE)
        self.TrigFrame.grid_propagate   (FALSE)
        self.ButtonFrame.grid_propagate (FALSE)
        self.PathFrame.grid_propagate   (FALSE)

        self.InitFrame.place    (x=25,y=20)
        self.TrigFrame.place    (x=25,y=160)
        self.ButtonFrame.place  (x=25,y=380)
        self.PathFrame.place    (x=25,y=520)


        ##-----------------------------------------------------------
        ##  VARIABLES
        ##-----------------------------------------------------------

        self.initConfigList             = []
        self.trigConfigList             = []
        
        ##--------------------
        ## INIT CONFIG
        ##--------------------
        self.switchModeVar              = tk.StringVar()
        self.switchODRVar               = tk.StringVar()
        self.switchResolutionVar        = tk.StringVar()
        self.switchReadModeVar          = tk.StringVar()
        self.gRangeVar                  = tk.StringVar()

        self.switchModeVar.set          ('Triggered')
        self.switchODRVar.set           ('25600 Hz')
        self.switchResolutionVar.set    ('16-Bit')
        self.switchReadModeVar.set      ('Synchronous Read')
        self.gRangeVar.set              ('8 g')

        self.switchModeChoices          = list(modeDict.keys())
        self.switchODRChoices           = list(odrDict.keys())
        self.switchResolutionChoices    = list(resolutionDict.keys())
        self.switchReadModeChoices      = list(readModeDict.keys())
        self.gRangeChoices              = list(gRangeDict.keys())

        ##--------------------
        ## TRIGGER CONFIG
        ##--------------------
        self.trigModeVar                = tk.StringVar()
        self.edgeDetVar                 = tk.StringVar()
        self.trigLogicVar               = tk.StringVar()
        self.trigBitmaskVar             = tk.StringVar()
        self.timeBeforeTrig             = tk.IntVar()
        self.timeAfterTrig              = tk.IntVar()
        self.xThresholdValue            = tk.IntVar()
        self.yThresholdValue            = tk.IntVar()
        self.zThresholdValue            = tk.IntVar()
        self.trigDetectSignal           = tk.StringVar()
        
        self.trigModeVar.set            ('offset')
        self.edgeDetVar.set             ('both')
        self.trigLogicVar.set           ("AND")
        self.trigBitmaskVar.set         ("X")
        self.timeBeforeTrig.set         (1)
        self.timeAfterTrig.set          (10)
        self.xThresholdValue.set        (8000)
        self.yThresholdValue.set        (8000)
        self.zThresholdValue.set        (8000)
        self.trigDetectSignal.set       ('FALSE')

        self.trigModeChoices            = list(triggerModeDict.keys())
        self.edgeDetChoices             = list(edgeDetectDict.keys())
        self.trigLogicChoices           = list(triggerLogicDict.keys())
        self.trigBitmaskChoices         = list(triggerBitmaskDict.keys())

        # self.trigModeVar.trace("w", self.changeThresholds)

        ##--------------------
        ## PATH CONFIG
        ##--------------------
        
        self.outputPathVar              = tk.StringVar()
        self.outputNameVar              = tk.StringVar()

        self.outputPathVar.set          (".\output\\")
        self.outputNameVar.set          ("kx132_output")          
        


        ##-----------------------------------------------------------
        ##  INIT FRAME
        ##-----------------------------------------------------------
        self.InitHeader                 = tk.Message(self.InitFrame, text=' Initial Config \n------------------------', width=300)

        self.ModeLabel                  = tk.Label(self.InitFrame, text='Modus',                    padx=10)
        self.OdrLabel                   = tk.Label(self.InitFrame, text='Frequenz',                 padx=10)
        self.ResLabel                   = tk.Label(self.InitFrame, text='Auflösung',                padx=10)
        self.ReadLabel                  = tk.Label(self.InitFrame, text='Lese-Modus',               padx=10)
        self.GLabel                     = tk.Label(self.InitFrame, text='Empfindlichkeit',          padx=10)
        
        self.ModeOpt                    = tk.OptionMenu(self.InitFrame, self.switchModeVar,         *self.switchModeChoices)
        self.OdrOpt                     = tk.OptionMenu(self.InitFrame, self.switchODRVar,          *self.switchODRChoices)
        self.ResOpt                     = tk.OptionMenu(self.InitFrame, self.switchResolutionVar,   *self.switchResolutionChoices)
        self.ReadOpt                    = tk.OptionMenu(self.InitFrame, self.switchReadModeVar,     *self.switchReadModeChoices)
        self.GOpt                       = tk.OptionMenu(self.InitFrame, self.gRangeVar,             *self.gRangeChoices)

        self.InitHeader.grid            (row=0, column=0)

        self.ModeLabel.grid             (row=1, column=0)  
        self.OdrLabel.grid              (row=1, column=1)   
        self.ResLabel.grid              (row=1, column=2)   
        self.ReadLabel.grid             (row=1, column=3)  
        self.GLabel.grid                (row=1, column=4)

        self.ModeOpt.grid               (row=2, column=0, padx=15)    
        self.OdrOpt.grid                (row=2, column=1, padx=15)     
        self.ResOpt.grid                (row=2, column=2, padx=15)     
        self.ReadOpt.grid               (row=2, column=3, padx=15)    
        self.GOpt.grid                  (row=2, column=4, padx=15)


        ##-----------------------------------------------------------
        ##  TRIGGER FRAME
        ##-----------------------------------------------------------
        self.TrigHeader                 = tk.Message(self.TrigFrame, text=' Trigger Config \n------------------------', width=300)

        self.TrigModeLabel              = tk.Label(self.TrigFrame, text='Trigger-Modus',                padx=10)
        self.EdgeLabel                  = tk.Label(self.TrigFrame, text='Flankenerkennung',             padx=10)
        self.trigLogicLabel             = tk.Label(self.TrigFrame, text='Logik',                        padx=10)
        self.trigBitmaskLabel           = tk.Label(self.TrigFrame, text='Bitmaske',                     padx=10)
        self.timeBeforeLabel            = tk.Label(self.TrigFrame, text='Zeit vor Trigger (ms)',        padx=10)
        self.timeAfterLabel             = tk.Label(self.TrigFrame, text='Zeit nach Trigger (ms)',       padx=10)
        self.xThresLabel                = tk.Label(self.TrigFrame, text='X-Threshold',                  padx=10)
        self.yThresLabel                = tk.Label(self.TrigFrame, text='Y-Threshold',                  padx=10)
        self.zThresLabel                = tk.Label(self.TrigFrame, text='Z-Threshold',                  padx=10)
        self.xNormalLabel               = tk.Label(self.TrigFrame, text='X-Normalized',                 padx=10)
        self.yNormalLabel               = tk.Label(self.TrigFrame, text='Y-Normalized',                 padx=10)
        self.zNormalLabel               = tk.Label(self.TrigFrame, text='Z-Normalized',                 padx=10)
        self.detectHeaderLabel          = tk.Label(self.TrigFrame, text='Trigger',                      padx=10)
        self.countHeaderLabel           = tk.Label(self.TrigFrame, text='Anzahl Trigger',               padx=10)          

        self.TrigOpt                    = tk.OptionMenu(self.TrigFrame, self.trigModeVar,    *self.trigModeChoices)
        self.EdgeOpt                    = tk.OptionMenu(self.TrigFrame, self.edgeDetVar,     *self.edgeDetChoices)
        self.LogicOpt                   = tk.OptionMenu(self.TrigFrame, self.trigLogicVar,   *self.trigLogicChoices)
        self.BitmaskOpt                 = tk.OptionMenu(self.TrigFrame, self.trigBitmaskVar, *self.trigBitmaskChoices)
        self.timeBeforeEntry            = tk.Entry(self.TrigFrame, textvariable=self.timeBeforeTrig,  justify=RIGHT, width = 10)
        self.timeAfterEntry             = tk.Entry(self.TrigFrame, textvariable=self.timeAfterTrig,   justify=RIGHT, width = 10)
        self.xThresEntry                = tk.Entry(self.TrigFrame, textvariable=self.xThresholdValue, justify=RIGHT, width = 10)
        self.yThresEntry                = tk.Entry(self.TrigFrame, textvariable=self.yThresholdValue, justify=RIGHT, width = 10)
        self.zThresEntry                = tk.Entry(self.TrigFrame, textvariable=self.zThresholdValue, justify=RIGHT, width = 10)
        self.xNormalValLabel            = tk.Label(self.TrigFrame, text='0',width = 10)
        self.yNormalValLabel            = tk.Label(self.TrigFrame, text='0',width = 10)
        self.zNormalValLabel            = tk.Label(self.TrigFrame, text='0',width = 10)
        self.trigDetectLabel            = tk.Label(self.TrigFrame, text=self.trigDetectSignal.get(),  font='bold', fg='red')
        self.triggerCountLabel          = tk.Label(self.TrigFrame, text=triggerCount)

        self.TrigHeader.grid            (row=0, column=0)

        self.TrigModeLabel.grid         (row=1, column=0)  
        self.EdgeLabel.grid             (row=1, column=1)  
        self.timeBeforeLabel.grid       (row=1, column=2)
        self.xThresLabel.grid           (row=1, column=3)   
        self.yThresLabel.grid           (row=1, column=4)  
        self.zThresLabel.grid           (row=1, column=5)
        self.countHeaderLabel.grid      (row=1, column=6)
        self.detectHeaderLabel.grid     (row=1, column=7)

        self.TrigOpt.grid               (row=2, column=0)    
        self.EdgeOpt.grid               (row=2, column=1)     
        self.timeBeforeEntry.grid       (row=2, column=2, pady=10)     
        self.xThresEntry.grid           (row=2, column=3, padx=30)    
        self.yThresEntry.grid           (row=2, column=4, padx=30)
        self.zThresEntry.grid           (row=2, column=5, padx=30)
        self.triggerCountLabel.grid     (row=2, column=6, padx=80)
        self.trigDetectLabel.grid       (row=2, column=7)

        self.trigLogicLabel.grid        (row=3, column=0, pady=10)
        self.trigBitmaskLabel.grid      (row=3, column=1, pady=10)
        self.timeAfterLabel.grid        (row=3, column=2, pady=10)
        self.xNormalLabel.grid          (row=3, column=3, pady=10)
        self.yNormalLabel.grid          (row=3, column=4, pady=10)
        self.zNormalLabel.grid          (row=3, column=5, pady=10)

        self.LogicOpt.grid              (row=4, column=0)     
        self.BitmaskOpt.grid            (row=4, column=1)     
        self.timeAfterEntry.grid        (row=4, column=2)     
        self.xNormalValLabel.grid       (row=4, column=3)
        self.yNormalValLabel.grid       (row=4, column=4)
        self.zNormalValLabel.grid       (row=4, column=5)


        ##-----------------------------------------------------------
        ##  BUTTON FRAME
        ##-----------------------------------------------------------
        self.ButtonHeader               = tk.Message(self.ButtonFrame,  text=' Buttons \n------------------------', width=300)

        self.startButton                = tk.Button(self.ButtonFrame,   text='Start',                     command=self.startButton,               width=14)
        self.stopButton                 = tk.Button(self.ButtonFrame,   text='Stop',                      command=self.stopButton,                width=14)
        self.resendButton               = tk.Button(self.ButtonFrame,   text='Trigger\naktualisieren',    command=self.resendTriggerConfigButton, width=14)
        self.quitButton                 = tk.Button(self.ButtonFrame,   text='Beenden',                   command=self.quitButton,                width=14)
        self.statusLabel                = tk.Label(self.ButtonFrame,    text='Status')
        self.statusVarLabel             = tk.Label(self.ButtonFrame,    text='STOP', font='bold', fg='red')

        self.ButtonHeader.grid          (row=0, column=0)
        self.statusLabel.grid           (row=0, column=4, padx=100)

        self.startButton.grid           (row=1, column=0, padx=20)  
        self.stopButton.grid            (row=1, column=1, padx=20)  
        self.resendButton.grid          (row=1, column=2, padx=20)   
        self.quitButton.grid            (row=1, column=3, padx=20)
        self.statusVarLabel.grid        (row=1, column=4)

        ##-----------------------------------------------------------
        ##  PATH FRAME
        ##-----------------------------------------------------------

        self.outputPathLabel            = tk.Label(self.PathFrame,      text='Zielordner auswählen')
        self.outputNameLabel            = tk.Label(self.PathFrame,      text='Name der Zieldatei')

        self.outputPathEntry            = tk.Entry(self.PathFrame,      textvariable=self.outputPathVar, width = 70)
        self.outputNameEntry            = tk.Entry(self.PathFrame,      textvariable=self.outputNameVar, width = 50)

        self.outputPathButton           = tk.Button(self.PathFrame,   text='Suchen',    command=self.pathButton,    width=14)


        self.outputPathLabel.grid           (row=0, column=0, padx=60, pady=7)  
        self.outputNameLabel.grid           (row=0, column=2, padx=60, pady=7)

        self.outputPathEntry.grid           (row=1, column=0, padx=15, pady=5)
        self.outputPathButton.grid          (row=1, column=1)
        self.outputNameEntry.grid           (row=1, column=2, padx=140, pady=5)  



    def toggleTriggerSignal(self):
        global triggerCount

        trigger = self.trigDetectSignal.get()

        if(trigger == 'FALSE'):
            # print('Trigger Detected.')
            self.trigDetectSignal.set('TRUE')
            self.trigDetectLabel.config(text=self.trigDetectSignal.get(), fg='green')

        elif(trigger == 'TRUE'):
            self.triggerCountLabel.config(text=triggerCount)
            self.trigDetectSignal.set('FALSE')
            self.trigDetectLabel.config(text=self.trigDetectSignal.get(), fg='red')



    def setNormalizedData(self, normalizedData):
        self.xNormalValLabel.config(text=str(normalizedData[X_INDEX]))
        self.yNormalValLabel.config(text=str(normalizedData[Y_INDEX]))
        self.zNormalValLabel.config(text=str(normalizedData[Z_INDEX]))


    def pathButton(self):
        print("Path Button pressed.")
        
        outputPathStr = askdirectory()
        
        if os.path.isdir(outputPathStr):
            self.outputPathVar.set(outputPathStr)
        else:
            messagebox.showinfo("Error", "The Path " + outputPathStr + " is not a directory")



    def stopButton(self):
        global runThreads
        print("Stopping Program.")

        runThreads = False

        ssh_close_kx132()

        try:
            tcp_quit()
        except:
            print("TCP-Socket couldn't be closed. Probably not started in the first place.")

        self.statusVarLabel.config(text='STOP', font='bold', fg='red')

        self.trigDetectSignal.set('FALSE')
        self.trigDetectLabel.config(text=self.trigDetectSignal.get(), fg='red')



    def resendTriggerConfigButton(self):
        self.getTrigConfig()
        triggerString = getTriggerFlags(self.trigConfigList)

        print(triggerString)

        sendButtonThread = threading.Thread(target=tcp_send_from_button, args=(triggerString,))
        sendButtonThread.daemon = True
        sendButtonThread.start()
        sendButtonThread.join()


    
    def startButton(self):
        global runThreads
        global firstTrigCycle
        global triggerCount

        if(runThreads):
            print('Program already running.')
            return

        firstTrigCycle = True

        self.statusVarLabel.config(text='AKTIV', font='bold', fg='green')

        self.trigDetectSignal.set('FALSE')
        self.trigDetectLabel.config(text=self.trigDetectSignal.get(), fg='red')

        triggerCount = 0
        self.triggerCountLabel.config(text=triggerCount)

        self.getInitConfig()
        self.getTrigConfig()

        outputPathStr = self.outputPathVar.get()
        outputNameStr = self.outputNameVar.get()

        mainLoopThread = threading.Thread(target=kx132, args=(self.initConfigList,self.trigConfigList, outputPathStr, outputNameStr))

        mainLoopThread.daemon = True
        mainLoopThread.start()
        # mainLoopThread.join()
        


    def quitButton(self):
        global runThreads
        print("Exiting Program.")

        runThreads = False

        try:
            tcp_quit()
        except:
            print("TCP-Socket couldn't be closed. Probably not started in the first place.")

        root.quit()
        exit()


    def getInitConfig(self):
        self.initConfigList = []

        self.initConfigList.append(self.switchModeVar.get())
        self.initConfigList.append(self.switchODRVar.get())
        self.initConfigList.append(self.switchResolutionVar.get())
        self.initConfigList.append(self.switchReadModeVar.get())
        self.initConfigList.append(self.gRangeVar.get())

    def getTrigConfig(self):
        self.trigConfigList = []

        self.trigConfigList.append(self.trigModeVar.get())
        self.trigConfigList.append(self.edgeDetVar.get())
        self.trigConfigList.append(self.timeBeforeTrig.get())
        self.trigConfigList.append(self.timeAfterTrig.get())
        self.trigConfigList.append(self.xThresholdValue.get())
        self.trigConfigList.append(self.yThresholdValue.get())
        self.trigConfigList.append(self.zThresholdValue.get())
        self.trigConfigList.append(self.trigLogicVar.get())
        self.trigConfigList.append(self.trigBitmaskVar.get())



##-------------------------------------------------------------------
##--- Main  ---------------------------------------------------------
##-------------------------------------------------------------------

if __name__ == "__main__":
    
    root = tk.Tk()
    main = mainWindow(root)
    root.mainloop()

    print("------------- PROGRAM FINISHED -------------")
    
