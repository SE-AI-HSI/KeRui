#include "GraphicsEffectUtil.h"

GraphicsEffectUtil::GraphicsEffectUtil() {
    // 不要创建实例,使用静态函数
    assert(false);
}

QGraphicsDropShadowEffect *GraphicsEffectUtil::newShadow(QWidget *const parent, int alpha, int blurRadius, int dx,
                                                         int dy) {
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(parent);
    shadow->setOffset(dx, dy);                // 阴影位置偏移量
    shadow->setColor(QColor(0, 0, 0, alpha)); // RGBA
    shadow->setBlurRadius(blurRadius);        // 阴影扩散半径

    return shadow;
}
