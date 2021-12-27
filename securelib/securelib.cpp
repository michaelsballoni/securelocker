#include "pch.h"
#include "securelib.h"

#include "../../SHA256/include/SHA256.h"

#include "Blowfish.h"

std::string securelib::Hash(const uint8_t* data, uint32_t len)
{
	// https://github.com/System-Glitch/SHA256
	SHA256 hasher;
	hasher.update(data, len);

	uint8_t* digestBytes = hasher.digest();
	std::string digest = hasher.toString(digestBytes);
	delete[] digestBytes;
	
	return digest;
}

std::string securelib::Hash(const std::wstring& str)
{
	return Hash(reinterpret_cast<const uint8_t*>(str.c_str()), static_cast<uint32_t>(str.size()) * sizeof(wchar_t));
}

std::vector<uint8_t> securelib::Encrypt(const std::vector<uint8_t>& data, const std::wstring& key)
{
	auto keyVec = StrToVec(key);
	CBlowFish enc(keyVec.data(), keyVec.size());
	std::vector<uint8_t> retVal;
	retVal.resize(data.size());
	enc.Encrypt(data.data(), retVal.data(), data.size());
    return retVal;
}

std::vector<uint8_t> securelib::Decrypt(const std::vector<uint8_t>& data, const std::wstring& key)
{
	auto keyVec = StrToVec(key);
	CBlowFish dec(keyVec.data(), keyVec.size());
	std::vector<uint8_t> retVal;
	retVal.resize(data.size());
	dec.Decrypt(data.data(), retVal.data(), data.size());
	return retVal;
}

std::wstring securelib::Encrypt(const std::wstring& str, const std::wstring& key)
{
	return VecToStr(Encrypt(StrToVec(str), key));
}

std::wstring securelib::Decrypt(const std::wstring& str, const std::wstring& key)
{
	return VecToStr(Decrypt(StrToVec(str), key));
}

std::vector<unsigned char> securelib::StrToVec(const std::wstring& str)
{
	std::vector<int> ints;
	ints.reserve(str.size());
	for (auto c : str)
		ints.push_back(c);

	std::vector<unsigned char> vec;
	vec.resize(ints.size() * sizeof(int));
	memcpy(vec.data(), ints.data(), vec.size());
	return vec;
}

std::wstring securelib::VecToStr(const std::vector<unsigned char>& data)
{
	std::vector<int> ints;
	ints.resize(data.size() / sizeof(int));
	memcpy(ints.data(), data.data(), data.size());

	std::wstring retVal;
	for (auto i : ints)
		retVal.push_back(static_cast<wchar_t>(i));
	return retVal;
}

std::string UniqueStr()
{
	return "blet monkey"; // FORNOW: GUID + rand() + local time + algorithmic salt
}
