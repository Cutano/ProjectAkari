#pragma once

/*
 *  Copyright(c) 2019 Jeremiah van Oosten
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
 *  @file Model.h
 *  @date March 21, 2019
 *  @author Jeremiah van Oosten
 *
 *  @brief Model file for storing scene data.
 */

#include <DirectXCollision.h> // For DirectX::BoundingBox
#include <map>

struct aiMaterial;
struct aiMesh;
struct aiNode;
struct aiScene;

namespace Akari
{
class CommandList;
class Device;
class ModelNode;
class Mesh;
class Material;
class Visitor;

class Model
{
public:
    Model()  = default;
    ~Model() = default;

    void SetRootNode( std::shared_ptr<ModelNode> node )
    {
        m_RootNode = node;
    }

    std::shared_ptr<ModelNode> GetRootNode() const
    {
        return m_RootNode;
    }

    /**
     * Get the AABB of the scene.
     * This returns the AABB of the root node of the scene.
     */
    DirectX::BoundingBox GetAABB() const;

    /**
     * Accept a visitor.
     * This will first visit the scene, then it will visit the root node of the scene.
     */
    virtual void Accept( Visitor& visitor );

protected:
    friend class CommandList;

    /**
     * Load a scene from a file on disc.
     */
    bool LoadModelFromFile( CommandList& commandList, const std::wstring& fileName,
                            const std::function<bool( float )>& loadingProgress );

    /**
     * Load a scene from a string.
     * The scene can be preloaded into a byte array and the
     * scene can be loaded from the loaded byte array.
     *
     * @param sceneStr The byte encoded scene file.
     * @param format The format of the scene file.
     */
    bool LoadModelFromString( CommandList& commandList, const std::string& sceneStr, const std::string& format );

private:
    void ImportModel( CommandList& commandList, const aiScene& scene, std::filesystem::path parentPath );
    void ImportMaterial( CommandList& commandList, const aiMaterial& material, std::filesystem::path parentPath );
    void ImportMesh( CommandList& commandList, const aiMesh& mesh );
    std::shared_ptr<ModelNode> ImportSceneNode( CommandList& commandList, std::shared_ptr<ModelNode> parent,
                                                const aiNode* aiNode );

    using MaterialMap  = std::map<std::string, std::shared_ptr<Material>>;
    using MaterialList = std::vector<std::shared_ptr<Material>>;
    using MeshList     = std::vector<std::shared_ptr<Mesh>>;

    MaterialMap  m_MaterialMap;
    MaterialList m_Materials;
    MeshList     m_Meshes;

    std::shared_ptr<ModelNode> m_RootNode;

    std::wstring m_SceneFile;
};
}  // namespace Akari