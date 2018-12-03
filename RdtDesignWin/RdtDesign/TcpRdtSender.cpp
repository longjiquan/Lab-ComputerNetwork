#include "stdafx.h"
#include "TcpRdtSender.h"
#include "Global.h"

void TcpRdtSender::Init()
{
	base = 0;
	nextSeqnum = 0;
	dupAckNum = 0;
}

bool TcpRdtSender::isInWindow(int seqnum)
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

void TcpRdtSender::printSlideWindow()
{
	int i;
	for (i = 0; i < seqsize; i++)
	{
		if (i == base)
			std::cout << "[";
		std::cout << i;
		if (i == nextSeqnum)
			std::cout << "*";
		if (i == (base + wndsize -1 ) % seqsize)
			std::cout << "]";
		std::cout << " ";
	}
	std::cout << std::endl;
}

TcpRdtSender::TcpRdtSender():
	wndsize(4),seqsize(8),sendBuf(new Packet[seqsize])
{
	Init();
}

TcpRdtSender::TcpRdtSender(int wsize, int ssize):
	wndsize(wsize),seqsize(ssize),sendBuf(new Packet[ssize])
{
	Init();
}


TcpRdtSender::~TcpRdtSender()
{
}

bool TcpRdtSender::getWaitingState()
{
	//根据实验文档，gbn协议中滑动窗口满则无法接收上层应用数据
	return (base + wndsize) % seqsize == (nextSeqnum) % seqsize;
}

bool TcpRdtSender::send(Message & message)
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

void TcpRdtSender::timeoutHandler(int seqNum)
{
	//快速重传最早的数据包
	std::cout << "\n[Debug]发送超时\n\n";
	pns->sendToNetworkLayer(RECEIVER, sendBuf[base]);
	pns->stopTimer(SENDER, 0);
	pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
	std::cout << "\n[Debug]重发数据包完毕\n\n";
}

void TcpRdtSender::receive(Packet & ackPkt)
{
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	if (checkSum != ackPkt.checksum)
	{
		pUtils->printPacket("[Debug]接收的ack损坏，校验和不相等", ackPkt);
	}
	else
	{
		//if (ackPkt.acknum >= base)
		if(isInWindow(ackPkt.acknum))
		{
			base = (ackPkt.acknum + 1) % seqsize;
			pns->stopTimer(SENDER, 0);
			if (base != nextSeqnum)
			{
				pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
			}
			dupAckNum = 0;
			std::cout << "\n[SENDER]收到ack:" << ackPkt.acknum << "，滑动窗口移动：";
			printSlideWindow();
			std::cout << std::endl;
		}
		else
		{//已经确认的冗余的ack
			dupAckNum = (dupAckNum + 1) % 3;
			if (dupAckNum == 2)
			{//快速重传
				pns->sendToNetworkLayer(RECEIVER, sendBuf[base]);
				std::cout << "\n[SENDER]收到连续三个冗余ack，快速重传\n\n";
			}
		}
	}
}
