/*
 * Copyright (c) 2015 Vrije Universiteit Brussel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "utils.h"

#include <cmath>
#include <string>
#include <random>
#include <fstream>
#include <iomanip>
#include <iostream>

#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <fenv.h>

#include <mergesum.h>
#include <mergeslice.h>
#include <mergeproduct.h>
#include <activation.h>
#include <shuffle.h>

// Warning: horrific C++ ahead

void set_mode(int want_key)
{
	static struct termios old, _new;
	if (!want_key) {
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
		return;
	}

	tcgetattr(STDIN_FILENO, &old);
	_new = old;
	_new.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &_new);
}

int get_key()
{
	int c = 0;
	struct timeval tv;
	fd_set fs;
	tv.tv_usec = tv.tv_sec = 0;

	FD_ZERO(&fs);
	FD_SET(STDIN_FILENO, &fs);
	select(STDIN_FILENO + 1, &fs, 0, 0, &tv);

	if (FD_ISSET(STDIN_FILENO, &fs)) {
		c = getchar();
		set_mode(0);
	}
	return c;
}

int getch()
{
	int c;
	set_mode(1);
	while(!(c = get_key())) usleep(10000);
	set_mode(1);
	return c;
}

const unsigned int encoding_size = 64;

Vector makeI(int i)
{
	Vector vec = Vector::Zero(encoding_size);
	vec[i % encoding_size] = 1.0f;

	return vec;
}

int invertI(Vector vec)
{
	int i = 0;
	float max = -1e20;
	for(int j = 0; j < vec.rows(); ++j)
	{
		i = vec[j] > max ? j : i;
		max = vec[j] > max ? vec[j] : max;
	}

	return i;
}

int alph(unsigned char c)
{
	if(c >= 97 && c <= 122)
		return c - 64;
	if(c == 195)
		return 64;
	if(c == 194)
		return 62;
	if(c == 226)
		return 60;
	return c - 32;
}

unsigned char ialph(int i)
{
	if(i == 60)
		return 226;
	if(i == 62)
		return 194;
	if(i == 64)
		return 195;
	if(i >= 33 && i <= 58)
		return i + 64;
	return i + 32;
}

const unsigned int max_size = 500;
const unsigned int seq_size = 10;
const unsigned int rounds = 5;

const char clean[] = "\x1b\x5b\x31\x4b\r";
const char bold[] = "\x1b\x5b\x31\x6d";
const char dim[] = "\x1b\x5b\x32\x6d";
const char green[] = "\x1b\x5b\x33\x32\x6d";
const char red[] = "\x1b\x5b\x33\x31\x6d";
const char reset[] = "\x1b\x28\x42\x1b\x5b\x6d";

Network *makeNet(int nb_in, int nhidden, int nb_out, float rate)
{
	Network *net = new Network(nb_in);

	MergeiSlice *sum = new MergeiSlice();
	Dense *out = new Dense(nb_out, rate);

	Dense::Port* last = net->inputPort();
	for(int i = 0; i < 4; ++i)
	{
		Dense *in = new Dense(nhidden, rate);
		Dense *gin = new Dense(nhidden, rate);
		Dense *gout = new Dense(nhidden, rate);
		Dense *gfor = new Dense(nhidden, rate);
		LSTM *lstm = new LSTM(nhidden, rate);

		in->setInput(last);
		gin->setInput(last);
		gout->setInput(last);
		gfor->setInput(last);

		lstm->addInput(in->output());
		lstm->addInGate(gin->output());
		lstm->addOutGate(gout->output());
		lstm->addForgetGate(gfor->output());

		net->addNode(in);
		net->addNode(gin);
		net->addNode(gout);
		net->addNode(gfor);
		net->addNode(lstm);

		sum->addInput(last);
		last = lstm->output();
	}
	sum->addInput(last);
	out->setInput(sum->output());

	net->addNode(sum);
	net->addNode(out);

	return net;
}

int main(int argc, char **argv)
{
	std::mt19937 rng;
	rng.seed(std::random_device()());

	Network* network = makeNet(encoding_size, seq_size * 2, encoding_size, 0.01);
	std::vector<Vector> in, out;

	in.push_back(makeI(alph(getch())));
	std::cout << clean << green << ialph(invertI(in[0])) << std::flush;

	while(out.size() < seq_size)
	{
		auto v = makeI(alph(getch()));
		out.push_back(v);
		in.push_back(v);

		if(out.size() < seq_size)
			std::cout << ialph(invertI(v)) << std::flush;
	}

	std::cout << reset << clean << green;
	for(int i = 0; i < seq_size; ++i)
	{
		if(i == seq_size - 1)
			std::cout << bold;
		std::cout << ialph(invertI(in[in.size() - seq_size + i]));
	}
	std::cout << std::flush;

	while(true)
	{
		auto v = makeI(alph(getch()));

		out.push_back(v);
		for(int i = 0; i < rounds; ++i)
		{
			std::uniform_int_distribution<std::mt19937::result_type> slice(0, in.size() - seq_size - 1);
			int offset = slice(rng);

			std::vector<Vector> input(in.begin() + offset, in.begin() + offset + seq_size);
			std::vector<Vector> output(out.begin() + offset, out.begin() + offset + seq_size);

			trainNetwork(network, input, output, 1, false, true, false);
		}
		in.push_back(v);

		int predict = 0;
		network->reset();
		for(int i = 0; i < seq_size; ++i)
		{
			network->setCurrentTimestep(i);
			predict = invertI(network->predict(in[in.size() - seq_size + i]));
		}

		std::cout << reset << clean << green;
		for(int i = 0; i < seq_size; ++i)
		{
			if(i == seq_size - 1)
				std::cout << bold;
			std::cout << ialph(invertI(in[in.size() - seq_size + i]));
		}

		std::cout << reset << red;
		Vector pre = makeI(predict);
		for(int i = 0; i < seq_size * 4; ++i)
		{
			if(i == seq_size * 1)
				std::cout << dim;
			std::cout << ialph(invertI(pre));

			network->setCurrentTimestep(seq_size + i);
			pre = network->predict(pre);
			pre = (makeI(invertI(pre)).array() * 2 + pre.array()).array() / 3;
		}
		std::cout << std::flush;


		if(in.size() > max_size * 2)
		{
			std::vector<Vector>(in.begin() + max_size, in.end()).swap(in);
			std::vector<Vector>(out.begin() + max_size, out.end()).swap(out);
		}
	}
	return 0;
}
