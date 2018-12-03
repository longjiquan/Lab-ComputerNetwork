#pragma once
#include "RdtReceiver.h"
class GBNRdtReceiver :
	public RdtReceiver
{
private:
	int expectedSeqNum;
	const int seqsize;
	Packet lastAckPkt;

private:
	void Init();

public:
	GBNRdtReceiver(int sSize);
	GBNRdtReceiver();
	virtual ~GBNRdtReceiver();

	void receive(Packet &packet);
};

