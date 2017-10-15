/*
 *    INTRODUCTION TO COMPUTER SCIENCE COURSE IN THE SPRING 2015 FROM NCTU
 *    THIS FILE IS PART OF THE FINAL PROJECT PROVIDING BY THE SECOND GROUP
 *    CONTRIBUTORS: Dong Nai-Jia
 */

#include "def.h"


const ColorID COLOR_TABLE[COLOR_NUM] =
{
    COLOR_R, COLOR_Y,  COLOR_G,
    COLOR_B, COLOR_BK, COLOR_WH,
    COLOR_HL_R, COLOR_HL_B
};


const AlphaID ALPHA_TABLE[ALPHA_NUM] =
{
    TRANSPARENT, TRANSLUCENT_1, TRANSLUCENT_2, OPAQUE
};


const LineID LINE_TABLE[LINE_NUM] =
{
    THIN, THICK
};


/* Table for district, stream and POI */
const Item ITEM_TABLE[ITEM_NUM + 1] =
{
    /* Hsinchu City */
    { 0x00, GC_COLOR_NULL, "無"    , "NULL"               },
    { 0x01, GC_COLOR_G   , "東區"  , "East District"      },
    { 0x02, GC_COLOR_Y   , "北區"  , "North District"     },
    { 0x03, GC_COLOR_R   , "香山區", "Xiangshan District" },

    /* Hsinchu County */
    { 0x04, GC_COLOR_B   , "竹北市", "Zhubei City"        },
    { 0x05, GC_COLOR_R   , "關西鎮", "Guanxi Township"    },
    { 0x06, GC_COLOR_G   , "新埔鎮", "Xinpu Township"     },
    { 0x07, GC_COLOR_R   , "竹東鎮", "Zhudong Township"   },
    { 0x08, GC_COLOR_B   , "寶山鄉", "Baoshan Township"   },
    { 0x09, GC_COLOR_Y   , "北埔鄉", "Beipu Township"     },
    { 0x0A, GC_COLOR_R   , "峨眉鄉", "Emei Township"      },
    { 0x0B, GC_COLOR_B   , "橫山鄉", "Hengshan Township"  },
    { 0x0C, GC_COLOR_Y   , "湖口鄉", "Hukou Township"     },
    { 0x0D, GC_COLOR_Y   , "尖石鄉", "Jianshi Township"   },
    { 0x0E, GC_COLOR_Y   , "芎林鄉", "Qionglin Township"  },
    { 0x0F, GC_COLOR_G   , "五峰鄉", "Wufeng Township"    },
    { 0x10, GC_COLOR_R   , "新豐鄉", "Xinfeng Township"   },

    /* Stream */
    { 0x11, GC_COLOR_STR , "頭前溪", "Touqian River"      },
    { 0x12, GC_COLOR_STR , "鳳山溪", "Fengshan River"     },

    /* POI (Point of Interest) */
    { 0x13, GC_COLOR_NULL, "小叮噹科學遊樂區",
                           "Little Ding-Dong Science Theme Park"           },
    { 0x14, GC_COLOR_NULL, "新瓦屋客家文化保存區",
                           "New Tile House Hakka Cultural District"        },
    { 0x15, GC_COLOR_NULL, "九芎湖休閒農場",
                           "Lagerstroemia Subcostata Lake Recreation Farm" },
    { 0x16, GC_COLOR_NULL, "褒忠亭義民廟",
                           "Bau-jong Yi-ming Temple"                       },
    { 0x17, GC_COLOR_NULL, "飛鳳山",
                           "Feifeng Mountain"                              },
    { 0x18, GC_COLOR_NULL, "六福村主題樂園",
                           "Leofoo Village Theme Park"                     },
    { 0x19, GC_COLOR_NULL, "內灣",
                           "Neiwan"                                        },
    { 0x1A, GC_COLOR_NULL, "萬瑞森林樂園",
                           "Wan Ruey Forest Paradise"                      },
    { 0x1B, GC_COLOR_NULL, "大霸尖山",
                           "Dabajian Mountain"                             },
    { 0x1C, GC_COLOR_NULL, "鎮西堡神木群",
                           "Jhen Si Bao Forest"                            },
    { 0x1D, GC_COLOR_NULL, "司馬庫斯神木群",
                           "Smangus Forest"                                },
    { 0x1E, GC_COLOR_NULL, "霞喀羅國家步道",
                           "Xiakelo National Trail"                        },
    { 0x1F, GC_COLOR_NULL, "清泉溫泉",
                           "Cing Cyuan Hot Spring"                         },
    { 0x20, GC_COLOR_NULL, "觀霧國家森林遊樂區",
                           "Guanwu National Forest Recreation Area"        },
    { 0x21, GC_COLOR_NULL, "雪霸休閒農場",
                           "Sheipa Leisure Farm"                           },
    { 0x22, GC_COLOR_NULL, "五指山",
                           "Five Finger Mountain"                          },
    { 0x23, GC_COLOR_NULL, "綠世界生態農場",
                           "Green World Ecological Farm"                   },
    { 0x24, GC_COLOR_NULL, "北埔老街",
                           "Beipu Old Street"                              },
    { 0x25, GC_COLOR_NULL, "獅頭山",
                           "Lion's Head Mountain"                          },
    { 0x26, GC_COLOR_NULL, "峨眉湖",
                           "Emei Lake"                                     },
    { 0x27, GC_COLOR_NULL, "寶山水庫",
                           "Baoshan Reservoir"                             },
    { 0x28, GC_COLOR_NULL, "新竹都城隍廟",
                           "Hsinchu Du Cheng Huang Temple"                 },
    { 0x29, GC_COLOR_NULL, "青草湖",
                           "Green Grass Lake"                              },
    { 0x2A, GC_COLOR_NULL, "孔廟",
                           "Confucian Temple"                              },
    { 0x2B, GC_COLOR_NULL, "新竹漁港",
                           "Hsinchu Fish Harbor"                           }
};

