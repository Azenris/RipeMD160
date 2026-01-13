
#pragma once

#include <iosfwd>

#include "types.h"

constexpr const u64 RIPEMD160_HASH_BYTES = 20;

struct RipeMD160Hash
{
	char value[ RIPEMD160_HASH_BYTES ];
};

RipeMD160Hash ripemd160( const u8 *data, u64 size );

inline RipeMD160Hash ripemd160( const char *data, u64 size )
{
	return ripemd160( reinterpret_cast<const u8*>( data ), size );
}

RipeMD160Hash ripemd160( const char *filepath, i32 *errorCode = nullptr );

std::ostream & operator << ( std::ostream &out, const RipeMD160Hash &hash );