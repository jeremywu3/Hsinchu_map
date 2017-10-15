/*
 *    INTRODUCTION TO COMPUTER SCIENCE COURSE IN THE SPRING 2015 FROM NCTU
 *    THIS FILE IS PART OF THE FINAL PROJECT PROVIDING BY THE SECOND GROUP
 *    CONTRIBUTORS: Wang Wei-Bin, Dong Nai-Jia, Chen Yi-Qun
 */

#include "query.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mysql/mysql.h>


extern const Item ITEM_TABLE[];


void query_init(MYSQL *mysql, const char *db_name)
{
    mysql_init(mysql);

    // Connect to a local database
    if (!mysql_real_connect(mysql, "localhost", "root",
                            NULL, db_name, 0, NULL, 0))
        fprintf(stderr, "%s\n", mysql_error(mysql));
}


void query_quit(MYSQL *mysql)
{
    mysql_close(mysql);
}


void query_item_coord(MYSQL *mysql, Coord ***coord)
{
    MYSQL_RES *res;

    for (int cnt_i = DISTRICT_START; cnt_i <= STREAM_END + 1; cnt_i++)
    {
        char sql_statement[128];
        mysql_query(mysql,"SET NAMES UTF8");

        if (cnt_i <= STREAM_END)
            sprintf(sql_statement, "select * from `0x%02hhX`", cnt_i);
        else
            sprintf(sql_statement, "select `x-axis`,`y-axis` from `POI`");

        if (mysql_real_query(mysql, sql_statement, strlen(sql_statement)))
        {
            printf("ERROR: %s\n",mysql_error(mysql));
            return;
        }

        res = mysql_store_result(mysql);

        int row_n = mysql_num_rows(res);
        if (cnt_i <= STREAM_END)
        {
            coord[cnt_i] = malloc((row_n + 1) * sizeof(Coord*));

            int cnt_r;
            for(cnt_r = 0; cnt_r < row_n; cnt_r++)
            {
                MYSQL_ROW row_data = mysql_fetch_row(res);
                coord[cnt_i][cnt_r] = malloc(sizeof(Coord));
                sscanf(row_data[0], "%hd", &coord[cnt_i][cnt_r]->x);
                sscanf(row_data[1], "%hd", &coord[cnt_i][cnt_r]->y);
            }

            coord[cnt_i][cnt_r] = NULL;
        }
        else
        {
            for (int cnt_r = 0; cnt_r < row_n; cnt_r++)
            {
                coord[cnt_i + cnt_r] = malloc(2 * sizeof(Coord*));
                coord[cnt_i + cnt_r][1] = NULL;

                MYSQL_ROW row_data = mysql_fetch_row(res);
                coord[cnt_i + cnt_r][0] = malloc(sizeof(Coord));

                sscanf(row_data[0], "%hd", &coord[cnt_i + cnt_r][0]->x);
                sscanf(row_data[1], "%hd", &coord[cnt_i + cnt_r][0]->y);
            }
        }

        mysql_free_result(res);
    }
}


ItemID name_to_id(const char *item_name_en)
{
    for (int i = 0 ; i <= ITEM_NUM ; ++i)
         if ((strcmp(item_name_en, ITEM_TABLE[i].name_en)) == 0)
             return ITEM_TABLE[i].id;

    return ITEM_TABLE[0].id;
}


bool query_by_command(MYSQL *mysql, const char *statement, Result *result)
{
    MYSQL_RES *res;
    MYSQL_ROW  row_data;

    mysql_query(mysql, "SET NAMES UTF8");

    if (mysql_real_query(mysql, statement, strlen(statement)))
    {
        fprintf(stderr, "ERROR: %s\n",mysql_error(mysql));

        result->id  = NULL;
        result->msg = NULL;

        return false;
    }

    res = mysql_store_result(mysql);
    int row_num = mysql_num_rows(res);

    if (strstr(statement, "ADJACENT")  == NULL
     && strstr(statement, "RiverPass") == NULL)
    {
        char *search_from =
        strstr(statement, "FROM") == NULL ? strstr(statement, "from") :
                                            strstr(statement, "FROM");

        char *find_population, *find_area_size, *find_river_len;

        result->id  = malloc((row_num + 1) * sizeof(ItemID));
        result->msg = malloc((row_num + 1) * sizeof(char**));

        for (int row = 0; row < row_num; row++)
        {
            row_data = mysql_fetch_row(res);

            ItemID id;
            sscanf(row_data[0] + 2, "%02hhX", &id);
            result->id[row] = id;

            int col_num = mysql_num_fields(res) - 1;
            result->msg[row] = malloc((col_num + 1) * sizeof(char*));

            if ((find_population =
                 strstr(statement, "population"))   > search_from)
                 find_population = NULL;
            if ((find_area_size  =
                 strstr(statement, "area-size"))    > search_from)
                 find_area_size  = NULL;
            if ((find_river_len  =
                 strstr(statement, "river-length")) > search_from)
                 find_river_len  = NULL;

            /* Start to save message */
            for (int col = 0; col < col_num; col++)
            {
                int row_data_len = strlen(row_data[col + 1]);
                char *output_msg = malloc((row_data_len + 64) * sizeof(char));
                strcpy(output_msg, row_data[col + 1]);

                if(find_population != NULL)
                    strcpy(output_msg + row_data_len, " people"),
                    find_population = NULL;
                else if(find_area_size != NULL)
                    strcpy(output_msg + row_data_len, " square kilometers"),
                    find_area_size  = NULL;
                else if(find_river_len != NULL)
                    strcpy(output_msg + row_data_len, " kilometers"),
                    find_river_len  = NULL;

                result->msg[row][col] = output_msg;
            }

            result->msg[row][col_num] = NULL;
        }
        result->id[row_num]  = ITEM_TABLE[0].id;
        result->msg[row_num] = NULL;
    }
    else
    {
        result->id  = malloc((row_num + 2) * sizeof(ItemID));
        result->msg = NULL;

        row_data = mysql_fetch_row(res);
        if (strstr(statement, "ADJACENT") != NULL)
            result->id[0] = name_to_id(row_data[0]);
        else if (strstr(statement, "RiverPass") != NULL)
        {
            ItemID id;
            sscanf(row_data[0] + 2, "%02hhX", &id);
            result->id[0] = id;
        }

        for (int row = 0; row < row_num; row++, row_data = mysql_fetch_row(res))
            result->id[row + 1] = name_to_id(row_data[1]);

        result->id[row_num + 1] = ITEM_TABLE[0].id;
    }

    mysql_free_result(res);

    return true;
}


