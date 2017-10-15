/*
 *    INTRODUCTION TO COMPUTER SCIENCE COURSE IN THE SPRING 2015 FROM NCTU
 *    THIS FILE IS PART OF THE FINAL PROJECT PROVIDING BY THE SECOND GROUP
 *    CONTRIBUTORS: Dong Nai-Jia
 */

#ifndef _DEF_H_
#define _DEF_H_


/* Item ID */
typedef unsigned char ItemID;
#define DISTRICT_START      0x01
#define DISTRICT_END        0x10
#define DISTRICT_NUM          16

#define STREAM_START        0x11
#define STREAM_END          0x12
#define STREAM_NUM             2

#define POI_START           0x13
#define POI_END             0x2B
#define POI_NUM               25

#define ITEM_NUM ((DISTRICT_NUM) + (STREAM_NUM) + (POI_NUM))


/* Color ID */
typedef unsigned long ColorID;
#define COLOR_R       0x00CB0C29    /* Color: Red      */
#define COLOR_Y       0x00CB6E00    /* Color: Yellow   */
#define COLOR_G       0x0049A835    /* Color: Green    */
#define COLOR_B       0x002D7DB3    /* Color: Blue     */
#define COLOR_BK      0x00000000    /* Color: Black    */
#define COLOR_WH      0x00FFFFFF    /* Color: White    */
#define COLOR_HL_R    0x00FF0000    /* Color: (For HL) */
#define COLOR_HL_B    0x000000FF    /* Color: (For HL) */
#define COLOR_NUM              8


/* Alpha ID */
typedef unsigned long AlphaID;
#define TRANSPARENT   0x00000000    /* Transparent   */
#define TRANSLUCENT_1 0x3F000000    /* Translucent_1 */
#define TRANSLUCENT_2 0x7F000000    /* Translucent_2 */
#define OPAQUE        0xFF000000    /* Opaque        */
#define ALPHA_NUM              4


/* Line thickness */
typedef unsigned char LineID;
#define THIN                   1
#define THICK                  5
#define LINE_NUM               2


/* Graphic context ID */
typedef unsigned char GCID;
#define GC_COLOR_R          0x00
#define GC_COLOR_Y          0x01
#define GC_COLOR_G          0x02
#define GC_COLOR_B          0x03
#define GC_COLOR_BK         0x04
#define GC_COLOR_WH         0x05
#define GC_COLOR_STR        0x07
#define GC_COLOR_POI        0x07
#define GC_COLOR_POI_HL     0x06
#define GC_COLOR_NULL       0x04
#define COLOR_BITS             3

#define GC_TRANSPARENT     (0x00 << (COLOR_BITS))
#define GC_TRANSLUCENT_1   (0x01 << (COLOR_BITS))
#define GC_TRANSLUCENT_2   (0x02 << (COLOR_BITS))
#define GC_OPAQUE          (0x03 << (COLOR_BITS))
#define ALPHA_BITS             2

#define GC_THIN            (0x00 << ((COLOR_BITS) + (ALPHA_BITS)))
#define GC_THICK           (0x01 << ((COLOR_BITS) + (ALPHA_BITS)))
#define LINE_BITS              1

#define GC_OR              (0x00 << ((COLOR_BITS) + (ALPHA_BITS) + (LINE_BITS)))
#define GC_COPY            (0x01 << ((COLOR_BITS) + (ALPHA_BITS) + (LINE_BITS)))
#define FUNC_BITS              1

#define GC_NUM  (1 << ((COLOR_BITS) + (ALPHA_BITS) + (FUNC_BITS) + (LINE_BITS)))


/* For ITEM_TABLE */
typedef struct _Item
{
    ItemID  id;
    GCID  gcid;
    char *name_zh, *name_en;
} Item;


/* Coordinate for district, stream and POI */
typedef struct _Coord
{
    short x, y;
} Coord;


/* Formatted result from query_by_command */
typedef struct _Result
{
    ItemID *id;
    char ***msg;
} Result;


#endif
