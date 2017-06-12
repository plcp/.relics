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

#include "mergeslice.h"

MergeSlice::MergeSlice(size_t start, size_t size)
    : _start(start), _size(size)
{
}

MergeiSlice::MergeiSlice()
{
}


void MergeSlice::forward()
{
    Port *input = _inputs[0];
    _output.value.array() = input->value.segment(_start, _size);
}

void MergeiSlice::forward()
{
    size_t i = 0;
    for(Port *input: _inputs)
    {
        _output.value.segment(_offset[i], input->value.rows()) = input->value;
        ++i;
    }
}

void MergeSlice::backward()
{ 
    Port *input = _inputs[0];
    input->error.segment(_start, _size) = _output.error;
}

void MergeiSlice::backward()
{
    size_t i = 0;
    for(Port *input: _inputs)
    {
	input->error = _output.error.segment(_offset[i], input->error.rows());
        ++i;
    }
}

void MergeSlice::addInput(Port *input)
{
    if(_inputs.size() > 0)
        throw;

    _output.value = Vector::Zero(_size);
    _output.error = Vector::Zero(_size);
    _inputs.push_back(input);
}

void MergeiSlice::addInput(Port *input)
{
    size_t last = _offset.size();
    if(last == 0)
    {
        _offset.push_back(0);
	++last;
    }
    size_t offset = _offset[last - 1];
    _output.value = Vector::Zero(offset + input->value.rows());
    _output.error = Vector::Zero(offset + input->value.rows());
    
    _offset.push_back(offset + input->value.rows());
    _inputs.push_back(input);
}

