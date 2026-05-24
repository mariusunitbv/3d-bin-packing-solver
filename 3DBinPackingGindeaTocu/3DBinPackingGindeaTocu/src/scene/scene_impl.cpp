module;
#include <pch.h>

module scene;

import loader;

const int Scene::kCrosshairSize = 4;
const Color Scene::kBackgroundColor = {20, 20, 20};

const Color Scene::kAxisColor = {255, 255, 255, 30};
const Color Scene::kContainerColor = {255, 255, 255, 130};

const ImVec4 Scene::kSolverColors[SOLVER_MAX] = {
    {0, 1.f, 1.f, 1.f},       // BLB - cyan
    {1.f, 0.647f, 0.f, 1.f},  // Layer-Based - orange
    {1.f, 0.f, 1.f, 1.f},     // Extreme Points - magenta
    {0.f, 1.f, 0.f, 1.f},     // Greedy - green
    {1.f, 1.f, 0.f, 1.f},     // Genetic - yellow
    {1.f, 0.5f, 0.5f, 1.f}    // Hill Climbing - light red
};

Scene& Scene::get() {
    static Scene instance;
    return instance;
}

void Scene::handleInput() {
    if (IsKeyPressed(KEY_M)) {
        m_lockCursor = !m_lockCursor;

        if (m_lockCursor) {
            DisableCursor();
        } else {
            EnableCursor();
        }
    }

    if (IsKeyPressed(KEY_F11) || IsKeyPressed(KEY_F)) {
        if (IsWindowMaximized()) {
            RestoreWindow();
        } else {
            MaximizeWindow();
        }
    }
}

bool Scene::isCursorLocked() const { return m_lockCursor; }

void Scene::prepareFrame() {
    if (m_anyOrientationHovered) {
        updateOrientationPreview(m_previewBoxSize, m_previewOrientation);
    }
}

void Scene::drawBackground() const { ClearBackground(kBackgroundColor); }

void Scene::drawScene() {
    drawAxis();
    drawContainer();
    drawPlacedBoxes();
}

void Scene::drawCrosshair() const {
    const auto cx = GetScreenWidth() / 2;
    const auto cy = GetScreenHeight() / 2;

    DrawCircle(cx, cy, kCrosshairSize, GRAY);
    DrawCircleLines(cx, cy, kCrosshairSize, BLACK);
}

void Scene::drawUI() {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    drawList->AddText(
        {10, 10}, IM_COL32_WHITE,
        "M = toggle cursor  |  F / F11 = fullscreen\nGindea Alexandru & Tocu Marius Daniel");

    auto& io = ImGui::GetIO();

    if (m_hoveredBoxIndex != -1) {
        const auto& box = m_lastSolution[m_hoveredBoxIndex];
        const char* text =
            TextFormat("%.0f x %.0f x %.0f", box.m_size.x, box.m_size.y, box.m_size.z);
        const auto screenSize = ImVec2(io.DisplaySize.x, io.DisplaySize.y);
        drawList->AddText(screenSize * 0.5f + ImVec2(15, 15), IM_COL32_WHITE, text);
    }

    // stiluri comune
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {8, 7});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.11f, 0.11f, 0.13f, 0.97f});
    ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.17f, 0.17f, 0.21f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_Separator, {0.3f, 0.3f, 0.35f, 1.0f});

    // ════════════════════════════════════════════════════════
    //  FEREASTRA 1 — Problem Setup
    // ════════════════════════════════════════════════════════
    ImGui::SetNextWindowPos({io.DisplaySize.x - 430, 10}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({410, 560}, ImGuiCond_Once);
    ImGui::Begin("  Problem Setup", nullptr, ImGuiWindowFlags_NoCollapse);

    // ── Container ───────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_Text, {0.6f, 0.85f, 1.0f, 1.0f});
    ImGui::SeparatorText("Container");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    int containerSize[3] = {static_cast<int>(m_model->getContainerSize().x),
                            static_cast<int>(m_model->getContainerSize().y),
                            static_cast<int>(m_model->getContainerSize().z)};

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::SliderInt3("##ContainerSize", containerSize, 1, 50)) {
        m_model->setContainerSize({static_cast<float>(containerSize[0]),
                                   static_cast<float>(containerSize[1]),
                                   static_cast<float>(containerSize[2])});
    }
    ImGui::PushStyleColor(ImGuiCol_Text, {0.5f, 0.5f, 0.55f, 1.0f});
    const auto& cs = m_model->getContainerSize();
    ImGui::Text("  W: %.0f    L: %.0f    H: %.0f    Volume: %.0f", cs.x, cs.y, cs.z,
                cs.x * cs.y * cs.z);
    ImGui::PopStyleColor();

    ImGui::Spacing();

