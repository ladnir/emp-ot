//#include "emp-ot.h"
#include "emp-ot/emp-ot.h"
#include <emp-tool.h>
#include <iostream>
#include "mot.h"
using namespace std;

template<typename IO, template<typename>typename T>
double test_com_ot(IO * io, int party, int length, T<IO>* ot = nullptr, int TIME = 10) {
	block 
		*b0 = new block[length], 
		*b1 = new block[length], 
		*r  = new block[length], 
		*op = new block[length];
	bool *b = new bool[length];

	PRG prg(fix_key);
	prg.random_block(b0, length);
	prg.random_block(b1, length);
	prg.random_bool(b, length);

	long long t1 = 0, t = 0;
	io->sync();
	io->set_nodelay();
	for (int i = 0; i < TIME; ++i) {
		t1 = timeStamp();
		ot = new T<IO>(io, true);
		if (party == ALICE) {
			ot->send(b0, b1, length);
			ot->open();
		}
		else {
			ot->recv(r, b, length);
			ot->open(op, b, length);
		}
		t += timeStamp() - t1;
		delete ot;
	}
	if (party == BOB) for (int i = 0; i < length; ++i) {
		if (b[i]) {
			assert(block_cmp(&r[i], &b1[i], 1));
			assert(block_cmp(&op[i], &b0[i], 1));
		}
		else {
			assert(block_cmp(&r[i], &b0[i], 1));
			assert(block_cmp(&op[i], &b1[i], 1));
		}
	}
	delete[] b0;
	delete[] b1;
	delete[] r;
	delete[] b;
	delete[] op;
	return (double)t / TIME;
}


template<typename IO, template<typename>typename T>
double test_ot(IO * io, int party, int length, T<IO>* ot = nullptr, int TIME = 10) {
	block *b0 = new block[length], *b1 = new block[length], *r = new block[length];
	bool *b = new bool[length];
	PRG prg(fix_key);
	prg.random_block(b0, length);
	prg.random_block(b1, length);
	prg.random_bool(b, length);

	long long t1 = 0, t = 0;
	io->sync();
	io->set_nodelay();
	for (int i = 0; i < TIME; ++i) {
		t1 = timeStamp();
		ot = new T<IO>(io);
		if (party == ALICE) {
			ot->send(b0, b1, length);
		}
		else {
			ot->recv(r, b, length);
		}
		t += timeStamp() - t1;
		delete ot;
	}
	if (party == BOB)
	{

		for (int i = 0; i < length; ++i) {
			if (b[i]) {
				if(block_cmp(&r[i], &b1[i], 1) == false)
				{
					std::cout << i << " failed\n   " 
						<< r[i] << "\n   " 
						<< b1[i] << std::endl;
					assert(0);
				}
				//else
				//{

				//	std::cout << i << " passed 1\n   "
				//		<< r[i] << "\n   "
				//		<< b1[i] << std::endl;
				//}
			}
			else {
				auto bb = block_cmp(&r[i], &b0[i], 1);
				//assert(bb);
				if (!bb)
				{
					using namespace osuCrypto;
					std::cout << i << " failed\n   "
						<< r[i] << "\n   "
						<< b0[i] <<  "  "<< eq(r[i], b0[i]) <<std::endl;
					assert(0);
				}
				//else {

				//	std::cout << i << " passed 0\n   "
				//		<< r[i] << "\n   "
				//		<< b0[i] << "  " << eq(r[i], b0[i]) << std::endl;
				//}
			}
		}
	}
	delete[] b0;
	delete[] b1;
	delete[] r;
	delete[] b;
	return (double)t / TIME;
}
template<typename IO, template<typename>typename T>
double test_cot(IO * io, int party, int length, T<IO>* ot = nullptr, int TIME = 10) {
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
	io->set_nodelay();
	for (int i = 0; i < TIME; ++i) {
		t1 = timeStamp();
		ot = new T<IO>(io);
		if (party == ALICE) {
			ot->send_cot(b0, delta, length);
		}
		else {
			ot->recv_cot(r, b, length);
		}
		t += timeStamp() - t1;
		delete ot;
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
	delete[] b0;
	delete[] r;
	delete[] b;
	return (double)t / TIME;
}

template<typename IO, template<typename>typename T>
double test_rot(NetIO * io, int party, int length, T<IO>* ot = nullptr, int TIME = 10) {
	block *b0 = new block[length], *r = new block[length];
	block *b1 = new block[length];
	bool *b = new bool[length];

	for (int i = 0; i < length; ++i) {
		b[i] = (rand() % 2) == 1;
	}

	long long t1 = 0, t = 0;
	io->sync();
	io->set_nodelay();
	for (int i = 0; i < TIME; ++i) {
		t1 = timeStamp();
		ot = new T<IO>(io);
		if (party == ALICE) {
			ot->send_rot(b0, b1, length);
		}
		else {
			ot->recv_rot(r, b, length);
		}
		t += timeStamp() - t1;
		delete ot;
	}
	if (party == ALICE) {
		io->send_block(b0, length);
		io->send_block(b1, length);
	} {
		io->recv_block(b0, length);
		io->recv_block(b1, length);
		for (int i = 0; i < length; ++i) {
			if (b[i]) assert(block_cmp(&r[i], &b1[i], 1));
			else assert(block_cmp(&r[i], &b0[i], 1));
		}
	}
	delete[] b0;
	delete[] b1;
	delete[] r;
	delete[] b;
	return (double)t / TIME;
}
void go(int party, int port, bool print = true)
{
	std::stringstream ss;
	std::ostream& out = print ? std::cout : ss;

	NetIO * io = new NetIO(party == ALICE ? nullptr : SERVER_IP, port);
	int pow = 10;

	out << "COOT\t" << test_ot<NetIO, OTCO>(io, party, 20, nullptr, 1) << endl;
	out << "2^" << pow << " Malicious OT Extension (KOS)\t" << test_ot<NetIO, MOTExtension_KOS>(io, party, 1 << pow, nullptr, 1) << endl;
	out << "2^" << pow << " Malicious OT Extension (ALSZ)\t" << test_ot<NetIO, MOTExtension_ALSZ>(io, party, 1 << pow, nullptr, 1) << endl;
	out << "2^" << pow << " Malicious Committing OT Extension (KOS)\t" << test_com_ot<NetIO, MOTExtension_KOS>(io, party, 1 << pow, nullptr, 1) << endl;
	out << "2^" << pow << " Malicious Committing OT Extension (ALSZ)\t" << test_com_ot<NetIO, MOTExtension_ALSZ>(io, party, 1 << pow, nullptr, 1) << endl;

	std::cout <<  "exit " << party << std::endl;
	delete io;
}

int main(int argc, char** argv) {

	if (argc == 3)
	{

		int port, party;
		parse_party_and_port(argv, &party, &port);

		go(party, port);
	}
	else
	{
		auto thrd = std::thread([=]() {go(1, 1212, false); });
		go(2, 1212);
		thrd.join();
	}
}

