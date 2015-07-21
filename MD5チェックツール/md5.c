#include "md5.h"

#include <stddef.h>
#include <stdlib.h>
#include <memory.h>


static const unsigned char fillbuf[64] = { 0x80, 0 /* , 0, 0, ...  */ };

void md5_init_ctx(md5_ctx *ctx)
{
	ctx->A = 0x67452301;
	ctx->B = 0xefcdab89;
	ctx->C = 0x98badcfe;
	ctx->D = 0x10325476;

	ctx->total[0] = ctx->total[1] = 0;
	ctx->buflen = 0;
}

void set_uint32 (char *cp, unsigned int v)
{
	memcpy (cp, &v, sizeof(v));
};

void * md5_read_ctx (const md5_ctx *ctx, void *resbuf)
{
	char *r = (char *)resbuf;
	set_uint32 (r + 0 * sizeof ctx->A, ctx->A);
	set_uint32 (r + 1 * sizeof ctx->B, ctx->B);
	set_uint32 (r + 2 * sizeof ctx->C, ctx->C);
	set_uint32 (r + 3 * sizeof ctx->D, ctx->D);

	return resbuf;
}

void * md5_finish_ctx (md5_ctx *ctx, void *resbuf)
{
	size_t bytes = ctx->buflen;
	size_t size = (bytes < 56) ? 64 / 4 : 64 * 2 / 4;

	ctx->total[0] += bytes;
	if (ctx->total[0] < bytes)
		++ctx->total[1];

	ctx->buffer[size - 2] = (unsigned int)(ctx->total[0] << 3);
	ctx->buffer[size - 1] = (unsigned int)((ctx->total[1] << 3) | (ctx->total[0] >> 29));

	memcpy (&((char *) ctx->buffer)[bytes], fillbuf, (size - 2) * 4 - bytes);

	md5_process_block (ctx->buffer, size * 4, ctx);

	return md5_read_ctx (ctx, resbuf);
}

void md5_process_bytes (const void *buffer, unsigned int len, md5_ctx *ctx)
{
	if (ctx->buflen != 0)
	{
		unsigned int left_over = ctx->buflen;
		unsigned int add = 128 - left_over > len ? len : 128 - left_over;

		memcpy (&((char *) ctx->buffer)[left_over], buffer, add);
		ctx->buflen += add;

		if (ctx->buflen > 64)
		{
			md5_process_block (ctx->buffer, ctx->buflen & ~63, ctx);

			ctx->buflen &= 63;

			memcpy (ctx->buffer,
				&((char *) ctx->buffer)[(left_over + add) & ~63],
				ctx->buflen);
		}

		buffer = (const char *) buffer + add;
		len -= add;
	}

	/* Process available complete blocks.  */
	if (len >= 64)
	{

# define alignof(type) offsetof (struct { char c; type x; }, x)
# define UNALIGNED_P(p) (((size_t) p) % alignof (unsigned int) != 0)

		if (UNALIGNED_P (buffer))
		{
			while (len > 64)
			{
				md5_process_block (memcpy (ctx->buffer, buffer, 64), 64, ctx);
				buffer = (const char *) buffer + 64;
				len -= 64;
			}
		}
		else

		{
			md5_process_block (buffer, len & ~63, ctx);
			buffer = (const char *) buffer + (len & ~63);
			len &= 63;
		}
	}


	if (len > 0)
	{
		unsigned int left_over = ctx->buflen;

		memcpy (&((char *) ctx->buffer)[left_over], buffer, len);
		left_over += len;
		if (left_over >= 64)
		{
			md5_process_block (ctx->buffer, 64, ctx);
			left_over -= 64;
			memcpy (ctx->buffer, &ctx->buffer[16], left_over);
		}
		ctx->buflen = left_over;
	}
}

#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF (d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))

