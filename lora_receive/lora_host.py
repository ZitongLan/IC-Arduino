import time
import serial
import datetime
import sched
import time
import threading

from apscheduler.schedulers.blocking import BlockingScheduler
a = 1
print(a)
ser = serial.Serial(port="COM16", baudrate=115200,
                    bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)

every_time = time.strftime('%Y-%m-%d %H:%M:%S')  # 时间戳
data = ''

data_loader = []

trans_value = 1
while 1:
    if trans_value == 51:
        trans_value = 1
    transmit_data = "%s data transmit\n" % trans_value
    ser.write(transmit_data.encode())
    print("write finish", transmit_data)
    
    trans_value += 1
    data = ser.readline()
    while data != b'Received\r\n':
        data = ser.readline()
        if data == b'Re-send\r\n':
            trans_value -= 1
            print("I am in re send")
            break
        elif data == b'trigger-restart\r\n':
            print("I am in restart")
            trans_value -= 1
            break
                # if time_start - time_end > 2:
                  # break
                # print(data)
    print(" %s" % (time.ctime(time.time())), 'Received ACK')
    time.sleep(0.3)


# def func():
#     data = ser.readline()
#     print(every_time, data)

# def func2():
#     #耗时2S
#     now = datetime.datetime.now()
#     ts = now.strftime('%Y-%m-%d %H:%M:%S')
#     print('do func2 time: ',ts)
#     # time.sleep(2)

# def dojob():
#     #创建调度器：BlockingScheduler
#     scheduler = BlockingScheduler()
#     #添加任务,时间间隔2S
#     scheduler.add_job(func, 'interval', seconds=2, id='test_job1')
#     #添加任务,时间间隔5S
#     scheduler.add_job(func2, 'interval', seconds=3, id='test_job2')

#     scheduler.start()

# dojob()

exitFlag = 0


class myThread (threading.Thread):  # 继承父类threading.Thread
    def __init__(self, threadID, name, counter):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.counter = counter

    def run(self):  # 把要执行的代码写到run函数里面 线程在创建后会直接运行run函数
        print("Starting " + self.name)
        print_time(self.threadID, self.name, self.counter, 5)
        print("Exiting " + self.name)


def print_time(threadID, threadName, delay, counter):
    global data
    if threadID == 1:
        while 1:
            tempa = 1
            # print("%s: %s" % (threadName, time.ctime(time.time())), data)
    if threadID == 2:
        trans_value = 1
        while 1:
            if trans_value == 51:
                trans_value = 1
            transmit_data = "data %s\n" % trans_value
            trans_value += 1
            ser.write(transmit_data.encode())
            print("write finish", transmit_data)
            time_start = time.time()
            data = ser.readline()
            while data != b'Received\r\n':
                data = ser.readline()
                if data == b'Re-send\r\n':
                    trans_value -= 1
                    break
                # if time_start - time_end > 2:
                    # break
                # print(data)
            print("%s: %s" % (threadName, time.ctime(time.time())), 'Received ACK')
            time.sleep(1)
            # if exitFlag:
            # (threading.Thread).exit()
            # time.sleep(delay)
            # counter -= 1


# 创建新线程
thread1 = myThread(1, "Thread-1", 2)
thread2 = myThread(2, "Thread-2", 1)

# 开启线程
thread1.start()
thread2.start()


# def time_printer():
#     now = datetime.datetime.now()
#     ts = now.strftime('%Y-%m-%d %H:%M:%S')
#     print('do func time :', ts)
#     print('I am herereerrerer')
#     loop_monitor()

# def receive_serial():
#     data = ser.readline()
#     print(every_time, data)
#     loop_monitor()

# def loop_monitor():
#     s = sched.scheduler(time.time, time.sleep)  # 生成调度器
#     s.enter(1, 1, time_printer, ())
#     s.enter(0.1,2,receive_serial,())
#     s.run()

# loop_monitor()

# while True:
#     data = ser.readline()
#     print(every_time, data)
