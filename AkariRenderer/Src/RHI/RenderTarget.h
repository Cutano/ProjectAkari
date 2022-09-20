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
 *  @file RenderTarget.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief A render target is used to store a set of textures that are the
 *  target for rendering.
 *  Maximum number of color textures that can be bound to the render target is 8
 *  (0 - 7) and one depth-stencil buffer.
 */

namespace Akari
{

class Texture;

// Don't use scoped enums to avoid the explicit cast required to use these as
// array indices.
enum AttachmentPoint
{
    Color0,
    Color1,
    Color2,
    Color3,
    Color4,
    Color5,
    Color6,
    Color7,
    DepthStencil,
    NumAttachmentPoints
};

class RenderTarget
{
public:
    // Create an empty render target.
    RenderTarget();

    RenderTarget( const RenderTarget& copy ) = default;
    RenderTarget( RenderTarget&& copy )      = default;

    RenderTarget& operator=( const RenderTarget& other ) = default;
    RenderTarget& operator=( RenderTarget&& other ) = default;

    /**
     * Attach a texture to a given attachment point.
     *
     * @param attachmentPoint The point to attach the texture to.
     * @param [texture] Optional texture to bind to the render target. Specify nullptr to remove the texture.
     */
    void                     AttachTexture( AttachmentPoint attachmentPoint, std::shared_ptr<Texture> texture );
    std::shared_ptr<Texture> GetTexture( AttachmentPoint attachmentPoint ) const;

    // Resize all of the textures associated with the render target.
    void             Resize( DirectX::XMUINT2 size );
    void             Resize( uint32_t width, uint32_t height );
    DirectX::XMUINT2 GetSize() const;
    uint32_t         GetWidth() const;
    uint32_t         GetHeight() const;

    // Get a viewport for this render target.
    // The scale and bias parameters can be used to specify a split-screen
    // viewport (the bias parameter is normalized in the range [0...1]).
    // By default, a fullscreen viewport is returned.
    D3D12_VIEWPORT GetViewport( DirectX::XMFLOAT2 scale = { 1.0f, 1.0f }, DirectX::XMFLOAT2 bias = { 0.0f, 0.0f },
                                float minDepth = 0.0f, float maxDepth = 1.0f ) const;

    // Get a list of the textures attached to the render target.
    // This method is primarily used by the CommandList when binding the
    // render target to the output merger stage of the rendering pipeline.
    const std::vector<std::shared_ptr<Texture>>& GetTextures() const;

    // Get the render target formats of the textures currently
    // attached to this render target object.
    // This is needed to configure the Pipeline state object.
    D3D12_RT_FORMAT_ARRAY GetRenderTargetFormats() const;

    // Get the format of the attached depth/stencil buffer.
    DXGI_FORMAT GetDepthStencilFormat() const;

    // Get the sample description of the render target.
    DXGI_SAMPLE_DESC GetSampleDesc() const;

    // Reset all textures
    void Reset()
    {
        m_Textures = RenderTargetList( AttachmentPoint::NumAttachmentPoints );
    }

private:
    using RenderTargetList = std::vector<std::shared_ptr<Texture>>;
    RenderTargetList m_Textures;
    DirectX::XMUINT2                      m_Size;
};
}  // namespace Akari