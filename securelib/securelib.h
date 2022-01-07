#include <stdio.h>

#include <string>
#include <vector>

namespace securelib
{
	/// <summary>
	/// Set the FILE where output logging should go to
	/// </summary>
	void setLogFile(FILE* file);
	/// <summary>
	/// Set whether trace-level logging should be enabled
	/// Off by default
	/// </summary>
	void setLogTrace(bool trace);

	/// <summary>
	/// Log a string to the output file
	/// </summary>
	void log(const std::string& msg);
	/// <summary>
	/// Log a wstring to the output file
	/// </summary>
	void log(const std::wstring& msg);

	/// <summary>
	/// Trace a string to the output file
	/// </summary>
	void trace(const std::string& msg);
	/// <summary>
	/// Trace a wstring to the output file
	/// </summary>
	void trace(const std::wstring& msg);

	/// <summary>
	/// Take data and return the SHA-256 hash of it
	/// </summary>
	std::string Hash(const uint8_t* data, size_t len);
	/// <summary>
	/// Take a string and return the SHA-256 hash of it
	/// </summary>
	std::string Hash(const std::string& str);

	/// <summary>
	/// Take data and a key, and return Blowfish encrypted output
	/// </summary>
	std::vector<uint8_t> Encrypt(std::vector<uint8_t> data, const std::string& key);
	/// <summary>
	/// Take data and a key, and return Blowfish decrypted output
	/// </summary>
	std::vector<uint8_t> Decrypt(std::vector<uint8_t> data, const std::string& key);

	/// <summary>
	/// Take string and a key, and return Blowfish encrypted output
	/// </summary>
	std::string Encrypt(const std::string& str, const std::string& key);
	/// <summary>
	/// Take string and a key, and return Blowfish decrypted output
	/// </summary>
	std::string Decrypt(const std::string& str, const std::string& key);

	/// <summary>
	/// Turn a string into a data vector
	/// </summary>
	std::vector<unsigned char> StrToVec(const std::string& str);
	/// <summary>
	/// Turn a data vector into a string
	/// </summary>
	std::string VecToStr(const std::vector<unsigned char>& data);

	/// <summary>
	/// Generate a unique string, the hash of various unique components
	/// </summary>
	/// <returns></returns>
	std::string UniqueStr();

	/// <summary>
	/// Load file contents into a data vector
	/// </summary>
	std::vector<uint8_t> LoadFile(const std::wstring& path, bool requiresExists = true);
	/// <summary>
	/// Save a data vector into a file
	/// </summary>
	void SaveFile(const std::wstring& path, const std::vector<uint8_t>& bytes);

	/// <summary>
	/// Is a filename legal across modern file systems?
	/// </summary>
	bool IsFilenameValid(const std::wstring& filename);
}
