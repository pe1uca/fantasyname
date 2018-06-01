#ifndef NAMEGEN_H
#define NAMEGEN_H

#include <stddef.h>

/* Interface */

#define NAMEGEN_SUCCESS    0
#define NAMEGEN_TRUNCATED  1
#define NAMEGEN_INVALID    2

/* Generate a name into DST of length LEN from PATTERN and using SEED.
 *
 * The lower 32 bits seed should be thoroughly initialized. The function
 * will return one of the above codes to report the result. Truncation
 * occurs when DST is too short, which also causes validation to bail
 * out early.
 */
static int
namegen(char *dst, size_t len, const char *pattern, unsigned long *seed);

/* Implementation */

/* Rather than compile the pattern into some internal representation,
 * the name is generated directly from the pattern using reservoir
 * sampling. Under alternation (|), an initial pass selects a random
 * subgroup token, and a second pass actually generates a name from that
 * token. This is applied recursively within each token.
 *
 * The substitution templates are stored in an efficient, packed form
 * that contains no pointers. This is to avoid cluttering up the
 * relocation table, but without any additional run-time overhead.
 */

/* Return offsets table offset for C, or -1 if not special.
 * The return value is suitable for namegen_offsets().
 */
static int
namegen_special(int c)
{
    static const signed char table[] = {
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1, 0x04, 0x05, 0x09,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1, 0x08,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1, 0x02,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1, 0x03, 0x0a,   -1,   -1,   -1,
        -1, 0x06,   -1,   -1,   -1, 0x07,   -1,   -1,
        -1,   -1,   -1, 0x00,   -1,   -1, 0x01,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1
    };
    if (c >= 0 && c < 128)
        return table[c];
    return -1;
}

/* Return offsets and number of offsets for special N.
 * Offsets point into namegen_argz.
 */
static int
namegen_offsets(int n, const short **offsets)
{
    static const short offsets_table[] = {
        0, 4, 8, 11, 15, 19, 23, 26, 30, 33, 37, 40, 44, 47, 51, 56, 59,
        63, 67, 71, 75, 79, 83, 87, 91, 95, 99, 103, 107, 111, 115, 119,
        122, 125, 129, 133, 138, 141, 145, 149, 152, 156, 160, 164, 168,
        172, 175, 180, 184, 187, 191, 195, 199, 202, 205, 209, 212, 216,
        220, 224, 228, 232, 236, 240, 244, 248, 252, 255, 259, 263, 266,
        269, 272, 276, 279, 284, 288, 292, 296, 300, 304, 308, 312, 316,
        320, 324, 328, 333, 337, 341, 345, 349, 353, 358, 362, 366, 370,
        374, 379, 383, 387, 391, 395, 399, 402, 406, 410, 414, 418, 422,
        426, 430, 434, 438, 442, 446, 448, 450, 452, 454, 456, 458, 460,
        462, 464, 466, 468, 470, 473, 476, 479, 482, 485, 488, 491, 494,
        497, 500, 503, 506, 509, 512, 515, 518, 520, 522, 524, 526, 528,
        530, 532, 534, 536, 538, 540, 542, 544, 546, 548, 550, 552, 554,
        556, 558, 560, 562, 565, 568, 570, 573, 577, 580, 583, 585, 588,
        590, 592, 594, 596, 598, 600, 603, 605, 607, 609, 612, 615, 617,
        620, 622, 626, 629, 632, 635, 638, 641, 645, 648, 650, 653, 657,
        660, 662, 664, 667, 669, 671, 674, 676, 678, 681, 684, 686, 688,
        690, 693, 695, 697, 699, 702, 705, 708, 710, 712, 715, 718, 721,
        723, 726, 728, 730, 733, 736, 739, 741, 744, 747, 750, 752, 755,
        757, 759, 761, 763, 767, 773, 778, 783, 788, 792, 799, 804, 811,
        816, 821, 827, 832, 836, 841, 846, 851, 856, 861, 868, 873, 880,
        885, 893, 898, 904, 912, 920, 925, 930, 935, 940, 945, 951, 955,
        960, 964, 969, 975, 981, 988, 996, 1002, 1007, 1011, 1016, 1021,
        1026, 1033, 1040, 1047, 1054, 1060, 1067, 1074, 1081, 1087,
        1094, 1100, 1106, 1113, 1121, 1128, 1136, 1143, 1151, 1156,
        1163, 1170, 1180, 1190, 1200, 1210, 1217, 1226, 1233, 1241,
        1248, 1256, 1264, 1271, 1279, 1286, 1292, 1299, 1306, 1313,
        1320, 1326, 1332, 1336, 1342, 1348, 1353, 1359, 1364, 1372,
        1381, 1391, 1396, 1401, 1405, 1410, 1414, 1419, 1424, 1429,
        1434, 1438, 1442, 1447, 1452, 1457, 1459, 1462, 1465, 1468,
        1470, 1472, 1475, 1478, 1480, 1483, 1486, 1489, 1491, 1493,
        1495, 1498, 1500, 1502, 1504, 1507, 1509, 1514, 1520, 1523,
        1526, 1529, 1534, 1538, 1543, 1547, 1551, 1554, 1558, 1562,
        1566, 1570, 1574, 1578, 1582, 1586, 1591, 1594, 1598, 1601,
        1605, 1609, 1612, 1616, 1620, 1625, 1628, 1632, 1636, 1640,
        1645, 1649
    };
    static const short off_len[] = {
        0, 115, 115, 6, 121, 22, 143, 21, 164, 43, 207,
        36, 243, 47, 290, 43, 333, 23, 356, 21, 377, 36
    };
    *offsets = offsets_table + off_len[n * 2 + 0];
    return off_len[n * 2 + 1];
}

