#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <string>

using namespace geode::prelude;

class $modify(FPSInfoPlayLayer, PlayLayer) {
    struct Fields {
        int frameCount = 0;
        float elapsed = 0.0f;
        CCLabelBMFont* label = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        auto label = CCLabelBMFont::create(
            "FPS: --\nLevel: Loading...\nMode: Normal",
            "bigFont.fnt"
        );

        if (!label) {
            log::warn("FPS overlay: could not create label");
            return true;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        label->setAnchorPoint({0.0f, 1.0f});
        label->setPosition({12.0f, winSize.height - 12.0f});
        label->setScale(0.45f);
        label->setColor({255, 255, 255});
        label->setOpacity(220);
        label->setID("idk-fps-info-overlay");

        this->addChild(label, 9999);
        m_fields->label = label;

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        auto label = m_fields->label;
        if (!label || dt <= 0.0f) {
            return;
        }

        m_fields->frameCount++;
        m_fields->elapsed += dt;

        // Update the displayed FPS twice per second.
        if (m_fields->elapsed < 0.5f) {
            return;
        }

        int fps = static_cast<int>(
            m_fields->frameCount / m_fields->elapsed + 0.5f
        );

        m_fields->frameCount = 0;
        m_fields->elapsed = 0.0f;

        std::string levelName = "Unknown";
        if (m_level && !m_level->m_levelName.empty()) {
            levelName = m_level->m_levelName.c_str();
        }

        const char* mode = m_isPracticeMode ? "Practice" : "Normal";

        std::string text =
            "FPS: " + std::to_string(fps) +
            "\nLevel: " + levelName +
            "\nMode: " + mode;

        label->setString(text.c_str());
    }
};
