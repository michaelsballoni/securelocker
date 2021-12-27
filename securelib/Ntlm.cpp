// https://www.codeproject.com/Articles/328761/NTLM-Hash-Generator
#include "pch.h"
#include "Ntlm.h"

//*************************************************************** 
//Init values
#define INIT_A 0x67452301
#define INIT_B 0xefcdab89
#define INIT_C 0x98badcfe
#define INIT_D 0x10325476

#define SQRT_2 0x5a827999
#define SQRT_3 0x6ed9eba1

const char itoa16[17] = "0123456789abcdef";

//*************************************************************** 
std::string securelib::ntlm(const uint8_t* data, uint32_t len)
{
	char hex_format[33];

	unsigned int nt_buffer[16];
	unsigned int output[4];

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Prepare the string for hash calculation
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	uint32_t i = 0;
	memset(nt_buffer, 0, 16 * 4);
	//The length of key need to be <= 27
	for (; i < len / 2; i++)
		nt_buffer[i] = data[2 * i] | (data[2 * i + 1] << 16);

	//padding
	if (len % 2 == 1)
		nt_buffer[i] = data[len - 1] | 0x800000;
	else
		nt_buffer[i] = 0x80;

	//put the length
	nt_buffer[14] = len << 4;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// NTLM hash calculation
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	unsigned int a = INIT_A;
	unsigned int b = INIT_B;
	unsigned int c = INIT_C;
	unsigned int d = INIT_D;

	/* Round 1 */
	a += (d ^ (b & (c ^ d))) + nt_buffer[0]; a = (a << 3) | (a >> 29);
	d += (c ^ (a & (b ^ c))) + nt_buffer[1]; d = (d << 7) | (d >> 25);
	c += (b ^ (d & (a ^ b))) + nt_buffer[2]; c = (c << 11) | (c >> 21);
	b += (a ^ (c & (d ^ a))) + nt_buffer[3]; b = (b << 19) | (b >> 13);

	a += (d ^ (b & (c ^ d))) + nt_buffer[4]; a = (a << 3) | (a >> 29);
	d += (c ^ (a & (b ^ c))) + nt_buffer[5]; d = (d << 7) | (d >> 25);
	c += (b ^ (d & (a ^ b))) + nt_buffer[6]; c = (c << 11) | (c >> 21);
	b += (a ^ (c & (d ^ a))) + nt_buffer[7]; b = (b << 19) | (b >> 13);

	a += (d ^ (b & (c ^ d))) + nt_buffer[8]; a = (a << 3) | (a >> 29);
	d += (c ^ (a & (b ^ c))) + nt_buffer[9]; d = (d << 7) | (d >> 25);
	c += (b ^ (d & (a ^ b))) + nt_buffer[10]; c = (c << 11) | (c >> 21);
	b += (a ^ (c & (d ^ a))) + nt_buffer[11]; b = (b << 19) | (b >> 13);

	a += (d ^ (b & (c ^ d))) + nt_buffer[12]; a = (a << 3) | (a >> 29);
	d += (c ^ (a & (b ^ c))) + nt_buffer[13]; d = (d << 7) | (d >> 25);
	c += (b ^ (d & (a ^ b))) + nt_buffer[14]; c = (c << 11) | (c >> 21);
	b += (a ^ (c & (d ^ a))) + nt_buffer[15]; b = (b << 19) | (b >> 13);

	/* Round 2 */
	a += ((b & (c | d)) | (c & d)) + nt_buffer[0] + SQRT_2; a = (a << 3) | (a >> 29);
	d += ((a & (b | c)) | (b & c)) + nt_buffer[4] + SQRT_2; d = (d << 5) | (d >> 27);
	c += ((d & (a | b)) | (a & b)) + nt_buffer[8] + SQRT_2; c = (c << 9) | (c >> 23);
	b += ((c & (d | a)) | (d & a)) + nt_buffer[12] + SQRT_2; b = (b << 13) | (b >> 19);

	a += ((b & (c | d)) | (c & d)) + nt_buffer[1] + SQRT_2; a = (a << 3) | (a >> 29);
	d += ((a & (b | c)) | (b & c)) + nt_buffer[5] + SQRT_2; d = (d << 5) | (d >> 27);
	c += ((d & (a | b)) | (a & b)) + nt_buffer[9] + SQRT_2; c = (c << 9) | (c >> 23);
	b += ((c & (d | a)) | (d & a)) + nt_buffer[13] + SQRT_2; b = (b << 13) | (b >> 19);

	a += ((b & (c | d)) | (c & d)) + nt_buffer[2] + SQRT_2; a = (a << 3) | (a >> 29);
	d += ((a & (b | c)) | (b & c)) + nt_buffer[6] + SQRT_2; d = (d << 5) | (d >> 27);
	c += ((d & (a | b)) | (a & b)) + nt_buffer[10] + SQRT_2; c = (c << 9) | (c >> 23);
	b += ((c & (d | a)) | (d & a)) + nt_buffer[14] + SQRT_2; b = (b << 13) | (b >> 19);

	a += ((b & (c | d)) | (c & d)) + nt_buffer[3] + SQRT_2; a = (a << 3) | (a >> 29);
	d += ((a & (b | c)) | (b & c)) + nt_buffer[7] + SQRT_2; d = (d << 5) | (d >> 27);
	c += ((d & (a | b)) | (a & b)) + nt_buffer[11] + SQRT_2; c = (c << 9) | (c >> 23);
	b += ((c & (d | a)) | (d & a)) + nt_buffer[15] + SQRT_2; b = (b << 13) | (b >> 19);

	/* Round 3 */
	a += (d ^ c ^ b) + nt_buffer[0] + SQRT_3; a = (a << 3) | (a >> 29);
	d += (c ^ b ^ a) + nt_buffer[8] + SQRT_3; d = (d << 9) | (d >> 23);
	c += (b ^ a ^ d) + nt_buffer[4] + SQRT_3; c = (c << 11) | (c >> 21);
	b += (a ^ d ^ c) + nt_buffer[12] + SQRT_3; b = (b << 15) | (b >> 17);

	a += (d ^ c ^ b) + nt_buffer[2] + SQRT_3; a = (a << 3) | (a >> 29);
	d += (c ^ b ^ a) + nt_buffer[10] + SQRT_3; d = (d << 9) | (d >> 23);
	c += (b ^ a ^ d) + nt_buffer[6] + SQRT_3; c = (c << 11) | (c >> 21);
	b += (a ^ d ^ c) + nt_buffer[14] + SQRT_3; b = (b << 15) | (b >> 17);

	a += (d ^ c ^ b) + nt_buffer[1] + SQRT_3; a = (a << 3) | (a >> 29);
	d += (c ^ b ^ a) + nt_buffer[9] + SQRT_3; d = (d << 9) | (d >> 23);
	c += (b ^ a ^ d) + nt_buffer[5] + SQRT_3; c = (c << 11) | (c >> 21);
	b += (a ^ d ^ c) + nt_buffer[13] + SQRT_3; b = (b << 15) | (b >> 17);

	a += (d ^ c ^ b) + nt_buffer[3] + SQRT_3; a = (a << 3) | (a >> 29);
	d += (c ^ b ^ a) + nt_buffer[11] + SQRT_3; d = (d << 9) | (d >> 23);
	c += (b ^ a ^ d) + nt_buffer[7] + SQRT_3; c = (c << 11) | (c >> 21);
	b += (a ^ d ^ c) + nt_buffer[15] + SQRT_3; b = (b << 15) | (b >> 17);

	output[0] = a + INIT_A;
	output[1] = b + INIT_B;
	output[2] = c + INIT_C;
	output[3] = d + INIT_D;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Convert the hash to hex (for being readable)
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	for (i = 0; i < 4; i++)
	{
		int j = 0;
		unsigned int n = output[i];
		//iterate the bytes of the integer		
		for (; j < 4; j++)
		{
			unsigned int convert = n % 256;
			hex_format[i * 8 + j * 2 + 1] = itoa16[convert % 16];
			convert = convert / 16;
			hex_format[i * 8 + j * 2 + 0] = itoa16[convert % 16];
			n = n / 256;
		}
	}

	//null terminate the string
	hex_format[32] = 0;
	return hex_format;
}
