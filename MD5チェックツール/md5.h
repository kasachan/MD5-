#pragma once

#ifndef __MD5_H__
#define __MD5_H__

#include <stdio.h>

#define MD5_DIGEST_SIZE	16
#define MD5_BLOCK_SIZE	64

typedef struct
{
	unsigned int A;
	unsigned int B;
	unsigned int C;
	unsigned int D;

	size_t total[2];
	size_t buflen;
	unsigned int buffer[32];
} md5_ctx, *lpmd5_ctx;

# ifdef __cplusplus
extern "C" {
# endif
void md5_init_ctx(md5_ctx *ctx);
void set_uint32(char *cp, unsigned int v);
void * md5_read_ctx(const md5_ctx *ctx, void *resbuf);
void md5_process_block(const void *buffer, unsigned int len, md5_ctx *ctx);
void md5_process_bytes(const void *buffer, unsigned int len, md5_ctx *ctx);
void * md5_finish_ctx(md5_ctx *ctx, void *resbuf);
# ifdef __cplusplus
}
# endif
#endif /* __MD5_H__ */
