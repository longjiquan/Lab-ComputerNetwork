#include "stdafx.h"
#include "CommonFunction.h"
#include "SeqStateEnum.h"

void printSlideWindow(int seqSize, int wndSize, int base, int nextSeqnum)
{
	int i;
	for (i = 0; i < seqSize; i++)
	{
		if (i == base)
			std::cout << "[";
		std::cout << i;
		if (i == nextSeqnum)
			std::cout << "* ";
		else
			std::cout << " ";
		if (i == (base + wndSize) % seqSize)
			std::cout << "]";
	}
	std::cout << std::endl << std::endl;
}
