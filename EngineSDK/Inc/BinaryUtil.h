#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL BinaryUtil
{
public:
	BinaryUtil() = delete;

	// Open 
	static ofstream OpenOut(const filesystem::path& filePath, bool truncate = true);
	static ifstream OpenIn(const filesystem::path& filePath);

	// Magic + Version 헤더
	static void WriteHeader(ofstream& outFile, _uint magic, _uint version);
	static bool ReadHeader(ifstream& inFile, _uint expectedMagic, _uint& outVersion);

	// POD
	template<typename T>
	static void WritePOD(ofstream& outFile, const T& data);
	template<typename T>
	static bool ReadPOD(ifstream& inFile, T& outData);

	// Raw Bytes
	static void WriteBytes(ofstream& outFile, span<const uint8_t> bytes);
	static bool ReadBytes (ifstream& inFile,  span<uint8_t> outBuffer);

	// string (UTF-8): _uint + Data
	static void WriteString(ofstream& outFile, const string& str);
	static bool ReadString (ifstream& inFile,  string& outStr);

	// vector<T>: _uint 길이 + 연속데이터
	template<typename T>
	static void WriteVector(ofstream& outFile, const vector<T>& values);
	template<typename T>
	static bool ReadVector(ifstream& inFile, vector<T>& outValues);

	// ------ Alignment padding ---------------
	static void WriteAlign(ofstream& outFile, _uint align);
	static bool ReadAlign (ifstream& inFile,  _uint align);

	// ------- Atomic Save (임시 파일 -> 교체) : 파일 깨짐 방지
	static bool AtomicSave(const filesystem::path& targetPath, const nlohmann::json& document, bool prettyPrint, string& outErrorMsg);
};

template<typename T>
inline void BinaryUtil::WritePOD(ofstream& outFile, const T& data)
{
	static_assert(is_trivially_copyable_v<T>, "T must be trivially copyable");
#ifdef _DEBUG
	assert(outFile.good());
#endif
	outFile.write(reinterpret_cast<const char*>(&data), sizeof(T));
}

template<typename T>
inline bool BinaryUtil::ReadPOD(ifstream& inFile, T& outData)
{
	static_assert(is_trivially_copyable_v<T>, "T must be trivially copyable");
	inFile.read(reinterpret_cast<char*>(&outData), sizeof(T));
	return static_cast<bool>(inFile);
}

template<typename T>
inline void BinaryUtil::WriteVector(ofstream& outFile, const vector<T>& values)
{
	static_assert(is_trivially_copyable_v<T>, "vector<T>: T must be trivially copyable");
	const uint64_t count64 = values.size();
#ifdef _DEBUG
	assert(count64 <= (numeric_limits<_uint>::max)() && "Vector too large");
#endif
	const _uint count = static_cast<_uint>(count64);
	WritePOD(outFile, count);
	
	if (count > 0)
	{
		const size_t totalBytes = sizeof(T) * static_cast<size_t>(count);
		outFile.write(reinterpret_cast<const char*>(values.data()), static_cast<streamsize>(totalBytes));
	}
}

template<typename T>
bool BinaryUtil::ReadVector(ifstream& inFile, vector<T>& outValues)
{
	static_assert(is_trivially_copyable_v<T>, "Vector<T>: T must be trivially copyable");
	uint32_t count = 0;
	if (!ReadPOD(inFile, count)) return false;

	if (count == 0) { outValues.clear(); return true; }

	// overflow 방지 
	const uint64_t totalBytes64 = static_cast<uint64_t>(count) * sizeof(T);
#ifdef _DEBUG
	assert(totalBytes64 <= (numeric_limits<size_t>::max)() && "Vector byte size overflow");
#endif
	outValues.resize(count);
	inFile.read(reinterpret_cast<char*>(outValues.data()), static_cast<streamsize>(totalBytes64));
	return static_cast<bool>(inFile);
}

NS_END