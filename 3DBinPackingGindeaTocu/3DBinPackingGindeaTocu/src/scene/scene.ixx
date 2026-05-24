module;
#include <pch.h>

export module scene;

import model;
import solver;

export class Scene {
   public:
    static Scene& get();

    void setCamera(Camera3D* camera) { m_camera = camera; }
    void setModel(ProblemModel* model) { m_model = model; }

    void handleInput();
    bool isCursorLocked() const;

    void prepareFrame();

    void drawBackground() const;
    void drawScene();
    void drawCrosshair() const;

    void drawUI();

   private:
    Scene();
    ~Scene();

    void drawStatsUI();
    void drawStatsPlot();

    void drawAxis() const;
    void drawContainer() const;
    void drawPlacedBoxes();

    void updateOrientationPreview(const Vector3& originalSize, Orientation o);
    void drawBoxEditPopup(int index);

    static const int kCrosshairSize;
    static const Color kBackgroundColor;

    static const Color kAxisColor;
    static const Color kContainerColor;

    static const ImVec4 kSolverColors[SOLVER_MAX];

    Camera3D* m_camera = nullptr;
    ProblemModel* m_model = nullptr;
    bool m_lockCursor = false;
    int m_hoveredBoxIndex = -1;

    RenderTexture2D m_orientationPreview;
    Camera3D m_previewCamera;

    bool m_anyOrientationHovered = false;
    Vector3 m_previewBoxSize;
    Orientation m_previewOrientation;
    Color m_previewBoxColor;

    std::vector<PlacedBox> m_lastSolution;
    std::vector<SolverStats> m_lastStats;
};
