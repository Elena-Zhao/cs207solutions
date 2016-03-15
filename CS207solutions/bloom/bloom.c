#include "bloom.h"

void set_bit(bloom_filter_t *B, index_t i) {
  // UINT64_C is a macro that tells the compiler to make a constant 64 bits long
  B->table[i>>6] |= UINT64_C(1)<<(i&63);
}

index_t get_bit(bloom_filter_t *B, index_t i) {
  return ( (B->table[i>>6]) & UINT64_C(1)<<(i&63) )>>(i&63);
}

// Note:
// These two hash functions are overkill, but they're presented as representative
// of what you might use in a normal hash function.

index_t hash1(bloom_filter_t *B, key_t k) {
  // 64-bit adapted version of murmurhash
  // https://github.com/aappleby/smhasher
  // Original 32-bit constant replaced with a 64-bit prime
  //   (not guaranteed to be as good as the original, but fine for this lab)
  const uint64_t h = INT64_C(0xc6a4a793c6a4a7bf);
  k += h;
  k *= k;
  k ^= k>>32;
  return k % (B->size);
}

index_t hash2(bloom_filter_t *B, key_t k) {
  // Unrolled version of FNV
  // http://www.isthe.com/chongo/tech/comp/fnv/index.html
  const uint64_t h = INT64_C(0xcbf29ce484222325); // offset basis
  const uint64_t p = INT64_C(0x100000001b3); // primesh = offset_basis
  uint64_t v = h;
  v = (v^((k>>0)&0xff))*p;
  v = (v^((k>>1)&0xff))*p;
  v = (v^((k>>2)&0xff))*p;
  v = (v^((k>>3)&0xff))*p;
  v = (v^((k>>4)&0xff))*p;
  v = (v^((k>>5)&0xff))*p;
  v = (v^((k>>6)&0xff))*p;
  v = (v^((k>>7)&0xff))*p;
  return v % (B->size);
}

void bloom_init(bloom_filter_t *B, index_t size) {
  // size in *bits*
  B->size = size;
  B->count = 0;
  // the +1 is for bits > size%64
  // (e.g., when size is 73, size>>6==1, but we need storage for 9 more bits)
  B->table = calloc((size>>6)+1, sizeof(index_t) );
}

void bloom_destroy(bloom_filter_t *B) {
  free(B->table);
}

int bloom_check(bloom_filter_t *B, key_t k) {
  index_t H;
  int found = 1;
  int i;

  for(i=0; i<N_HASHES; i++ ) {
    H = (hash1(B,k) + i*hash2(B,k)) % (B->size);
    found &= get_bit(B, H);
  }
  return found;
}

void bloom_add(bloom_filter_t *B, key_t k) {
  index_t H;
  int i;

  for(i=0; i<N_HASHES; i++ ) {
    H = (hash1(B,k) + i*hash2(B,k)) % B->size;
    set_bit(B, H);
  }

  B->count++;
}

int bloom_count_bits(bloom_filter_t *B) {
  index_t i, count=0;
  for( i=0; i<B->size; i++ )
    if( get_bit(B, i) )
      count++;
  return count;
}

void usage() {
  printf("Usage: bloom <number>\n"
         "  0: hash functions\n"
         "  1: smoke test\n"
         "  2: experiment\n");
}

void run_hash_test() {
  bloom_filter_t B;
  int i;

  printf("Hash functions:\n");
  bloom_init(&B, 100);
  i=0;  printf("%d\t%llu\t%llu\n", i, hash1(&B,i), hash2(&B,i) );
  i=1;  printf("%d\t%llu\t%llu\n", i, hash1(&B,i), hash2(&B,i) );
  i=2;  printf("%d\t%llu\t%llu\n", i, hash1(&B,i), hash2(&B,i) );
  i=3;  printf("%d\t%llu\t%llu\n", i, hash1(&B,i), hash2(&B,i) );
  i=13; printf("%d\t%llu\t%llu\n", i, hash1(&B,i), hash2(&B,i) );
  i=97; printf("%d\t%llu\t%llu\n", i, hash1(&B,i), hash2(&B,i) );
  bloom_destroy(&B);
}

void run_smoke_test() {
  bloom_filter_t B;
  int i;

  bloom_init(&B, 1000);
  for(i=1; i<=70; i++)
    bloom_add(&B, i);
  printf("%d bits\n", bloom_count_bits(&B));
  bloom_destroy(&B);
}

void run_experiment() {
  bloom_filter_t B;
  int i, false_pos;
  index_t A0[100], A1[100];

  printf("Experiment (N_HASHES=%d):\n", N_HASHES);
  // Technically, the numbers should be between 0 and 2^64-1, but since our
  // Bloom filter is only 1000000 elements, 32-bit randoms are fine.
  srand(7); // Determinism is good science!
  for(i=0; i<100; i++) {
    A0[i] = rand();
    A1[i] = rand();
  }
  bloom_init(&B, 1000);

  for(i=0; i<100; i++)
    bloom_add(&B, i);
  printf("%d bits\n", bloom_count_bits(&B));

  false_pos = 0;
  for(i=0; i<100; i++)
    if( bloom_check(&B, A1[i]) )
      false_pos++;
  printf("%d false positives\n", false_pos);

  bloom_destroy(&B);
}

int main(int argc, char **argv) {
  int a;

  if( argc<2 ) {
    usage();
    return -1;
  }
  a = atoi(argv[1]);
  switch(a){
  case 0:
    run_hash_test();
    break;
  case 1:
    run_smoke_test();
    break;
  case 2:
    run_experiment();
    break;
  default:
    printf("Invalid argument.\n");
    usage();
    return -1;
  }

  return 0;
}

