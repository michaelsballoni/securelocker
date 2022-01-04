#include "pch.h"
#include "CppUnitTest.h"

#include "securelib.h"
#include "lockerfiles.h"
#include "lockerserver.h"
#include "lockerclient.h"
using namespace securelib;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fs = std::filesystem;

namespace securelib
{
	TEST_CLASS(SystemTests)
	{
	public:
		TEST_CLASS_INITIALIZE(InitSystemTests)
		{
			setLogFile(stdout);
#ifdef _DEBUG
			setLogTrace(true);
#endif
		}

		TEST_METHOD(TestSystem)
		{
			std::wstring rootDirPath = 
				fs::path(__FILE__).parent_path().append("SystemTest");
			if (fs::exists(rootDirPath))
				fs::remove_all(rootDirPath);
			fs::create_directories(rootDirPath);

			std::wstring legerFilePath = fs::path(rootDirPath).append("leger.dat");

			std::wstring testFilename = L"test.dat";
			std::wstring testFilePath = 
				fs::path(__FILE__).parent_path().append(testFilename);
			if (fs::exists(testFilePath))
				fs::remove(testFilePath);
			std::string testFileText = "foo foo bar blet monkey";
			SaveFile(testFilePath, StrToVec(testFileText));
			std::string fileText = VecToStr(LoadFile(testFilePath));
			Assert::AreEqual(testFileText, fileText);

			lockerleger leger(L"foobar", legerFilePath);
			leger.load();

			lockerserver server
			(
				static_cast<uint16_t>(9914),
				leger,
				rootDirPath
			);
			server.start();

			leger.registerClient(L"Blet Monkey");
			uint32_t room = 0;
			std::string key;
			leger.checkin(L"Blet Monkey", room, key);

			lockerclient client("127.0.0.1", static_cast <uint16_t>(9914), room, key);

			client.put(testFilename, LoadFile(testFilePath));

			{
				auto dirResults = client.dir();
				Assert::AreEqual(size_t(1), dirResults.size());
				Assert::AreEqual(testFilename, dirResults[0]);
			}

			fs::remove(testFilePath);

			{
				std::string gottenText = VecToStr(client.get(testFilename));
				Assert::AreEqual(testFileText, gottenText);
				SaveFile(testFilePath, StrToVec(gottenText));
			}

			{
				auto dirResults = client.dir();
				Assert::AreEqual(size_t(0), dirResults.size());
			}

			client.put(testFilename, LoadFile(testFilePath));
			fs::remove(testFilePath);

			{
				auto dirResults = client.dir();
				Assert::AreEqual(size_t(1), dirResults.size());
				Assert::AreEqual(testFilename, dirResults[0]);
			}

			client.del(testFilename);

			{
				auto dirResults = client.dir();
				Assert::AreEqual(size_t(0), dirResults.size());
			}

			leger.checkout(L"Blet Monkey", room);
			lockerfiles files(rootDirPath, room);
			files.checkout();

			server.stop();
		}
	};
}
