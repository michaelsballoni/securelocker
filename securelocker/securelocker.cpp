#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING // toWideString / toNarrowString

#include "Core.h"
#pragma comment(lib, "httplite")
using namespace httplite;

#include "securelib.h"
#include "lockerclient.h"
#pragma comment(lib, "securelib")
using namespace securelib;

#include <conio.h> // _getch()

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int wmain(int argc, wchar_t* argv[])
{
#ifndef _DEBUG
	try
#endif
	{
		// Validate command line usage
		if (argc < 4)
		{
			printf("Usage: <server> <port> <room #>\n");
			return 0;
		}

		std::string server = toNarrowStr(argv[1]);

		int port = _wtoi(argv[2]);
		if (port <= 0 || port > USHRT_MAX) {
			printf("ERROR: Invalid port number: %S\n", argv[2]);
			return 1;
		}

		int room = _wtoi(argv[3]);
		if (room <= 0) {
			printf("ERROR: Invalid room number: %S\n", argv[3]);
			return 1;
		}

		// Get the password from the client
		// The password is used for encrypting files before sending (PUT) to server
		// and decrypting files after downloaded (GET) from the server
		// And that's it
		std::string key;
		while (true)
		{
			printf("Room Access Key: ");
			std::getline(std::cin, key);
			if (!key.empty())
				break;
		}

		setLogFile(stdout);
#ifdef _DEBUG
		setLogTrace(true);
#endif

		SocketUse useSockets;

		lockerclient client(server, port, static_cast<uint32_t>(room), key);
		while (true)
		{
			printf("\nput <path> | dir | get <path> | delete <path>\n");
			printf("> ");
			std::wstring line;
			std::getline(std::wcin, line);
			if (line.empty())
				continue;
			if (line == L"quit" || line == L"exit")
				break;

			if (line == L"dir" || line == L"DIR")
			{
				printf("Getting file listing...");
				std::vector<std::wstring> dirResult = client.dir();
				printf("done!\n");
				if (dirResult.empty())
					printf("The locker contains no files.\n");
				else
					printf("Files in the locker:\n%S", (Join(dirResult, L"\n") + L"\n").c_str());
			}
			else
			{
				int space = line.find(' ');
				if (space == std::wstring::npos)
				{
					printf("Provide a file path after the command\n");
					continue;
				}

				std::string verb = toNarrowStr(line.substr(0, space));
				for (size_t s = 0; s < verb.size(); ++s)
					verb[s] = toupper(verb[s]);

				std::wstring originalFilePath = line.substr(space + 1);
				auto originalPath = fs::path(originalFilePath);
				std::wstring parentPath =
					originalPath.has_parent_path()
					? originalPath.parent_path()
					: "";
				if (!parentPath.empty())
					parentPath += fs::path::preferred_separator;
				std::wstring filename = originalFilePath.substr(parentPath.length());

				if (verb == "PUT")
				{
					printf("Putting file in locker...");
					client.put(filename, LoadFile(originalFilePath));
					printf("done!\n");
				}
				else if (verb == "GET")
				{
					printf("Getting file...");
					SaveFile(originalPath, client.get(filename));
					printf("done!\n");
					printf("\nFile is now removed from the locker and stored locally.\n");
				}
				else if (verb == "DEL")
				{
					printf("Deleting file from locker...");
					client.del(filename);
					printf("done!\n");
				}
				else
				{
					printf("Invalid command\n");
				}
			} // dir / else
		} // command loop
	}
#ifndef _DEBUG
	catch (const std::exception& exp)
	{
		printf("\ERROR: %s\n", exp.what());
		return 1;
	}
#endif
	return 0;
}
