#include <stdbool.h>

#include "physics.h"

bool pointInRect(Vector2i rPosition, Vector2i rSize, Vector2i pPosition) {
    return (pPosition.x >= rPosition.x && pPosition.x <= rPosition.x + rSize.x)
        && (pPosition.y >= rPosition.y && pPosition.y <= rPosition.y + rSize.y);
}