void md5_process_block(const void *buffer, unsigned int len, md5_ctx *ctx)
{
	unsigned int correct_words[16];
	const unsigned int *words = (const unsigned int *)buffer;
	unsigned int nwords = len / sizeof (unsigned int);
	const unsigned int *endp = words + nwords;
	unsigned int A = ctx->A;
	unsigned int B = ctx->B;
	unsigned int C = ctx->C;
	unsigned int D = ctx->D;

	ctx->total[0] += len;
	if (ctx->total[0] < len)
		++ctx->total[1];

	while (words < endp)
	{
		unsigned int *cwp = correct_words;
		unsigned int A_save = A;
		unsigned int B_save = B;
		unsigned int C_save = C;
		unsigned int D_save = D;


#define OP(a, b, c, d, s, T)						\
	do								\
		{								\
		a += FF (b, c, d) + (*cwp++ = *words) + T;		\
		++words;							\
		CYCLIC (a, s);						\
		a += b;							\
		}								\
		while (0)

#define CYCLIC(w, s) (w = (w << s) | (w >> (32 - s)))

		OP (A, B, C, D, 7, 0xd76aa478);
		OP (D, A, B, C, 12, 0xe8c7b756);
		OP (C, D, A, B, 17, 0x242070db);
		OP (B, C, D, A, 22, 0xc1bdceee);
		OP (A, B, C, D, 7, 0xf57c0faf);
		OP (D, A, B, C, 12, 0x4787c62a);
		OP (C, D, A, B, 17, 0xa8304613);
		OP (B, C, D, A, 22, 0xfd469501);
		OP (A, B, C, D, 7, 0x698098d8);
		OP (D, A, B, C, 12, 0x8b44f7af);
		OP (C, D, A, B, 17, 0xffff5bb1);
		OP (B, C, D, A, 22, 0x895cd7be);
		OP (A, B, C, D, 7, 0x6b901122);
		OP (D, A, B, C, 12, 0xfd987193);
		OP (C, D, A, B, 17, 0xa679438e);
		OP (B, C, D, A, 22, 0x49b40821);


#undef OP
#define OP(f, a, b, c, d, k, s, T)					\
	do								\
		{								\
		a += f (b, c, d) + correct_words[k] + T;			\
		CYCLIC (a, s);						\
		a += b;							\
		}								\
		while (0)

		OP (FG, A, B, C, D, 1, 5, 0xf61e2562);
		OP (FG, D, A, B, C, 6, 9, 0xc040b340);
		OP (FG, C, D, A, B, 11, 14, 0x265e5a51);
		OP (FG, B, C, D, A, 0, 20, 0xe9b6c7aa);
		OP (FG, A, B, C, D, 5, 5, 0xd62f105d);
		OP (FG, D, A, B, C, 10, 9, 0x02441453);
		OP (FG, C, D, A, B, 15, 14, 0xd8a1e681);
		OP (FG, B, C, D, A, 4, 20, 0xe7d3fbc8);
		OP (FG, A, B, C, D, 9, 5, 0x21e1cde6);
		OP (FG, D, A, B, C, 14, 9, 0xc33707d6);
		OP (FG, C, D, A, B, 3, 14, 0xf4d50d87);
		OP (FG, B, C, D, A, 8, 20, 0x455a14ed);
		OP (FG, A, B, C, D, 13, 5, 0xa9e3e905);
		OP (FG, D, A, B, C, 2, 9, 0xfcefa3f8);
		OP (FG, C, D, A, B, 7, 14, 0x676f02d9);
		OP (FG, B, C, D, A, 12, 20, 0x8d2a4c8a);


		OP (FH, A, B, C, D, 5, 4, 0xfffa3942);
		OP (FH, D, A, B, C, 8, 11, 0x8771f681);
		OP (FH, C, D, A, B, 11, 16, 0x6d9d6122);
		OP (FH, B, C, D, A, 14, 23, 0xfde5380c);
		OP (FH, A, B, C, D, 1, 4, 0xa4beea44);
		OP (FH, D, A, B, C, 4, 11, 0x4bdecfa9);
		OP (FH, C, D, A, B, 7, 16, 0xf6bb4b60);
		OP (FH, B, C, D, A, 10, 23, 0xbebfbc70);
		OP (FH, A, B, C, D, 13, 4, 0x289b7ec6);
		OP (FH, D, A, B, C, 0, 11, 0xeaa127fa);
		OP (FH, C, D, A, B, 3, 16, 0xd4ef3085);
		OP (FH, B, C, D, A, 6, 23, 0x04881d05);
		OP (FH, A, B, C, D, 9, 4, 0xd9d4d039);
		OP (FH, D, A, B, C, 12, 11, 0xe6db99e5);
		OP (FH, C, D, A, B, 15, 16, 0x1fa27cf8);
		OP (FH, B, C, D, A, 2, 23, 0xc4ac5665);


		OP (FI, A, B, C, D, 0, 6, 0xf4292244);
		OP (FI, D, A, B, C, 7, 10, 0x432aff97);
		OP (FI, C, D, A, B, 14, 15, 0xab9423a7);
		OP (FI, B, C, D, A, 5, 21, 0xfc93a039);
		OP (FI, A, B, C, D, 12, 6, 0x655b59c3);
		OP (FI, D, A, B, C, 3, 10, 0x8f0ccc92);
		OP (FI, C, D, A, B, 10, 15, 0xffeff47d);
		OP (FI, B, C, D, A, 1, 21, 0x85845dd1);
		OP (FI, A, B, C, D, 8, 6, 0x6fa87e4f);
		OP (FI, D, A, B, C, 15, 10, 0xfe2ce6e0);
		OP (FI, C, D, A, B, 6, 15, 0xa3014314);
		OP (FI, B, C, D, A, 13, 21, 0x4e0811a1);
		OP (FI, A, B, C, D, 4, 6, 0xf7537e82);
		OP (FI, D, A, B, C, 11, 10, 0xbd3af235);
		OP (FI, C, D, A, B, 2, 15, 0x2ad7d2bb);
		OP (FI, B, C, D, A, 9, 21, 0xeb86d391);


		A += A_save;
		B += B_save;
		C += C_save;
		D += D_save;
	}

	ctx->A = A;
	ctx->B = B;
	ctx->C = C;
	ctx->D = D;
}

/*
	md5_init_ctx(&ctx);
	do
	{
		if(lpmt->SuspendMessage != 0)
		{
			bRet = FALSE;
			goto FILE_EXIT;
		}

		//64KB‚¸‚Âˆ—
		bRet = ReadFile(hFile, bBuffer, FILE_BUFFER_SIZE, &dwRead, NULL);
		if(bRet == FALSE)
		{
			dwLasterror = GetLastError();
			break;
		}
		lpmt->FileReadSize += dwRead;

		if(lpmt->FileReadSize == lpmt->FileSize)
		{
			goto process_partial_block;
		}

		md5_process_block(bBuffer, FILE_BUFFER_SIZE, &ctx);
	}
	while(lpmt->FileReadSize < lpmt->FileSize);

process_partial_block:
	//ƒnƒbƒVƒ…Žæ“¾
	if(dwRead > 0)
		md5_process_bytes(bBuffer, dwRead, &ctx);

	md5_finish_ctx(&ctx, bHash);
*/
