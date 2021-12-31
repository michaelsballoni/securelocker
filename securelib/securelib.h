#include <string>
#include <vector>

namespace securelib
{
	std::string Hash(const uint8_t* data, size_t len);
	std::string Hash(const std::string& str);

	std::vector<uint8_t> Encrypt(std::vector<uint8_t> data, const std::string& key);
	std::vector<uint8_t> Decrypt(std::vector<uint8_t> data, const std::string& key);

	std::string Encrypt(const std::string& str, const std::string& key);
	std::string Decrypt(const std::string& str, const std::string& key);

	std::vector<unsigned char> StrToVec(const std::string& str);
	std::string VecToStr(const std::vector<unsigned char>& data);

	std::string UniqueStr();

	std::vector<uint8_t> LoadFile(const std::wstring& path);
	void SaveFile(const std::wstring& path, const std::vector<uint8_t>& bytes);
}
