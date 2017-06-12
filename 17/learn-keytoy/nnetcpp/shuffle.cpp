#include "shuffle.h"

Shuffle::Shuffle()
{
	_merge = nullptr;
	_activ = new LinearActivation;
	_attached = nullptr;

	std::random_device rnd;
	_rng = new std::mt19937(rnd());
}

Shuffle::Shuffle(Shuffle *attached)
{
	if(attached->is_locked())
		throw;
	attached->lock();

	_merge = nullptr;
	_activ = new LinearActivation;
	_attached = attached;
	_state = attached->get_state();
	_istate = attached->get_istate();
	
	std::random_device rnd;
	_rng = new std::mt19937(rnd());
}

void Shuffle::addInput(Port *input)
{
	_ports.push_back(input);
	_state.push_back(_ports.size() - 1);
	_istate.push_back(_ports.size() - 1);
	reset();
}

void Shuffle::forward()
{
	if(_merge == nullptr)
		throw;
	_merge->forward();
	_activ->forward();
	AbstractRecurrentNetworkNode::forward();
}

void Shuffle::backward()
{
	if(_merge == nullptr)
		throw;
	_activ->backward();
	_merge->backward();
	AbstractRecurrentNetworkNode::backward();
}

void Shuffle::reset()
{
	if(_merge != nullptr)
		delete _merge;

	if(_attached != nullptr)
	{
		_attached->reset();
		_state = _attached->get_istate();
		_istate = _attached->get_state();
	} else {
		for(size_t i = 0; i < _state.size(); ++i)
		{
			std::uniform_int_distribution<>
				rndiv(i, _state.size() - 1);
			size_t j = rndiv(*_rng);
			std::swap(_state[i], _state[j]);
			std::swap(
				_istate[_state[i]],
				_istate[_state[j]]
				);
		}
	}

	_merge = new MergeiSlice;
	for(size_t i = 0; i < _ports.size(); ++i)
	{
		if(_state[i] >= _ports.size())
			continue;

		_merge->addInput(_ports[_state[i]]);
	}
	_activ->setInput(_merge->output());
	AbstractRecurrentNetworkNode::reset();
}

Shuffle::Port* Shuffle::output()
{
	return _activ->output();
}