static const char namegen_argz[] = {
    0x61, 0x63, 0x68, 0x00, 0x61, 0x63, 0x6b, 0x00, 0x61, 0x64, 0x00,
    0x61, 0x67, 0x65, 0x00, 0x61, 0x6c, 0x64, 0x00, 0x61, 0x6c, 0x65,
    0x00, 0x61, 0x6e, 0x00, 0x61, 0x6e, 0x67, 0x00, 0x61, 0x72, 0x00,
    0x61, 0x72, 0x64, 0x00, 0x61, 0x73, 0x00, 0x61, 0x73, 0x68, 0x00,
    0x61, 0x74, 0x00, 0x61, 0x74, 0x68, 0x00, 0x61, 0x75, 0x67, 0x68,
    0x00, 0x61, 0x77, 0x00, 0x62, 0x61, 0x6e, 0x00, 0x62, 0x65, 0x6c,
    0x00, 0x62, 0x75, 0x72, 0x00, 0x63, 0x65, 0x72, 0x00, 0x63, 0x68,
    0x61, 0x00, 0x63, 0x68, 0x65, 0x00, 0x64, 0x61, 0x6e, 0x00, 0x64,
    0x61, 0x72, 0x00, 0x64, 0x65, 0x6c, 0x00, 0x64, 0x65, 0x6e, 0x00,
    0x64, 0x72, 0x61, 0x00, 0x64, 0x79, 0x6e, 0x00, 0x65, 0x63, 0x68,
    0x00, 0x65, 0x6c, 0x64, 0x00, 0x65, 0x6c, 0x6d, 0x00, 0x65, 0x6d,
    0x00, 0x65, 0x6e, 0x00, 0x65, 0x6e, 0x64, 0x00, 0x65, 0x6e, 0x67,
    0x00, 0x65, 0x6e, 0x74, 0x68, 0x00, 0x65, 0x72, 0x00, 0x65, 0x73,
    0x73, 0x00, 0x65, 0x73, 0x74, 0x00, 0x65, 0x74, 0x00, 0x67, 0x61,
    0x72, 0x00, 0x67, 0x68, 0x61, 0x00, 0x68, 0x61, 0x74, 0x00, 0x68,
    0x69, 0x6e, 0x00, 0x68, 0x6f, 0x6e, 0x00, 0x69, 0x61, 0x00, 0x69,
    0x67, 0x68, 0x74, 0x00, 0x69, 0x6c, 0x64, 0x00, 0x69, 0x6d, 0x00,
    0x69, 0x6e, 0x61, 0x00, 0x69, 0x6e, 0x65, 0x00, 0x69, 0x6e, 0x67,
    0x00, 0x69, 0x72, 0x00, 0x69, 0x73, 0x00, 0x69, 0x73, 0x73, 0x00,
    0x69, 0x74, 0x00, 0x6b, 0x61, 0x6c, 0x00, 0x6b, 0x65, 0x6c, 0x00,
    0x6b, 0x69, 0x6d, 0x00, 0x6b, 0x69, 0x6e, 0x00, 0x6c, 0x65, 0x72,
    0x00, 0x6c, 0x6f, 0x72, 0x00, 0x6c, 0x79, 0x65, 0x00, 0x6d, 0x6f,
    0x72, 0x00, 0x6d, 0x6f, 0x73, 0x00, 0x6e, 0x61, 0x6c, 0x00, 0x6e,
    0x79, 0x00, 0x6e, 0x79, 0x73, 0x00, 0x6f, 0x6c, 0x64, 0x00, 0x6f,
    0x6d, 0x00, 0x6f, 0x6e, 0x00, 0x6f, 0x72, 0x00, 0x6f, 0x72, 0x6d,
    0x00, 0x6f, 0x73, 0x00, 0x6f, 0x75, 0x67, 0x68, 0x00, 0x70, 0x65,
    0x72, 0x00, 0x70, 0x6f, 0x6c, 0x00, 0x71, 0x75, 0x61, 0x00, 0x71,
    0x75, 0x65, 0x00, 0x72, 0x61, 0x64, 0x00, 0x72, 0x61, 0x6b, 0x00,
    0x72, 0x61, 0x6e, 0x00, 0x72, 0x61, 0x79, 0x00, 0x72, 0x69, 0x6c,
    0x00, 0x72, 0x69, 0x73, 0x00, 0x72, 0x6f, 0x64, 0x00, 0x72, 0x6f,
    0x74, 0x68, 0x00, 0x72, 0x79, 0x6e, 0x00, 0x73, 0x61, 0x6d, 0x00,
    0x73, 0x61, 0x79, 0x00, 0x73, 0x65, 0x72, 0x00, 0x73, 0x68, 0x79,
    0x00, 0x73, 0x6b, 0x65, 0x6c, 0x00, 0x73, 0x75, 0x6c, 0x00, 0x74,
    0x61, 0x69, 0x00, 0x74, 0x61, 0x6e, 0x00, 0x74, 0x61, 0x73, 0x00,
    0x74, 0x68, 0x65, 0x72, 0x00, 0x74, 0x69, 0x61, 0x00, 0x74, 0x69,
    0x6e, 0x00, 0x74, 0x6f, 0x6e, 0x00, 0x74, 0x6f, 0x72, 0x00, 0x74,
    0x75, 0x72, 0x00, 0x75, 0x6d, 0x00, 0x75, 0x6e, 0x64, 0x00, 0x75,
    0x6e, 0x74, 0x00, 0x75, 0x72, 0x6e, 0x00, 0x75, 0x73, 0x6b, 0x00,
    0x75, 0x73, 0x74, 0x00, 0x76, 0x65, 0x72, 0x00, 0x76, 0x65, 0x73,
    0x00, 0x76, 0x6f, 0x72, 0x00, 0x77, 0x61, 0x72, 0x00, 0x77, 0x6f,
    0x72, 0x00, 0x79, 0x65, 0x72, 0x00, 0x61, 0x00, 0x65, 0x00, 0x69,
    0x00, 0x6f, 0x00, 0x75, 0x00, 0x79, 0x00, 0x61, 0x00, 0x65, 0x00,
    0x69, 0x00, 0x6f, 0x00, 0x75, 0x00, 0x79, 0x00, 0x61, 0x65, 0x00,
    0x61, 0x69, 0x00, 0x61, 0x75, 0x00, 0x61, 0x79, 0x00, 0x65, 0x61,
    0x00, 0x65, 0x65, 0x00, 0x65, 0x69, 0x00, 0x65, 0x75, 0x00, 0x65,
    0x79, 0x00, 0x69, 0x61, 0x00, 0x69, 0x65, 0x00, 0x6f, 0x65, 0x00,
    0x6f, 0x69, 0x00, 0x6f, 0x6f, 0x00, 0x6f, 0x75, 0x00, 0x75, 0x69,
    0x00, 0x62, 0x00, 0x63, 0x00, 0x64, 0x00, 0x66, 0x00, 0x67, 0x00,
    0x68, 0x00, 0x6a, 0x00, 0x6b, 0x00, 0x6c, 0x00, 0x6d, 0x00, 0x6e,
    0x00, 0x70, 0x00, 0x71, 0x00, 0x72, 0x00, 0x73, 0x00, 0x74, 0x00,
    0x76, 0x00, 0x77, 0x00, 0x78, 0x00, 0x79, 0x00, 0x7a, 0x00, 0x62,
    0x00, 0x62, 0x6c, 0x00, 0x62, 0x72, 0x00, 0x63, 0x00, 0x63, 0x68,
    0x00, 0x63, 0x68, 0x72, 0x00, 0x63, 0x6c, 0x00, 0x63, 0x72, 0x00,
    0x64, 0x00, 0x64, 0x72, 0x00, 0x66, 0x00, 0x67, 0x00, 0x68, 0x00,
    0x6a, 0x00, 0x6b, 0x00, 0x6c, 0x00, 0x6c, 0x6c, 0x00, 0x6d, 0x00,
    0x6e, 0x00, 0x70, 0x00, 0x70, 0x68, 0x00, 0x71, 0x75, 0x00, 0x72,
    0x00, 0x72, 0x68, 0x00, 0x73, 0x00, 0x73, 0x63, 0x68, 0x00, 0x73,
    0x68, 0x00, 0x73, 0x6c, 0x00, 0x73, 0x6d, 0x00, 0x73, 0x6e, 0x00,
    0x73, 0x74, 0x00, 0x73, 0x74, 0x72, 0x00, 0x73, 0x77, 0x00, 0x74,
    0x00, 0x74, 0x68, 0x00, 0x74, 0x68, 0x72, 0x00, 0x74, 0x72, 0x00,
    0x76, 0x00, 0x77, 0x00, 0x77, 0x68, 0x00, 0x79, 0x00, 0x7a, 0x00,
    0x7a, 0x68, 0x00, 0x62, 0x00, 0x63, 0x00, 0x63, 0x68, 0x00, 0x63,
    0x6b, 0x00, 0x64, 0x00, 0x66, 0x00, 0x67, 0x00, 0x67, 0x68, 0x00,
    0x68, 0x00, 0x6b, 0x00, 0x6c, 0x00, 0x6c, 0x64, 0x00, 0x6c, 0x6c,
    0x00, 0x6c, 0x74, 0x00, 0x6d, 0x00, 0x6e, 0x00, 0x6e, 0x64, 0x00,
    0x6e, 0x6e, 0x00, 0x6e, 0x74, 0x00, 0x70, 0x00, 0x70, 0x68, 0x00,
    0x71, 0x00, 0x72, 0x00, 0x72, 0x64, 0x00, 0x72, 0x72, 0x00, 0x72,
    0x74, 0x00, 0x73, 0x00, 0x73, 0x68, 0x00, 0x73, 0x73, 0x00, 0x73,
    0x74, 0x00, 0x74, 0x00, 0x74, 0x68, 0x00, 0x76, 0x00, 0x77, 0x00,
    0x79, 0x00, 0x7a, 0x00, 0x61, 0x69, 0x72, 0x00, 0x61, 0x6e, 0x6b,
    0x6c, 0x65, 0x00, 0x62, 0x61, 0x6c, 0x6c, 0x00, 0x62, 0x65, 0x65,
    0x66, 0x00, 0x62, 0x6f, 0x6e, 0x65, 0x00, 0x62, 0x75, 0x6d, 0x00,
    0x62, 0x75, 0x6d, 0x62, 0x6c, 0x65, 0x00, 0x62, 0x75, 0x6d, 0x70,
    0x00, 0x63, 0x68, 0x65, 0x65, 0x73, 0x65, 0x00, 0x63, 0x6c, 0x6f,
    0x64, 0x00, 0x63, 0x6c, 0x6f, 0x74, 0x00, 0x63, 0x6c, 0x6f, 0x77,
    0x6e, 0x00, 0x63, 0x6f, 0x72, 0x6e, 0x00, 0x64, 0x69, 0x70, 0x00,
    0x64, 0x6f, 0x6c, 0x74, 0x00, 0x64, 0x6f, 0x6f, 0x66, 0x00, 0x64,
    0x6f, 0x72, 0x6b, 0x00, 0x64, 0x75, 0x6d, 0x62, 0x00, 0x66, 0x61,
    0x63, 0x65, 0x00, 0x66, 0x69, 0x6e, 0x67, 0x65, 0x72, 0x00, 0x66,
    0x6f, 0x6f, 0x74, 0x00, 0x66, 0x75, 0x6d, 0x62, 0x6c, 0x65, 0x00,
    0x67, 0x6f, 0x6f, 0x66, 0x00, 0x67, 0x72, 0x75, 0x6d, 0x62, 0x6c,
    0x65, 0x00, 0x68, 0x65, 0x61, 0x64, 0x00, 0x6b, 0x6e, 0x6f, 0x63,
    0x6b, 0x00, 0x6b, 0x6e, 0x6f, 0x63, 0x6b, 0x65, 0x72, 0x00, 0x6b,
    0x6e, 0x75, 0x63, 0x6b, 0x6c, 0x65, 0x00, 0x6c, 0x6f, 0x61, 0x66,
    0x00, 0x6c, 0x75, 0x6d, 0x70, 0x00, 0x6c, 0x75, 0x6e, 0x6b, 0x00,
    0x6d, 0x65, 0x61, 0x74, 0x00, 0x6d, 0x75, 0x63, 0x6b, 0x00, 0x6d,
    0x75, 0x6e, 0x63, 0x68, 0x00, 0x6e, 0x69, 0x74, 0x00, 0x6e, 0x75,
    0x6d, 0x62, 0x00, 0x70, 0x69, 0x6e, 0x00, 0x70, 0x75, 0x66, 0x66,
    0x00, 0x73, 0x6b, 0x75, 0x6c, 0x6c, 0x00, 0x73, 0x6e, 0x61, 0x72,
    0x6b, 0x00, 0x73, 0x6e, 0x65, 0x65, 0x7a, 0x65, 0x00, 0x74, 0x68,
    0x69, 0x6d, 0x62, 0x6c, 0x65, 0x00, 0x74, 0x77, 0x65, 0x72, 0x70,
    0x00, 0x74, 0x77, 0x69, 0x74, 0x00, 0x77, 0x61, 0x64, 0x00, 0x77,
    0x69, 0x6d, 0x70, 0x00, 0x77, 0x69, 0x70, 0x65, 0x00, 0x62, 0x61,
    0x62, 0x79, 0x00, 0x62, 0x6f, 0x6f, 0x62, 0x6c, 0x65, 0x00, 0x62,
    0x75, 0x6e, 0x6b, 0x65, 0x72, 0x00, 0x63, 0x75, 0x64, 0x64, 0x6c,
    0x65, 0x00, 0x63, 0x75, 0x64, 0x64, 0x6c, 0x79, 0x00, 0x63, 0x75,
    0x74, 0x69, 0x65, 0x00, 0x64, 0x6f, 0x6f, 0x64, 0x6c, 0x65, 0x00,
    0x66, 0x6f, 0x6f, 0x66, 0x69, 0x65, 0x00, 0x67, 0x6f, 0x6f, 0x62,
    0x6c, 0x65, 0x00, 0x68, 0x6f, 0x6e, 0x65, 0x79, 0x00, 0x6b, 0x69,
    0x73, 0x73, 0x69, 0x65, 0x00, 0x6c, 0x6f, 0x76, 0x65, 0x72, 0x00,
    0x6c, 0x6f, 0x76, 0x65, 0x79, 0x00, 0x6d, 0x6f, 0x6f, 0x66, 0x69,
    0x65, 0x00, 0x6d, 0x6f, 0x6f, 0x67, 0x6c, 0x69, 0x65, 0x00, 0x6d,
    0x6f, 0x6f, 0x70, 0x69, 0x65, 0x00, 0x6d, 0x6f, 0x6f, 0x70, 0x73,
    0x69, 0x65, 0x00, 0x6e, 0x6f, 0x6f, 0x6b, 0x75, 0x6d, 0x00, 0x70,
    0x6f, 0x6f, 0x63, 0x68, 0x69, 0x65, 0x00, 0x70, 0x6f, 0x6f, 0x66,
    0x00, 0x70, 0x6f, 0x6f, 0x66, 0x69, 0x65, 0x00, 0x70, 0x6f, 0x6f,
    0x6b, 0x69, 0x65, 0x00, 0x73, 0x63, 0x68, 0x6d, 0x6f, 0x6f, 0x70,
    0x69, 0x65, 0x00, 0x73, 0x63, 0x68, 0x6e, 0x6f, 0x6f, 0x67, 0x6c,
    0x65, 0x00, 0x73, 0x63, 0x68, 0x6e, 0x6f, 0x6f, 0x6b, 0x69, 0x65,
    0x00, 0x73, 0x63, 0x68, 0x6e, 0x6f, 0x6f, 0x6b, 0x75, 0x6d, 0x00,
    0x73, 0x6d, 0x6f, 0x6f, 0x63, 0x68, 0x00, 0x73, 0x6d, 0x6f, 0x6f,
    0x63, 0x68, 0x69, 0x65, 0x00, 0x73, 0x6d, 0x6f, 0x6f, 0x73, 0x68,
    0x00, 0x73, 0x6e, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x00, 0x73, 0x6e,
    0x6f, 0x6f, 0x67, 0x79, 0x00, 0x73, 0x6e, 0x6f, 0x6f, 0x6b, 0x69,
    0x65, 0x00, 0x73, 0x6e, 0x6f, 0x6f, 0x6b, 0x75, 0x6d, 0x00, 0x73,
    0x6e, 0x75, 0x67, 0x67, 0x79, 0x00, 0x73, 0x77, 0x65, 0x65, 0x74,
    0x69, 0x65, 0x00, 0x77, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x00, 0x77,
    0x6f, 0x6f, 0x67, 0x79, 0x00, 0x77, 0x6f, 0x6f, 0x6b, 0x69, 0x65,
    0x00, 0x77, 0x6f, 0x6f, 0x6b, 0x75, 0x6d, 0x00, 0x77, 0x75, 0x64,
    0x64, 0x6c, 0x65, 0x00, 0x77, 0x75, 0x64, 0x64, 0x6c, 0x79, 0x00,
    0x77, 0x75, 0x67, 0x67, 0x79, 0x00, 0x77, 0x75, 0x6e, 0x6e, 0x79,
    0x00, 0x62, 0x6f, 0x6f, 0x00, 0x62, 0x75, 0x6e, 0x63, 0x68, 0x00,
    0x62, 0x75, 0x6e, 0x6e, 0x79, 0x00, 0x63, 0x61, 0x6b, 0x65, 0x00,
    0x63, 0x61, 0x6b, 0x65, 0x73, 0x00, 0x63, 0x75, 0x74, 0x65, 0x00,
    0x64, 0x61, 0x72, 0x6c, 0x69, 0x6e, 0x67, 0x00, 0x64, 0x75, 0x6d,
    0x70, 0x6c, 0x69, 0x6e, 0x67, 0x00, 0x64, 0x75, 0x6d, 0x70, 0x6c,
    0x69, 0x6e, 0x67, 0x73, 0x00, 0x66, 0x61, 0x63, 0x65, 0x00, 0x66,
    0x6f, 0x6f, 0x66, 0x00, 0x67, 0x6f, 0x6f, 0x00, 0x68, 0x65, 0x61,
    0x64, 0x00, 0x6b, 0x69, 0x6e, 0x00, 0x6b, 0x69, 0x6e, 0x73, 0x00,
    0x6c, 0x69, 0x70, 0x73, 0x00, 0x6c, 0x6f, 0x76, 0x65, 0x00, 0x6d,
    0x75, 0x73, 0x68, 0x00, 0x70, 0x69, 0x65, 0x00, 0x70, 0x6f, 0x6f,
    0x00, 0x70, 0x6f, 0x6f, 0x68, 0x00, 0x70, 0x6f, 0x6f, 0x6b, 0x00,
    0x70, 0x75, 0x6d, 0x73, 0x00, 0x62, 0x00, 0x62, 0x6c, 0x00, 0x62,
    0x72, 0x00, 0x63, 0x6c, 0x00, 0x64, 0x00, 0x66, 0x00, 0x66, 0x6c,
    0x00, 0x66, 0x72, 0x00, 0x67, 0x00, 0x67, 0x68, 0x00, 0x67, 0x6c,
    0x00, 0x67, 0x72, 0x00, 0x68, 0x00, 0x6a, 0x00, 0x6b, 0x00, 0x6b,
    0x6c, 0x00, 0x6d, 0x00, 0x6e, 0x00, 0x70, 0x00, 0x74, 0x68, 0x00,
    0x77, 0x00, 0x65, 0x6c, 0x63, 0x68, 0x00, 0x69, 0x64, 0x69, 0x6f,
    0x74, 0x00, 0x6f, 0x62, 0x00, 0x6f, 0x67, 0x00, 0x6f, 0x6b, 0x00,
    0x6f, 0x6c, 0x70, 0x68, 0x00, 0x6f, 0x6c, 0x74, 0x00, 0x6f, 0x6d,
    0x70, 0x68, 0x00, 0x6f, 0x6e, 0x67, 0x00, 0x6f, 0x6e, 0x6b, 0x00,
    0x6f, 0x6f, 0x00, 0x6f, 0x6f, 0x62, 0x00, 0x6f, 0x6f, 0x66, 0x00,
    0x6f, 0x6f, 0x67, 0x00, 0x6f, 0x6f, 0x6b, 0x00, 0x6f, 0x6f, 0x7a,
    0x00, 0x6f, 0x72, 0x67, 0x00, 0x6f, 0x72, 0x6b, 0x00, 0x6f, 0x72,
    0x6d, 0x00, 0x6f, 0x72, 0x6f, 0x6e, 0x00, 0x75, 0x62, 0x00, 0x75,
    0x63, 0x6b, 0x00, 0x75, 0x67, 0x00, 0x75, 0x6c, 0x66, 0x00, 0x75,
    0x6c, 0x74, 0x00, 0x75, 0x6d, 0x00, 0x75, 0x6d, 0x62, 0x00, 0x75,
    0x6d, 0x70, 0x00, 0x75, 0x6d, 0x70, 0x68, 0x00, 0x75, 0x6e, 0x00,
    0x75, 0x6e, 0x62, 0x00, 0x75, 0x6e, 0x67, 0x00, 0x75, 0x6e, 0x6b,
    0x00, 0x75, 0x6e, 0x70, 0x68, 0x00, 0x75, 0x6e, 0x74, 0x00, 0x75,
    0x7a, 0x7a, 0x00
};

