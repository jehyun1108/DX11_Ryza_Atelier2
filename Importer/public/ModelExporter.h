#pragma once

NS_BEGIN(Importer)

class ModelExporter
{
public:
	bool Export(const ImportedData& data, const filesystem::path& outPath);

private:
	// -------- Binary ----------------------------------
	void WriteMaterials(const vector<unique_ptr<MaterialData>>& materials);
	void WriteMeshes(const vector<unique_ptr<MeshData>>& meshes);
	void WriteSkeleton(const SkeletonInfo& skeleton);
	void WriteAnimations(const vector<unique_ptr<AnimClip>>& animClips);

private:
	void WriteString(const string& str);

	template<typename T>
	void WriteVector(const vector<T>& vec);

	template<typename T>
	void WriteData(const T& data);
private:
	ofstream outFile;
};

NS_END

template<typename T>
inline void ModelExporter::WriteVector(const vector<T>& vec)
{
	_uint size = static_cast<_uint>(vec.size());
	outFile.write(reinterpret_cast<const char*>(&size), sizeof(_uint));

	if (size > 0)
	{
		static_assert(is_trivially_copyable_v<T>, "T must be trivially copyable");
		outFile.write(reinterpret_cast<const char*>(vec.data()), sizeof(T) * size);
	}
}

template<typename T>
inline void ModelExporter::WriteData(const T& data)
{
	static_assert(is_trivially_copyable_v<T>, "T must be trivially copyable");
	outFile.write(reinterpret_cast<const char*>(&data), sizeof(T));
}
