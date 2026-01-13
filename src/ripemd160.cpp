
#include <iostream>
#include <filesystem>
#include <fstream>

#include "ripemd160.h"

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

constexpr const u32 RipeMD160WordIndex[ 80 ] =
{
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 7,  4, 13,  1, 10,  6, 15,  3, 12,  0,  9,  5,  2, 14, 11,  8,
	 3, 10, 14,  4,  9, 15,  8,  1,  2,  7,  0,  6, 13, 11,  5, 12,
	 1,  9, 11, 10,  0,  8, 12,  4, 13,  3,  7, 15, 14,  5,  6,  2,
	 4,  0,  5,  9,  7, 12,  2, 10, 14,  1,  3,  8, 11,  6, 15, 13
};

constexpr const u32 RipeMD160WordIndexPrime[ 80 ] =
{
	 5, 14,  7,  0,  9,  2, 11,  4, 13,  6, 15,  8,  1, 10,  3, 12,
	 6, 11,  3,  7,  0, 13,  5, 10, 14, 15,  8, 12,  4,  9,  1,  2,
	15,  5,  1,  3,  7, 14,  6,  9, 11,  8, 12,  2, 10,  0,  4, 13,
	 8,  6,  4,  1,  3, 11, 15,  0,  5, 12,  2, 13,  9,  7, 10, 14,
	12, 15, 10,  4,  1,  5,  8,  7,  6,  2, 13, 14,  0,  3,  9, 11
};

constexpr const u32 RipeMD160RotLeftCount[ 80 ] =
{
	11, 14, 15, 12,  5,  8,  7,  9, 11, 13, 14, 15,  6,  7,  9,  8,
	 7,  6,  8, 13, 11,  9,  7, 15,  7, 12, 15,  9, 11,  7, 13, 12,
	11, 13,  6,  7, 14,  9, 13, 15, 14,  8, 13,  6,  5, 12,  7,  5,
	11, 12, 14, 15, 14, 15,  9,  8,  9, 14,  5,  6,  8,  6,  5, 12,
	 9, 15,  5, 11,  6,  8, 13, 12,  5, 12, 13, 14, 11,  8,  5,  6
};

constexpr const u32 RipeMD160RotLeftCountPrime[ 80 ] =
{
	 8,  9,  9, 11, 13, 15, 15,  5,  7,  7,  8, 11, 14, 14, 12,  6,
	 9, 13, 15,  7, 12,  8,  9, 11,  7,  7, 12,  7,  6, 15, 13, 11,
	 9,  7, 15, 11,  8,  6,  6, 14, 12,  13, 5, 14, 13, 13,  7,  5,
	15,  5,  8, 11, 14, 14,  6, 14,  6,  9, 12,  9, 12,  5, 15,  8,
	 8,  5, 12,  9, 12,  5, 14,  6,  8, 13,  6,  5, 15, 13, 11, 11
};

struct RipeMD160Context
{
	const u8 *data;
	u32 a;
	u32 b;
	u32 c;
	u32 d;
	u32 e;
};

static RipeMD160Hash ripemd160_create_hash( RipeMD160Context *context )
{
	RipeMD160Hash hash;
	hash.value[  0 ] = ( context->a >>  0 ) & 0xFF;
	hash.value[  1 ] = ( context->a >>  8 ) & 0xFF;
	hash.value[  2 ] = ( context->a >> 16 ) & 0xFF;
	hash.value[  3 ] = ( context->a >> 24 ) & 0xFF;
	hash.value[  4 ] = ( context->b >>  0 ) & 0xFF;
	hash.value[  5 ] = ( context->b >>  8 ) & 0xFF;
	hash.value[  6 ] = ( context->b >> 16 ) & 0xFF;
	hash.value[  7 ] = ( context->b >> 24 ) & 0xFF;
	hash.value[  8 ] = ( context->c >>  0 ) & 0xFF;
	hash.value[  9 ] = ( context->c >>  8 ) & 0xFF;
	hash.value[ 10 ] = ( context->c >> 16 ) & 0xFF;
	hash.value[ 11 ] = ( context->c >> 24 ) & 0xFF;
	hash.value[ 12 ] = ( context->d >>  0 ) & 0xFF;
	hash.value[ 13 ] = ( context->d >>  8 ) & 0xFF;
	hash.value[ 14 ] = ( context->d >> 16 ) & 0xFF;
	hash.value[ 15 ] = ( context->d >> 24 ) & 0xFF;
	hash.value[ 16 ] = ( context->e >>  0 ) & 0xFF;
	hash.value[ 17 ] = ( context->e >>  8 ) & 0xFF;
	hash.value[ 18 ] = ( context->e >> 16 ) & 0xFF;
	hash.value[ 19 ] = ( context->e >> 24 ) & 0xFF;
	return hash;
}

