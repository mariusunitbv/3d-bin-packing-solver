module;
#include <pch.h>

export module loader;

import model;

export class ProblemLoader {
   public:
    static ProblemLoader& get();

    ProblemModel loadFromFilePicker();
    void saveToFilePicker(const ProblemModel& model);

    ProblemModel loadFromFile(const std::string& filename = "last.json");
    void saveToFile(const ProblemModel& model, const std::string& filename = "last.json");

   private:
    ProblemLoader();

    ProblemModel m_defaultModel;
};
