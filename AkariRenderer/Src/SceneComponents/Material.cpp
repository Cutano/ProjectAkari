#include "pch.h"

#include "Material.h"
#include "RHI/Texture.h"

using namespace Akari;

// Material properties must be 16-byte aligned.
// In order to ensure alignment, the matierial properties is allocated in aligned memory.
static MaterialProperties* NewMaterialProperties( const MaterialProperties& props )
{
    MaterialProperties* materialProperties = (MaterialProperties*)_aligned_malloc( sizeof( MaterialProperties ), 16 );
    *materialProperties                    = props;

    return materialProperties;
}

// Aligned memory must be deleted using the _aligned_free method.
static void DeleteMaterialProperties( MaterialProperties* p )
{
    _aligned_free( p );
}

Material::Material( const MaterialProperties& materialProperties )
: m_MaterialProperties( NewMaterialProperties( materialProperties ), &DeleteMaterialProperties )
{}

Material::Material( const Material& copy )
: m_MaterialProperties( NewMaterialProperties( *copy.m_MaterialProperties ), &DeleteMaterialProperties )
, m_Textures( copy.m_Textures )
{}

const DirectX::XMFLOAT4& Material::GetBaseColor() const
{
    return m_MaterialProperties->BaseColor;
}

void Material::SetBaseColor( const DirectX::XMFLOAT4& baseColor )
{
    m_MaterialProperties->BaseColor = baseColor;
}

const DirectX::XMFLOAT4& Material::GetEmissiveColor() const
{
    return m_MaterialProperties->Emissive;
}

void Material::SetEmissiveColor( const DirectX::XMFLOAT4& emissive )
{
    m_MaterialProperties->Emissive = emissive;
}

float Material::GetRoughness() const
{
    return m_MaterialProperties->Roughness;
}

void Material::SetRoughness( float roughness )
{
    m_MaterialProperties->Roughness = roughness;
}

const float Material::GetOpacity() const
{
    return m_MaterialProperties->Opacity;
}

void Material::SetOpacity( float opacity )
{
    m_MaterialProperties->Opacity = opacity;
}

float Material::GetMetallic() const
{
    return m_MaterialProperties->Metallic;
}

void Material::SetMetallic( float metallic )
{
    m_MaterialProperties->Metallic = metallic;
}

float Material::GetNormalScale() const
{
    return m_MaterialProperties->NormalScale;
}

void Material::SetNormalScale( float normalScale )
{
    m_MaterialProperties->NormalScale = normalScale;
}

std::shared_ptr<Texture> Material::GetTexture( TextureType ID ) const
{
    TextureMap::const_iterator iter = m_Textures.find( ID );
    if ( iter != m_Textures.end() )
    {
        return iter->second;
    }

    return nullptr;
}

void Material::SetTexture( TextureType type, std::shared_ptr<Texture> texture )
{
    m_Textures[type] = texture;

    switch ( type )
    {
    case TextureType::BaseColor:
    {
        m_MaterialProperties->HasBaseColorTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Metallic:
    {
        m_MaterialProperties->HasEmissiveTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Roughness:
    {
        m_MaterialProperties->HasMetallicTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Emissive:
    {
        m_MaterialProperties->HasRoughnessTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Occlusion:
    {
        m_MaterialProperties->HasOcclusionTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Normal:
    {
        m_MaterialProperties->HasNormalTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Bump:
    {
        m_MaterialProperties->HasBumpTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Opacity:
    {
        m_MaterialProperties->HasOpacityTexture = ( texture != nullptr );
    }
    break;
    }
}

bool Material::IsTransparent() const
{
    return ( m_MaterialProperties->Opacity < 1.0f || m_MaterialProperties->HasOpacityTexture );
}

const MaterialProperties& Material::GetMaterialProperties() const
{
    return *m_MaterialProperties;
}

void Material::SetMaterialProperties( const MaterialProperties& materialProperties )
{
    *m_MaterialProperties = materialProperties;
}

// clang-format off
const MaterialProperties Material::Zero = {
    { 0.0f, 0.0f, 0.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Red = {
    { 1.0f, 0.0f, 0.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Green = {
    { 0.0f, 1.0f, 0.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Blue = {
    { 0.0f, 0.0f, 1.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Cyan = {
    { 0.0f, 1.0f, 1.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Magenta = {
    { 1.0f, 0.0f, 1.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Yellow = {
    { 0.0f, 1.0f, 1.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::White = {
    { 1.0f, 1.0f, 1.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::WhiteDiffuse = {
    { 1.0f, 1.0f, 1.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Black = {
    { 0.0f, 0.0f, 0.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Emerald = {
    { 0.07568f, 0.61424f, 0.07568f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Jade = {
    { 0.54f, 0.89f, 0.63f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Obsidian = {
    { 0.18275f, 0.17f, 0.22525f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Pearl = {
    { 1.0f, 0.829f, 0.829f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Ruby = {
    { 0.61424f, 0.04136f, 0.04136f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Turquoise = {
    { 0.396f, 0.74151f, 0.69102f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Brass = {
    { 0.780392f, 0.568627f, 0.113725f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Bronze = {
    { 0.714f, 0.4284f, 0.18144f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Chrome = {
    { 0.4f, 0.4f, 0.4f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Copper = {
    { 0.7038f, 0.27048f, 0.0828f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Gold = {
    { 0.75164f, 0.60648f, 0.22648f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::Silver = {
    { 0.50754f, 0.50754f, 0.50754f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::BlackPlastic = {
    { 0.01f, 0.01f, 0.01f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::CyanPlastic = {
    { 0.0f, 0.50980392f, 0.50980392f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::GreenPlastic = {
    { 0.1f, 0.35f, 0.1f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::RedPlastic = {
    { 0.5f, 0.0f, 0.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::WhitePlastic = {
    { 0.55f, 0.55f, 0.55f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::YellowPlastic = {
    { 0.5f, 0.5f, 0.0f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::BlackRubber = {
    { 0.01f, 0.01f, 0.01f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::CyanRubber = {
    { 0.4f, 0.5f, 0.5f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::GreenRubber = {
    { 0.4f, 0.5f, 0.4f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::RedRubber = {
    { 0.5f, 0.4f, 0.4f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::WhiteRubber = {
    { 0.5f, 0.5f, 0.5f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};

const MaterialProperties Material::YellowRubber = {
    { 0.5f, 0.5f, 0.4f, 1.0f },
    0.0f,
    { 0.0f, 0.0f, 0.0f, 1.0f }, 
    1.0f
};
// clang-format on