static void ripemd160_process_block( RipeMD160Context *context )
{
	u32 words[ 16 ];

	const u8 *data = context->data;

	// -- message schedule ---
	for ( i32 w = 0; w < 16; ++w )
	{
		u32 d0 = *data++;
		u32 d1 = *data++;
		u32 d2 = *data++;
		u32 d3 = *data++;

		words[ w ] = ( d3 << 24 ) | ( d2 << 16 ) | ( d1 << 8 ) | d0;
	}

	// -- compression --
	u32 a0 = context->a;
	u32 b0 = context->b;
	u32 c0 = context->c;
	u32 d0 = context->d;
	u32 e0 = context->e;
	u32 a1 = context->a;
	u32 b1 = context->b;
	u32 c1 = context->c;
	u32 d1 = context->d;
	u32 e1 = context->e;

	i32 w = 0;

	for ( ; w < 16; ++w )
	{
		u32 t = rl( a0 + f0( b0, c0, d0 ) + words[ RipeMD160WordIndex[ w ] ] + 0x00000000, RipeMD160RotLeftCount[ w ] ) + e0;

		a0 = e0;
		e0 = d0;
		d0 = rl( c0, 10 );
		c0 = b0;
		b0 = t;

		u32 p = rl( a1 + f4( b1, c1, d1 ) + words[ RipeMD160WordIndexPrime[ w ] ] + 0x50a28be6, RipeMD160RotLeftCountPrime[ w ] ) + e1;

		a1 = e1;
		e1 = d1;
		d1 = rl( c1, 10 );
		c1 = b1;
		b1 = p;
	}

	for ( ; w < 32; ++w )
	{
		u32 t = rl( a0 + f1( b0, c0, d0 ) + words[ RipeMD160WordIndex[ w ] ] + 0x5a827999, RipeMD160RotLeftCount[ w ] ) + e0;

		a0 = e0;
		e0 = d0;
		d0 = rl( c0, 10 );
		c0 = b0;
		b0 = t;

		u32 p = rl( a1 + f3( b1, c1, d1 ) + words[ RipeMD160WordIndexPrime[ w ] ] + 0x5c4dd124, RipeMD160RotLeftCountPrime[ w ] ) + e1;

		a1 = e1;
		e1 = d1;
		d1 = rl( c1, 10 );
		c1 = b1;
		b1 = p;
	}

	for ( ; w < 48; ++w )
	{
		u32 t = rl( a0 + f2( b0, c0, d0 ) + words[ RipeMD160WordIndex[ w ] ] + 0x6ed9eba1, RipeMD160RotLeftCount[ w ] ) + e0;

		a0 = e0;
		e0 = d0;
		d0 = rl( c0, 10 );
		c0 = b0;
		b0 = t;

		u32 p = rl( a1 + f2( b1, c1, d1 ) + words[ RipeMD160WordIndexPrime[ w ] ] + 0x6d703ef3, RipeMD160RotLeftCountPrime[ w ] ) + e1;

		a1 = e1;
		e1 = d1;
		d1 = rl( c1, 10 );
		c1 = b1;
		b1 = p;
	}

	for ( ; w < 64; ++w )
	{
		u32 t = rl( a0 + f3( b0, c0, d0 ) + words[ RipeMD160WordIndex[ w ] ] + 0x8f1bbcdc, RipeMD160RotLeftCount[ w ] ) + e0;

		a0 = e0;
		e0 = d0;
		d0 = rl( c0, 10 );
		c0 = b0;
		b0 = t;

		u32 p = rl( a1 + f1( b1, c1, d1 ) + words[ RipeMD160WordIndexPrime[ w ] ] + 0x7a6d76e9, RipeMD160RotLeftCountPrime[ w ] ) + e1;

		a1 = e1;
		e1 = d1;
		d1 = rl( c1, 10 );
		c1 = b1;
		b1 = p;
	}

	for ( ; w < 80; ++w )
	{
		u32 t = rl( a0 + f4( b0, c0, d0 ) + words[ RipeMD160WordIndex[ w ] ] + 0xa953fd4e, RipeMD160RotLeftCount[ w ] ) + e0;

		a0 = e0;
		e0 = d0;
		d0 = rl( c0, 10 );
		c0 = b0;
		b0 = t;

		u32 p = rl( a1 + f0( b1, c1, d1 ) + words[ RipeMD160WordIndexPrime[ w ] ] + 0x00000000, RipeMD160RotLeftCountPrime[ w ] ) + e1;

		a1 = e1;
		e1 = d1;
		d1 = rl( c1, 10 );
		c1 = b1;
		b1 = p;
	}

	u32 t = context->b + c0 + d1;
	context->b = context->c + d0 + e1;
	context->c = context->d + e0 + a1;
	context->d = context->e + a0 + b1;
	context->e = context->a + b0 + c1;
	context->a = t;
}

