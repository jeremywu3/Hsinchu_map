/*
 *    INTRODUCTION TO COMPUTER SCIENCE COURSE IN THE SPRING 2015 FROM NCTU
 *    THIS FILE IS PART OF THE FINAL PROJECT PROVIDING BY THE SECOND GROUP
 *    CONTRIBUTORS: Dong Nai-Jia
 */

#ifndef _UI_H_
#define _UI_H_


#include "def.h"
#include <stdbool.h>

#include <X11/Xlib.h>

#include <mysql/mysql.h>


/* Map definition */
#define MAP_MAX_W           1024
#define MAP_MAX_H            768
#define MAP_DEPTH             24
#define MAP_DEPTH_PER_BYTE     8
#define MAP_COLORS           255
#define MAP_BYTES_PER_PIXEL ((MAP_DEPTH) / (MAP_DEPTH_PER_BYTE))
#define MAP_TOTAL_BYTES(W, H) ((W) * (H) * (MAP_BYTES_PER_PIXEL))


/* X Pixmap macro */
#define XPM_MAX_W      MAP_MAX_W
#define XPM_MAX_H      MAP_MAX_H
#define XPM_DEPTH             32
#define XPM_DEPTH_PER_BYTE     8
#define XPM_BYTES_PER_PIXEL ((XPM_DEPTH) / (XPM_DEPTH_PER_BYTE))
#define XPM_TOTAL_BYTES(W, H) ((W) * (H) * (XPM_BYTES_PER_PIXEL))


/* Window */
#define WIN_MAX_W      MAP_MAX_W
#define WIN_MAX_H      MAP_MAX_H
#define WIN_DEPTH             24
#define WIN_DEPTH_PER_BYTE     8
#define WIN_BYTES_PER_PIXEL ((WIN_DEPTH) / (WIN_DEPTH_PER_BYTE))


/* Layer ID */
typedef unsigned char LayerID;
#define LAYER_BUFFER        0x00    /* Layer: (buffer)        */
#define LAYER_B             0x01    /* Layer: Background      */
#define LAYER_O             0x02    /* Layer: Item Outline    */
#define LAYER_H             0x03    /* Layer: Item Highlight  */
#define LAYER_I             0x04    /* Layer: User Interface  */

#define LAYER_BOTTOM        0x01    /* Bottom                 */
#define LAYER_TOP           0x04    /* Top                    */

#define LAYER_NUM              5


/* Area ID (in Redraw) */
#define AREA_LAST           0x00
#define AREA_THIS           0x01
#define AREA_NUM               2


/* Background ID */
typedef unsigned char BackgroundID;
#define STREET_MAP          0x00
#define SATELLITE_IMAGE     0x01
#define BACKGROUND_WHITE    0x02
#define BACKGROUND_BLACK    0x03


/* Size */
#define POI_R                  3
#define POI_D                  7
#define POI_HL_R               7
#define POI_HL_D              15
#define POI_MAX_R              7
#define POI_MAX_D             15


/* Miscellaneous macro */
#define SELECT_MAX(x, y) (((x) < (y)) ? (y) : (x))
#define SELECT_MIN(x, y) (((x) > (y)) ? (y) : (x))

#define IS_INCR(x, y, z)          ((x) <= (y) && (y) <= (z))
#define IS_STRICTLY_INCR(x, y, z) ((x) <  (y) && (y) <  (z))

#define IS_DISTRICT(x) IS_INCR((DISTRICT_START), (x), (DISTRICT_END))
#define IS_STREAM(x)   IS_INCR((STREAM_START)  , (x), (STREAM_END)  )
#define IS_POI(x)      IS_INCR((POI_START)     , (x), (POI_END)     )


/* Miscellaneous definition */
#define MATH_CONST_PI 3.14159265358979323846

#define ITEM_NAME_OFS_X_BYTES  1
#define ITEM_NAME_OFS_Y_BYTES -1
#define ITEM_NAME_MAR_X_PIXELS 8
#define ITEM_NAME_MAR_Y_PIXELS 8

#define MESSAGE_OFS_X_BYTES    2
#define MESSAGE_OFS_Y_BYTES    2
#define MESSAGE_MAR_X_PIXELS   8
#define MESSAGE_MAR_Y_PIXELS   8
#define MESSAGE_LINE_SPACES_PIXEL 20


/* Constant string */
const char WIN_TITLENAME[]            = "Hsinchu Map";
const char STREET_MAP_LOCATION[]      = "image/Google Street.ppm";
const char SATELLITE_IMAGE_LOCATION[] = "image/Google Satellite.ppm";


/* GC structure */
typedef struct _GCs
{
    GC           gc[GC_NUM];
    GC           gc_24bit_rgb;
    XGCValues    xgcv[GC_NUM];
    XGCValues    xgcv_24bit_rgb;
    XFontStruct *font_info;
    char        *font_name;
    int          font_ascent;
} GCs;


/* Layer structure */
typedef struct _Layers
{
    Pixmap  pxm[LAYER_NUM];
    XImage *xmg[LAYER_NUM];
} Layers;


/* Area Structure */
typedef struct _Area
{
    short x, y;
    unsigned short width, height;
} Area;


/* Redraw structure */
typedef struct _Redraw
{
    bool exception;
    Area area[AREA_NUM];
} Redraw;


/* Background type */
typedef struct _Bright
{
    bool background;
} Bright;


/* Initialize layers and gcs */
void init_structs(Display*, Visual*, Window, GCs*, Layers*, Redraw*, Bright*);


/* Load PPM P6 image file to a pixmap */
int  load_p6_file(Layers*, const char*);
#define LOAD_SUCCESS    0
#define LOAD_FAILURE    1


/* Reset Layer(s) to all zeros */
void reset_one_layer  (Display*, Layers*, LayerID, GCs*);
void reset_every_layer(Display*, Layers*, GCs*);


/* Load or reload a specified layer */
void load_map_background(Display*, Window, Layers*, GCs*, Coord***, Redraw*,
                         BackgroundID, Bright*);
void load_item_generic  (Display*, Window, Layers*, GCs*, Coord***, Redraw*,
                         Result*, ItemID*, Bright*);
void load_item_outline  (Display*, Window, Layers*, GCs*, Coord***, Redraw*,
                         Bright*);
void load_item_highlight(Display*, Window, Layers*, GCs*, Coord***, Redraw*,
                         Result*, ItemID*, Bright*);
void load_user_interface(Display*, Layers*, GCs*);
void reload_buffer_layer(Display*, Layers*, GCs*, Redraw*);
void map_onto_the_window(Display*, Window , Layers*, GCs*);


/* Find Result or ItemID by querying database or by coordinate respectively */
bool exec_sql_statement( /* _IN__ */ MYSQL*,
                         /* _OUT_ */ Result*);
void coord_to_item_list( /* _IN__ */ Coord***, short, short,
                         /* _OUT_ */ ItemID(*)[2]);


/* Redraw area */
void set_redraw_area  (Redraw*, short, short, unsigned short, unsigned short);
void reset_redraw_area(Redraw*);
void clear_redraw_area(Redraw*);
void store_redraw_area(Redraw*);
bool check_redraw_area(Redraw*);
void union_redraw_area( /* _IN__ */ Area*, Area*,
                        /* _OUT_ */ Area*);


/* Get item's area */
void get_area_of_item( /* _IN__ */ Coord***, ItemID,
                       /* _OUT_ */ Area*);


/* Deallocate memory */
void free_result(Result*);


/* Miscellaneous */
size_t coord_ptr_len(Coord**);


#endif