#ifndef __EMSCRIPTEN__
    // ── File IO ─────────────────────────────────────────────
    float btnW = (ImGui::GetContentRegionAvail().x - 8) / 2;
    ImGui::PushStyleColor(ImGuiCol_Button, {0.18f, 0.38f, 0.65f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.28f, 0.50f, 0.82f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.13f, 0.28f, 0.55f, 1.0f});
    if (ImGui::Button("Load File", {btnW, 26}))
        *m_model = ProblemLoader::get().loadFromFilePicker();
    ImGui::SameLine();
    if (ImGui::Button("Save File", {btnW, 26})) ProblemLoader::get().saveToFilePicker(*m_model);
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
#endif

    // ── Add Box ─────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_Text, {0.6f, 0.85f, 1.0f, 1.0f});
    ImGui::SeparatorText("Add New Box");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    static int newSize[3] = {1, 1, 1};
    static float newColor[3] = {1.0f, 0.3f, 0.3f};
    static uint8_t newOrientations = ORIENTATIONS_ALL;

    ImGui::PushStyleColor(ImGuiCol_Text, {0.75f, 0.75f, 0.75f, 1.0f});
    ImGui::TextUnformatted("Dimensions  (W / L / H)");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::SliderInt3("##NewSize", newSize, 1, 20);

    ImGui::PushStyleColor(ImGuiCol_Text, {0.75f, 0.75f, 0.75f, 1.0f});
    ImGui::TextUnformatted("Color");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::ColorEdit3("##NewColor", newColor);

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, {0.75f, 0.75f, 0.75f, 1.0f});
    ImGui::TextUnformatted("Allowed Orientations");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    static const struct {
        Orientation flag;
        const char* label;
    } kOrientations[] = {
        {ORIENTATION_WLH, "(W,L,H)"}, {ORIENTATION_WHL, "(W,H,L)"}, {ORIENTATION_LWH, "(L,W,H)"},
        {ORIENTATION_LHW, "(L,H,W)"}, {ORIENTATION_HWL, "(H,W,L)"}, {ORIENTATION_HLW, "(H,L,W)"},
    };

    m_anyOrientationHovered = false;
    float colWidth = ImGui::GetContentRegionAvail().x / 2 - 4;

    for (int i = 0; i < 6; i++) {
        if (i % 2 != 0) ImGui::SameLine(colWidth);
        const auto& entry = kOrientations[i];
        bool checked = (newOrientations & entry.flag) != 0;
        if (ImGui::Checkbox(entry.label, &checked)) {
            if (checked)
                newOrientations |= entry.flag;
            else
                newOrientations &= ~entry.flag;
        }
        if (ImGui::IsItemHovered()) {
            m_previewOrientation = entry.flag;
            m_previewBoxSize = {(float)newSize[0], (float)newSize[1], (float)newSize[2]};
            m_previewBoxColor = {(unsigned char)(newColor[0] * 255),
                                 (unsigned char)(newColor[1] * 255),
                                 (unsigned char)(newColor[2] * 255), 255};
            m_anyOrientationHovered = true;
            if (ImGui::BeginTooltip()) {
                ImGui::TextUnformatted("Orientation Preview");
                ImGui::Separator();
                ImGui::Spacing();
                rlImGuiImageRect(&m_orientationPreview.texture, 200, 200, {0, 0, 256, -256});
                ImGui::EndTooltip();
            }
        }
    }

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, {0.18f, 0.58f, 0.28f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.28f, 0.72f, 0.38f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.13f, 0.45f, 0.20f, 1.0f});
    if (ImGui::Button("+ Add Box", {-FLT_MIN, 30})) {
        Box box;
        box.m_size = {(float)newSize[0], (float)newSize[1], (float)newSize[2]};
        box.m_color = {(unsigned char)(newColor[0] * 255), (unsigned char)(newColor[1] * 255),
                       (unsigned char)(newColor[2] * 255), 255};
        box.m_allowedOrientations = newOrientations;
        m_model->addBox(box);
        newOrientations = ORIENTATIONS_ALL;
    }
    ImGui::PopStyleColor(3);

    ImGui::Spacing();

    // ── Box List ─────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_Text, {0.6f, 0.85f, 1.0f, 1.0f});
    const auto& boxes = m_model->getBoxes();
    ImGui::SeparatorText(TextFormat("Boxes  (%d)", (int)boxes.size()));
    ImGui::PopStyleColor();
    ImGui::Spacing();

    int removeIndex = -1;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, {0.14f, 0.14f, 0.17f, 1.0f});
    ImGui::BeginChild("##BoxList", {0, 0}, ImGuiChildFlags_Borders);

    if (boxes.empty()) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
        ImGui::PushStyleColor(ImGuiCol_Text, {0.38f, 0.38f, 0.42f, 1.0f});
        ImGui::SetCursorPosX(
            (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("No boxes added yet.").x) / 2);
        ImGui::TextUnformatted("No boxes added yet.");
        ImGui::PopStyleColor();
    }

    for (int i = 0; i < (int)boxes.size(); i++) {
        ImGui::PushID(i);

        ImVec4 col = {boxes[i].m_color.r / 255.0f, boxes[i].m_color.g / 255.0f,
                      boxes[i].m_color.b / 255.0f, 1.0f};
        ImGui::ColorButton("##col", col, ImGuiColorEditFlags_NoTooltip, {14, 14});
        ImGui::SameLine();
        ImGui::Text("Box %-2d   %.0f x %.0f x %.0f", i, boxes[i].m_size.x, boxes[i].m_size.y,
                    boxes[i].m_size.z);

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 58);

        ImGui::PushStyleColor(ImGuiCol_Button, {0.22f, 0.22f, 0.28f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.30f, 0.30f, 0.38f, 1.0f});
        if (ImGui::SmallButton("Edit")) ImGui::OpenPopup("##EditBox");
        ImGui::PopStyleColor(2);

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, {0.55f, 0.13f, 0.13f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.75f, 0.18f, 0.18f, 1.0f});
        if (ImGui::SmallButton(" X ")) removeIndex = i;
        ImGui::PopStyleColor(2);

        drawBoxEditPopup(i);

        ImGui::PopID();
        ImGui::Separator();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (removeIndex != -1) m_model->removeBox(removeIndex);

    ImGui::End();  // Problem Setup

    // ════════════════════════════════════════════════════════
    //  FEREASTRA 2 — Solver
    // ════════════════════════════════════════════════════════
    ImGui::SetNextWindowPos({io.DisplaySize.x - 430, 580}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({410, 200}, ImGuiCond_Once);
    ImGui::Begin("  Solver", nullptr, ImGuiWindowFlags_NoCollapse);

    static SolverType_t selectedSolver = SOLVER_BLB;

    // GENETIC ALGORITHM PARAMS
    static int gaPopulation = 100;
    static int gaGenerations = 500;
    static float gaMutation = 0.05f;
    static int gaTournamentSize = 3;
    static int optimalFitness = 100;
    static SolverType_t decoderSolverType = SOLVER_BLB;

    // HILL CLIMBING PARAMS
    static int hcIterations = 1000;

    ImGui::PushStyleColor(ImGuiCol_Text, {0.75f, 0.75f, 0.75f, 1.0f});
    ImGui::TextUnformatted("Algorithm");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##Solver", getSolverName(selectedSolver))) {
        for (int i = 0; i < (int)SOLVER_MAX; i++) {
            SolverType_t s = static_cast<SolverType_t>(i);
            if (ImGui::Selectable(getSolverName(s), selectedSolver == s)) selectedSolver = s;
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    if (selectedSolver == SOLVER_GENETIC) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, {0.14f, 0.14f, 0.17f, 1.0f});
        ImGui::BeginChild("##GAParams", {0, 160}, ImGuiChildFlags_Borders);

        ImGui::PushStyleColor(ImGuiCol_Text, {0.75f, 0.75f, 0.75f, 1.0f});
        ImGui::TextUnformatted("Population:");
        ImGui::PopStyleColor();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::SliderInt("##GAPop", &gaPopulation, 10, 500);

        ImGui::PushStyleColor(ImGuiCol_Text, {0.75f, 0.75f, 0.75f, 1.0f});
        ImGui::TextUnformatted("Generations:");
        ImGui::PopStyleColor();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::SliderInt("##GAGen", &gaGenerations, 10, 2000);

        ImGui::PushStyleColor(ImGuiCol_Text, {0.75f, 0.75f, 0.75f, 1.0f});
        ImGui::TextUnformatted("Mutation rate:");
        ImGui::PopStyleColor();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::SliderFloat("##GAMut", &gaMutation, 0.01f, 0.5f, "%.2f");

        ImGui::PushStyleColor(ImGuiCol_Text, {0.75f, 0.75f, 0.75f, 1.0f});
        ImGui::TextUnformatted("Tournament size:");
        ImGui::PopStyleColor();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::SliderInt("##GATour", &gaTournamentSize, 2, 10);

        ImGui::PushStyleColor(ImGuiCol_Text, {0.75f, 0.75f, 0.75f, 1.0f});
        ImGui::TextUnformatted("Optimal fitness:");
        ImGui::PopStyleColor();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::SliderInt("##GAOpt", &optimalFitness, 10, 100);

        ImGui::PushStyleColor(ImGuiCol_Text, {0.75f, 0.75f, 0.75f, 1.0f});
        ImGui::TextUnformatted("Decoder Strategy:");
        ImGui::PopStyleColor();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##DecoderType", getSolverName(decoderSolverType))) {
            for (int i = 0; i <= (int)SOLVER_GREEDY; i++) {
                SolverType_t s = static_cast<SolverType_t>(i);
                if (ImGui::Selectable(getSolverName(s), decoderSolverType == s))
                    decoderSolverType = s;
            }
            ImGui::EndCombo();
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::Spacing();
    } else if (selectedSolver == SOLVER_HILLCLIMBING) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, {0.14f, 0.14f, 0.17f, 1.0f});
        ImGui::BeginChild("##HCParams", {0, 60}, ImGuiChildFlags_Borders);
        ImGui::PushStyleColor(ImGuiCol_Text, {0.75f, 0.75f, 0.75f, 1.0f});
        ImGui::TextUnformatted("Iterations:");
        ImGui::PopStyleColor();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::SliderInt("##HCIter", &hcIterations, 100, 5000);
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    bool canSolve = !boxes.empty();
    if (!canSolve) ImGui::BeginDisabled();

    ImGui::PushStyleColor(ImGuiCol_Button, {0.45f, 0.18f, 0.65f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.58f, 0.28f, 0.80f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.35f, 0.13f, 0.52f, 1.0f});
    if (ImGui::Button("Run Solver", {-FLT_MIN, 34})) {
        auto solver = ISolver::create(selectedSolver);

        if (selectedSolver == SOLVER_GENETIC) {
            solver->setParameter("population", std::to_string(gaPopulation));
            solver->setParameter("generations", std::to_string(gaGenerations));
            solver->setParameter("mutation", std::to_string(gaMutation));
            solver->setParameter("tournament", std::to_string(gaTournamentSize));
            solver->setParameter("optimal", std::to_string(optimalFitness));
        } else if (selectedSolver == SOLVER_HILLCLIMBING) {
            solver->setParameter("maxIterations", std::to_string(hcIterations));
        }

        solver->setModel(m_model);
        solver->setDecoderType(decoderSolverType);
        solver->initializeParameters();
        solver->solve();
        m_lastSolution = solver->getSolution();
        m_lastStats.push_back(solver->getStats());
    }

    if (ImGui::Button("Run All Solvers", {-FLT_MIN, 34})) {
        for (int i = 0; i < (int)SOLVER_MAX; i++) {
            SolverType_t s = static_cast<SolverType_t>(i);
            auto solver = ISolver::create(s);

            if (selectedSolver == SOLVER_GENETIC) {
                solver->setParameter("population", std::to_string(gaPopulation));
                solver->setParameter("generations", std::to_string(gaGenerations));
                solver->setParameter("mutation", std::to_string(gaMutation));
                solver->setParameter("tournament", std::to_string(gaTournamentSize));
                solver->setParameter("optimal", std::to_string(optimalFitness));
            } else if (selectedSolver == SOLVER_HILLCLIMBING) {
                solver->setParameter("maxIterations", std::to_string(hcIterations));
            }

            solver->setModel(m_model);
            solver->setDecoderType(decoderSolverType);
            solver->initializeParameters();
            solver->solve();
            m_lastStats.push_back(solver->getStats());
        }
    }

    if (ImGui::Button("Clear Solution", {-FLT_MIN, 34})) {
        m_lastSolution.clear();
    }

    if (ImGui::Button("Clear Statistics", {-FLT_MIN, 34})) {
        m_lastStats.clear();
    }

    ImGui::PopStyleColor(3);

    if (!canSolve) {
        ImGui::EndDisabled();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, {0.55f, 0.28f, 0.28f, 1.0f});
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x -
                              ImGui::CalcTextSize("Add boxes before solving.").x) /
                             2);
        ImGui::TextUnformatted("Add boxes before solving.");
        ImGui::PopStyleColor();
    }

    ImGui::End();  // Solver

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);

    drawStatsUI();
}

