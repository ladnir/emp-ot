#ifndef OT_M_EXTENSION_ALSZ_H__
#define OT_M_EXTENSION_ALSZ_H__
#include "ot.h"
#include "co.h"

/** @addtogroup OT
    @{
  */
template<typename IO>
class MOTExtension_ALSZ: public OT<MOTExtension_ALSZ<IO>> { public:
	OTCO<IO> * mBase_ot;
	PRG mPrg;
	PRP mPi;
	int mL, mSsp;

	std::vector<block> mData_open;
	block *mK0, *mK1;// , *mData_open = nullptr;
	bool *mS;

	uint8_t * qT, *tT, *q = nullptr, **t, *block_s;
	int tTLength;
	int u = 0;
	bool setup = false;
	bool committing = false;
	char com[Hash::DIGEST_SIZE];
	IO* io = nullptr;
	MOTExtension_ALSZ(IO * io, const block& seed, bool committing = false, int ssp = 40)
        : mSsp(ssp)
        , mPrg(seed)
    {
		this->io = io;
		this->mL = 192;
		u = 2;
		this->mBase_ot = new OTCO<IO>(io, mPrg.random_block());
		this->mS = new bool[mL];
		this->mK0 = new block[mL];
		this->mK1 = new block[mL];
		block_s = new uint8_t[mL/8];
		this->committing = committing;
	}

	~MOTExtension_ALSZ() {
		delete mBase_ot;
		delete[] mS;
		delete[] mK0;
		delete[] mK1;
		delete[] block_s;
		//if(mData_open != nullptr) {
		//	delete[] mData_open;
		//}
	}

	void xor_arr (uint8_t * a, uint8_t * b, uint8_t * c, int n) {
		if(n%16 == 0)
			xorBlocks_arr((block*)a, (block *)b, (block*)c, n/16);
		else {
			uint8_t* end_a = a + n;
			for(;a!= end_a;)
				*(a++) = *(b++) ^ *(c++);
		}
	}

	block H(uint8_t* in, long id, int len) {
		block res = zero_block();
		for(int i = 0; i < len/16; ++i) {
			res = xorBlocks(res, mPi.H(_mm_loadl_epi64((block *)(in)), id));
			in+=16;
		}
		return res;	
	}

	void bool_to_uint8(uint8_t * out, const bool*in, int len) {
		for(int i = 0; i < len/8; ++i)
			out[i] = 0;
		for(int i = 0; i < len; ++i)
			if(in[i])
				out[i/8]|=(1<<(i%8));
	}
	void setup_send(block * in_mK0 = nullptr, bool * in_s = nullptr){
		setup = true;
		if(in_s != nullptr) {
			memcpy(mK0, in_mK0, mL*sizeof(block));
			memcpy(mS, in_s, mL);
			bool_to_uint8(block_s, mS, mL);
			return;
		}
		mPrg.random_bool(mS, mL);
		mBase_ot->recv(mK0, mS, mL);
		bool_to_uint8(block_s, mS, mL);
	}
	void setup_recv(block * in_mK0 = nullptr, block * in_mK1 =nullptr) {
		setup = true;
		if(in_mK0 !=nullptr) {
			memcpy(mK0, in_mK0, mL*sizeof(block));
			memcpy(mK1, in_mK1, mL*sizeof(block));
			return;
		}
		mPrg.random_block(mK0, mL);
		mPrg.random_block(mK1, mL);
		mBase_ot->send(mK0, mK1, mL);
		setup = true;
	}

	void ot_extension_send_pre(int length) {
		assert(length%8==0);
		if (length%128 !=0) length = (length/128 + 1)*128;

		q = new uint8_t[length/8*mL];
		if(!setup)setup_send();
		setup = false;
		if(committing) {
			Hash::hash_once(com, mS, mL);		
			io->send_data(com, Hash::DIGEST_SIZE);
		}
		//get u, compute q
		qT = new uint8_t[length/8*mL];
		uint8_t * q2 = new uint8_t[length/8*mL];
		uint8_t*tmp = new uint8_t[length/8];
		PRG G(ZeroBlock);
		for(int i = 0; i < mL; ++i) {
			io->recv_data(tmp, length/8);
			G.reseed(&mK0[i]);
			G.random_data(q+(i*length/8), length/8);
			if (mS[i])
				xor_arr(q2+(i*length/8), q+(i*length/8), tmp, length/8);
			else
				memcpy(q2+(i*length/8), q+(i*length/8), length/8);
		}
		sse_trans(qT, q2, mL, length);
		delete[] tmp;
		delete[] q2;
	}

	void ot_extension_recv_pre(block * data, const bool* r, int length) {
		int old_length = length;
		if (length%128 !=0) length = (length/128 + 1)*128;
		if(!setup)setup_recv();
		setup = false;
		if(committing) {
			io->recv_data(com, Hash::DIGEST_SIZE);
		}
		uint8_t *block_r = new uint8_t[length/8];
		bool_to_uint8(block_r, r, old_length);
		// send u
		t = new uint8_t*[2];
		t[0] = new uint8_t[length/8*mL];
		t[1] = new uint8_t[length/8*mL];
		tT = new uint8_t[length/8*mL];
		tTLength = length / 8 * mL;
		uint8_t* tmp = new uint8_t[length/8];
		PRG G(ZeroBlock);
		for(int i = 0; i < mL; ++i) {
			G.reseed(&mK0[i]);
			G.random_data(&(t[0][i*length/8]), length/8);
			G.reseed(&mK1[i]);
			G.random_data(t[1]+(i*length/8), length/8);
			xor_arr(tmp, t[0]+(i*length/8), t[1]+(i*length/8), length/8);
			xor_arr(tmp, block_r, tmp, length/8);
			io->send_data(tmp, length/8);
		}

		sse_trans(tT, t[0], mL, length);

		delete[] tmp;
		delete[] block_r;
	}

