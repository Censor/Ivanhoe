
#define CLOCK ( (double) (GetClock() - START_CLOCK) / 1000000.0)

typedef struct { uint8* DATA; uint32* INDEX; uint8 user; } TripleBaseCache;
typedef struct { int len, bits; } TypeSymbols;

typedef struct
{
  uint8 *data;
  uint32* ind; /* HACK, giga limited */
  uint32* ind_split[8]; /* giga */
  uint32 packet_count[8];
  uint32* packet_offset[8];
  uint32* huffman_table[8];
  int bs_split[8];
  TripleBaseCache direct_cache[8];
  uint8 variant, scacco; /* ROBBO_DYNAMIC_LOAD, Huffman */
  int prop_ind; /* bytes per index */
  uint8 w[4], wi[4]; /* results plus inverses */
  sint8 p[4]; /* pieces */
  int num, index, by, bs;
  uint64 size, weak, hit;
  uint64 m[4]; /* multipliers */
  char nome[8];
  FILE* disk;
  char DIR_NOME[256];
  char string[16], path[32];
  boolean pawns, direct; /* pawns only with size */
  sint8 efgh1, efgh2;
  uint16 efgh[4];
  uint8 efgh_shift[4], efgh_file[4];
} RobboTripleBase;

RobboTripleBase* TABLE_TRIPLE_BASES;
int NUM_TRIPLE_BASES;
uint64 BULK_BYTES;
uint64 BULK_IND;
uint32 BULK_COUNT, LOAD_COUNT, DIRECT_COUNT;
int TRIVIALLY_COUNT;

#define ROBBO_SCACCO_KNOW_BIT 8
#define ROBBO_SCACCO_BIT 4
#define ROBBO_DYNAMIC_LOAD 2
#define ROBBO_DIRECT_DYNAMIC_BIT 1

#define vNON 0
#define eVIT 1
#define ePAT 2
#define ePER 3
void robbo_triple_utilita (RobboTripleBase*, char*);
RobboTripleBase* LookupTriple (char*);
void un_windows_triple (char*);
void INIT_TRIPLE_BASE ();
void READY_TRIPLE_BASE ();
void VerifyTripleBase (int*);
boolean DemandDynamicTripleCache(RobboTripleBase*, int, int,TripleBaseCache* *, uint8* *, uint32* *, boolean, boolean);

#define TRIPLE_PREFISSO "TRIPLE"

/*
   3 bytes: results,
   1 byte: variant, ROBBO_DYNAMIC_LOAD | ROBBO_STYLE_HUFFMAN
   FullLoad: // indices 4-byte limited
      4 bytes, bytes of data
      4 bytes, full count // 4-byte limited // extenuous
      2 bytes, bytes per index (16/64)
      2 bytes, no impact
   DynamicLoad:
      4 bytes, 0x53 0xa1 0x78 0x0e
      2 bytes, data bytes per index (16/64), demand 64
      1 byte, stream size (shift), 18 as 256k (3 step)
      1 bytes, giga_split count, no impact
      4 bytes, no impact, buffer
      GigaSplit
          4 bytes, above level index count
          [indexes]
          [packet offsets]
          Huffman type, 4 bytes 0x93 0x06 0xe1 0x88 (omit)
          Huffman type, tabular consist (omit)
      256k packets
          4 bytes, 0xf2 0x77 0x83 0x83
          4 bytes, index count
          4 bytes, data bytes count
          [indexes]
          [data]

   Care: 2 first bytes, second is 0, acknowledged single result
   Attend (now): ROBBO_STYLE_HUFFMAN for the ignore
*/