Scene::Scene() {
    m_orientationPreview = LoadRenderTexture(256, 256);
    m_previewCamera = {.position = {20, 20, 20},
                       .target = {0, 0, 0},
                       .up = {0, 1, 0},
                       .fovy = 45,
                       .projection = CAMERA_PERSPECTIVE};
}

Scene::~Scene() { UnloadRenderTexture(m_orientationPreview); }

void Scene::drawStatsUI() {
    if (m_lastStats.empty()) return;

    auto& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({10, io.DisplaySize.y - 220}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({500, 210}, ImGuiCond_Once);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {10, 6});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.11f, 0.11f, 0.13f, 0.97f});
    ImGui::PushStyleColor(ImGuiCol_TableBorderLight, {0.25f, 0.25f, 0.30f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, {0.35f, 0.35f, 0.40f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, {0.15f, 0.15f, 0.18f, 1.0f});

    ImGui::Begin("  Statistics", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::PushStyleColor(ImGuiCol_Text, {0.6f, 0.85f, 1.0f, 1.0f});
    ImGui::TextUnformatted("Last Run Statistics");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();

    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_Resizable      // ← resize coloane
                            | ImGuiTableFlags_Reorderable  // ← reordoneaza coloane drag
                            | ImGuiTableFlags_Hideable     // ← ascunde coloane click dreapta
                            | ImGuiTableFlags_ScrollY      // ← scroll daca sunt multe randuri
                            | ImGuiTableFlags_SizingStretchSame;

    if (ImGui::BeginTable("##StatsTable", 6, flags)) {
        ImGui::PushStyleColor(ImGuiCol_Text, {0.8f, 0.8f, 0.5f, 1.0f});
        ImGui::TableSetupColumn("Solver", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Boxes", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Volume Used", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Boxes Used", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Remove", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupScrollFreeze(0, 1);  // ← header fix la scroll
        ImGui::TableHeadersRow();
        ImGui::PopStyleColor();

        size_t removeIndex = -1;
        for (const auto& s : m_lastStats) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::PushStyleColor(ImGuiCol_Text, kSolverColors[s.m_solverType]);
            ImGui::TextUnformatted(getSolverName(s.m_solverType));
            ImGui::PopStyleColor();

            ImGui::TableSetColumnIndex(1);
            bool allPlaced = s.m_boxesPlaced == s.m_totalBoxes;
            ImGui::PushStyleColor(ImGuiCol_Text, allPlaced ? ImVec4{0.3f, 0.9f, 0.4f, 1.0f}
                                                           : ImVec4{0.9f, 0.5f, 0.3f, 1.0f});
            ImGui::Text("%d / %d", s.m_boxesPlaced, s.m_totalBoxes);
            ImGui::PopStyleColor();

            ImGui::TableSetColumnIndex(2);
            float vol = s.m_volumeUtilization;
            ImVec4 volColor = vol >= 80.0f   ? ImVec4{0.3f, 0.9f, 0.4f, 1.0f}
                              : vol >= 50.0f ? ImVec4{0.9f, 0.8f, 0.2f, 1.0f}
                                             : ImVec4{0.9f, 0.4f, 0.3f, 1.0f};
            ImGui::PushStyleColor(ImGuiCol_Text, volColor);
            ImGui::Text("%.1f%%", vol);
            ImGui::PopStyleColor();

            int boxesUtil = s.getBoxesUtilization();
            ImVec4 boxColor = boxesUtil >= 80   ? ImVec4{0.3f, 0.9f, 0.4f, 1.0f}
                              : boxesUtil >= 50 ? ImVec4{0.9f, 0.8f, 0.2f, 1.0f}
                                                : ImVec4{0.9f, 0.4f, 0.3f, 1.0f};

            ImGui::TableSetColumnIndex(3);
            ImGui::PushStyleColor(ImGuiCol_Text, boxColor);
            ImGui::Text("%d%%", boxesUtil);
            ImGui::PopStyleColor();

            ImGui::TableSetColumnIndex(4);
            ImGui::PushStyleColor(ImGuiCol_Text, {0.6f, 0.6f, 0.65f, 1.0f});
            if (s.m_timeTaken < 1000.0f)
                ImGui::Text("%.1f ms", s.m_timeTaken);
            else
                ImGui::Text("%.2f s", s.m_timeTaken / 1000.0f);
            ImGui::PopStyleColor();

            ImGui::TableSetColumnIndex(5);
            ImGui::PushID(&s);
            if (ImGui::SmallButton("X")) {
                removeIndex = &s - &m_lastStats[0];
            }
            ImGui::PopID();
        }

        if (removeIndex != -1) {
            m_lastStats.erase(m_lastStats.begin() + removeIndex);
        }

        ImGui::EndTable();
    }

    ImGui::End();

    drawStatsPlot();

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(3);
}

void Scene::drawStatsPlot() {
    if (ImGui::Begin("Algorithm Analyisis")) {
        if (ImPlot::BeginPlot("Efficiency vs Execution Time", ImVec2(-FLT_MIN, -FLT_MIN))) {
            ImPlot::SetupAxes("Execution Time (ms)", "Boxes Utilized (%)");

            ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 110.0, ImGuiCond_Once);
            ImPlot::SetupAxisLimits(ImAxis_X1, -0.1, 2.0, ImGuiCond_Once);

            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0.0, std::numeric_limits<double>::max());
            ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0.0, 100.0);

            for (int typeInt = 0; typeInt < SOLVER_MAX; ++typeInt) {
                SolverType_t currentType = static_cast<SolverType_t>(typeInt);

                std::vector<float> xTimes;
                std::vector<float> yVolumes;
                std::vector<float> markerSizes;

                for (const auto& stat : m_lastStats) {
                    if (stat.m_solverType == currentType) {
                        xTimes.push_back(stat.m_timeTaken);

                        yVolumes.push_back((float)stat.getBoxesUtilization());

                        float baseSize = 12.f * (stat.getBoxesUtilization() / 100.f);
                        markerSizes.push_back(baseSize);
                    }
                }

                if (xTimes.empty()) continue;

                ImPlot::PushStyleColor(ImPlotCol_MarkerFill, kSolverColors[currentType]);
                ImPlot::PushStyleColor(ImPlotCol_MarkerOutline, ImVec4(1, 1, 1, 0.5f));
                ImPlot::PushStyleVar(ImPlotStyleVar_Marker, ImPlotMarker_Circle);

                ImPlot::SetNextLineStyle(kSolverColors[currentType]);
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 8.0f, kSolverColors[currentType],
                                           IMPLOT_AUTO, kSolverColors[currentType]);

                ImPlot::PlotScatter(getSolverName(currentType), xTimes.data(), yVolumes.data(),
                                    (int)xTimes.size());

                ImPlot::PopStyleVar();
                ImPlot::PopStyleColor(2);
            }

            ImPlot::EndPlot();
        }
    }
    ImGui::End();
}

