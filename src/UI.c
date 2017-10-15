/*
 *    INTRODUCTION TO COMPUTER SCIENCE COURSE IN THE SPRING 2015 FROM NCTU
 *    THIS FILE IS PART OF THE FINAL PROJECT PROVIDING BY THE SECOND GROUP
 *    CONTRIBUTORS: Dong Nai-Jia
 */

/* For popen() */
#define _GNU_SOURCE

#include "UI.h"
#include "query.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <mysql/mysql.h>


extern const Item    ITEM_TABLE[];
extern const LineID  LINE_TABLE[];
extern const AlphaID ALPHA_TABLE[];
extern const ColorID COLOR_TABLE[];


int main (void)
{
    /* Open a display */
    Display *dsp = XOpenDisplay(NULL);
    int      scr = DefaultScreen(dsp);
    Window   win_root = DefaultRootWindow(dsp);

    /* Receive a matched visual (usually 24-bit depth RGB) */
    XVisualInfo vis_info;
    XMatchVisualInfo(dsp, scr, WIN_DEPTH, TrueColor, &vis_info);
    Visual *vis = vis_info.visual;

    /* Set window attribute */
    XSetWindowAttributes attr;
    attr.colormap = XCreateColormap(dsp, win_root, vis_info.visual, AllocNone);
    attr.border_pixel      = 1;
    attr.background_pixel  = OPAQUE | COLOR_BK;
    attr.override_redirect = False;
    Window win = XCreateWindow(dsp, win_root, 0, 0, WIN_MAX_W, WIN_MAX_H,
                               0, vis_info.depth, InputOutput, vis_info.visual,
                               CWColormap | CWBackPixel |
                               CWOverrideRedirect | CWBorderPixel, &attr);

    Atom wm_delete = XInternAtom(dsp, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dsp, win, &wm_delete, 1);

    XSelectInput(dsp, win, ExposureMask |
                           ButtonPressMask | KeyPressMask | PointerMotionMask);

    XStoreName(dsp, win, WIN_TITLENAME);

    XMapWindow(dsp, win);

    XEvent xev;

    do
        XNextEvent(dsp, &xev);
    while (xev.type != Expose);

    GCs    gcs;
    Layers layers;
    Redraw redraw;
    Bright bright;

    init_structs(dsp, vis, win, &gcs, &layers, &redraw, &bright);
    reset_every_layer(dsp, &layers, &gcs);

    Coord ***coord;
    coord = malloc((ITEM_NUM + 2) * sizeof(Coord**));

    MYSQL mysql;
    query_init(&mysql, "database20150615");
    query_item_coord(&mysql, coord);

    /* Generate initial screen */
    load_map_background(dsp, win, &layers, &gcs,
                        coord, &redraw, STREET_MAP, &bright);
    load_item_outline(dsp, win, &layers, &gcs, coord, &redraw, &bright);
    reload_buffer_layer(dsp, &layers, &gcs, &redraw);
    map_onto_the_window(dsp, win, &layers, &gcs);

    /* If locked is set as true,
       then every input will be discarded
       EXCEPT 5, F5 and Escape, which can turn locked into false */
    bool locked = false;

    while (true)
    {
        /* Discard all events in the queue in order to avoid delay */
        while (XPending(dsp) > 1)
            XNextEvent(dsp, &xev);
        XNextEvent(dsp, &xev);

        /* Waiting for unlock */
        if (locked)
        {
            if (xev.type == KeyPress
             && (XLookupKeysym(&xev.xkey, 0) == XK_5
              || XLookupKeysym(&xev.xkey, 0) == XK_F5
              || XLookupKeysym(&xev.xkey, 0) == XK_Escape))
                locked = false;
            else
                continue;
        }

        switch (xev.type)
        {
            case KeyPress:
            switch (XLookupKeysym(&xev.xkey, 0))
            {
                case XK_1: case XK_F1: case XK_M: case XK_m:
                load_map_background(dsp, win, &layers, &gcs,
                                    coord, &redraw, STREET_MAP, &bright);
                break;

                case XK_2: case XK_F2: case XK_S: case XK_s:
                load_map_background(dsp, win, &layers, &gcs,
                                    coord, &redraw, SATELLITE_IMAGE, &bright);
                break;

                case XK_3: case XK_F3: case XK_W: case XK_w:
                load_map_background(dsp, win, &layers, &gcs,
                                    coord, &redraw, BACKGROUND_WHITE, &bright);
                break;

                case XK_4: case XK_F4: case XK_B: case XK_b:
                load_map_background(dsp, win, &layers, &gcs,
                                    coord, &redraw, BACKGROUND_BLACK, &bright);
                break;

                case XK_6: case XK_F6: case XK_Q: case XK_q:
                {
                    /* Get a result from executing an SQL statement */
                    Result result;
                    if(exec_sql_statement(&mysql, &result) != true)
                        continue;

                    load_item_highlight(dsp, win, &layers, &gcs,
                                        coord, &redraw, &result, NULL, &bright);
                    free_result(&result);

                    locked = true;
                }
                break;

                default:
                /* Don't redraw the window, and wait for the next event */
                continue;
            }
            break;

            case MotionNotify:
            {
                /* Get an id list by current cursor position */
                ItemID id[2];
                coord_to_item_list(coord, xev.xmotion.x, xev.xmotion.y, &id);

                /* If the cursur points to an item, then highlight it */
                /* If the cursur points to nothing, then reset layers */
                if (id[0] != ITEM_TABLE[0].id)
                    load_item_highlight(dsp, win, &layers, &gcs,
                                        coord, &redraw, NULL, id, &bright);
                else
                    reset_one_layer(dsp, &layers, LAYER_H, &gcs);
            }
            break;

            case ClientMessage:
            {
                query_quit(&mysql);
                printf("\nWindow closed.\n");
                return EXIT_SUCCESS;
            }
            break;

            default:
            /* Don't redraw the window, and wait for the next event */
            continue;
        }

        /* Redraw the window */
        reload_buffer_layer(dsp, &layers, &gcs, &redraw);
        map_onto_the_window(dsp, win, &layers, &gcs);
        reset_one_layer(dsp, &layers, LAYER_I, &gcs);
        store_redraw_area(&redraw);
        redraw.exception = false;
    }

    return EXIT_FAILURE;
}


