//#include "emp-ot.h"
#include <emp-ot.h>
#include <emp-tool.h>
#include <iostream>
using namespace std;

template<typename IO, template<typename>typename T>
double test_ot(IO * io, EmpParty party, int length, T<IO>* ot = nullptr, int TIME = 10) {
	block *b0 = new block[length], *b1 = new block[length], *r = new block[length];
	PRG prg(fix_key);
	prg.random_block(b0, length);
	prg.random_block(b1, length);
	bool *b = new bool[length];
	for (int i = 0; i < length; ++i) {
		b[i] = (rand() % 2) == 1;
	}

	long long t1 = 0, t = 0;
	io->sync();
	for (int i = 0; i < TIME; ++i) {
		t1 = timeStamp();
		if (ot == nullptr)
			ot = new T<IO>(io, ZeroBlock);
		if (party == ALICE) {
			ot->send(b0, b1, length);
		}
		else {
			ot->recv(r, b, length);
		}
		t += timeStamp() - t1;
	}
	if (party == BOB) for (int i = 0; i < length; ++i) {
		if (b[i]) assert(block_cmp(&r[i], &b1[i], 1));
		else assert(block_cmp(&r[i], &b0[i], 1));
	}
	delete ot;
	delete[] b0;
	delete[] b1;
	delete[] r;
	delete[] b;
	return (double)t / TIME;
}
template<typename IO, template<typename>typename T>
double test_cot(NetIO * io, EmpParty party, int length, T<IO>* ot = nullptr, int TIME = 10) {
	block *b0 = new block[length], *r = new block[length];
	bool *b = new bool[length];
	block delta;
	PRG prg(fix_key);
	prg.random_block(&delta, 1);

	for (int i = 0; i < length; ++i) {
		b[i] = (rand() % 2) == 1;
	}

	long long t1 = 0, t = 0;
	io->sync();
	for (int i = 0; i < TIME; ++i) {
		t1 = timeStamp();
		if (ot == nullptr)
			ot = new T<IO>(io, ZeroBlock);
		if (party == ALICE) {
			ot->send_cot(b0, delta, length);
		}
		else {
			ot->recv_cot(r, b, length);
		}
		t += timeStamp() - t1;
	}
	if (party == ALICE)
		io->send_block(b0, length);
	else {
		io->recv_block(b0, length);
		for (int i = 0; i < length; ++i) {
			block b1 = xorBlocks(b0[i], delta);
			if (b[i]) assert(block_cmp(&r[i], &b1, 1));
			else assert(block_cmp(&r[i], &b0[i], 1));
		}
	}

	io->sync();

	delete ot;
	delete[] b0;
	delete[] r;
	delete[] b;
	return (double)t / TIME;
}

template<typename IO, template<typename>typename T>
double test_rot(IO * io, EmpParty party, int length, T<IO>* ot = nullptr, int TIME = 10) {
	block *b0 = new block[length], *r = new block[length];
	block *b1 = new block[length];
	bool *b = new bool[length];
	PRG prg(fix_key);

	long long t1 = 0, t = 0;
	io->sync();
	for (int i = 0; i < TIME; ++i) {
		prg.random_bool(b, length);
		t1 = timeStamp();
		if (ot == nullptr)
			ot = new T<IO>(io, ZeroBlock);
		if (party == ALICE) {
			ot->send_rot(b0, b1, length);
		}
		else {
			ot->recv_rot(r, b, length);
		}
		t += timeStamp() - t1;
	}
	if (party == ALICE) {
		io->send_block(b0, length);
		io->send_block(b1, length);
	}
	else {
		io->recv_block(b0, length);
		io->recv_block(b1, length);
		for (int i = 0; i < length; ++i) {
			if (b[i]) assert(block_cmp(&r[i], &b1[i], 1));
			else assert(block_cmp(&r[i], &b0[i], 1));
		}
	}
	delete ot;
	delete[] b0;
	delete[] b1;
	delete[] r;
	delete[] b;
	return (double)t / TIME;
}



void go(EmpParty party, int port, int length, bool print = true)
{
	std::stringstream ss;
	std::ostream& out = print ? std::cout : ss;

	NetIO * io = new NetIO(party == ALICE ? nullptr : SERVER_IP, port);

	io->set_nodelay();
	out << "128" << " NPOT                     \t" << test_ot<NetIO, OTNP>(io, party, 128) << endl;
	out << length << " Semi Honest OT Extension \t" << test_ot<NetIO, SHOTExtension>(io, party, length) << endl;
	out << length << " Semi Honest COT Extension\t" << test_cot<NetIO, SHOTExtension>(io, party, length) << endl;
	out << length << " Semi Honest ROT Extension\t" << test_rot<NetIO, SHOTExtension>(io, party, length) << endl;
	delete io;
}

int main(int argc, char** argv) {
	int length = 1 << 10;
	int port, party;

	if (argc == 1)
	{
		std::thread thrd = std::thread([=]() { go(ALICE, 1212, length, false); });
		go(BOB, 1212, length);
		thrd.join();
	}
	else if (argc >= 3)
	{
		parse_party_and_port(argv, argc, &party, &port);
		std::cout << "PARTY=" << party << " port=" << port << std::endl;

		go((EmpParty)party, port, length);
	}
	else
	{
		std::cout << "please provide no arguments or exact two: PARTY \\in {1,2} and PORT \\in {0,~6000}" << std::endl;
	}
}