void Scene::drawAxis() const {
    const auto& size = m_model->getContainerSize();

    // X-axis
    DrawLine3D({size.x, 0, 0}, {300, 0, 0}, kAxisColor);

    // Y-axis
    DrawLine3D({0, size.y, 0}, {0, 300, 0}, kAxisColor);

    // Z-axis
    DrawLine3D({0, 0, size.z}, {0, 0, 300}, kAxisColor);
}

void Scene::drawContainer() const {
    const auto& size = m_model->getContainerSize();

    const Vector3 centeredPos = {size.x / 2, size.y / 2, size.z / 2};
    DrawCubeWires(centeredPos, size.x, size.y, size.z, kContainerColor);
}

void Scene::drawPlacedBoxes() {
    Ray ray = GetMouseRay({(float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2}, *m_camera);

    m_hoveredBoxIndex = -1;
    float closestDist = FLT_MAX;

    for (int i = 0; i < (int)m_lastSolution.size(); i++) {
        const auto& box = m_lastSolution[i];

        BoundingBox bb = {.min = box.m_position,
                          .max = {box.m_position.x + box.m_size.x, box.m_position.y + box.m_size.y,
                                  box.m_position.z + box.m_size.z}};

        RayCollision col = GetRayCollisionBox(ray, bb);
        if (col.hit && col.distance < closestDist) {
            closestDist = col.distance;
            m_hoveredBoxIndex = i;
        }
    }

    for (int i = 0; i < (int)m_lastSolution.size(); i++) {
        const auto& box = m_lastSolution[i];

        const Vector3 centeredPos = {box.m_position.x + box.m_size.x / 2,
                                     box.m_position.y + box.m_size.y / 2,
                                     box.m_position.z + box.m_size.z / 2};

        Color col = box.m_color;
        col.a = (i == m_hoveredBoxIndex) ? 150 : 255;

        DrawCube(centeredPos, box.m_size.x, box.m_size.y, box.m_size.z, col);
        DrawCubeWires(centeredPos, box.m_size.x, box.m_size.y, box.m_size.z, WHITE);
    }
}

void Scene::updateOrientationPreview(const Vector3& originalSize, Orientation o) {
    Vector3 size = getOrientatedSize(originalSize, o);

    float maxDim = std::max({size.x, size.y, size.z});
    float dist = maxDim * 1.8f + 2.0f;

    m_previewCamera.position = {dist, dist * 0.8f, dist};
    m_previewCamera.target = {size.x / 2, size.y / 2, size.z / 2};

    BeginTextureMode(m_orientationPreview);
    ClearBackground(kBackgroundColor);
    BeginMode3D(m_previewCamera);

    DrawLine3D({size.x, 0, 0}, {200, 0, 0}, GRAY);
    DrawLine3D({0, size.y, 0}, {0, 200, 0}, GRAY);
    DrawLine3D({0, 0, size.z}, {0, 0, 200}, GRAY);

    const Vector3 centeredPos = {size.x / 2, size.y / 2, size.z / 2};
    DrawCube(centeredPos, size.x, size.y, size.z, m_previewBoxColor);
    DrawCubeWires(centeredPos, size.x, size.y, size.z, WHITE);

    EndMode3D();
    EndTextureMode();
}

void Scene::drawBoxEditPopup(int index) {
    const auto& boxes = m_model->getBoxes();

    if (ImGui::BeginPopup("##EditBox")) {
        ImGui::PushStyleColor(ImGuiCol_Text, {0.8f, 0.8f, 0.5f, 1.0f});
        ImGui::Text("Edit Box %d", index);
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();

        int s[3] = {static_cast<int>(boxes[index].m_size.x),
                    static_cast<int>(boxes[index].m_size.y),
                    static_cast<int>(boxes[index].m_size.z)};
        float c[3] = {boxes[index].m_color.r / 255.0f, boxes[index].m_color.g / 255.0f,
                      boxes[index].m_color.b / 255.0f};

        ImGui::TextUnformatted("Size  W / L / H");
        ImGui::SetNextItemWidth(200);
        if (ImGui::SliderInt3("##s", s, 1, 20)) {
            m_model->setBoxSize(index, {static_cast<float>(s[0]), static_cast<float>(s[1]),
                                        static_cast<float>(s[2])});
        }

        ImGui::TextUnformatted("Color");
        ImGui::SetNextItemWidth(200);
        if (ImGui::ColorEdit3("##c", c)) {
            m_model->setBoxColor(index, {static_cast<unsigned char>(c[0] * 255),
                                         static_cast<unsigned char>(c[1] * 255),
                                         static_cast<unsigned char>(c[2] * 255), 255});
        }

        // ── Orientari ───────────────────────────────────────
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("Allowed Orientations:");
        ImGui::Spacing();

        static const struct {
            Orientation flag;
            const char* label;
        } kOrientations[] = {
            {ORIENTATION_WLH, "(W, L, H)"}, {ORIENTATION_WHL, "(W, H, L)"},
            {ORIENTATION_LWH, "(L, W, H)"}, {ORIENTATION_LHW, "(L, H, W)"},
            {ORIENTATION_HWL, "(H, W, L)"}, {ORIENTATION_HLW, "(H, L, W)"},
        };

        m_anyOrientationHovered = false;

        for (const auto& entry : kOrientations) {
            bool checked = (boxes[index].m_allowedOrientations & entry.flag) != 0;
            if (ImGui::Checkbox(entry.label, &checked)) {
                uint8_t o = boxes[index].m_allowedOrientations;
                m_model->setBoxOrientation(index, checked ? o | entry.flag : o & ~entry.flag);
            }

            if (ImGui::IsItemHovered()) {
                m_previewOrientation = entry.flag;
                m_previewBoxSize = boxes[index].m_size;
                m_previewBoxColor = boxes[index].m_color;
                m_anyOrientationHovered = true;
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (m_anyOrientationHovered) {
            rlImGuiImageRect(&m_orientationPreview.texture, 200, 200, {0, 0, 256, -256});
        } else {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, {0.1f, 0.1f, 0.12f, 1.0f});
            ImGui::BeginChild("##PreviewPlaceholder", {200, 200}, ImGuiChildFlags_Borders);
            ImGui::SetCursorPos({40, 90});
            ImGui::PushStyleColor(ImGuiCol_Text, {0.3f, 0.3f, 0.3f, 1.0f});
            ImGui::TextUnformatted("Hover an orientation");
            ImGui::PopStyleColor();
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        if (ImGui::Button("Close", {200, 0})) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}
