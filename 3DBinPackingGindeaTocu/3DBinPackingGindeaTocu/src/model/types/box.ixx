module;
#include <pch.h>

export module box;

export import orientation;

export struct Box {
    Vector3 m_size;
    Color m_color;
    uint8_t m_allowedOrientations = ORIENTATIONS_ALL;
};

export struct PlacedBox {
    Vector3 m_size;
    Vector3 m_position;
    Color m_color;
};

export bool overlaps(const PlacedBox& a, const PlacedBox& b) {
    return (a.m_position.x < b.m_position.x + b.m_size.x &&
            a.m_position.x + a.m_size.x > b.m_position.x &&
            a.m_position.y < b.m_position.y + b.m_size.y &&
            a.m_position.y + a.m_size.y > b.m_position.y &&
            a.m_position.z < b.m_position.z + b.m_size.z &&
            a.m_position.z + a.m_size.z > b.m_position.z);
}
