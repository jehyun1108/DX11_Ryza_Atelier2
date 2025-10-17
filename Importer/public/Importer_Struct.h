#pragma once

namespace Importer
{
	struct MaterialData
	{
		string name;
		map<TEXSLOT, filesystem::path> textures;
	};

	struct MeshData
	{
		string name;
		MESHTYPE type;
		VertexLayoutID layoutID = VertexLayoutID::Unknown;

		vector<Vertex_PNUTAN> verticesPNUTan;
		vector<Vertex_Anim>   verticesPNUTanSkin;

		vector<_uint> indices;
		_uint materialIdx;
	};

	struct ImportedData
	{
		unique_ptr<SkeletonInfo> skeleton;
		vector<unique_ptr<AnimClip>> animations;
		vector<unique_ptr<MaterialData>> materials;
		vector<unique_ptr<MeshData>> meshes;

		bool hasSkeletonBlock = false;
	};
}