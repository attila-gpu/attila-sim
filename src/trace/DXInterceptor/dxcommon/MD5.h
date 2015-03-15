////////////////////////////////////////////////////////////////////////////////
//
// C++/object oriented translation and modification of MD5.
//
// Translation and modification (c) 1995 by Mordechai T. Abzug 
//
// This translation/modification is provided "as is," without express or implied
// warranty of any kind.
//
// The translator/modifier does not claim (1) that MD5 will do what you think it
// does; (2) that this translation/ modification is accurate; or (3) that this
// software is "merchantible." (Language for this disclaimer partially copied
// from the disclaimer below).
//
// Based on:
//
// MD5.H - header file for MD5C.C
// MDDRIVER.C - test driver for MD2, MD4 and MD5
//
// Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991.
// All rights reserved.
//
// License to copy and use this software is granted provided that it is
// identified as the "RSA Data Security, Inc. MD5 Message-Digest Algorithm" in
// all material mentioning or referencing this software or this function.
//
// License is also granted to make and use derivative works provided that such
// works are identified as "derived from the RSA Data Security, Inc. MD5
// Message-Digest Algorithm" in all material mentioning or referencing the
// derived work.
//
// RSA Data Security, Inc. makes no representations concerning either the
// merchantability of this software or the suitability of this software for any
// particular purpose. It is provided "as is" without express or implied
// warranty of any kind.
//
// These notices must be retained in any copies of any part of this
// documentation and/or software.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class MD5
{
public:
  
  typedef struct
  {
    unsigned char value[16];
  } md5_t;
  
  static void Calculate(const unsigned char* buffer, unsigned int size, unsigned char* hash);

  static void Init();
  static void Update(const unsigned char* input, unsigned int input_length);
  static void Finalize();
  static void GetMD5(unsigned char* hash);

  static bool IsEmpty(const unsigned char* hash);
  static std::string ToString(const unsigned char* hash);

protected:

  typedef unsigned char  uint1; // BYTE
  typedef unsigned short uint2; // WORD
  typedef unsigned int   uint4; // DWORD
  
  static uint4 ms_count[2];
  static uint4 ms_state[4];
  static uint1 ms_buffer[64];
  static uint1 ms_digest[16];
  static uint1 ms_read_buffer[1024];

  static void transform(const uint1* buffer);
  static void encode(uint1* dest, const uint4* src, uint4 length);
  static void decode(uint4* dest, const uint1* src, uint4 length);
  static void memcpy(uint1* dest, const uint1* src, uint4 length);
  static void memset(uint1* dest,       uint1  val, uint4 length);

  static inline uint4 rotate_left(uint4 x, uint4 n);
  static inline uint4 F(uint4 x, uint4 y, uint4 z);
  static inline uint4 G(uint4 x, uint4 y, uint4 z);
  static inline uint4 H(uint4 x, uint4 y, uint4 z);
  static inline uint4 I(uint4 x, uint4 y, uint4 z);
  static inline void FF(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
  static inline void GG(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
  static inline void HH(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
  static inline void II(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);

};

bool operator < (const MD5::md5_t& left, const MD5::md5_t& right);

////////////////////////////////////////////////////////////////////////////////