void init_structs(Display *dsp, Visual *vis, Window win,
                  GCs *gcs, Layers *layers, Redraw *redraw, Bright *bright)
{
    /* Create all X Pixmaps (ONLY LAYER_BUFFER is 24-bit depth) */
    for (int cnt = 0; cnt < LAYER_NUM; cnt++)
        layers->pxm[cnt] = XCreatePixmap(dsp, win, XPM_MAX_W, XPM_MAX_H,
                                         (cnt == LAYER_BUFFER) ?
                                         WIN_DEPTH : XPM_DEPTH);

    /* Create an XImage of LAYER_B to contain background images */
    char *data_b = malloc(XPM_TOTAL_BYTES(XPM_MAX_W, XPM_MAX_H));
    layers->xmg[LAYER_B] = XCreateImage(dsp, vis, XPM_DEPTH,
                                        ZPixmap, 0, data_b,
                                        XPM_MAX_W, XPM_MAX_H, 32, 0);

    /* Create an XImage of LAYER_BUFFER for frame buffering */
    char *data_buffer = malloc(XPM_TOTAL_BYTES(XPM_MAX_W, XPM_MAX_H));
    layers->xmg[LAYER_BUFFER] = XCreateImage(dsp, vis, WIN_DEPTH,
                                             ZPixmap, 0, data_buffer,
                                             XPM_MAX_W, XPM_MAX_H, 32, 0);

    /* Load specified font */
    gcs->font_name   = "8x16";
    gcs->font_info   = XLoadQueryFont(dsp, gcs->font_name);
    gcs->font_ascent = gcs->font_info->ascent;

    /* Create several graphics context (GC) */
    for (int cnt = 0,
             cnt_1 = 0; cnt_1 < (1 << FUNC_BITS); cnt_1++)
        for (int cnt_2 = 0; cnt_2 < (1 << LINE_BITS); cnt_2++)
            for (int cnt_3 = 0; cnt_3 < (1 << ALPHA_BITS); cnt_3++)
                for (int cnt_4 = 0; cnt_4 < (1 << COLOR_BITS); cnt_4++, cnt++)
                {
                    gcs->xgcv[cnt].font       = gcs->font_info->fid;
                    gcs->xgcv[cnt].function   = (cnt_1 == 0) ? GXor : GXcopy;
                    gcs->xgcv[cnt].line_width = LINE_TABLE[cnt_2];
                    gcs->xgcv[cnt].foreground = ALPHA_TABLE[cnt_3] |
                                                COLOR_TABLE[cnt_4];
                    gcs->gc[cnt] = XCreateGC(dsp, layers->pxm[LAYER_B],
                                             GCFont | GCFunction |
                                             GCForeground | GCLineWidth,
                                             &gcs->xgcv[cnt]);
                }

    /* Create an graphics context for
       XPutImage() and XCopyArea() on LAYER_BUFFER:
       24-bit XImage -> 24-bit X Pixmap -> 24-bit Window */
    gcs->gc_24bit_rgb = XCreateGC(dsp, layers->pxm[LAYER_BUFFER],
                                  0, &gcs->xgcv_24bit_rgb);

    reset_redraw_area(redraw);
    redraw->exception = false;

    bright->background = true;
}


