#include "pch.h"
#include "CppUnitTest.h"

#include "securelib.h"
#pragma comment(lib, "securelib")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

template<typename T>
bool AreVecsEqual(const std::vector<T>& vec1, const std::vector<T>& vec2)
{
	if (vec1.size() != vec2.size())
		return false;
	for (size_t v = 0; v < vec1.size(); ++v)
	{
		if (vec1[v] != vec2[v])
			return false;
	}
	return true;
}

namespace securelib
{
	TEST_CLASS(LibTests)
	{
	public:
		TEST_METHOD(TestHash)
		{
			std::string hash1 = Hash("foo");
			Logger::WriteMessage(("hash1: " + hash1 + "\n").c_str());

			std::string hash2 = Hash("bar");
			Logger::WriteMessage(("hash2: " + hash2 + "\n").c_str());

			Assert::IsTrue(hash1 != hash2);

			std::string longHash1 = Hash("this is really quite a lot of text, more than any reasonable hash algorithm would get tripped up by, some");
			Logger::WriteMessage(("longHash1: " + longHash1 + "\n").c_str());

			std::string longHash2 = Hash("again, there's not reason why a completely reasonable hash algorithm would have issue with text this long");
			Logger::WriteMessage(("longHash2: " + longHash2 + "\n").c_str());

			Assert::IsTrue(longHash1 != longHash2);
		}

		TEST_METHOD(TestEnDecrypt)
		{
			{
				std::vector<unsigned char> plain{ 1, 2 ,3, 4 };
				std::string key = "test";
				auto enc = Encrypt(plain, key);
				auto dec = Decrypt(enc, key);
				Assert::IsTrue(AreVecsEqual<uint8_t>(plain, dec));
			}

			{
				std::string key = "blet monkey";
				std::string plain = "foobar";

				auto enc = Encrypt(plain, key);
				Logger::WriteMessage(("enc: " + enc + "\n").c_str());

				auto dec = Decrypt(enc, key);
				Logger::WriteMessage(("dec: " + dec + "\n").c_str());

				Assert::AreEqual(plain, dec);
			}

			{
				std::string key = "so much longer that only 56 bytes, it's hard to imagine how completely beyond 56 bytes this is, but it's really quite a ways past it";
				std::string plain = "foobar blet monkey a key that's way too long that it exceeds the block size so it gets truncated but that's still okay because it's self-consistent";

				auto enc = Encrypt(plain, key);
				Logger::WriteMessage(("enc: " + enc + "\n").c_str());

				auto dec = Decrypt(enc, key);
				Logger::WriteMessage(("dec: " + dec + "\n").c_str());

				Assert::AreEqual(plain, dec);
			}
		}
	};
}
