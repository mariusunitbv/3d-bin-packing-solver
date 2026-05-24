module;
#include <pch.h>

module loader;

ProblemLoader::ProblemLoader() {
    m_defaultModel.setContainerSize({5, 5, 5});

    m_defaultModel.addBox({{1, 1, 1}, RED});
    m_defaultModel.addBox({{1, 2, 3}, GREEN});
    m_defaultModel.addBox({{2, 1, 1}, BLUE});
}

ProblemLoader& ProblemLoader::get() {
    static ProblemLoader instance;
    return instance;
}

ProblemModel ProblemLoader::loadFromFilePicker() {
#ifdef __EMSCRIPTEN__
    return m_defaultModel;
#else
    const char* filterPatterns[] = {"*.json"};
    auto result =
        tinyfd_openFileDialog("Select Model File", "", 1, filterPatterns, "JSON Files (*.json)", 0);
    if (!result) return m_defaultModel;
    return loadFromFile(result);
#endif
}

void ProblemLoader::saveToFilePicker(const ProblemModel& model) {
#ifndef __EMSCRIPTEN__
    const char* filterPatterns[] = {"*.json"};
    auto result = tinyfd_saveFileDialog("Save Model File", "model.json", 1, filterPatterns,
                                        "JSON Files (*.json)");
    if (!result) return;
    saveToFile(model, result);
#endif
}

ProblemModel ProblemLoader::loadFromFile(const std::string& filename) {
#ifdef __EMSCRIPTEN__
    return m_defaultModel;
#else
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return m_defaultModel;
    }

    nlohmann::json j;

    try {
        file >> j;
        ProblemModel model;
        auto containerSize = j.at("container_size");
        model.setContainerSize({containerSize[0], containerSize[1], containerSize[2]});
        for (const auto& box : j.at("boxes")) {
            auto size = box.at("size");
            auto color = box.at("color");
            auto orientations = box.at("allowed_orientations");
            model.addBox(
                {{size[0], size[1], size[2]},
                 {static_cast<unsigned char>(color[0]), static_cast<unsigned char>(color[1]),
                  static_cast<unsigned char>(color[2]), 255},
                 static_cast<uint8_t>(orientations)});
        }

        std::cout << "Loaded model from " << filename << std::endl;

        return model;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return m_defaultModel;
    }
#endif
}

void ProblemLoader::saveToFile(const ProblemModel& model, const std::string& filename) {
#ifndef __EMSCRIPTEN__
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    nlohmann::json j;

    j["container_size"] = {model.getContainerSize().x, model.getContainerSize().y,
                           model.getContainerSize().z};

    for (const auto& box : model.getBoxes()) {
        j["boxes"].push_back({{"size", {box.m_size.x, box.m_size.y, box.m_size.z}},
                              {"color", {box.m_color.r, box.m_color.g, box.m_color.b}},
                              {"allowed_orientations", box.m_allowedOrientations}});
    }

    file << j.dump(4);
#endif
}
