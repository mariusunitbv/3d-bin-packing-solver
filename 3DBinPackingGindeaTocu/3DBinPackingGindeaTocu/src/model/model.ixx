module;
#include <pch.h>

export module model;

export import box;

export class ProblemModel {
   public:
    void setContainerSize(const Vector3& size) { m_containerSize = size; }
    const Vector3& getContainerSize() const { return m_containerSize; }

    void addBox(const Box& box) { m_boxes.push_back(box); }
    void setBoxes(std::vector<Box>&& boxes);
    void removeBox(size_t index) { m_boxes.erase(m_boxes.begin() + index); }
    const std::vector<Box>& getBoxes() const { return m_boxes; }

    void setBoxSize(size_t index, const Vector3& size) { m_boxes[index].m_size = size; }
    void setBoxColor(size_t index, const Color& color) { m_boxes[index].m_color = color; }
    void setBoxOrientation(size_t index, uint8_t orientations) {
        m_boxes[index].m_allowedOrientations = orientations;
    }

   private:
    Vector3 m_containerSize{5, 5, 5};
    std::vector<Box> m_boxes;
};

void ProblemModel::setBoxes(std::vector<Box>&& boxes) { m_boxes = std::move(boxes); }
