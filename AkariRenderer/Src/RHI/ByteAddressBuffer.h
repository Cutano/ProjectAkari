#pragma once

/*
 *  Copyright(c) 2018 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

/**
 *  @file ByteAddressBuffer.h
 *  @date October 22, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief ByteAddressBuffer.
 *  @see https://msdn.microsoft.com/en-us/library/ff471453(v=vs.85).aspx
 */

#include "Buffer.h"
#include "DescriptorAllocation.h"

namespace Akari
{

class Device;

class ByteAddressBuffer : public Buffer
{
public:
    size_t GetBufferSize() const
    {
        return m_BufferSize;
    }

protected:
    ByteAddressBuffer( Device& device, const D3D12_RESOURCE_DESC& resDesc );
    ByteAddressBuffer( Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource );
    virtual ~ByteAddressBuffer() = default;

private:
    size_t m_BufferSize;
};
}  // namespace Akari