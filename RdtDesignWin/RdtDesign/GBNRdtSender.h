#pragma once
#include "RdtSender.h"
#include "DataStructure.h"
class GBNRdtSender :
	public RdtSender
{

private:
	bool waitingState;
	int base;							//基序号，最早的未确认分组的序号
	int nextSeqNum;						//下一个待发分组的序号
	//int sentNum;						//已经发送的分组数目
	const int wndSize;					//滑动窗口大小，实验建议为4
	const int seqSize;					//序号大小，实验建议位数为3位，即0~7
	//int *const stateArray;				//序号的状态，数组大小应该是seqSize
	Packet *sendBuf;					//发送缓冲区，保存发送的报文，用于重传，大小应该是seqSize

private:
	void InitState();

public:
	GBNRdtSender();
	GBNRdtSender(int wSize, int sSize);
	virtual ~GBNRdtSender();

	bool getWaitingState();
	bool send(Message &message);
	void receive(Packet &ackPkt);
	void timeoutHandler(int seqNum);

};

