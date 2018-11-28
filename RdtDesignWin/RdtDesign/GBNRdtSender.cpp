#include "stdafx.h"
#include "GBNRdtSender.h"
#include "SeqStateEnum.h"
#include "Global.h"
#include "CommonFunction.h"


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
	if (nextSeqNum < (base + wndSize) % seqSize)
	{
		sendBuf[nextSeqNum].acknum = -1;//这个地方不太懂
		sendBuf[nextSeqNum].seqnum = nextSeqNum;//序列号
		memcpy(sendBuf[nextSeqNum].payload, message.data, sizeof(message.data));
		sendBuf[nextSeqNum].checksum = pUtils->calculateCheckSum(sendBuf[nextSeqNum]);
		pUtils->printPacket("发送方发送报文", sendBuf[nextSeqNum]);
		//显示滑动窗口
		std::cout << "\n发送前："; 
		printSlideWindow(seqSize, wndSize, base, nextSeqNum);
		if (base == nextSeqNum)//开启定时器
			pns->startTimer(SENDER, Configuration::TIME_OUT, sendBuf[nextSeqNum].seqnum);
		pns->sendToNetworkLayer(RECEIVER, sendBuf[nextSeqNum]);
		//发送完毕，更新状态
		nextSeqNum = (nextSeqNum + 1) % seqSize;
		//显示滑动窗口
		std::cout << "\n发送后：";
		printSlideWindow(seqSize, wndSize, base, nextSeqNum);
		return true;
	}
	else
	{//滑动窗口已满
		std::cout << "\n滑动窗口已满\n\n";
		waitingState = true;//窗口满，等待确认
		return false;
	}
}

//返回是否处于等待状态
bool GBNRdtSender::getWaitingState()
{
	/*if (waitingState == true)
	{
		std::cout << "for debug, waiting state:true\n\n";
	}*/
	return waitingState;
}

//接收ack
void GBNRdtSender::receive(Packet &ackPkt)
{
	if (ackPkt.checksum != pUtils->calculateCheckSum(ackPkt))
	{//ack损坏，不做处理
		return;
	}
	base = ackPkt.acknum;
	if (base == nextSeqNum)
	{
		pns->stopTimer(SENDER, ackPkt.acknum);
	}
	else
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, (ackPkt.acknum + 1) % seqSize);
	}
	waitingState = false;
}

//处理超时
void GBNRdtSender::timeoutHandler(int seqNum)
{
	if ((seqNum<base || seqNum>nextSeqNum) && (base <= nextSeqNum))
	{
		return;
	}
	if ((base > nextSeqNum) && (seqNum<base && seqNum>nextSeqNum))
	{
		return;
	}
	int i;
	pns->startTimer(SENDER,Configuration::TIME_OUT,seqNum);
	//回退重传
	std::cout << "\n回退重传!\n\n";
	if (base <= seqNum)
	{
		for (i = base; i <= seqNum; i++)
		{
			pns->sendToNetworkLayer(RECEIVER, sendBuf[i]);
		}
	}
	else
	{
		for (i = base; i < seqSize; i++)
		{
			pns->sendToNetworkLayer(RECEIVER, sendBuf[i]);
		}
		for (i = 0; i <= seqNum; i++)
		{
			pns->sendToNetworkLayer(RECEIVER, sendBuf[i]);
		}
	}
}

//初始化滑动窗口状态
void GBNRdtSender::InitState()
{
	sendBuf = new Packet[seqSize];
	base = nextSeqNum = 0;
	waitingState = false;
}