int load_p6_file(Layers *layers, const char *path)
{
    char magic_number[3];
    int  width, height, colors;

    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return LOAD_FAILURE;

    if (fscanf(fp, "%s %d %d %d", magic_number, &width, &height, &colors) != 4)
        return LOAD_FAILURE;

    fseek(fp, 1, SEEK_CUR);

    /* Check PPM type and size */
    if (magic_number[0] != 'P' || magic_number[1] != '6' ||
        width != MAP_MAX_W || height != MAP_MAX_H || colors != MAP_COLORS)
        return LOAD_FAILURE;

    char *map = malloc(MAP_TOTAL_BYTES(width, height));
    int ret = fread(map, MAP_BYTES_PER_PIXEL, width * height, fp);
    fclose(fp);
    if (ret != width * height)
        return LOAD_FAILURE;

    /* Load PPM to X Pixmap and set alpha value as 0xFF */
    for (int cnt_1 = 0, cnt_3 = 0; cnt_1 < width * height; cnt_1++)
        for (int cnt_2 = 3; cnt_2 >= 0; cnt_2--)
            if (cnt_2 == 3)
                layers->xmg[LAYER_B]->
                data[cnt_1 * XPM_BYTES_PER_PIXEL + cnt_2] = (char)0xFF;
            else
                layers->xmg[LAYER_B]->
                data[cnt_1 * XPM_BYTES_PER_PIXEL + cnt_2] = map[cnt_3++];
    free(map);

    return LOAD_SUCCESS;
}


void reset_one_layer(Display *dsp, Layers *layers, LayerID id, GCs *gcs)
{
    XFillRectangle(dsp, layers->pxm[id],
                   gcs->gc[GC_COPY | GC_TRANSPARENT | GC_COLOR_BK],
                   0, 0, XPM_MAX_W, XPM_MAX_H);
}


void reset_every_layer(Display *dsp, Layers *layers, GCs *gcs)
{
    for (int cnt = LAYER_BOTTOM; cnt <= LAYER_TOP; cnt++)
        reset_one_layer(dsp, layers, cnt, gcs);
}


void load_map_background(Display *dsp, Window win, Layers *layers, GCs *gcs,
                         Coord ***coord, Redraw *redraw,
                         BackgroundID id, Bright *bright)
{
    switch (id)
    {
        case STREET_MAP:
        if (load_p6_file(layers, STREET_MAP_LOCATION) == LOAD_FAILURE)
            exit(EXIT_FAILURE);
        bright->background = true;
        break;

        case SATELLITE_IMAGE:
        if (load_p6_file(layers, SATELLITE_IMAGE_LOCATION) == LOAD_FAILURE)
            exit(EXIT_FAILURE);
        bright->background = false;
        break;

        case BACKGROUND_WHITE:
        memset(layers->xmg[LAYER_B]->data,
               0xFF, XPM_TOTAL_BYTES(XPM_MAX_W, XPM_MAX_H));
        bright->background = true;
        break;

        case BACKGROUND_BLACK:
        memset(layers->xmg[LAYER_B]->data,
               0x00, XPM_TOTAL_BYTES(XPM_MAX_W, XPM_MAX_H));
        bright->background = false;
        break;
    }

    /* Redraw outline after background image is changed */
    load_item_outline(dsp, win, layers, gcs, coord, redraw, bright);

    set_redraw_area(redraw, 0, 0, XPM_MAX_W, XPM_MAX_H);
}


