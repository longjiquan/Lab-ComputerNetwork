#include "stdafx.h"
#include "GBNRdtSender.h"
#include "SeqStateEnum.h"
#include "Global.h"


GBNRdtSender::GBNRdtSender():
	wndSize(4),seqSize(8)
{
	InitState();
}

GBNRdtSender::GBNRdtSender(int wSize, int sSize):
	wndSize(wSize),seqSize(sSize)
{
	InitState();
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
		std::cout << "窗口已满，发送数据请稍等\n\n";
		return false;
	}
	else
	{
		sendBuf[nextSeqNum].acknum = -1;//这个地方不太懂，单向传输发给接收方的ack应该没用吧
		sendBuf[nextSeqNum].seqnum = nextSeqNum;//序列号
		memcpy(sendBuf[nextSeqNum].payload, message.data, sizeof(message.data));
		sendBuf[nextSeqNum].checksum = pUtils->calculateCheckSum(sendBuf[nextSeqNum]);
		pUtils->printPacket("发送方发送报文", sendBuf[nextSeqNum]);
		//显示滑动窗口
		std::cout << "\n发送前窗口："; 
		printSlideWindow();
		if (base == nextSeqNum)
		{//gbn协议中只有一个定时器，即不管哪个分组超时，都会重传所有未确认的分组
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		pns->sendToNetworkLayer(RECEIVER, sendBuf[nextSeqNum]);
		//发送完毕，更新状态
		nextSeqNum = (nextSeqNum + 1) % seqSize;//不要忘了取模运算
		//显示滑动窗口
		std::cout << "发送后窗口：";
		printSlideWindow();
		std::cout << std::endl;
		return true;
	}
}

//返回是否处于等待状态，窗口满返回true，否则返回false
bool GBNRdtSender::getWaitingState()
{
	//根据实验文档，gbn协议中滑动窗口满则无法接收上层应用数据
	return (base + wndSize) % seqSize == (nextSeqNum) % seqSize;
}

//接收ack
void GBNRdtSender::receive(Packet &ackPkt)
{
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	if (checkSum != ackPkt.checksum) 
	{
		pUtils->printPacket("接收的ack损坏，校验和不相等", ackPkt);
	}
	else
	{
		base = (ackPkt.acknum + 1) % seqSize;//累积确认，别忘了取模
		if (base == nextSeqNum)
		{
			pns->stopTimer(SENDER, 0);
		}
		else
		{//重启计时器
			pns->stopTimer(SENDER, 0);
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		std::cout << "\n收到ack，滑动窗口移动：";
		printSlideWindow();
		std::cout << std::endl;
	}
}

//处理超时
void GBNRdtSender::timeoutHandler(int seqNum)
{
	//某个分组发送超时，
	//因为接收方没有缓存不会保留顺序，因此必须重发所有已经发送且未确认的分组
	if (nextSeqNum == base)
	{//超时特例，不做处理
		return;
	}
	else
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);//重启计时器，重新计时
		int i;
		for (i = base; i != nextSeqNum; i = (i + 1) % seqSize)
		{//这里忘记取模了，总是丢包
			pns->sendToNetworkLayer(RECEIVER, sendBuf[i]);
			pUtils->printPacket("超时重传的分组:", sendBuf[i]);
		}
	}
}

//初始化滑动窗口状态
void GBNRdtSender::InitState()
{
	sendBuf = new Packet[seqSize];
	base = nextSeqNum = 0;
}

void GBNRdtSender::printSlideWindow()
{
	int i;
	for (i = 0; i < seqSize; i++)
	{
		if (i == base)
			std::cout << "[";
		std::cout << i;
		if (i == nextSeqNum)
			std::cout << "* ";
		else
			std::cout << " ";
		if (i == (base + wndSize) % seqSize)
			std::cout << "]";
	}
	std::cout << std::endl;
}

