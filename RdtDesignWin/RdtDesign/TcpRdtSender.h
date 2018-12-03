#pragma once
#include "RdtSender.h"
class TcpRdtSender :
	public RdtSender
{
private:
	//bool waitingState;
	int base;							//基序号，最早的未确认分组的序号
	int nextSeqnum;						//下一个待发分组的序号
	const int wndsize;					//滑动窗口大小，实验建议为4
	const int seqsize;					//序号大小，实验建议位数为3位，即0~7
	Packet *const sendBuf;				//发送缓冲区，保存发送的报文，用于重传，大小应该是seqsize
	//int dupAck[3];						//冗余ack
	int dupAckNum;							//收到3个冗余ack快速重传

private:
	void Init();
	bool isInWindow(int seqnum);
	void printSlideWindow();

public:
	TcpRdtSender();
	TcpRdtSender(int wsize, int ssize);
	virtual ~TcpRdtSender();

public:
	bool getWaitingState();
	bool send(Message &message);
	void timeoutHandler(int seqNum);
	void receive(Packet &ackPkt);
};

