clc,clear,close all;

try
    s=serialport('com16',115200);
catch
    error('cant serial');
end
% set(s,'BaudRate', 115200,'DataBits',8,'StopBits',1,'Parity','none','FlowControl','none');
% fopen(s);

while 1
    out = readline(s);
    disp(out);
end