void load_item_generic(Display *dsp, Window win, Layers *layers, GCs *gcs,
                       Coord ***coord, Redraw *redraw,
                       Result *result, ItemID *id, Bright *bright)
{
    bool is_outline   = ((result == NULL) && (id == NULL));
    bool is_highlight = ((result == NULL) && (id != NULL));
    bool with_message = ((result != NULL) && (id == NULL));

    if (is_outline)
    {
        /* Generate an item list that contains every item */
        id = malloc((ITEM_NUM + 1) * sizeof(ItemID));
        for(int cnt = 0; cnt < ITEM_NUM; cnt++)
            id[cnt] = cnt + 1;
        id[ITEM_NUM] = ITEM_TABLE[0].id;
    }
    else if (with_message)
    {
        id = result->id;
    }

    for (int cnt_1 = 0; id[cnt_1] != ITEM_TABLE[0].id; cnt_1++)
    {
        if (IS_DISTRICT(id[cnt_1]) || IS_STREAM(id[cnt_1]))
        {
            size_t len = coord_ptr_len(coord[id[cnt_1]]);
            XPoint *xpt = malloc(len * sizeof(XPoint));

            /* Put all coordinates into XPoint */
            for (int cnt_2 = 0; coord[id[cnt_1]][cnt_2] != NULL; cnt_2++)
            {
                xpt[cnt_2].x = coord[id[cnt_1]][cnt_2]->x;
                xpt[cnt_2].y = coord[id[cnt_1]][cnt_2]->y;
            }

            /* Draw items outlines that have different color and thickness
               depend on whether is_outline is set as true or not */
            /* If is_outline is set as true, draw with thin and BK/WH color;
               otherwise, draw with thick and pre-defined color in "def.h" */
            XDrawLines(dsp, layers->pxm[is_outline ? LAYER_O : LAYER_H],
                       gcs->gc[GC_OR | GC_OPAQUE |
                              (is_outline ? GC_THIN : GC_THICK) |
                              (is_outline ? (bright->background ?
                                             GC_COLOR_BK : GC_COLOR_WH) :
                                            (ITEM_TABLE[id[cnt_1]].gcid))],
                       xpt, len, CoordModeOrigin);

            /* If is_highlight or with_message is set as true,
               then fill the outline polygon with translucent color */
            if ((is_highlight || with_message) && IS_DISTRICT(id[cnt_1]))
                XFillPolygon(dsp, layers->pxm[LAYER_H],
                             gcs->gc[GC_OR | GC_TRANSLUCENT_2 |
                                     ITEM_TABLE[id[cnt_1]].gcid],
                             xpt, len, Nonconvex, CoordModeOrigin);

            /* Only redraw the certain area to improve the efficiency */
            Area area;
            get_area_of_item(coord, id[cnt_1], &area);
            set_redraw_area(redraw, area.x - THICK,
                                    area.y - THICK,
                                    area.width  + 2 * THICK + 1,
                                    area.height + 2 * THICK + 1);
        }
        else if (IS_POI(id[cnt_1]))
        {
            if (is_outline)
                XFillArc(dsp, layers->pxm[LAYER_O],
                         gcs->gc[GC_OR | GC_OPAQUE | GC_COLOR_POI],
                         coord[id[cnt_1]][0]->x - POI_R,
                         coord[id[cnt_1]][0]->y - POI_R,
                         POI_D, POI_D, 0, 360 * 64);
            else
                XFillArc(dsp, layers->pxm[LAYER_H],
                         gcs->gc[GC_OR | GC_OPAQUE | GC_COLOR_POI_HL],
                         coord[id[cnt_1]][0]->x - POI_HL_R,
                         coord[id[cnt_1]][0]->y - POI_HL_R,
                         POI_HL_D, POI_HL_D, 0, 360 * 64);

            set_redraw_area(redraw, coord[id[cnt_1]][0]->x - POI_MAX_R,
                                    coord[id[cnt_1]][0]->y - POI_MAX_R,
                                    POI_MAX_D, POI_MAX_D);
        }

        /* Draw item name or its message */
        if (is_highlight || with_message)
        {
            int x = 0, y = 0;
            int item_name_len = strlen(ITEM_TABLE[id[cnt_1]].name_en);
            int item_name_width
              = XTextWidth(gcs->font_info,
                           ITEM_TABLE[id[cnt_1]].name_en, item_name_len);
            int item_name_height = gcs->font_info->ascent
                                 + gcs->font_info->descent;
            int item_name_ofs_x
              = XTextWidth(gcs->font_info,
                           ITEM_TABLE[id[cnt_1]].name_en,
                           ITEM_NAME_OFS_X_BYTES);
            int item_name_ofs_y = item_name_height * ITEM_NAME_OFS_Y_BYTES;

            if (is_highlight)
            {
                Window win_root, win_child;
                int x_root, y_root, x_win, y_win;
                unsigned int mask;

                /* Retrieve the current cursor position */
                XQueryPointer(dsp, win, &win_root, &win_child,
                              &x_root, &y_root, &x_win, &y_win, &mask);
                x = x_win + item_name_ofs_x;
                y = y_win + item_name_ofs_y;
            }
            else if (with_message)
            {
                Area area;
                get_area_of_item(coord, id[cnt_1], &area);

                if (IS_DISTRICT(id[cnt_1]) || IS_STREAM(id[cnt_1]))
                {
                    x = area.x + (area.width  - item_name_width)  / 2;
                    y = area.y + (area.height - item_name_height) / 2;
                }
                else if (IS_POI(id[cnt_1]))
                {
                    x = area.x + item_name_ofs_x;
                    y = area.y + item_name_ofs_y;
                }
            }

            /* Draw item name */
            XFillRectangle(dsp, layers->pxm[LAYER_I],
                           gcs->gc[GC_OR | GC_TRANSLUCENT_2 | GC_COLOR_BK],
                           x - ITEM_NAME_MAR_X_PIXELS,
                           y - ITEM_NAME_MAR_Y_PIXELS,
                           item_name_width  + 2 * ITEM_NAME_MAR_X_PIXELS,
                           item_name_height + 2 * ITEM_NAME_MAR_Y_PIXELS);
            XDrawString(dsp, layers->pxm[LAYER_I],
                        gcs->gc[GC_COPY | GC_OPAQUE | GC_COLOR_WH],
                        x, y + gcs->font_ascent,
                        ITEM_TABLE[id[cnt_1]].name_en, item_name_len);

            set_redraw_area(redraw,
                            x - ITEM_NAME_MAR_X_PIXELS,
                            y - ITEM_NAME_MAR_Y_PIXELS,
                            item_name_width  + 2 * ITEM_NAME_MAR_X_PIXELS,
                            item_name_height + 2 * ITEM_NAME_MAR_Y_PIXELS);

            if (with_message && result->msg != NULL
                             && result->msg[cnt_1] != NULL)
            {
                for (int cnt_3 = 0; result->msg[cnt_1][cnt_3] != NULL; cnt_3++)
                {
                    int message_len = strlen(result->msg[cnt_1][cnt_3]);
                    int message_width
                      = XTextWidth(gcs->font_info,
                        result->msg[cnt_1][cnt_3], message_len);
                    int message_height = item_name_height;
                    int message_ofs_x
                      = XTextWidth(gcs->font_info,
                        result->msg[cnt_1][cnt_3], MESSAGE_OFS_X_BYTES);
                    int message_ofs_y = message_height * MESSAGE_OFS_Y_BYTES;

                    /* Draw corresponding message */
                    XFillRectangle(dsp, layers->pxm[LAYER_I],
                                   gcs->gc[GC_OR |
                                           GC_TRANSLUCENT_1 | GC_COLOR_BK],
                                   x + message_ofs_x
                                     - MESSAGE_MAR_X_PIXELS,
                                   y + message_ofs_y
                                     - MESSAGE_MAR_Y_PIXELS
                                     + MESSAGE_LINE_SPACES_PIXEL * cnt_3,
                                   message_width  + 2 * MESSAGE_MAR_X_PIXELS,
                                   message_height + 2 * MESSAGE_MAR_Y_PIXELS);
                    XDrawString(dsp, layers->pxm[LAYER_I],
                                gcs->gc[GC_COPY | GC_OPAQUE | GC_COLOR_WH],
                                x + message_ofs_x,
                                y + message_ofs_y
                                  + MESSAGE_LINE_SPACES_PIXEL * cnt_3
                                  + gcs->font_ascent,
                                result->msg[cnt_1][cnt_3], message_len);

                    set_redraw_area(redraw, 0, 0, WIN_MAX_W, WIN_MAX_H);
                }
            }

            redraw->exception = true;
        }
    }
}