static unsigned long
namegen_rand32(unsigned long *s)
{
    unsigned long x = *s;
    x ^= x << 13;
    x ^= (x & 0xffffffffUL) >> 17;
    x ^= x << 5;
    return (*s = x) & 0xffffffffUL;
}

struct namegen {
    const char *pattern;
    char *dst;
    size_t len;
    unsigned long *rng;
};

/* Return a random token from the current group.
 */
static int
namegen_randtok(struct namegen *ng, const char **beg, const char **end)
{
    int nest = 0;
    unsigned long n = 0;
    const char *cur_beg = ng->pattern;

    *end = *beg = ng->pattern;
    for (;;) {
        int c = *ng->pattern;
        if (nest) {
            switch (c) {
                case 0:
                    return NAMEGEN_INVALID;
                case '<':
                case '(':
                    nest++;
                    break;
                case '>':
                case ')':
                    nest--;
                    break;
            }
        } else {
            switch (c) {
                case 0:
                case '>':
                case ')':
                    if (!n++ || namegen_rand32(ng->rng) < 0xffffffffUL / n) {
                        *beg = cur_beg;
                        *end = ng->pattern;
                    }
                    return NAMEGEN_SUCCESS;
                case '<':
                case '(':
                    nest++;
                    break;
                case '|':
                    if (!n++ || namegen_rand32(ng->rng) < 0xffffffffUL / n) {
                        *beg = cur_beg;
                        *end = ng->pattern;
                    }
                    cur_beg = ng->pattern + 1;
                    break;
            }
        }
        ng->pattern++;
    }
}