	void ot_extension_send_post(const block* data0, const block* data1, int length) {
		int old_length = length;
		if (length%128 !=0) length = (length/128 + 1)*128;
		//	uint8_t *pad0 = new uint8_t[l/8];
		uint8_t *pad1 = new uint8_t[mL/8];
		block pad[2];
		for(int i = 0; i < old_length; ++i) {
			xor_arr(pad1, qT+i*mL/8, block_s, mL/8);
			pad[0] = xorBlocks( H(qT+i*mL/8, i, mL/8), data0[i]);
			pad[1] = xorBlocks( H(pad1, i, mL/8), data1[i]);
			io->send_data(pad, 2*sizeof(block));
		}
		delete[] pad1;
		delete[] qT;
	}

	void ot_extension_recv_check(int length) {
		if (length%128 !=0) length = (length/128 + 1)*128;
		block seed; 
		PRG prg(ZeroBlock);
        int beta;
		uint8_t * tmp = new uint8_t[length/8];
		char dgst[20];
		for(int i = 0; i < u; ++i) {
			io->recv_block(&seed, 1);
			prg.reseed(&seed);
			for(int j = 0; j < mL; ++j) {
				prg.random_data(&beta, 4);
				beta = beta>0?beta:-1*beta;
				beta %= mL;
				for(int k = 0; k < 2; ++k)
					for(int l = 0; l < 2; ++l) {
						xor_arr(tmp, t[k]+(j*length/8), t[l]+(beta*length/8), length/8);
						Hash::hash_once(dgst, tmp, length/8);
						io->send_data(dgst, 20);
					}
			}
		}
		delete []tmp;
	}

	void ot_extension_recv_post(block* data, const bool* r, int length) {
		int old_length = length;
		mData_open.resize(length);
		if (length%128 !=0) length = (length/128 + 1)*128;
		block res[2];
		for(int i = 0; i < old_length; ++i) {
			io->recv_data(res, 2*sizeof(block));
			block tmp = H(tT+i*mL/8, i, mL/8);
			if(r[i]) {
				data[i] = xorBlocks(res[1], tmp);
				mData_open[i] = res[0];
			} else {
				data[i] = xorBlocks(res[0], tmp);
				mData_open[i] = res[1];
			}
		}
		if(!committing) {
			delete[] tT;
			tT=nullptr;
		}
	}
	bool ot_extension_send_check(int length) {
		if (length%128 !=0) length = (length/128 + 1)*128;
		bool cheat = false;
		PRG prg(ZeroBlock), sprg(ZeroBlock);
		block seed;int beta;
		char dgst[2][2][20]; char dgstchk[20];
		uint8_t * tmp = new uint8_t[length/8];
		for(int i = 0; i < u; ++i) {
			prg.random_block(&seed, 1);
			io->send_block(&seed, 1);
			sprg.reseed(&seed);
			for(int j = 0; j < mL; ++j) {
				sprg.random_data(&beta, 4);
				beta = beta>0?beta:-1*beta;
				beta %= mL;
				io->recv_data(dgst[0][0], 20);
				io->recv_data(dgst[0][1], 20);
				io->recv_data(dgst[1][0], 20);
				io->recv_data(dgst[1][1], 20);

				int ind1 = mS[j]? 1:0;
				int ind2 = mS[beta]? 1:0;
				xor_arr(tmp, q+(j*length/8), q+(beta*length/8), length/8);
				Hash::hash_once(dgstchk, tmp, length/8);	
				if (strncmp(dgstchk, dgst[ind1][ind2], 20)!=0)
					cheat = true;
			}
		}
		delete[] tmp;
		return cheat;
	}

	void send_impl(const block* data0, const block* data1, int length) {
		ot_extension_send_pre(length);
		assert(!ot_extension_send_check(length)?"T":"F");
		delete[] q; q = nullptr;
		ot_extension_send_post(data0, data1, length);
	}

	void recv_impl(block* data, const bool* b, int length) {
		ot_extension_recv_pre(data, b, length);
		ot_extension_recv_check(length);
		delete[] t[0];
		delete[] t[1];
		delete[] t;
		ot_extension_recv_post(data, b, length);
	}

	void open() {		
		io->send_data(mS, mL);		
	}		
	//return data[1-b]		
	void open(block * data, const bool * r, int length) {		
		io->recv_data(mS, mL);		
		char com_recv[Hash::DIGEST_SIZE];
		Hash::hash_once(com_recv, mS, mL);		
		if (strncmp(com_recv, com, Hash::DIGEST_SIZE)!= 0)
			assert(false);		
		bool_to_uint8(block_s, mS, mL);		
		for(int i = 0; i < length; ++i) {	
			xor_arr(tT+i*mL/8, tT+i*mL/8, block_s, mL/8);		
			block tmp = H(tT+i*mL/8, i, mL/8);		
			data[i] = xorBlocks(mData_open[i], tmp);		
		}		
		delete[] tT;
		//delete[] mData_open;
		tT=nullptr;	
		//mData_open = nullptr;	
	}
};
  /**@}*/
#endif// OT_M_EXTENSION_ALSZ_H__