void load_item_outline(Display *dsp, Window win, Layers *layers, GCs *gcs,
                       Coord ***coord, Redraw *redraw, Bright *bright)
{
    reset_one_layer(dsp, layers, LAYER_O, gcs);
    load_item_generic(dsp, win, layers, gcs, coord, redraw, NULL, NULL, bright);
}


void load_item_highlight(Display *dsp, Window win, Layers *layers, GCs *gcs,
                         Coord ***coord, Redraw *redraw,
                         Result *result, ItemID *id, Bright *bright)
{
    reset_one_layer(dsp, layers, LAYER_H, gcs);
    load_item_generic(dsp, win, layers, gcs, coord, redraw, result, id, bright);
}


void load_user_interface(Display *dsp, Layers *layers, GCs *gcs)
{
    reset_one_layer(dsp, layers, LAYER_I, gcs);
}


void reload_buffer_layer(Display *dsp, Layers *layers, GCs *gcs, Redraw *redraw)
{
    bool dup;

    /* Reload buffer layer
       only if two areas in redraw are different
       or exception is set as true */
    if ((dup = check_redraw_area(redraw)) && !redraw->exception)
        return;

    Area area;
    union_redraw_area(&redraw->area[AREA_LAST],
                      &redraw->area[AREA_THIS], &area);

    short x = area.x, y = area.y;
    unsigned short width = area.width, height = area.height;

    /* Check boundary */
    x = SELECT_MAX(0, x);
    y = SELECT_MAX(0, y);
    width  = SELECT_MIN(width,  XPM_MAX_W - x);
    height = SELECT_MIN(height, XPM_MAX_H - y);

    if (width == 0 || height == 0)
        return;

    static int cnt_redraw = 0;
    printf("Redraw %d frame:  "
           "x = %-4d y = %-4d width = %-4d height = %-4d "
           "exception = %c\n",
           ++cnt_redraw, x, y, width, height, redraw->exception ? 'T' : 'F');

    /* 32-bit X Pixmap -> 32-bit XImage */
    for (int cnt = LAYER_BOTTOM; cnt <= LAYER_TOP; cnt++)
        if (cnt != LAYER_BUFFER && cnt != LAYER_BOTTOM)
            layers->xmg[cnt] = XGetImage(dsp, layers->pxm[cnt], x, y,
                                         width, height, AllPlanes, ZPixmap);

    /* Alpha blending for 32-bit XImage */
    for (unsigned int cnt_x = 0; cnt_x < width; cnt_x++)
        for (unsigned int cnt_y = 0; cnt_y < height; cnt_y++)
        {
            for (int pxl_ofs = 0; pxl_ofs < WIN_BYTES_PER_PIXEL; pxl_ofs++)
            {
                int rel_ofs = pxl_ofs + XPM_BYTES_PER_PIXEL *
                              (cnt_x + cnt_y * width);
                int abs_ofs = pxl_ofs + XPM_BYTES_PER_PIXEL *
                              ((x + cnt_x) + (y + cnt_y) * XPM_MAX_W);

                layers->xmg[LAYER_BUFFER]->data[abs_ofs] =
                layers->xmg[LAYER_BOTTOM]->data[abs_ofs];

                for (int cnt_l = LAYER_BOTTOM; cnt_l < LAYER_TOP; cnt_l++)
                {
                    char alpha = layers->xmg[cnt_l + 1]->
                                 data[rel_ofs - pxl_ofs + 3];

                    layers->xmg[LAYER_BUFFER]->data[abs_ofs] =
                    (unsigned char)layers->xmg[cnt_l + 1]->data[rel_ofs]
                  * (unsigned char)alpha / MAP_COLORS
                  + (unsigned char)layers->xmg[LAYER_BUFFER]->data[abs_ofs]
                  * (0xFF - (unsigned char)alpha) / MAP_COLORS;
                }
            }
        }

    /* LAYER_BUFFER: 24-bit XImage -> 24-bit X Pixmap */
    XPutImage(dsp, layers->pxm[LAYER_BUFFER], gcs->gc_24bit_rgb,
              layers->xmg[LAYER_BUFFER], x, y, x, y, width, height);

    for (int cnt = LAYER_BOTTOM; cnt <= LAYER_TOP; cnt++)
        if (cnt != LAYER_BUFFER && cnt != LAYER_BOTTOM)
            XDestroyImage(layers->xmg[cnt]);
}


