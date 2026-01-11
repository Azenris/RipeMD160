
// System Includes
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>

// Includes
#include "types.h"

static inline u32 f0( u32 data0, u32 data1, u32 data2 )
{
	return data0 ^ data1 ^ data2;
}

static inline u32 f1( u32 data0, u32 data1, u32 data2 )
{
	return ( data0 & data1 ) | ( ~data0 & data2 );
}

static inline u32 f2( u32 data0, u32 data1, u32 data2 )
{
	return ( data0 | ~data1 ) ^ data2;
}

static inline u32 f3( u32 data0, u32 data1, u32 data2 )
{
	return ( data0 & data2 ) | ( data1 & ~data2 );
}

static inline u32 f4( u32 data0, u32 data1, u32 data2 )
{
	return data0 ^ ( data1 | ~data2 );
}

static inline u32 rl( u32 data, i32 rotates )
{
	return ( data << rotates ) | ( data >> ( 32 - rotates ) );
}

constexpr const u32 constants[ 5 ] =
{
	0x00000000,
	0x5a827999,
	0x6ed9eba1,
	0x8f1bbcdc,
	0xa953fd4e,
};

constexpr const u32 pConstants[ 5 ] =
{
	0x50a28be6,
	0x5c4dd124,
	0x6d703ef3,
	0x7a6d76e9,
	0x00000000,
};

constexpr const u32 wordIndex[ 80 ] =
{
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 7,  4, 13,  1, 10,  6, 15,  3, 12,  0,  9,  5,  2, 14, 11,  8,
	 3, 10, 14,  4,  9, 15,  8,  1,  2,  7,  0,  6, 13, 11,  5, 12,
	 1,  9, 11, 10,  0,  8, 12,  4, 13,  3,  7, 15, 14,  5,  6,  2,
	 4,  0,  5,  9,  7, 12,  2, 10, 14,  1,  3,  8, 11,  6, 15, 13
};

constexpr const u32 pWordIndex[ 80 ] =
{
	 5, 14,  7,  0,  9,  2, 11,  4, 13,  6, 15,  8,  1, 10,  3, 12,
	 6, 11,  3,  7,  0, 13,  5, 10, 14, 15,  8, 12,  4,  9,  1,  2,
	15,  5,  1,  3,  7, 14,  6,  9, 11,  8, 12,  2, 10,  0,  4, 13,
	 8,  6,  4,  1,  3, 11, 15,  0,  5, 12,  2, 13,  9,  7, 10, 14,
	12, 15, 10,  4,  1,  5,  8,  7,  6,  2, 13, 14,  0,  3,  9, 11
};

constexpr const u32 rotLeftCount[ 80 ] =
{
	11, 14, 15, 12,  5,  8,  7,  9, 11, 13, 14, 15,  6,  7,  9,  8,
	 7,  6,  8, 13, 11,  9,  7, 15,  7, 12, 15,  9, 11,  7, 13, 12,
	11, 13,  6,  7, 14,  9, 13, 15, 14,  8, 13,  6,  5, 12,  7,  5,
	11, 12, 14, 15, 14, 15,  9,  8,  9, 14,  5,  6,  8,  6,  5, 12,
	 9, 15,  5, 11,  6,  8, 13, 12,  5, 12, 13, 14, 11,  8,  5,  6
};

constexpr const u32 pRotLeftCount[ 80 ] =
{
	 8,  9,  9, 11, 13, 15, 15,  5,  7,  7,  8, 11, 14, 14, 12,  6,
	 9, 13, 15,  7, 12,  8,  9, 11,  7,  7, 12,  7,  6, 15, 13, 11,
	 9,  7, 15, 11,  8,  6,  6, 14, 12,  13, 5, 14, 13, 13,  7,  5,
	15,  5,  8, 11, 14, 14,  6, 14,  6,  9, 12,  9, 12,  5, 15,  8,
	 8,  5, 12,  9, 12,  5, 14,  6,  8, 13,  6,  5, 15, 13, 11, 11
};

enum RESULT_CODE
{
	RESULT_CODE_SUCCESS,
	RESULT_CODE_MISSING_ARGUMENTS,
	RESULT_CODE_NOT_A_FILE,
	RESULT_CODE_FAILED_TO_OPEN_FILE,
};

constexpr const u64 RIPEMD160_HASH_BYTES = 20;

struct RipeMD160Hash
{
	char value[ RIPEMD160_HASH_BYTES ];
};

