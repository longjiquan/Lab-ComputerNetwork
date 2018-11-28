#pragma once
#include "RdtReceiver.h"
class GBNRdtReceiver :
	public RdtReceiver
{
private:
	int expectedSeqNum;
	Packet lastAckPkt;
	const int seqSize;

private:
	void initState();

public:
	GBNRdtReceiver(int sSize);
	GBNRdtReceiver();
	virtual ~GBNRdtReceiver();

	void receive(Packet &packet);
};

