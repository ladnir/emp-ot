#ifndef OT_ITERATED_H__
#define OT_ITERATED_H__
#include "emp-ot/emp-ot.h"
/** @addtogroup OT
    @{
  */
  
template<typename IO, template<typename> typename OTExtension>
class OTIterated: public OT<OTIterated<IO, OTExtension>> { public:
	OTExtension<IO> *seed_ot;
	OTExtension<IO> *ot;
	block * k0 = nullptr, *k1 = nullptr;
	bool * sel = nullptr, is_sender;
	int buffer_size, size = 0;
	PRG prg;
	IO * io = nullptr;
	OTIterated(IO* io, bool is_sender, int buffer_size = 1<<10) {
		this->io = io;
		seed_ot = new OTExtension<IO>(io);
		ot = new OTExtension<IO>(io);
		this->buffer_size = buffer_size;
		this->is_sender = is_sender;
		k0 = new block[buffer_size];
		k1 = new block[buffer_size];
		sel = new bool[buffer_size];
		fill();
	}
	~OTIterated() {
		delete[] k0;
		delete[] k1;
		delete[] sel;
		delete seed_ot;
		delete ot;
	}
	void fill() {
		if(is_sender) {
			if( size != 0) {
				ot->setup_send(k0, sel);
			}
			ot->send_rot(k0, k1, seed_ot->l);
			seed_ot->setup_recv(k0, k1);
			prg.random_bool(sel, buffer_size);
			seed_ot->recv_rot(k0, sel, buffer_size);
		} else {
			if( size != 0)
				ot->setup_recv(k0 ,k1);
			prg.random_bool(sel, seed_ot->l);
			ot->recv_rot(k0, sel, seed_ot->l);
			seed_ot->setup_send(k0, sel);
			seed_ot->send_rot(k0, k1, buffer_size);
		}
		size = ot->l;
	}
	void send_impl(const block* data0, const block* data1, int length) {
		if(size + ot->l > buffer_size)
			fill();
		ot->setup_send(k0+size, sel+size);
		ot->send(data0, data1, length);
		size+=ot->l;
	}
	void recv_impl(block* data, const bool* b, int length) {
		if(size + ot->l > buffer_size)
			fill();
		ot->setup_recv(k0+size, k1+size);
		ot->recv(data, b, length);
		size+=ot->l;
	}
	void send_cot(block * data0, block delta, int length) {
		if(size + ot->l > buffer_size)
			fill();

		ot->setup_send(k0+size, sel+size);
		ot->send_cot(data0, delta, length);
		size+=ot->l;
	}
	void recv_cot(block* data, const bool* b, int length) {
		if(size + ot->l > buffer_size)
			fill();
		ot->setup_recv(k0+size, k1+size);
		ot->recv_cot(data, b, length);
		size+=ot->l;
	}
	void send_rot(block * data0, block * data1, int length) {
		if(size + ot->l > buffer_size)
			fill();
		ot->setup_send(k0+size, sel+size);
		ot->send_rot(data0, data1, length);
		size+=ot->l;
	}
	void recv_rot(block* data, const bool* b, int length) {
		if(size + ot->l > buffer_size)
			fill();
		ot->setup_recv(k0+size, k1+size);
		ot->recv_rot(data, b, length);
		size+=ot->l;
	}
};
template<typename IO>
using SHOTIterated = OTIterated<IO, SHOTExtension>;
template<typename IO>
using MOTIterated = OTIterated<IO, MOTExtension>;

/**@}*/
#endif// OT_ITERATED_H__