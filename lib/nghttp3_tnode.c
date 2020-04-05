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
#include "nghttp3_tnode.h"

#include <assert.h>

#include "nghttp3_macro.h"
#include "nghttp3_stream.h"
#include "nghttp3_conn.h"

nghttp3_node_id *nghttp3_node_id_init(nghttp3_node_id *nid,
                                      nghttp3_node_id_type type, int64_t id) {
  nid->type = type;
  nid->id = id;
  return nid;
}

int nghttp3_node_id_eq(const nghttp3_node_id *a, const nghttp3_node_id *b) {
  return a->type == b->type && a->id == b->id;
}

void nghttp3_tnode_init(nghttp3_tnode *tnode, const nghttp3_node_id *nid,
                        uint64_t seq, uint32_t urgency, int inc) {
  tnode->pe.index = NGHTTP3_PQ_BAD_INDEX;
  tnode->nid = *nid;
  tnode->seq = seq;
  tnode->cycle = 0;
  tnode->urgency = urgency;
  tnode->inc = inc;
  tnode->active = 0;
}

void nghttp3_tnode_free(nghttp3_tnode *tnode) { (void)tnode; }

int nghttp3_tnode_is_active(nghttp3_tnode *tnode) {
  nghttp3_push_promise *pp;

  switch (tnode->nid.type) {
  case NGHTTP3_NODE_ID_TYPE_STREAM:
    return nghttp3_stream_is_active(
        nghttp3_struct_of(tnode, nghttp3_stream, node));
  case NGHTTP3_NODE_ID_TYPE_PUSH:
    pp = nghttp3_struct_of(tnode, nghttp3_push_promise, node);
    return pp->stream && nghttp3_stream_is_active(pp->stream);
  case NGHTTP3_NODE_ID_TYPE_UT:
    /* For unit test */
    return tnode->active;
  default:
    return 0;
  }
}

static void tnode_unschedule(nghttp3_tnode *tnode, nghttp3_pq *pq) {
  assert(tnode->pe.index != NGHTTP3_PQ_BAD_INDEX);

  nghttp3_pq_remove(pq, &tnode->pe);
  tnode->pe.index = NGHTTP3_PQ_BAD_INDEX;
}

void nghttp3_tnode_unschedule(nghttp3_tnode *tnode, nghttp3_pq *pq) {
  if (tnode->pe.index == NGHTTP3_PQ_BAD_INDEX) {
    return;
  }

  tnode_unschedule(tnode, pq);
}

static uint64_t pq_get_first_cycle(nghttp3_pq *pq) {
  nghttp3_tnode *top;

  if (nghttp3_pq_empty(pq)) {
    return 0;
  }

  top = nghttp3_struct_of(nghttp3_pq_top(pq), nghttp3_tnode, pe);
  return top->cycle;
}

int nghttp3_tnode_schedule(nghttp3_tnode *tnode, nghttp3_pq *pq,
                           size_t nwrite) {
  uint64_t cycle;

  if (tnode->pe.index == NGHTTP3_PQ_BAD_INDEX) {
    cycle = pq_get_first_cycle(pq);
  } else if (nwrite > 0) {
    if (!tnode->inc || nghttp3_pq_size(pq) == 1) {
      return 0;
    }

    cycle = tnode->cycle;
    nghttp3_pq_remove(pq, &tnode->pe);
    tnode->pe.index = NGHTTP3_PQ_BAD_INDEX;
  } else {
    return 0;
  }

  tnode->cycle = cycle + nwrite;

  return nghttp3_pq_push(pq, &tnode->pe);
}

int nghttp3_tnode_is_scheduled(nghttp3_tnode *tnode) {
  return tnode->pe.index != NGHTTP3_PQ_BAD_INDEX;
}