void map_onto_the_window(Display *dsp, Window win, Layers *layers, GCs *gcs)
{
    /* LAYER_BUFFER: 24-bit X Pixmap -> 24-bit Window */
    XCopyArea(dsp, layers->pxm[LAYER_BUFFER],
              win, gcs->gc_24bit_rgb, 0, 0, XPM_MAX_W, XPM_MAX_H, 0, 0);

    XFlush(dsp);
}


bool exec_sql_statement(MYSQL *mysql, Result *result)
{
    /* Launch a GNOME dialogue provided by zenity */
    FILE *fp = popen("zenity --width 500 --height 100 --title \"MariaDB\" \
                      --entry --text=\"Please enter the SQL statement:\"", "r");

    if (fp == NULL)
        printf("Failed to run zenity\n"),
        exit(EXIT_FAILURE);

    char statement[1024] = {0};
    if(fgets(statement, sizeof(statement) - 1, fp) != NULL)
        statement[strlen(statement) - 1] = '\0';

    pclose(fp);

    return query_by_command(mysql, statement, result);
}


void coord_to_item_list(Coord ***coord, short x, short y, ItemID (*id)[2])
{
    (*id)[0] = ITEM_TABLE[0].id;
    (*id)[1] = ITEM_TABLE[0].id;

    /* Point in polygon (PIP) problem
       by using ray casting algorithm
       and by setting slope as an irrational number */
    for (int cnt_1 = DISTRICT_START; cnt_1 <= DISTRICT_END; cnt_1++)
    {
        int cnt_pos = 0, cnt_neg = 0;

        for (int cnt_2 = 0; coord[cnt_1][cnt_2 + 1] != NULL; cnt_2++)
        {
            double x_0 = x,
                   y_0 = y,
                   x_1 = coord[cnt_1][cnt_2]->x,
                   y_1 = coord[cnt_1][cnt_2]->y,
                   x_2 = coord[cnt_1][cnt_2 + 1]->x,
                   y_2 = coord[cnt_1][cnt_2 + 1]->y;
            double b_1 = (x_1 != x_2) ? 1.0 : 0.0;
            double m_0 = MATH_CONST_PI,
                   m_1 = (b_1 == 1.0) ? ((y_2 - y_1) / (x_2 - x_1)) : 1.0;

            double u = ((y_0 - y_1) - m_0 * (x_0 - x_1)) / (m_1 - m_0 * b_1),
                   v = x_1 + b_1 * u - x_0;

            if (((b_1 == 1.0) && (u * (u - x_2 + x_1) <= 0)) ||
                ((b_1 == 0.0) && (u * (u - y_2 + y_1) <= 0)))
                {
                    if (v >= 0.0)
                        cnt_pos++;
                    else
                        cnt_neg++;
                }
        }

        if (cnt_neg != 0 && cnt_pos % 2 == 1)
            (*id)[0] = cnt_1;
    }

    for (int cnt_1 = STREAM_START; cnt_1 <= POI_END; cnt_1++)
        for (int cnt_2 = 0; coord[cnt_1][cnt_2] != NULL; cnt_2++)
            if (abs(x - coord[cnt_1][cnt_2]->x) <= POI_MAX_D
             && abs(y - coord[cnt_1][cnt_2]->y) <= POI_MAX_D)
                (*id)[0] = cnt_1;
}


