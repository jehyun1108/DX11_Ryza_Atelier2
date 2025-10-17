#include "Enginepch.h"
#include "BinaryUtil.h"

ofstream BinaryUtil::OpenOut(const filesystem::path& filePath, bool truncate)
{
	ios::openmode mode = ios::binary | ios::out;
	if (truncate) mode |= ios::trunc;
	ofstream outFile(filePath, mode);
	return outFile;
}

ifstream BinaryUtil::OpenIn(const filesystem::path& filePath)
{
	ifstream inFile(filePath, ios::binary | ios::in);
	return inFile;
}

void BinaryUtil::WriteHeader(ofstream& outFile, _uint magic, _uint version)
{
#ifdef _DEBUG
	assert(outFile.good());
#endif
	WritePOD(outFile, magic);
	WritePOD(outFile, version);
}

bool BinaryUtil::ReadHeader(ifstream& inFile, _uint expectedMagic, _uint& outVersion)
{
	_uint magic{};
	if (!ReadPOD(inFile, magic))      return false;
	if (magic != expectedMagic)       return false;

	if (!ReadPOD(inFile, outVersion)) return false;
	return true;
}

void BinaryUtil::WriteBytes(ofstream& outFile, span<const uint8_t> bytes)
{
#ifdef _DEBUG
	assert(outFile.good());
#endif
	if (!bytes.empty())
		outFile.write(reinterpret_cast<const char*>(bytes.data()), static_cast<streamsize>(bytes.size()));
}

bool BinaryUtil::ReadBytes(ifstream& inFile, span<uint8_t> outBuffer)
{
	if (outBuffer.empty()) return true;
	inFile.read(reinterpret_cast<char*>(outBuffer.data()), static_cast<streamsize>(outBuffer.size()));
	return static_cast<bool>(inFile);
}

void BinaryUtil::WriteString(ofstream& outFile, const string& str)
{
	const uint64_t byteCount64 = str.size();
#ifdef _DEBUG
	assert(byteCount64 <= (numeric_limits<_uint>::max)() && "String too large");
#endif
	const _uint byteCount = static_cast<_uint>(byteCount64);
	WritePOD(outFile, byteCount);
	if (byteCount > 0)
		outFile.write(str.data(), static_cast<streamsize>(byteCount));
}

bool BinaryUtil::ReadString(ifstream& inFile, string& outStr)
{
	_uint byteCount = 0;
	if (!ReadPOD(inFile, byteCount)) return false;
	outStr.clear();
	if (byteCount == 0) return true;

	outStr.resize(byteCount);
	inFile.read(outStr.data(), static_cast<streamsize>(byteCount));
	return static_cast<bool>(inFile);
}

void BinaryUtil::WriteAlign(ofstream& outFile, _uint align)
{
#ifdef _DEBUG
	assert(align > 0 && "Align must be > 0");
#endif
	const streamoff pos = outFile.tellp();
	const _uint padding = static_cast<_uint>((align - (pos % align)) % align);
	if (padding)
	{
		static const uint8_t zeros[16] = { 0 };
		_uint remain = padding;
		while (remain > 0)
		{
			const _uint chunk = min<_uint>(remain, 16);
			WriteBytes(outFile, span<const uint8_t>(zeros, chunk));
			remain -= chunk;
		}
	}
}

bool BinaryUtil::ReadAlign(ifstream& inFile, _uint align)
{
#ifdef _DEBUG
	assert(align > 0 && "Align must be > 0");
#endif
	const streamoff pos = inFile.tellg();
	const _uint padding = static_cast<_uint>((align - (pos % align)) % align);
	if (padding)
	{
		vector<uint8_t> scratch(padding);
		inFile.read(reinterpret_cast<char*>(scratch.data()), static_cast<streamsize>(padding));
		if (!inFile) return false;
	}
	return true;
}

bool BinaryUtil::AtomicSave(const filesystem::path& targetPath, const nlohmann::json& document, bool prettyPrint, string& outErrorMsg)
{
	const filesystem::path tempPath = targetPath.string() + ".tmp";
	{
		ofstream tempOut = OpenOut(tempPath, /*truncate*/true);
		if (!tempOut)
		{
			outErrorMsg = "Failed to open temp file: " + tempPath.string();
			return false;
		}
		const string payload = prettyPrint ? document.dump(4) : document.dump();
		tempOut.write(payload.data(), static_cast<streamsize>(payload.size()));
		if (!tempOut) { outErrorMsg = "Failed to write temp file: " + tempPath.string(); return false; }
	}
	// 교체
	error_code errorCode;
	filesystem::rename(tempPath, targetPath, errorCode);
	if (errorCode)
	{
		// 일부 파일시스템에서는 rename이 실패할 수 있어 remove 후 rename 재시도 가능
		outErrorMsg = "Failed to replace file: " + targetPath.string() + " reason: " + errorCode.message();
		return false;
	}
	return true;
}
