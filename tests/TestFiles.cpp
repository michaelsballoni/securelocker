#include "pch.h"
#include "CppUnitTest.h"

#include "lockerfiles.h"
#include "securelib.h"
#pragma comment(lib, "securelib")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace fs = std::filesystem;

namespace securelib
{
	TEST_CLASS(FilesTests)
	{
	public:
		TEST_CLASS_INITIALIZE(InitFilesTests)
		{
			setLogFile(stdout);
#ifdef _DEBUG
			setLogTrace(true);
#endif
		}

		TEST_METHOD(TestFiles)
		{
			std::wstring rootDirPath = fs::path(__FILE__).parent_path().append("files");
			if (fs::exists(rootDirPath))
				fs::remove_all(rootDirPath);
			fs::create_directories(rootDirPath);

			lockerfiles files(rootDirPath, 1234U);
			Assert::AreEqual(std::wstring(), files.dir());
			files.put(L"foo.bar", StrToVec("blet monkey"));
			Assert::AreEqual(std::string("blet monkey"), VecToStr(files.get(L"foo.bar")));
			Assert::AreEqual(std::wstring(L"foo.bar\n"), files.dir());
			files.del(L"foo.bar");
			Assert::AreEqual(std::wstring(), files.dir());
		}
	};
}