void set_redraw_area(Redraw* redraw, short x, short y,
                                     unsigned short width,
                                     unsigned short height)
{
    /* Ensure that it will redraw whole window under this condition */
    if (x == 0 && y == 0 && width == XPM_MAX_W && height == XPM_MAX_H)
    {
        clear_redraw_area(redraw);
        store_redraw_area(redraw);
    }

    /* Redraw an area, whose width and height are zero, is meaningless */
    if (redraw->area[AREA_THIS].width  == 0 &&
        redraw->area[AREA_THIS].height == 0)
    {
        redraw->area[AREA_THIS].x = x;
        redraw->area[AREA_THIS].y = y;
        redraw->area[AREA_THIS].width  = width;
        redraw->area[AREA_THIS].height = height;
    }
    else
    {
        Area area;
        area.x = x;
        area.y = y;
        area.width  = width;
        area.height = height;

        /* Find the minimum area that should be redrawn */
        union_redraw_area(&redraw->area[AREA_THIS], &area,
                          &redraw->area[AREA_THIS]);
    }
}


void reset_redraw_area(Redraw *redraw)
{
    memset(&redraw->area, 0, sizeof(redraw->area) * AREA_NUM);
}


void clear_redraw_area(Redraw *redraw)
{
    memset(&redraw->area[AREA_THIS], 0, sizeof(redraw->area[AREA_THIS]));
}


