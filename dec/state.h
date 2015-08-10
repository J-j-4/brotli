/* Copyright 2015 Google Inc. All Rights Reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

/* Brotli state for partial streaming decoding. */

#ifndef BROTLI_DEC_STATE_H_
#define BROTLI_DEC_STATE_H_

#include <stdio.h>
#include "./bit_reader.h"
#include "./huffman.h"
#include "./streams.h"
#include "./types.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum {
  BROTLI_STATE_UNINITED,
  BROTLI_STATE_BITREADER_WARMUP,
  BROTLI_STATE_METABLOCK_BEGIN,
  BROTLI_STATE_METABLOCK_HEADER_1,
  BROTLI_STATE_METABLOCK_HEADER_2,
  BROTLI_STATE_BLOCK_BEGIN,
  BROTLI_STATE_BLOCK_INNER,
  BROTLI_STATE_BLOCK_DISTANCE,
  BROTLI_STATE_BLOCK_POST,
  BROTLI_STATE_UNCOMPRESSED,
  BROTLI_STATE_METADATA,
  BROTLI_STATE_BLOCK_INNER_WRITE,
  BROTLI_STATE_METABLOCK_DONE,
  BROTLI_STATE_BLOCK_POST_WRITE_1,
  BROTLI_STATE_BLOCK_POST_WRITE_2,
  BROTLI_STATE_BLOCK_POST_WRAP_COPY,
  BROTLI_STATE_HUFFMAN_CODE_0,
  BROTLI_STATE_HUFFMAN_CODE_1,
  BROTLI_STATE_HUFFMAN_CODE_2,
  BROTLI_STATE_CONTEXT_MAP_1,
  BROTLI_STATE_CONTEXT_MAP_2,
  BROTLI_STATE_TREE_GROUP,
  BROTLI_STATE_DONE
} BrotliRunningState;

typedef enum {
  BROTLI_STATE_SUB0_NONE,
  BROTLI_STATE_SUB0_UNCOMPRESSED_SHORT,
  BROTLI_STATE_SUB0_UNCOMPRESSED_FILL,
  BROTLI_STATE_SUB0_UNCOMPRESSED_COPY,
  BROTLI_STATE_SUB0_UNCOMPRESSED_WARMUP,
  BROTLI_STATE_SUB0_UNCOMPRESSED_WRITE_1,
  BROTLI_STATE_SUB0_UNCOMPRESSED_WRITE_2,
  BROTLI_STATE_SUB0_UNCOMPRESSED_WRITE_3,
  BROTLI_STATE_SUB0_TREE_GROUP,
  BROTLI_STATE_SUB0_CONTEXT_MAP_HUFFMAN,
  BROTLI_STATE_SUB0_CONTEXT_MAPS
} BrotliRunningSub0State;

typedef enum {
  BROTLI_STATE_SUB1_NONE,
  BROTLI_STATE_SUB1_HUFFMAN_LENGTH_BEGIN,
  BROTLI_STATE_SUB1_HUFFMAN_LENGTH_SYMBOLS,
  BROTLI_STATE_SUB1_HUFFMAN_DONE
} BrotliRunningSub1State;

typedef struct {
  BrotliRunningState state;
  BrotliRunningSub0State sub0_state;  /* State inside function call */
  BrotliRunningSub1State sub1_state;  /* State inside function call */

  int pos;
  int input_end;
  uint32_t window_bits;
  int max_backward_distance;
  int max_distance;
  int ringbuffer_size;
  int ringbuffer_mask;
  uint8_t* ringbuffer;
  uint8_t* ringbuffer_end;
  /* This ring buffer holds a few past copy distances that will be used by */
  /* some special distance codes. */
  int dist_rb[4];
  int dist_rb_idx;
  HuffmanTreeGroup literal_hgroup;
  HuffmanTreeGroup insert_copy_hgroup;
  HuffmanTreeGroup distance_hgroup;

  HuffmanCode* block_type_trees;
  HuffmanCode* block_len_trees;
  BrotliBitReader br;
  /* This counter is reused for several disjoint loops. */
  int loop_counter;
  /* This is true if the literal context map histogram type always matches the
  block type. It is then not needed to keep the context (faster decoding). */
  int trivial_literal_context;

  int meta_block_remaining_len;
  int is_metadata;
  int is_uncompressed;
  int block_length[3];
  int num_block_types[3];
  int block_type_rb[6];
  int distance_postfix_bits;
  int num_direct_distance_codes;
  int distance_postfix_mask;
  uint8_t* context_map;
  uint8_t* context_modes;
  int num_literal_htrees;
  uint8_t* dist_context_map;
  int num_dist_htrees;
  uint8_t* context_map_slice;
  uint8_t* dist_context_map_slice;
  uint8_t literal_htree_index;
  uint8_t dist_htree_index;
  uint8_t prev_code_len;
  uint8_t repeat_code_len;

  const uint8_t* context_lookup1;
  const uint8_t* context_lookup2;
  HuffmanCode* htree_command;

  int insert_code;
  int copy_code;
  int copy_length;
  int distance_code;
  int distance;

  /* For CopyUncompressedBlockToOutput */
  int nbytes;

  /* For partial write operations */
  int to_write;
  int partially_written;

  /* For HuffmanTreeGroupDecode */
  int htrees_decoded;

  /* For ReadHuffmanCodeLengths */
  int symbol;
  int repeat;
  int space;
  HuffmanCode table[32];
  uint8_t code_length_code_lengths[18];

  /* For ReadHuffmanCode */
  uint8_t* code_lengths;
  /* The maximum non-zero code length index in code lengths */
  uint32_t huffman_max_nonzero;
  /* Population counts for the code lengths */
  uint16_t code_length_histo[16];

  /* For HuffmanTreeGroupDecode */
  int htree_index;
  HuffmanCode* next;

  /* For DecodeContextMap */
  int context_index;
  int max_run_length_prefix;
  HuffmanCode* context_map_table;

  /* For InverseMoveToFrontTransform */
  int mtf_upper_bound;
  uint8_t mtf[256];

  /* For custom dictionaries */
  const uint8_t* custom_dict;
  int custom_dict_size;
} BrotliState;

void BrotliStateInit(BrotliState* s);
void BrotliStateCleanup(BrotliState* s);
void BrotliStateMetablockBegin(BrotliState* s);
void BrotliStateCleanupAfterMetablock(BrotliState* s);

#if defined(__cplusplus) || defined(c_plusplus)
} /* extern "C" */
#endif

#endif  /* BROTLI_DEC_STATE_H_ */
