
#define I(a,b,c) ( (a & 0xff) | (b << 8) | (0 << 27))
const static uint32 MATERIAL_VALUE[16] =
  { 0, I (0, 0x1440, 1), I (1, 0x240, 1), 0,
    I (1, 0x24, 1), I (1, 0x48, 1), I (3, 0x04, 1), I (6, 0x1, 1),
    0, I (0, 0xb640, 1), I (1, 0x6c0, 1), 0,
    I (1, 0x90, 1), I (1, 0x120, 1), I (3, 0xc, 1), I (6, 0x2, 1)
  };
/* SMP read */