void store_redraw_area(Redraw *redraw)
{
    redraw->area[AREA_LAST].x = redraw->area[AREA_THIS].x;
    redraw->area[AREA_LAST].y = redraw->area[AREA_THIS].y;
    redraw->area[AREA_LAST].width  = redraw->area[AREA_THIS].width;
    redraw->area[AREA_LAST].height = redraw->area[AREA_THIS].height;
    clear_redraw_area(redraw);
}


bool check_redraw_area(Redraw *redraw)
{
    if ((redraw->area[AREA_LAST].x == redraw->area[AREA_THIS].x)
     && (redraw->area[AREA_LAST].y == redraw->area[AREA_THIS].y)
     && (redraw->area[AREA_LAST].width  == redraw->area[AREA_THIS].width)
     && (redraw->area[AREA_LAST].height == redraw->area[AREA_THIS].height))
        return true;
    else
        return false;
}


void union_redraw_area(Area *area_in_1, Area *area_in_2, Area *area_out)
{
    if (area_in_1->width == 0 && area_in_1->height == 0)
    {
        area_in_1->x = area_in_2->x;
        area_in_1->y = area_in_2->y;
    }

    if (area_in_2->width == 0 && area_in_2->height == 0)
    {
        area_in_2->x = area_in_1->x;
        area_in_2->y = area_in_1->y;
    }

    short w = SELECT_MAX(area_in_1->x + area_in_1->width,
                         area_in_2->x + area_in_2->width);
    short h = SELECT_MAX(area_in_1->y + area_in_1->height,
                         area_in_2->y + area_in_2->height);
    area_out->x = SELECT_MIN(area_in_1->x, area_in_2->x);
    area_out->y = SELECT_MIN(area_in_1->y, area_in_2->y);
    area_out->width  = w - area_out->x;
    area_out->height = h - area_out->y;
}


void get_area_of_item(Coord ***coord, ItemID id, Area *area)
{
    memset(area, 0, sizeof(Area));

    short x_min = XPM_MAX_W, y_min = XPM_MAX_H, x_max = 0, y_max = 0;

    for (int cnt = 0; coord[id][cnt] != NULL; cnt++)
    {
        x_min = SELECT_MIN(x_min, coord[id][cnt]->x);
        y_min = SELECT_MIN(y_min, coord[id][cnt]->y);
        x_max = SELECT_MAX(x_max, coord[id][cnt]->x);
        y_max = SELECT_MAX(y_max, coord[id][cnt]->y);
    }

    area->x = x_min;
    area->y = y_min;
    area->width  = x_max - x_min;
    area->height = y_max - y_min;
}


void free_result(Result *result)
{
    if (result->id != NULL)
        free(result->id);

    if (result->msg != NULL)
        for (int cnt_1 = 0; result->msg[cnt_1] != NULL; cnt_1++)
        {
            for (int cnt_2 = 0; result->msg[cnt_1][cnt_2] != NULL; cnt_2++)
                 free(result->msg[cnt_1][cnt_2]);

            free(result->msg[cnt_1]);
        }

    free(result->msg);
}


size_t coord_ptr_len(Coord **coord)
{
    size_t sz = 0;
    while(coord[sz++] != NULL);

    return sz - 1;
}
