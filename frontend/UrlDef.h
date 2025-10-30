#ifndef URLDEF_H
#define URLDEF_H

/**
 * @file UrlDef.h
 * @brief 统一定义标签中的Url格式
 */

enum UrlDef {
    JUMP_TO_ROW = 700100,
    JUMP_TO_ROW_COLUMN = 700101,
    JUMP_TO_BLOCK = 700102,

    TO_PREV_LABEL = 700200,
    TO_NEXT_LABEL = 700201,
    TO_SPECIFIC_LABEL = 700202,

    OPTIMIZE_CLONE = 700300,

    MASK_WARNING = 700400,

    JUMP_TO_FILE = 700500,
};

#endif // URLDEF_H
