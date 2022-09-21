#include "pch.h"

#include "Mesh.h"
#include "SceneNode.h"
#include "Visitor.h"

using namespace Akari;
using namespace DirectX;

SceneNode::SceneNode( const DirectX::XMMATRIX& localTransform )
: m_Name( "SceneNode" )
, m_AABB( { 0, 0, 0 }, {0, 0, 0} )
{
    m_AlignedData                     = (AlignedData*)_aligned_malloc( sizeof( AlignedData ), 16 );
    m_AlignedData->m_LocalTransform   = localTransform;
    m_AlignedData->m_InverseTransform = XMMatrixInverse( nullptr, localTransform );
}

SceneNode::~SceneNode()
{
    _aligned_free( m_AlignedData );
}

const std::string& SceneNode::GetName() const
{
    return m_Name;
}

void SceneNode::SetName( const std::string& name )
{
    m_Name = name;
}

DirectX::XMMATRIX SceneNode::GetLocalTransform() const
{
    return m_AlignedData->m_LocalTransform;
}

void SceneNode::SetLocalTransform( const DirectX::XMMATRIX& localTransform )
{
    m_AlignedData->m_LocalTransform   = localTransform;
    m_AlignedData->m_InverseTransform = XMMatrixInverse( nullptr, localTransform );
}

DirectX::XMMATRIX SceneNode::GetInverseLocalTransform() const
{
    return m_AlignedData->m_InverseTransform;
}

DirectX::XMMATRIX SceneNode::GetWorldTransform() const
{
    return m_AlignedData->m_LocalTransform * GetParentWorldTransform();
}

DirectX::XMMATRIX SceneNode::GetInverseWorldTransform() const
{
    return XMMatrixInverse( nullptr, GetWorldTransform() );
}

DirectX::XMMATRIX SceneNode::GetParentWorldTransform() const
{
    XMMATRIX parentTransform = XMMatrixIdentity();
    if ( auto parentNode = m_ParentNode.lock() )
    {
        parentTransform = parentNode->GetWorldTransform();
    }

    return parentTransform;
}

void SceneNode::AddChild( std::shared_ptr<SceneNode> childNode )
{
    if ( childNode )
    {
        NodeList::iterator iter = std::find( m_Children.begin(), m_Children.end(), childNode );
        if ( iter == m_Children.end() )
        {
            XMMATRIX worldTransform = childNode->GetWorldTransform();
            childNode->m_ParentNode = shared_from_this();
            XMMATRIX localTransform = worldTransform * GetInverseWorldTransform();
            childNode->SetLocalTransform( localTransform );
            m_Children.push_back( childNode );
            if ( !childNode->GetName().empty() )
            {
                m_ChildrenByName.emplace( childNode->GetName(), childNode );
            }
        }
    }
}

void SceneNode::RemoveChild( std::shared_ptr<SceneNode> childNode )
{
    if ( childNode )
    {
        NodeList::const_iterator iter = std::find( m_Children.begin(), m_Children.end(), childNode );
        if ( iter != m_Children.cend() )
        {
            childNode->SetParent( nullptr );
            m_Children.erase( iter );

            // Also remove it from the name map.
            NodeNameMap::iterator iter2 = m_ChildrenByName.find( childNode->GetName() );
            if ( iter2 != m_ChildrenByName.end() )
            {
                m_ChildrenByName.erase( iter2 );
            }
        }
        else
        {
            // Maybe the child appears deeper in the scene graph.
            for ( auto child: m_Children )
            {
                child->RemoveChild( childNode );
            }
        }
    }
}

void SceneNode::SetParent( std::shared_ptr<SceneNode> parentNode )
{
    // Parents own their children.. If this node is not owned
    // by anyone else, it will cease to exist if we remove it from it's parent.
    // As a precaution, store myself as a shared pointer so I don't get deleted
    // half-way through this function!
    // Technically self deletion shouldn't occur because the thing invoking this function
    // should have a shared_ptr to it.
    std::shared_ptr<SceneNode> me = shared_from_this();
    if ( parentNode )
    {
        parentNode->AddChild( me );
    }
    else if ( auto parent = m_ParentNode.lock() )
    {
        // Setting parent to NULL.. remove from current parent and reset parent node.
        auto worldTransform = GetWorldTransform();
        parent->RemoveChild( me );
        m_ParentNode.reset();
        SetLocalTransform( worldTransform );
    }
}

size_t SceneNode::AddMesh( std::shared_ptr<Mesh> mesh )
{
    size_t index = (size_t)-1;
    if ( mesh )
    {
        MeshList::const_iterator iter = std::find( m_Meshes.begin(), m_Meshes.end(), mesh );
        if ( iter == m_Meshes.cend() )
        {
            index = m_Meshes.size();
            m_Meshes.push_back( mesh );

            // Merge the mesh's AABB with AABB of the scene node.
            BoundingBox::CreateMerged( m_AABB, m_AABB, mesh->GetAABB() );
        }
        else
        {
            index = iter - m_Meshes.begin();
        }
    }

    return index;
}

void SceneNode::RemoveMesh( std::shared_ptr<Mesh> mesh )
{
    if ( mesh )
    {
        MeshList::const_iterator iter = std::find( m_Meshes.begin(), m_Meshes.end(), mesh );
        if ( iter != m_Meshes.end() )
        {
            m_Meshes.erase( iter );
        }
    }
}

std::shared_ptr<Mesh> SceneNode::GetMesh(size_t pos) 
{
    std::shared_ptr<Mesh> mesh = nullptr;

    if (pos < m_Meshes.size()) {
        mesh = m_Meshes[pos];
    }
    
    return mesh;
}

const DirectX::BoundingBox& SceneNode::GetAABB() const 
{
    return m_AABB;
}

void SceneNode::Accept( Visitor& visitor )
{
    visitor.Visit( *this );

    // Visit meshes
    for ( auto& mesh: m_Meshes )
    {
        mesh->Accept( visitor );
    }

    // Visit children
    for ( auto& child: m_Children )
    {
        child->Accept( visitor );
    }
}
