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
 *  @file VertexBuffer.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief Vertex buffer resource.
 */

#include "Buffer.h"

namespace Akari
{
class VertexBuffer : public Buffer
{
public:
    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const
    {
        return m_VertexBufferView;
    }

    size_t GetNumVertices() const
    {
        return m_NumVertices;
    }

    size_t GetVertexStride() const
    {
        return m_VertexStride;
    }

protected:
    VertexBuffer( Device& device, size_t numVertices, size_t vertexStride );
    VertexBuffer( Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numVertices,
                  size_t vertexStride );
    virtual ~VertexBuffer();

    void CreateVertexBufferView();

private:
    size_t                   m_NumVertices;
    size_t                   m_VertexStride;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
};
}  // namespace Akari