#ifndef __SHUFFLE_H__
#define __SHUFFLE_H__

#include "abstractrecurrentnetworknode.h"
#include "mergeslice.h"
#include "activation.h"

#include <random>

class Shuffle : public AbstractRecurrentNetworkNode
{
	public:
		Shuffle();
		Shuffle(Shuffle *attached);

		void addInput(Port *input);

		virtual void forward();
		virtual void backward();
		virtual void reset();
		virtual Port* output();

		std::vector<size_t>& get_state()
		{ return _state; }
		std::vector<size_t>& get_istate()
		{ return _istate; }

		void lock()
		{ _locked = true; }
		bool is_locked()
		{ return _locked; }

	private:
		bool _locked = false;
		Shuffle *_attached;
		MergeiSlice *_merge;
		LinearActivation *_activ;
		std::vector<Port*> _ports;
		std::vector<size_t> _state;
		std::vector<size_t> _istate;
		std::mt19937 *_rng;
};

#endif
