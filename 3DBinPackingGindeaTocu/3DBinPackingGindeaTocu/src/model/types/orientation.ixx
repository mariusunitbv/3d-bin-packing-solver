module;
#include <pch.h>

export module orientation;

export enum Orientation : uint8_t {
    ORIENTATIONS_NONE = 0x00,
    ORIENTATION_WLH = 0x01,  // (w, l, h) - original
    ORIENTATION_WHL = 0x02,  // (w, h, l)
    ORIENTATION_LWH = 0x04,  // (l, w, h)
    ORIENTATION_LHW = 0x08,  // (l, h, w)
    ORIENTATION_HWL = 0x10,  // (h, w, l)
    ORIENTATION_HLW = 0x20,  // (h, l, w)

    ORIENTATIONS_ALL = 0x3F,
};

export constexpr Orientation kAllOrientations[] = {ORIENTATION_WLH, ORIENTATION_WHL,
                                                   ORIENTATION_LWH, ORIENTATION_LHW,
                                                   ORIENTATION_HWL, ORIENTATION_HLW};

export Vector3 getOrientatedSize(const Vector3& size, Orientation o) {
    const auto [w, l, h] = size;
    switch (o) {
        case ORIENTATION_WLH:
            return {w, l, h};
        case ORIENTATION_WHL:
            return {w, h, l};
        case ORIENTATION_LWH:
            return {l, w, h};
        case ORIENTATION_LHW:
            return {l, h, w};
        case ORIENTATION_HWL:
            return {h, w, l};
        case ORIENTATION_HLW:
            return {h, l, w};
        default:
            return {w, l, h};
    }
}
