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

#ifndef __MERGESLICE_H__
#define __MERGESLICE_H__

#include "abstractmergenode.h"

/**
 * @brief Slice one input port to a smaller output port by element-wise association
 */
class MergeSlice : public AbstractMergeNode
{
    public:
        MergeSlice(size_t start, size_t size);

        virtual void forward();
        virtual void backward();
	virtual void addInput(Port *input);

    private:
	size_t _start;
	size_t _size;
};

/**
 * @brief Concatenate multiple input port to a bigger output port by element-wise association
 */
class MergeiSlice : public AbstractMergeNode
{
    public:
        MergeiSlice();

        virtual void forward();
        virtual void backward();
	virtual void addInput(Port *input);

    private:
	std::vector<size_t> _offset;
};

#endif
