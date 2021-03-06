/*
 * This is from the Android Project,
 * Repository: https://android.googlesource.com/platform/system/core
 * File: libsparse/sparse_format.h
 * Commit: 28fa5bc347390480fe190294c6c385b6a9f0d68b
 * Copyright (C) 2010 The Android Open Source Project
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _IMAGE_SPARSE_H
#define _IMAGE_SPARSE_H

struct sparse_header {
  __le32	magic;		/* 0xed26ff3a */
  __le16	major_version;	/* (0x1) - reject images with higher major versions */
  __le16	minor_version;	/* (0x0) - allow images with higer minor versions */
  __le16	file_hdr_sz;	/* 28 bytes for first revision of the file format */
  __le16	chunk_hdr_sz;	/* 12 bytes for first revision of the file format */
  __le32	blk_sz;		/* block size in bytes, must be a multiple of 4 (4096) */
  __le32	total_blks;	/* total blocks in the non-sparse output image */
  __le32	total_chunks;	/* total chunks in the sparse input image */
  __le32	image_checksum; /* CRC32 checksum of the original data, counting "don't care" */
				/* as 0. Standard 802.3 polynomial, use a Public Domain */
				/* table implementation */
};

#define SPARSE_HEADER_MAGIC	0xed26ff3a

#define CHUNK_TYPE_RAW		0xCAC1
#define CHUNK_TYPE_FILL		0xCAC2
#define CHUNK_TYPE_DONT_CARE	0xCAC3
#define CHUNK_TYPE_CRC32    0xCAC4

struct chunk_header {
  __le16	chunk_type;	/* 0xCAC1 -> raw; 0xCAC2 -> fill; 0xCAC3 -> don't care */
  __le16	reserved1;
  __le32	chunk_sz;	/* in blocks in output image */
  __le32	total_sz;	/* in bytes of chunk input file including chunk header and data */
};

/* Following a Raw or Fill or CRC32 chunk is data.
 *  For a Raw chunk, it's the data in chunk_sz * blk_sz.
 *  For a Fill chunk, it's 4 bytes of the fill data.
 *  For a CRC32 chunk, it's 4 bytes of CRC32
 */

static inline int is_sparse_image(const void *buf)
{
	const struct sparse_header *s = buf;

	if ((le32_to_cpu(s->magic) == SPARSE_HEADER_MAGIC) &&
	    (le16_to_cpu(s->major_version) == 1))
		return 1;

	return 0;
}

struct sparse_image_ctx;

struct sparse_image_ctx *sparse_image_open(const char *path);
int sparse_image_read(struct sparse_image_ctx *si, void *buf,
		      loff_t *pos, size_t len, size_t *retlen);
void sparse_image_close(struct sparse_image_ctx *si);
loff_t sparse_image_size(struct sparse_image_ctx *si);

#endif /* _IMAGE_SPARSE_H */
