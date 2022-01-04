#include "pch.h"
#include "CppUnitTest.h"

#include "securelib.h"
#include "lockerleger.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fs = std::filesystem;

namespace securelib
{
	TEST_CLASS(LegerTests)
	{
	public:
		TEST_CLASS_INITIALIZE(InitLegerTests)
		{
			setLogFile(stdout);
#ifdef _DEBUG
			setLogTrace(true);
#endif
		}

		TEST_METHOD(TestLeger)
		{
			std::wstring filePath = fs::path(__FILE__).parent_path().append("leger.dat");
			if (fs::exists(filePath))
				fs::remove(filePath);

			{
				lockerleger leger(L"foobar", filePath);
				leger.load();
				leger.registerClient(L"Blet Monkey");
			}

			{
				lockerleger leger(L"foobar", filePath);
				leger.load();
				Assert::AreEqual(size_t(1), leger.entries().size());
				Assert::AreEqual(std::wstring(L"Blet Monkey"), leger.entries()[0]->name);
				Assert::AreEqual(uint32_t(0), leger.entries()[0]->room);
				Assert::AreEqual(std::string("_"), leger.entries()[0]->key);
			}

			uint32_t room = 0;
			std::string key;
			{
				lockerleger leger(L"foobar", filePath);
				leger.load();
				leger.checkin(L"Blet Monkey", room, key);
				Assert::IsTrue(room != 0);
				Assert::IsTrue(!key.empty() && key != "_");
			}

			{
				lockerleger leger(L"foobar", filePath);
				leger.load();
				Assert::AreEqual(size_t(1), leger.entries().size());
				Assert::AreEqual(std::wstring(L"Blet Monkey"), leger.entries()[0]->name);
				Assert::AreEqual(room, leger.entries()[0]->room);
				Assert::AreEqual(key, leger.entries()[0]->key);
			}

			{
				lockerleger leger(L"foobar", filePath);
				leger.load();
				uint32_t roomOut = 0;
				leger.checkout(L"Blet Monkey", roomOut);
				Assert::AreEqual(size_t(0), leger.entries().size());
			}

			{
				lockerleger leger(L"foobar", filePath);
				leger.load();
				Assert::AreEqual(size_t(0), leger.entries().size());
			}
		}
	};
}
