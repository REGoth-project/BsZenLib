/** \file
 * Import and cache static meshes (.3DS, .MRM)
 */

#pragma once
#include <Material/BsMaterial.h>
#include <Components/BsCRenderable.h>
#include <Mesh/BsMesh.h>
#include "ZenResources.hpp"

namespace ZenLoad
{
  struct PackedMesh;
  class zCMesh;
}  // namespace ZenLoad

namespace VDFS
{
  class FileIndex;
}

namespace BsZenLib
{
	/**
	 * Checks whether the given static mesh has been cached.
	 * 
	 * @param orignalFileName  Name of the file in the original game (eg. "STONE.3DS")
	 * 
	 * @return True, if the file was cached. False otherwise.
	 */
	bool HasCachedStaticMesh(const bs::String& originalFileName);

	/**
	 * Loads the cached version representing the given original file.
	 * 
	 * If unsure whether the cached file exists, call HasCachedStaticMesh().
	 * To create the cache, call ImportAndCacheStaticMesh().
	 * 
	 * @param originalFileName Name of the mesh in the original game files (eg. "STONE.3DS")
	 * 
	 * @return Handle to the cached mesh (Empty handle if none was found)
	 */
	Res::HMeshWithMaterials LoadCachedStaticMesh(const bs::String& originalFileName);

	/**
	 * Imports and caches a static mesh (.3DS) from the original game.
	 * 
	 * @note This will also cache Materials and Textures.
	 * 
	 * @param originalFileName Name of the static mesh in the original game (eg. "STONE.3DS")
	 * @param vdfs             VDFS containing the file to be imported.
	 * 
	 * @return Handle to the imported mesh (Empty if unsuccessfull)
	 */
	Res::HMeshWithMaterials ImportAndCacheStaticMesh(const bs::String& originalFileName, const VDFS::FileIndex& vdfs);

	/**
	 * Imports and caches a static mesh (.3DS) from the original game.
	 * 
	 * @note This will also cache Materials and Textures.
	 * 
	 * @param originalFileName Name of the static mesh in the original game (eg. "STONE.3DS")
	 * @param vdfs             VDFS containing the file to be imported.
	 * 
	 * @return Handle to the imported mesh (Empty if unsuccessfull)
	 */
	Res::HMeshWithMaterials ImportAndCacheStaticMesh(const bs::String& originalFileName, const ZenLoad::PackedMesh& packedMesh, const VDFS::FileIndex& vdfs);

	/**
	 * Imports and caches only the geometry of a static mesh (.3DS) from the original game.
	 * 
	 * @param originalFileName Name of the static mesh in the original game (eg. "STONE.3DS")
	 * @param vdfs             VDFS containing the file to be imported.
	 * 
	 * @return Handle to the imported mesh (Empty if unsuccessfull)
	 */
	bs::HMesh ImportAndCacheStaticMeshGeometry(const bs::String& originalFileName, const VDFS::FileIndex& vdfs);

	/**
	 * Imports and caches only the geometry from custom mesh data.
	 * 
	 * @param originalFileName Name of the static mesh in the original game (eg. "STONE.3DS")
	 * @param packedMesh       Custom mesh data.
	 * 
	 * @return Handle to the imported mesh (Empty if unsuccessfull)
	 */
	bs::HMesh ImportAndCacheStaticMeshGeometry(const bs::String& originalFileName, const ZenLoad::PackedMesh& packedMesh);


	/**
	 * Imports and caches only the materials of a static mesh (.3DS) from the original game.
	 * 
	 * @note This will also cache Textures.
	 * 
	 * @param originalFileName Name of the static mesh in the original game (eg. "STONE.3DS")
	 * @param vdfs             VDFS containing textures needed by the materials.
	 * 
	 * @return Handles to the imported materials (Empty if unsuccessfull)
	 */
	bs::Vector<bs::HMaterial> ImportAndCacheStaticMeshMaterials(const bs::String& originalFileName, const VDFS::FileIndex& vdfs);

	/**
	 * Imports and caches only the materials from custom mesh data.
	 * 
	 * @note This will also cache Textures.
	 * 
	 * @param originalFileName Name of the static mesh in the original game (eg. "STONE.3DS")
	 * @param packedMesh       Custom mesh data.
	 * @param vdfs             VDFS containing textures needed by the materials.
	 * 
	 * @return Handles to the imported materials (Empty if unsuccessfull)
	 */
	bs::Vector<bs::HMaterial> ImportAndCacheStaticMeshMaterials(const bs::String& originalFileName, const ZenLoad::PackedMesh& packedMesh, const VDFS::FileIndex& vdfs);

	/**
	 * Imports only the geometry from custom mesh data without caching it.
	 * 
	 * @param packedMesh       Custom mesh data.
	 * 
	 * @return Handle to the imported mesh (Empty if unsuccessfull)
	 */

  bs::HMesh ImportStaticMeshGeometry(const ZenLoad::PackedMesh& packedMesh);
}  // namespace BsZenLib