std::ostream & operator << ( std::ostream &out, const RipeMD160Hash &hash )
{
	constexpr char hex[] = "0123456789abcdef";
	for ( i32 i = 0; i < RIPEMD160_HASH_BYTES; ++i )
	{
		u8 v = hash.value[ i ];
		out << hex[ ( v >> 4 ) & 15 ] << hex[ v & 15 ];
	}
	return out;
}

RipeMD160Hash ripemd160( const char *dataIn, u64 size )
{
	// 1 byte of 0b10000000 + 8 bytes of the length, &63 = %64, which is 512 bit blocks
	u64 padding = 64 - ( ( size + 1 + 8 ) & 63 );

	std::vector<u8> data;
	data.reserve( size + 1 + 8 + padding );
	data.assign( dataIn, dataIn + size );

	// -- padding --
	data.push_back( 0b10000000 );
	data.resize( data.size() + padding );

	// -- length of message (little endian) --
	u64 bits = size * 8;
	data.push_back( ( bits >>  0 ) & 0xFF );
	data.push_back( ( bits >>  8 ) & 0xFF );
	data.push_back( ( bits >> 16 ) & 0xFF );
	data.push_back( ( bits >> 24 ) & 0xFF );
	data.push_back( ( bits >> 32 ) & 0xFF );
	data.push_back( ( bits >> 40 ) & 0xFF );
	data.push_back( ( bits >> 48 ) & 0xFF );
	data.push_back( ( bits >> 56 ) & 0xFF );

	u32 a0 = 0x67452301;
	u32 b0 = 0xefcdab89;
	u32 c0 = 0x98badcfe;
	u32 d0 = 0x10325476;
	u32 e0 = 0xc3d2e1f0;

	u32 a1 = a0;
	u32 b1 = b0;
	u32 c1 = c0;
	u32 d1 = d0;
	u32 e1 = e0;

	u8 *dataByte = data.data();
	u32 words[ 16 ];

	for ( u64 blockIdx = 0, blockCount = ( data.size() / 64 ); blockIdx < blockCount; ++blockIdx )
	{
		// -- message schedule ---
		for ( i32 w = 0; w < 16; ++w )
		{
			u32 byte0 = *dataByte++;
			u32 byte1 = *dataByte++;
			u32 byte2 = *dataByte++;
			u32 byte3 = *dataByte++;

			#if defined( LITTLE_ENDIAN )
				words[ w ] = ( byte3 << 24 ) | ( byte2 << 16 ) | ( byte1 << 8 ) | byte0;
			#else
				words[ w ] = ( byte0 << 24 ) | ( byte1 << 16 ) | ( byte2 << 8 ) | byte3;
			#endif
		}

		// -- compression --
		u32 h0 = a0;
		u32 h1 = b0;
		u32 h2 = c0;
		u32 h3 = d0;
		u32 h4 = e0;

		i32 w = 0;

		for ( ; w < 16; ++w )
		{
			u32 t = rl( a0 + f0( b0, c0, d0 ) + words[ wordIndex[ w ] ] + constants[ 0 ], rotLeftCount[ w ] ) + e0;

			a0 = e0;
			e0 = d0;
			d0 = rl( c0, 10 );
			c0 = b0;
			b0 = t;

			u32 p = rl( a1 + f4( b1, c1, d1 ) + words[ pWordIndex[ w ] ] + pConstants[ 0 ], pRotLeftCount[ w ] ) + e1;

			a1 = e1;
			e1 = d1;
			d1 = rl( c1, 10 );
			c1 = b1;
			b1 = p;
		}

		for ( ; w < 32; ++w )
		{
			u32 t = rl( a0 + f1( b0, c0, d0 ) + words[ wordIndex[ w ] ] + constants[ 1 ], rotLeftCount[ w ] ) + e0;

			a0 = e0;
			e0 = d0;
			d0 = rl( c0, 10 );
			c0 = b0;
			b0 = t;

			u32 p = rl( a1 + f3( b1, c1, d1 ) + words[ pWordIndex[ w ] ] + pConstants[ 1 ], pRotLeftCount[ w ] ) + e1;

			a1 = e1;
			e1 = d1;
			d1 = rl( c1, 10 );
			c1 = b1;
			b1 = p;
		}

		for ( ; w < 48; ++w )
		{
			u32 t = rl( a0 + f2( b0, c0, d0 ) + words[ wordIndex[ w ] ] + constants[ 2 ], rotLeftCount[ w ] ) + e0;

			a0 = e0;
			e0 = d0;
			d0 = rl( c0, 10 );
			c0 = b0;
			b0 = t;

			u32 p = rl( a1 + f2( b1, c1, d1 ) + words[ pWordIndex[ w ] ] + pConstants[ 2 ], pRotLeftCount[ w ] ) + e1;

			a1 = e1;
			e1 = d1;
			d1 = rl( c1, 10 );
			c1 = b1;
			b1 = p;
		}

		for ( ; w < 64; ++w )
		{
			u32 t = rl( a0 + f3( b0, c0, d0 ) + words[ wordIndex[ w ] ] + constants[ 3 ], rotLeftCount[ w ] ) + e0;

			a0 = e0;
			e0 = d0;
			d0 = rl( c0, 10 );
			c0 = b0;
			b0 = t;

			u32 p = rl( a1 + f1( b1, c1, d1 ) + words[ pWordIndex[ w ] ] + pConstants[ 3 ], pRotLeftCount[ w ] ) + e1;

			a1 = e1;
			e1 = d1;
			d1 = rl( c1, 10 );
			c1 = b1;
			b1 = p;
		}

		for ( ; w < 80; ++w )
		{
			u32 t = rl( a0 + f4( b0, c0, d0 ) + words[ wordIndex[ w ] ] + constants[ 4 ], rotLeftCount[ w ] ) + e0;

			a0 = e0;
			e0 = d0;
			d0 = rl( c0, 10 );
			c0 = b0;
			b0 = t;

			u32 p = rl( a1 + f0( b1, c1, d1 ) + words[ pWordIndex[ w ] ] + pConstants[ 4 ], pRotLeftCount[ w ] ) + e1;

			a1 = e1;
			e1 = d1;
			d1 = rl( c1, 10 );
			c1 = b1;
			b1 = p;
		}

		u32 t = h1 + c0 + d1;
		h1 = h2 + d0 + e1;
		h2 = h3 + e0 + a1;
		h3 = h4 + a0 + b1;
		h4 = h0 + b0 + c1;

		a0 = a1 = t;
		b0 = b1 = h1;
		c0 = c1 = h2;
		d0 = d1 = h3;
		e0 = e1 = h4;
	}

	// -- output --
	RipeMD160Hash hash;
	hash.value[  0 ] = ( a0 >>  0 ) & 0xFF;
	hash.value[  1 ] = ( a0 >>  8 ) & 0xFF;
	hash.value[  2 ] = ( a0 >> 16 ) & 0xFF;
	hash.value[  3 ] = ( a0 >> 24 ) & 0xFF;
	hash.value[  4 ] = ( b0 >>  0 ) & 0xFF;
	hash.value[  5 ] = ( b0 >>  8 ) & 0xFF;
	hash.value[  6 ] = ( b0 >> 16 ) & 0xFF;
	hash.value[  7 ] = ( b0 >> 24 ) & 0xFF;
	hash.value[  8 ] = ( c0 >>  0 ) & 0xFF;
	hash.value[  9 ] = ( c0 >>  8 ) & 0xFF;
	hash.value[ 10 ] = ( c0 >> 16 ) & 0xFF;
	hash.value[ 11 ] = ( c0 >> 24 ) & 0xFF;
	hash.value[ 12 ] = ( d0 >>  0 ) & 0xFF;
	hash.value[ 13 ] = ( d0 >>  8 ) & 0xFF;
	hash.value[ 14 ] = ( d0 >> 16 ) & 0xFF;
	hash.value[ 15 ] = ( d0 >> 24 ) & 0xFF;
	hash.value[ 16 ] = ( e0 >>  0 ) & 0xFF;
	hash.value[ 17 ] = ( e0 >>  8 ) & 0xFF;
	hash.value[ 18 ] = ( e0 >> 16 ) & 0xFF;
	hash.value[ 19 ] = ( e0 >> 24 ) & 0xFF;

	return hash;
}

int main( int argc, char *argv[] )
{
	if ( argc <= 1 )
		return RESULT_CODE_MISSING_ARGUMENTS;

	const char *filepath = argv[ 1 ];

	if ( !std::filesystem::is_regular_file( filepath ) )
		return RESULT_CODE_NOT_A_FILE;

	std::ifstream file( filepath, std::ios::binary );
	if ( !file.is_open() )
		return RESULT_CODE_FAILED_TO_OPEN_FILE;

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string fileData = buffer.str();

	RipeMD160Hash ripemd160Result = ripemd160( fileData.data(), fileData.size() );

	std::cout << ripemd160Result;

	return RESULT_CODE_SUCCESS;
}