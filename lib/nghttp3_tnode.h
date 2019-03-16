/*
 * nghttp3
 *
 * Copyright (c) 2019 nghttp3 contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef NGHTTP3_TNODE_H
#define NGHTTP3_TNODE_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <nghttp3/nghttp3.h>

#include "nghttp3_pq.h"

#define NGHTTP3_DEFAULT_WEIGHT 16
#define NGHTTP3_MAX_WEIGHT 256
#define NGHTTP3_TNODE_MAX_CYCLE_GAP ((1llu << 24) * 256 + 255)

typedef enum {
  NGHTTP3_ID_TYPE_STREAM,
  NGHTTP3_ID_TYPE_PUSH,
  NGHTTP3_ID_TYPE_PLACEHOLDER,
  NGHTTP3_ID_TYPE_ROOT,
} nghttp3_id_type;

struct nghttp3_tnode;
typedef struct nghttp3_tnode nghttp3_tnode;

struct nghttp3_tnode {
  nghttp3_pq_entry pe;
  nghttp3_pq pq;
  nghttp3_tnode *parent;
  int64_t id;
  uint64_t seq;
  uint64_t cycle;
  uint32_t pending_penalty;
  uint32_t weight;
  nghttp3_id_type id_type;
};

void nghttp3_tnode_init(nghttp3_tnode *tnode, nghttp3_id_type id_type,
                        int64_t id, uint64_t seq, uint32_t weight,
                        nghttp3_tnode *parent, const nghttp3_mem *mem);

void nghttp3_tnode_free(nghttp3_tnode *tnode);

void nghttp3_tnode_unschedule(nghttp3_tnode *tnode);

/*
 * nghttp3_tnode_schedule schedules |node|.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * TBD
 */
int nghttp3_tnode_schedule(nghttp3_tnode *tnode, size_t nwrite);

/*
 * nghttp3_tnode_get_next returns node which has highest priority.
 * This function returns NULL if there is no node.
 */
nghttp3_tnode *nghttp3_tnode_get_next(nghttp3_tnode *node);

#endif /* NGHTTP3_TNODE_H */
