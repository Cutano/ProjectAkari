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
 *  @file IndexBuffer.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief Index buffer resource.
 */

#include "Buffer.h"

namespace Akari
{
class IndexBuffer : public Buffer
{
public:
    /**
     * Get the index buffer view for biding to the Input Assembler stage.
     */
    D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
    {
        return m_IndexBufferView;
    }

    size_t GetNumIndices() const
    {
        return m_NumIndices;
    }

    DXGI_FORMAT GetIndexFormat() const
    {
        return m_IndexFormat;
    }

protected:
    IndexBuffer( Device& device, size_t numIndices, DXGI_FORMAT indexFormat );
    IndexBuffer( Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndices,
                 DXGI_FORMAT indexFormat );
    virtual ~IndexBuffer() = default;

    void CreateIndexBufferView();

private:
    size_t      m_NumIndices;
    DXGI_FORMAT m_IndexFormat;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};
}  // namespace Akari