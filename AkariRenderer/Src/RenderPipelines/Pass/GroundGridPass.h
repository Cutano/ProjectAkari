#pragma once
#include "RPI/RenderPass.h"
#include "SceneComponents/Visitor.h"

namespace Akari
{
    class VertexBuffer;
    class IndexBuffer;
    
    class GroundGridPass : RenderPass
    {
    public:
        GroundGridPass(std::shared_ptr<RenderTarget> renderTarget);

        void Record(const RenderContext& context) override;
        void Execute() override;

    private:
        enum RootParams
        {
            // cbuffer vertexBuffer : register(b0)
            MatrixCB,
            NumRootParams
        };

        std::shared_ptr<VertexBuffer> m_VertexBuffer = nullptr;
        std::shared_ptr<IndexBuffer> m_IndexBuffer = nullptr;
    };
}
