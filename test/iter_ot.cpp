//#include "emp-ot.h"
#include <emp-ot.h>
#include <emp-tool.h>
#include <iostream>
using namespace std;

template<typename IO, template<typename> typename T>
double test_ot(IO * io, EmpParty party, int length, T<IO>* ot, int TIME = 10) {
	block *b0 = new block[length], *b1 = new block[length], *r = new block[length];
	PRG prg(fix_key);
	prg.random_block(b0, length);
	prg.random_block(b1, length);
	bool *b = new bool[length];
	for(int i = 0; i < length; ++i) {
		b[i] = (rand()%2)==1;
	}

	long long t1 = 0, t = 0;
	io->sync();
	for(int i = 0; i < TIME; ++i) {
		t1 = timeStamp();
		if (party == ALICE) {
			ot->send(b0, b1, length);
		} else {
			ot->recv(r, b, length);
		}
		t += timeStamp()-t1;
	}
	if(party == BOB) for(int i = 0; i < length; ++i) {
		if (b[i]) assert(block_cmp(&r[i], &b1[i], 1));
		else assert(block_cmp(&r[i], &b0[i], 1));
	}
	delete[] b0;
	delete[] b1;
	delete[] r;
	delete[] b;
	return (double)t/TIME;
}
template<typename IO, template<typename> class T>
double test_cot(IO * io, EmpParty party, int length, T<IO>* ot, int TIME = 10) {
	block *b0 = new block[length], *r = new block[length];
	bool *b = new bool[length];
	block delta;
	PRG prg(fix_key);
	prg.random_block(&delta, 1);
	
	for(int i = 0; i < length; ++i) {
		b[i] = (rand()%2)==1;
	}

	long long t1 = 0, t = 0;
	io->sync();
	for(int i = 0; i < TIME; ++i) {
		t1 = timeStamp();
		if (party == ALICE) {
			ot->send_cot(b0, delta, length);
		} else {
			ot->recv_cot(r, b, length);
		}
		t += timeStamp()-t1;
	}
	if(party == ALICE)
			io->send_block(b0, length);
	else if(party == BOB)  {
			io->recv_block(b0, length);
		for(int i = 0; i < length; ++i) {
			block b1 = xorBlocks(b0[i], delta); 
			if (b[i]) assert(block_cmp(&r[i], &b1, 1));
			else assert(block_cmp(&r[i], &b0[i], 1));
		}
	}
	delete[] b0;
	delete[] r;
	delete[] b;
	return (double)t/TIME;
}

template<typename IO, template<typename> class T>
double test_rot(IO * io, EmpParty party, int length, T<IO>* ot, int TIME = 10) {
	block *b0 = new block[length], *r = new block[length];
	block *b1 = new block[length];
	bool *b = new bool[length];
	PRG prg(ZeroBlock);
	prg.random_bool(b, length);
	prg.random_block(b0, length);
	prg.random_block(b1, length);

	long long t1 = 0, t = 0;
	io->sync();
	for(int i = 0; i < TIME; ++i) {
		t1 = timeStamp();
		if (party == ALICE) {
			ot->send_rot(b0, b1, length);
		} else {
			ot->recv_rot(r, b, length);
		}
		t += timeStamp()-t1;
	}
	if(party == ALICE) {
			io->send_block(b0, length);
			io->send_block(b1, length);
	} else if(party == BOB)  {
			io->recv_block(b0, length);
			io->recv_block(b1, length);
		for(int i = 0; i < length; ++i) {
			if (b[i]) assert(block_cmp(&r[i], &b1[i], 1));
			else assert(block_cmp(&r[i], &b0[i], 1));
		}
	}
	delete[] b0;
	delete[] b1;
	delete[] r;
	delete[] b;
	return (double)t/TIME;
}

void go(EmpParty party, int port, bool print)
{
	std::stringstream ss;
	std::ostream& out = print ? std::cout : ss;
	int pow = 10;
	int length = 1 << pow;

	NetIO * io = new NetIO(party == ALICE ? nullptr : SERVER_IP, port);
	io->set_nodelay();

	double t1 = (double)timeStamp();
	SHOTIterated<NetIO> * ot = new SHOTIterated<NetIO>(io, party == ALICE, ZeroBlock,length);
	out << (timeStamp() - t1) << endl;


	out << "2^"<< pow << " Semi Honest OT Extension\t" << test_ot<NetIO, SHOTIterated>(io, party, length, ot) << endl;
	out << "2^"<< pow << " Semi Honest COT Extension\t" << test_cot<NetIO, SHOTIterated>(io, party, length, ot) << endl;
	out << "2^"<< pow << " Semi Honest ROT Extension\t" << test_rot<NetIO, SHOTIterated>(io, party, length, ot) << endl;
	delete io;
}

int main(int argc, char** argv) {

	if (argc > 1)
	{
		int port, party;
		parse_party_and_port(argv, argc, &party, &port);
		go((EmpParty)party, port, true);
	}
	else
	{
		auto thrd = std::thread([]() {go(ALICE, 1212, false); });
		go(BOB, 1212, true);
		thrd.join();
	}
}
