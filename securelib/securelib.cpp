#include "pch.h"
#include "securelib.h"

namespace fs = std::filesystem;

FILE* g_logFile = nullptr;
bool g_logTrace = false;

void securelib::setLogFile(FILE* file)
{
	g_logFile = file;
}

void securelib::setLogTrace(bool trace)
{
	g_logTrace = trace;
}

void securelib::log(const std::string& msg)
{
	if (g_logFile == nullptr)
		return;
	fprintf(g_logFile, "%s\n", msg.c_str());
	fflush(g_logFile);
}

void securelib::log(const std::wstring& msg)
{
	if (g_logFile == nullptr)
		return;
	fprintf(g_logFile, "%S\n", msg.c_str());
	fflush(g_logFile);
}

void securelib::trace(const std::string& msg)
{
	if (!g_logTrace)
		return;
	log(msg);
}

void securelib::trace(const std::wstring& msg)
{
	if (!g_logTrace)
		return;
	log(msg);
}

std::string securelib::Hash(const uint8_t* data, size_t len)
{
	// https://github.com/System-Glitch/SHA256
	SHA256 hasher;
	hasher.update(data, len);

	uint8_t* digestBytes = hasher.digest();
	std::string digest = hasher.toString(digestBytes);
	delete[] digestBytes; // don't forget to delete the digest!
	
	return digest;
}

std::string securelib::Hash(const std::string& str)
{
	return Hash(reinterpret_cast<const uint8_t*>(str.c_str()), str.size());
}

inline void PadVectorTo8ths(std::vector<uint8_t>& vec)
{
	while (vec.size() % 8)
		vec.push_back(0);
}

inline uint32_t htonl(uint32_t x)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	uint8_t* s = (uint8_t*)&x;
	return (uint32_t)(s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3]);
#else
	return x;
#endif
}

inline uint32_t ntohl(uint32_t const net) {
#if BYTE_ORDER == LITTLE_ENDIAN
	uint8_t data[4] = {};
	memcpy(&data, &net, sizeof(data));

	return ((uint32_t)data[3] << 0)
		| ((uint32_t)data[2] << 8)
		| ((uint32_t)data[1] << 16)
		| ((uint32_t)data[0] << 24);
#else
	return x;
#endif
}

union lenbytes
{
	uint32_t len;
	uint8_t bytes[4];
};

inline void AddLengthToVector(std::vector<uint8_t>& vec, uint32_t len)
{
	lenbytes holder;
	holder.len = htonl(len);
	vec.push_back(holder.bytes[0]);
	vec.push_back(holder.bytes[1]);
	vec.push_back(holder.bytes[2]);
	vec.push_back(holder.bytes[3]);
}

inline uint32_t RemoveLengthFromVector(std::vector<uint8_t>& vec)
{
	lenbytes holder;
	memcpy(holder.bytes, vec.data() + vec.size() - 4, 4);
	vec.resize(vec.size() - 4);
	return ntohl(holder.len);
}

std::vector<uint8_t> securelib::Encrypt(std::vector<uint8_t> data, const std::string& key)
{
	uint32_t originalInputSize = static_cast<uint32_t>(data.size());
	if (data.size() > 0)
	{
		PadVectorTo8ths(data);

		auto keyVec = StrToVec(key);
		CBlowFish enc(keyVec.data(), keyVec.size());

		enc.Encrypt(data.data(), data.size());
	}
	AddLengthToVector(data, originalInputSize);
	return data;
}

std::vector<uint8_t> securelib::Decrypt(std::vector<uint8_t> data, const std::string& key)
{
	if (data.size() < sizeof(uint32_t))
		return std::vector<uint8_t>();

	uint32_t originalInputSize = RemoveLengthFromVector(data);
	if (originalInputSize == 0)
		return data;

	auto keyVec = StrToVec(key);
	CBlowFish dec(keyVec.data(), keyVec.size());

	dec.Decrypt(data.data(), data.size());

	data.resize(originalInputSize);
	return data;
}

std::string securelib::Encrypt(const std::string& str, const std::string& key)
{
	return VecToStr(Encrypt(StrToVec(str), key));
}

std::string securelib::Decrypt(const std::string& str, const std::string& key)
{
	return VecToStr(Decrypt(StrToVec(str), key));
}

std::vector<unsigned char> securelib::StrToVec(const std::string& str)
{
	std::vector<unsigned char> vec;
	vec.resize(str.size());
	memcpy(vec.data(), str.c_str(), vec.size());
	return vec;
}

std::string securelib::VecToStr(const std::vector<unsigned char>& data)
{
	std::string retVal;
	retVal.resize(data.size());
	memcpy(const_cast<char*>(retVal.c_str()), data.data(), data.size());
	return retVal;
}

std::string securelib::UniqueStr()
{
	static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex); // protect rand()
	std::stringstream stream;
	// https://github.com/graeme-hill/crossguid
	stream << xg::newGuid() << rand() << time(nullptr);
	return Hash(stream.str());
}

std::vector<uint8_t> securelib::LoadFile(const std::wstring& path, bool requireExists)
{
	trace(L"Loading file: " + path);
	std::ifstream file(path, std::ios::binary);
	if (!file)
	{
		if (requireExists)
			throw std::runtime_error("File does not exist");
		trace(L"Loading file failed");
		return std::vector<uint8_t>();
	}

	std::vector<char> chars(std::istreambuf_iterator<char>(file), {});
	trace("File loaded: " + std::to_string(chars.size()) + " bytes");

	std::vector<uint8_t> bytes;
	bytes.resize(chars.size());
	memcpy(bytes.data(), chars.data(), bytes.size());
	return bytes;
}

void securelib::SaveFile(const std::wstring& path, const std::vector<uint8_t>& bytes)
{
	trace(L"Saving file: " + path + L" - " + std::to_wstring(bytes.size()) + L" bytes");
	fs::create_directories(fs::path(path).parent_path());
	{
		std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!file)
			throw new std::runtime_error("Saving file failed");
		file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
	}
	trace("File saved");
}

bool securelib::IsFilenameValid(const std::wstring& filename)
{
	if (filename.empty())
		return false;

	if
	(
		filename == L"."
		||
		filename.front() == ' '
		||
		filename.back() == ' '
		||
		filename.back() == '.'
		||
		filename.find(L"..") != std::wstring::npos
	)
	{
		return false;
	}

	for (auto c : filename)
	{
		if ((c >= 0x0 && c <= 0x1F) || c == 0x7F)
			return false;

		switch (static_cast<char>(c))
		{
		case '\"':
		case '\\':
		case ':':
		case '/':
		case '<':
		case '>':
		case '|':
		case '?':
		case '*':
			return false;
		}
	}
	return true;
}