static int
namegen_recurse(struct namegen *ng, int literal)
{
    int r;
    int capitalize = 0;
    const char *beg, *end;

    /* Fetch a random token */
    r = namegen_randtok(ng, &beg, &end);
    if (r != NAMEGEN_SUCCESS)
        return r;

    /* Generate the selected token */
    for (; ng->len && beg < end; beg++) {
        int n;
        int c = *beg;
        char *init_dst = ng->dst;

        /* Recurse into a subgroup */
        if (c == '(' || c == '<') {
            int e;
            const char *save = ng->pattern;
            ng->pattern = beg + 1;
            r = namegen_recurse(ng, c == '(');
            beg = ng->pattern;
            e = *ng->pattern;
            ng->pattern = save;
            if (r != NAMEGEN_SUCCESS)
                return r;
            if (e != (c == '(' ? ')' : '>'))
                return NAMEGEN_INVALID;

        /* Capitalize the next thing */
        } else if (c == '!') {
            capitalize = 1;

        /* Pass through a literal */
        } else if (literal || (n = namegen_special(c)) == -1) {
            *ng->dst++ = c;
            ng->len--;

        /* Draw from a pre-defined template */
        } else {
            const short *offsets;
            int count = namegen_offsets(n, &offsets);
            int select = namegen_rand32(ng->rng) % count;
            const char *s = namegen_argz + offsets[select];
            while (*s && ng->len) {
                *ng->dst++ = *s++;
                ng->len--;
            }
        }

        /* Apply capitalization */
        if (capitalize && init_dst != ng->dst) {
            capitalize = 0;
            if (*init_dst >= 'a' && *init_dst <= 'z')
                *init_dst &= ~0x20;
        }
    }

    /* Terminate and return */
    if (!ng->len) {
        ng->dst[-1] = 0;
        return NAMEGEN_TRUNCATED;
    } else {
        ng->dst[0] = 0;
        return NAMEGEN_SUCCESS;
    }
}

static int
namegen(char *dst, size_t len, const char *pattern, unsigned long *seed)
{
    int r;
    struct namegen ng;
    ng.pattern = pattern;
    ng.dst = dst;
    ng.len = len;
    ng.rng = seed;
    r = namegen_recurse(&ng, 0);
    if (*ng.pattern)
        return NAMEGEN_INVALID; /* Did not end at the terminator */
    return r;
}

#endif