RipeMD160Hash ripemd160( const u8 *data, u64 size )
{
	RipeMD160Context context =
	{
		.a = 0x67452301,
		.b = 0xefcdab89,
		.c = 0x98badcfe,
		.d = 0x10325476,
		.e = 0xc3d2e1f0,
	};

	// -- full blocks --
	for ( u64 blockIdx = 0, blockCount = size / 64; blockIdx < blockCount; ++blockIdx )
	{
		context.data = data;
		ripemd160_process_block( &context );
		data += 64;
	}

	// -- final blocks --
	u8 finalBlocks[ 64 * 2 ];
	u64 finalBlockSize = size & 63;
	memcpy( finalBlocks, data, finalBlockSize );

	finalBlocks[ finalBlockSize++ ] = 0b10000000;

	// 1 byte of 0b10000000 + 8 bytes of the length, &63 = %64, which is 512 bit blocks
	u64 padding = 64 - ( ( size + 1 + 8 ) & 63 );
	memset( &finalBlocks[ finalBlockSize ], 0, padding );
	finalBlockSize += padding;

	// -- length of message (little endian) --
	u64 bits = size * 8;
	finalBlocks[ finalBlockSize++ ] = ( bits >>  0 ) & 0xFF;
	finalBlocks[ finalBlockSize++ ] = ( bits >>  8 ) & 0xFF;
	finalBlocks[ finalBlockSize++ ] = ( bits >> 16 ) & 0xFF;
	finalBlocks[ finalBlockSize++ ] = ( bits >> 24 ) & 0xFF;
	finalBlocks[ finalBlockSize++ ] = ( bits >> 32 ) & 0xFF;
	finalBlocks[ finalBlockSize++ ] = ( bits >> 40 ) & 0xFF;
	finalBlocks[ finalBlockSize++ ] = ( bits >> 48 ) & 0xFF;
	finalBlocks[ finalBlockSize++ ] = ( bits >> 56 ) & 0xFF;

	context.data = finalBlocks;
	ripemd160_process_block( &context );

	if ( finalBlockSize > 64 )
	{
		context.data = finalBlocks + 64;
		ripemd160_process_block( &context );
	}

	// -- output --
	return ripemd160_create_hash( &context );
}

RipeMD160Hash ripemd160( const char *filepath, i32 *errorCode )
{
	if ( errorCode )
		*errorCode = 0;

	if ( !std::filesystem::is_regular_file( filepath ) )
	{
		if ( errorCode )
			*errorCode = 2;
		return {};
	}

	std::ifstream file( filepath, std::ios::binary );
	if ( !file )
	{
		if ( errorCode )
			*errorCode = 3;
		return {};
	}

	u8 buffer[ 64 * 2 ];
	u64 totalBytes = 0;

	RipeMD160Context context =
	{
		.data = buffer,
		.a = 0x67452301,
		.b = 0xefcdab89,
		.c = 0x98badcfe,
		.d = 0x10325476,
		.e = 0xc3d2e1f0,
	};

	while ( file.read( (char *)buffer, 64 ) || file.gcount() >= 0 )
	{
		std::streamsize bytesRead = file.gcount();

		if ( bytesRead == 64 )
		{
			// -- full blocks --
			ripemd160_process_block( &context );
			totalBytes += bytesRead;
		}
		else
		{
			// -- final blocks --
			totalBytes += bytesRead;

			buffer[ bytesRead++ ] = 0b10000000;

			// 1 byte of 0b10000000 + 8 bytes of the length, &63 = %64, which is 512 bit blocks
			u64 padding = 64 - ( ( totalBytes + 1 + 8 ) & 63 );
			memset( &buffer[ bytesRead ], 0, padding );
			bytesRead += padding;

			// -- length of message (little endian) --
			u64 bits = totalBytes * 8;
			buffer[ bytesRead++ ] = ( bits >>  0 ) & 0xFF;
			buffer[ bytesRead++ ] = ( bits >>  8 ) & 0xFF;
			buffer[ bytesRead++ ] = ( bits >> 16 ) & 0xFF;
			buffer[ bytesRead++ ] = ( bits >> 24 ) & 0xFF;
			buffer[ bytesRead++ ] = ( bits >> 32 ) & 0xFF;
			buffer[ bytesRead++ ] = ( bits >> 40 ) & 0xFF;
			buffer[ bytesRead++ ] = ( bits >> 48 ) & 0xFF;
			buffer[ bytesRead++ ] = ( bits >> 56 ) & 0xFF;

			ripemd160_process_block( &context );

			if ( bytesRead > 64 )
			{
				context.data = buffer + 64;
				ripemd160_process_block( &context );
			}

			break;
		}
	}

	// -- output --
	return ripemd160_create_hash( &context );
}

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