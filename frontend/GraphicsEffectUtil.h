#ifndef GRAPHICSEFFECTUTIL_H
#define GRAPHICSEFFECTUTIL_H

#include <QGraphicsDropShadowEffect>
#include <QWidget>

/**
 * @class GraphicsEffectUtil
 * @brief 简单封装一些静态函数
 */

class GraphicsEffectUtil {
public:
    GraphicsEffectUtil();

    // 返回一个新的阴影,因为一个阴影只能用于一个组件//
    static QGraphicsDropShadowEffect *newShadow(QWidget *const parent, int alpha = 64, int blurRadius = 20, int dx = 0,
                                                int dy = 0);
};

#endif // GRAPHICSEFFECTUTIL_H
