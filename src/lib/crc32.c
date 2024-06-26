//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/crc32.c                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      width   : 32                                                        //
//      endian  : little                                                    //
//      poly    : 0xEDB88320u                                               //
//      xor-in  : 0xFFFFFFFFu                                               //
//      xor-out : 0xFFFFFFFFu                                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t

#include "../bits.h"

#include "crc32.h"

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/
HIDDEN const u32 crc32_table[] = {
(u32) 0x00000000u, (u32) 0x77073096u, (u32) 0xEE0E612Cu, (u32) 0x990951BAu,
(u32) 0x076DC419u, (u32) 0x706AF48Fu, (u32) 0xE963A535u, (u32) 0x9E6495A3u,
(u32) 0x0EDB8832u, (u32) 0x79DCB8A4u, (u32) 0xE0D5E91Eu, (u32) 0x97D2D988u,
(u32) 0x09B64C2Bu, (u32) 0x7EB17CBDu, (u32) 0xE7B82D07u, (u32) 0x90BF1D91u,
(u32) 0x1DB71064u, (u32) 0x6AB020F2u, (u32) 0xF3B97148u, (u32) 0x84BE41DEu,
(u32) 0x1ADAD47Du, (u32) 0x6DDDE4EBu, (u32) 0xF4D4B551u, (u32) 0x83D385C7u,
(u32) 0x136C9856u, (u32) 0x646BA8C0u, (u32) 0xFD62F97Au, (u32) 0x8A65C9ECu,
(u32) 0x14015C4Fu, (u32) 0x63066CD9u, (u32) 0xFA0F3D63u, (u32) 0x8D080DF5u,
(u32) 0x3B6E20C8u, (u32) 0x4C69105Eu, (u32) 0xD56041E4u, (u32) 0xA2677172u,
(u32) 0x3C03E4D1u, (u32) 0x4B04D447u, (u32) 0xD20D85FDu, (u32) 0xA50AB56Bu,
(u32) 0x35B5A8FAu, (u32) 0x42B2986Cu, (u32) 0xDBBBC9D6u, (u32) 0xACBCF940u,
(u32) 0x32D86CE3u, (u32) 0x45DF5C75u, (u32) 0xDCD60DCFu, (u32) 0xABD13D59u,
(u32) 0x26D930ACu, (u32) 0x51DE003Au, (u32) 0xC8D75180u, (u32) 0xBFD06116u,
(u32) 0x21B4F4B5u, (u32) 0x56B3C423u, (u32) 0xCFBA9599u, (u32) 0xB8BDA50Fu,
(u32) 0x2802B89Eu, (u32) 0x5F058808u, (u32) 0xC60CD9B2u, (u32) 0xB10BE924u,
(u32) 0x2F6F7C87u, (u32) 0x58684C11u, (u32) 0xC1611DABu, (u32) 0xB6662D3Du,
(u32) 0x76DC4190u, (u32) 0x01DB7106u, (u32) 0x98D220BCu, (u32) 0xEFD5102Au,
(u32) 0x71B18589u, (u32) 0x06B6B51Fu, (u32) 0x9FBFE4A5u, (u32) 0xE8B8D433u,
(u32) 0x7807C9A2u, (u32) 0x0F00F934u, (u32) 0x9609A88Eu, (u32) 0xE10E9818u,
(u32) 0x7F6A0DBBu, (u32) 0x086D3D2Du, (u32) 0x91646C97u, (u32) 0xE6635C01u,
(u32) 0x6B6B51F4u, (u32) 0x1C6C6162u, (u32) 0x856530D8u, (u32) 0xF262004Eu,
(u32) 0x6C0695EDu, (u32) 0x1B01A57Bu, (u32) 0x8208F4C1u, (u32) 0xF50FC457u,
(u32) 0x65B0D9C6u, (u32) 0x12B7E950u, (u32) 0x8BBEB8EAu, (u32) 0xFCB9887Cu,
(u32) 0x62DD1DDFu, (u32) 0x15DA2D49u, (u32) 0x8CD37CF3u, (u32) 0xFBD44C65u,
(u32) 0x4DB26158u, (u32) 0x3AB551CEu, (u32) 0xA3BC0074u, (u32) 0xD4BB30E2u,
(u32) 0x4ADFA541u, (u32) 0x3DD895D7u, (u32) 0xA4D1C46Du, (u32) 0xD3D6F4FBu,
(u32) 0x4369E96Au, (u32) 0x346ED9FCu, (u32) 0xAD678846u, (u32) 0xDA60B8D0u,
(u32) 0x44042D73u, (u32) 0x33031DE5u, (u32) 0xAA0A4C5Fu, (u32) 0xDD0D7CC9u,
(u32) 0x5005713Cu, (u32) 0x270241AAu, (u32) 0xBE0B1010u, (u32) 0xC90C2086u,
(u32) 0x5768B525u, (u32) 0x206F85B3u, (u32) 0xB966D409u, (u32) 0xCE61E49Fu,
(u32) 0x5EDEF90Eu, (u32) 0x29D9C998u, (u32) 0xB0D09822u, (u32) 0xC7D7A8B4u,
(u32) 0x59B33D17u, (u32) 0x2EB40D81u, (u32) 0xB7BD5C3Bu, (u32) 0xC0BA6CADu,
(u32) 0xEDB88320u, (u32) 0x9ABFB3B6u, (u32) 0x03B6E20Cu, (u32) 0x74B1D29Au,
(u32) 0xEAD54739u, (u32) 0x9DD277AFu, (u32) 0x04DB2615u, (u32) 0x73DC1683u,
(u32) 0xE3630B12u, (u32) 0x94643B84u, (u32) 0x0D6D6A3Eu, (u32) 0x7A6A5AA8u,
(u32) 0xE40ECF0Bu, (u32) 0x9309FF9Du, (u32) 0x0A00AE27u, (u32) 0x7D079EB1u,
(u32) 0xF00F9344u, (u32) 0x8708A3D2u, (u32) 0x1E01F268u, (u32) 0x6906C2FEu,
(u32) 0xF762575Du, (u32) 0x806567CBu, (u32) 0x196C3671u, (u32) 0x6E6B06E7u,
(u32) 0xFED41B76u, (u32) 0x89D32BE0u, (u32) 0x10DA7A5Au, (u32) 0x67DD4ACCu,
(u32) 0xF9B9DF6Fu, (u32) 0x8EBEEFF9u, (u32) 0x17B7BE43u, (u32) 0x60B08ED5u,
(u32) 0xD6D6A3E8u, (u32) 0xA1D1937Eu, (u32) 0x38D8C2C4u, (u32) 0x4FDFF252u,
(u32) 0xD1BB67F1u, (u32) 0xA6BC5767u, (u32) 0x3FB506DDu, (u32) 0x48B2364Bu,
(u32) 0xD80D2BDAu, (u32) 0xAF0A1B4Cu, (u32) 0x36034AF6u, (u32) 0x41047A60u,
(u32) 0xDF60EFC3u, (u32) 0xA867DF55u, (u32) 0x316E8EEFu, (u32) 0x4669BE79u,
(u32) 0xCB61B38Cu, (u32) 0xBC66831Au, (u32) 0x256FD2A0u, (u32) 0x5268E236u,
(u32) 0xCC0C7795u, (u32) 0xBB0B4703u, (u32) 0x220216B9u, (u32) 0x5505262Fu,
(u32) 0xC5BA3BBEu, (u32) 0xB2BD0B28u, (u32) 0x2BB45A92u, (u32) 0x5CB36A04u,
(u32) 0xC2D7FFA7u, (u32) 0xB5D0CF31u, (u32) 0x2CD99E8Bu, (u32) 0x5BDEAE1Du,
(u32) 0x9B64C2B0u, (u32) 0xEC63F226u, (u32) 0x756AA39Cu, (u32) 0x026D930Au,
(u32) 0x9C0906A9u, (u32) 0xEB0E363Fu, (u32) 0x72076785u, (u32) 0x05005713u,
(u32) 0x95BF4A82u, (u32) 0xE2B87A14u, (u32) 0x7BB12BAEu, (u32) 0x0CB61B38u,
(u32) 0x92D28E9Bu, (u32) 0xE5D5BE0Du, (u32) 0x7CDCEFB7u, (u32) 0x0BDBDF21u,
(u32) 0x86D3D2D4u, (u32) 0xF1D4E242u, (u32) 0x68DDB3F8u, (u32) 0x1FDA836Eu,
(u32) 0x81BE16CDu, (u32) 0xF6B9265Bu, (u32) 0x6FB077E1u, (u32) 0x18B74777u,
(u32) 0x88085AE6u, (u32) 0xFF0F6A70u, (u32) 0x66063BCAu, (u32) 0x11010B5Cu,
(u32) 0x8F659EFFu, (u32) 0xF862AE69u, (u32) 0x616BFFD3u, (u32) 0x166CCF45u,
(u32) 0xA00AE278u, (u32) 0xD70DD2EEu, (u32) 0x4E048354u, (u32) 0x3903B3C2u,
(u32) 0xA7672661u, (u32) 0xD06016F7u, (u32) 0x4969474Du, (u32) 0x3E6E77DBu,
(u32) 0xAED16A4Au, (u32) 0xD9D65ADCu, (u32) 0x40DF0B66u, (u32) 0x37D83BF0u,
(u32) 0xA9BCAE53u, (u32) 0xDEBB9EC5u, (u32) 0x47B2CF7Fu, (u32) 0x30B5FFE9u,
(u32) 0xBDBDF21Cu, (u32) 0xCABAC28Au, (u32) 0x53B39330u, (u32) 0x24B4A3A6u,
(u32) 0xBAD03605u, (u32) 0xCDD70693u, (u32) 0x54DE5729u, (u32) 0x23D967BFu,
(u32) 0xB3667A2Eu, (u32) 0xC4614AB8u, (u32) 0x5D681B02u, (u32) 0x2A6F2B94u,
(u32) 0xB40BBE37u, (u32) 0xC30C8EA1u, (u32) 0x5A05DF1Bu, (u32) 0x2D02EF8Du
};

//////////////////////////////////////////////////////////////////////////////

/**@fn libttaR_crc32
 * @brief calculate a TTA CRC
 *
 * @param buf[in] the input buffer
 * @param size size of the buffer
 *
 * @return the CRC
 *
 * @note read the manpage for more info
 * @note This function is obviously not as fast as Intel's slicing method. It
 *   is meant for TTA's header and seektable. Given that those are rather
 *   small and calculating their CRCs is an insignificant part of a program's
 *   runtime, size is more important. Frame CRC calculation is inlined into
 *   the rice coder.
**/
PURE u32
libttaR_crc32(const u8 *const restrict buf, const size_t size)
/*@*/
{
	u32 crc = CRC32_INIT;
	size_t i;

	for ( i = 0; i < size; ++i ){
		crc = crc32_cont(buf[i], crc);
	}
	return crc32_end(crc);
}

// EOF ///////////////////////////////////////////////////////////////////////
