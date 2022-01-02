#include "pch.h"
#include "CppUnitTest.h"

#include "securelib.h"
#pragma comment(lib, "securelib")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fs = std::filesystem;

namespace securelib
{
	TEST_CLASS(LibTests)
	{
	public:
		TEST_CLASS_INITIALIZE(InitLibTests)
		{
			setLogFile(stdout);
#ifdef _DEBUG
			setLogTrace(true);
#endif
		}		
		
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
				Assert::IsTrue(httplite::AreVecsEqual<uint8_t>(plain, dec));
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

		TEST_METHOD(TestUnique)
		{
			std::string unique1 = UniqueStr();
			Logger::WriteMessage(("unique1: " + unique1 + "\n").c_str());

			std::string unique2 = UniqueStr();
			Logger::WriteMessage(("unique2: " + unique2 + "\n").c_str());

			Assert::IsTrue(unique1 != unique2);
		}

		TEST_METHOD(TestFiles)
		{
			std::wstring filePath = fs::path(__FILE__).parent_path().append("test.dat");
			if (fs::exists(filePath))
				fs::remove(filePath);

			std::vector<uint8_t> loaded = LoadFile(filePath, false);
			Assert::IsTrue(loaded.empty());

			std::vector<uint8_t> saved = StrToVec("foobar");
			SaveFile(filePath, saved);

			saved = StrToVec("foobar");
			SaveFile(filePath, saved);

			std::string loadedStr = VecToStr(LoadFile(filePath));
			Assert::AreEqual(std::string("foobar"), loadedStr);

			std::string loadedStr2 = VecToStr(LoadFile(filePath));
			Assert::AreEqual(std::string("foobar"), loadedStr2);

			std::vector<uint8_t> saved2 = StrToVec("blet monkey");
			SaveFile(filePath, saved2);

			std::string loadedStr3 = VecToStr(LoadFile(filePath));
			Assert::AreEqual(std::string("blet monkey"), loadedStr3);

			std::string loadedStr4 = VecToStr(LoadFile(filePath));
			Assert::AreEqual(std::string("blet monkey"), loadedStr4);
		}
	};
}
