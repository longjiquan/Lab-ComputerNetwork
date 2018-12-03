// main.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"
#include "GBNRdtReceiver.h"
#include "GBNRdtSender.h"
#include "SRRdtReceiver.h"
#include "SRRdtSender.h"


int main(int argc, char* argv[])
{
	RdtSender *pStopWaitSender = new StopWaitRdtSender();
	RdtReceiver * pStopWaitReceiver = new StopWaitRdtReceiver();
	RdtSender *pGBNSender = new GBNRdtSender();
	RdtReceiver *pGBNReceiver = new GBNRdtReceiver();
	RdtSender *pSRSender = new SRRdtSender();
	RdtReceiver *pSRReceiver = new SRRdtReceiver();

	ofstream stopWatiLog;
	ofstream gbnLog;
	ofstream srLog;
	streambuf *coutBackup = cout.rdbuf();

	pns->init();
	pns->setRtdSender(pStopWaitSender);
	pns->setRtdReceiver(pStopWaitReceiver);
	pns->setInputFile("..\\data\\input.txt");
	pns->setOutputFile("..\\data\\StopWaitOutput.txt");
	stopWatiLog.open("..\\data\\StopWaitLog.txt");
	cout.rdbuf(stopWatiLog.rdbuf());
	pns->start();
	stopWatiLog.close();
	
	pns->init();
	pns->setRtdSender(pGBNSender);
	pns->setRtdReceiver(pGBNReceiver);
	pns->setInputFile("..\\data\\input.txt");
	pns->setOutputFile("..\\data\\GBNOutput.txt");
	gbnLog.open("..\\data\\GBNLog.txt");
	cout.rdbuf(gbnLog.rdbuf());
	pns->start();
	gbnLog.close();

	pns->init();
	pns->setRtdSender(pSRSender);
	pns->setRtdReceiver(pSRReceiver);
	pns->setInputFile("..\\data\\input.txt");
	pns->setOutputFile("..\\data\\SROutput.txt");
	srLog.open("..\\data\\SRLog.txt");
	cout.rdbuf(srLog.rdbuf());
	pns->start();
	srLog.close();

	cout.rdbuf(coutBackup);

	delete pStopWaitSender;
	delete pStopWaitReceiver;
	delete pGBNSender;
	delete pGBNReceiver;
	delete pSRSender;
	delete pSRReceiver;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	
	return 0;

}

