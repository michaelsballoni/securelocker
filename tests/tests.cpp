#include "pch.h"
#include "CppUnitTest.h"

#include "securelib.h"
#pragma comment(lib, "securelib")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace securelib
{
	TEST_CLASS(LibTests)
	{
	public:
		TEST_METHOD(TestHash)
		{
			std::string hash1 = Hash(L"foo");
			Logger::WriteMessage(("hash1: " + hash1 + "\n").c_str());

			std::string hash2 = Hash(L"bar");
			Logger::WriteMessage(("hash2: " + hash2 + "\n").c_str());

			Assert::IsTrue(hash1 != hash2);

			std::string longHash1 = Hash(L"this is really quite a lot of text, more than any reasonable hash algorithm would get tripped up by, some");
			Logger::WriteMessage(("longHash1: " + longHash1 + "\n").c_str());

			std::string longHash2 = Hash(L"again, there's not reason why a completely reasonable hash algorithm would have issue with text this long");
			Logger::WriteMessage(("longHash2: " + longHash2 + "\n").c_str());

			Assert::IsTrue(longHash1 != longHash2);
		}

		TEST_METHOD(TestEnDecrypt)
		{
			{
				std::vector<unsigned char> plain{ 1, 2 ,3, 4 };
				auto enc = Encrypt(plain, L"test");
				auto dec = Decrypt(enc, L"test");
				// FORNOW - Debug from here
			}

			{
				auto enc = Encrypt(L"foobar", L"blet monkey");
				Logger::WriteMessage((L"enc: " + enc + L"\n").c_str());

				auto dec = Decrypt(enc, L"blet monkey");
				Logger::WriteMessage((L"dec: " + dec + L"\n").c_str());

				Assert::AreEqual(std::wstring(L"foobar"), dec);
			}
		}
	};
}
