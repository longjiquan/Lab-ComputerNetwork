#include "stdafx.h"
#include "GBNRdtSender.h"
#include "SeqStateEnum.h"
#include "Global.h"

//初始化滑动窗口状态
void GBNRdtSender::Init()
{
	//sendBuf = new Packet[seqsize];
	base = 0;
	nextSeqnum = 0;
}

bool GBNRdtSender::isInWindow(int seqnum)
{
	if (base < (base + wndsize) % seqsize)
	{
		return seqnum >= base && seqnum < (base + wndsize) % seqsize;
	}
	else
	{
		return seqnum >= base || seqnum < (base + wndsize) % seqsize;
	}
}

GBNRdtSender::GBNRdtSender():
	wndsize(4),seqsize(8),sendBuf(new Packet[8])
{
	Init();
}

GBNRdtSender::GBNRdtSender(int wsize, int sSize):
	wndsize(wsize),seqsize(sSize),sendBuf(new Packet[sSize])
{
	Init();
}


GBNRdtSender::~GBNRdtSender()
{
	delete[] sendBuf;
}

//上层调用send函数发送数据
bool GBNRdtSender::send(Message & message)
{
	if (getWaitingState())
	{//窗口已满，无法继续发送数据
		std::cout << "[SENDER]窗口已满，发送数据请稍等\n\n";
		return false;
	}
	else
	{
		sendBuf[nextSeqnum].acknum = -1;//这个地方不太懂，单向传输发给接收方的ack应该没用吧
		sendBuf[nextSeqnum].seqnum = nextSeqnum;//序列号
		memcpy(sendBuf[nextSeqnum].payload, message.data, sizeof(message.data));
		sendBuf[nextSeqnum].checksum = pUtils->calculateCheckSum(sendBuf[nextSeqnum]);
		pUtils->printPacket("[Debug]发送方发送报文", sendBuf[nextSeqnum]);
		//显示滑动窗口
		std::cout << "\n[SENDER]发送前窗口："; 
		printSlideWindow();
		if (base == nextSeqnum)
		{//gbn协议中只有一个定时器，即不管哪个分组超时，都会重传所有未确认的分组
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		pns->sendToNetworkLayer(RECEIVER, sendBuf[nextSeqnum]);
		//发送完毕，更新状态
		nextSeqnum = (nextSeqnum + 1) % seqsize;//不要忘了取模运算
		//显示滑动窗口
		std::cout << "[SENDER]发送后窗口：";
		printSlideWindow();
		std::cout << std::endl;
		return true;
	}
}

//返回是否处于等待状态，窗口满返回true，否则返回false
bool GBNRdtSender::getWaitingState()
{
	//根据实验文档，gbn协议中滑动窗口满则无法接收上层应用数据
	return (base + wndsize) % seqsize == (nextSeqnum) % seqsize;
}

//接收ack
void GBNRdtSender::receive(Packet &ackPkt)
{
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	if (checkSum != ackPkt.checksum) 
	{
		pUtils->printPacket("[Debug]接收的ack损坏，校验和不相等", ackPkt);
	}
	else
	{
		base = (ackPkt.acknum + 1) % seqsize;//累积确认，别忘了取模
		if (base == nextSeqnum)
		{
			pns->stopTimer(SENDER, 0);
		}
		else
		{//重启计时器
			pns->stopTimer(SENDER, 0);
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		std::cout << "\n[SENDER]收到ack，滑动窗口移动：";
		printSlideWindow();
		std::cout << std::endl;
	}
}

//处理超时
void GBNRdtSender::timeoutHandler(int seqNum)
{
	std::cout << "\n[Debug]发送超时\n\n";
	//某个分组发送超时，
	//因为接收方没有缓存不会保留顺序，因此必须重发所有已经发送且未确认的分组
	if (nextSeqnum == base)
	{//超时特例，不做处理
		return;
	}
	else
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);//重启计时器，重新计时
		int i;
		for (i = base; i != nextSeqnum; i = (i + 1) % seqsize)
		{//这里忘记取模了，总是丢包
			pns->sendToNetworkLayer(RECEIVER, sendBuf[i]);
			pUtils->printPacket("[Debug]超时重传的分组:", sendBuf[i]);
		}
	}
	std::cout << "\n[Debug]重发数据包完毕\n\n";
}

void GBNRdtSender::printSlideWindow()
{
	int i;
	for (i = 0; i < seqsize; i++)
	{
		if (i == base)
			std::cout << "[";
		std::cout << i;
		if (i == nextSeqnum)
			std::cout << "*";
		if (i == (base + wndsize - 1) % seqsize)
			std::cout << "]";
		std::cout << " ";
	}
	std::cout << std::endl;
}

