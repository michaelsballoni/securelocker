#include <string>
#include <vector>

namespace securelib
{
	std::string Hash(const uint8_t* data, uint32_t len);
	std::string Hash(const std::wstring& str);

	std::vector<uint8_t> Encrypt(const std::vector<uint8_t>& data, const std::wstring& key);
	std::vector<uint8_t> Decrypt(const std::vector<uint8_t>& data, const std::wstring& key);

	std::wstring Encrypt(const std::wstring& str, const std::wstring& key);
	std::wstring Decrypt(const std::wstring& str, const std::wstring& key);

	std::vector<unsigned char> StrToVec(const std::wstring& str);
	std::wstring VecToStr(const std::vector<unsigned char>& data);

	std::string UniqueStr();
}