#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

namespace Nova {
    static bool fpsEnabled() {
        return Mod::get()->getSavedValue<bool>("show-fps", true);
    }

    static void setFpsEnabled(bool value) {
        Mod::get()->setSavedValue<bool>("show-fps", value);
    }

    static float savedX() {
        return Mod::get()->getSavedValue<float>("overlay-x", -1.f);
    }

    static float savedY() {
        return Mod::get()->getSavedValue<float>("overlay-y", -1.f);
    }

    static void savePosition(CCPoint p) {
        Mod::get()->setSavedValue<float>("overlay-x", p.x);
        Mod::get()->setSavedValue<float>("overlay-y", p.y);
    }
}

class NovaPopup final : public Popup {
protected:
    CCMenu* m_options = nullptr;
    CCLabelBMFont* m_heading = nullptr;
    CCLabelBMFont* m_status = nullptr;
    int m_category = 1;

    bool init() {
        if (!Popup::init(440.f, 292.f))
            return false;

        this->setTitle("NOVA");

        auto panel = CCLayerColor::create(
            {28, 31, 49, 245}, 420.f, 258.f
        );
        panel->setPosition({10.f, 12.f});
        m_mainLayer->addChild(panel, -1);

        auto sidebar = CCLayerColor::create(
            {37, 41, 62, 255}, 112.f, 244.f
        );
        sidebar->setPosition({17.f, 19.f});
        m_mainLayer->addChild(sidebar);

        auto categories = CCMenu::create();
        categories->setPosition({73.f, 250.f});
        m_mainLayer->addChild(categories, 2);

        const char* names[] = {
            "Global", "Level", "Bypass", "Player", "Bot",
            "Creator", "Labels", "Shortcuts", "Interface"
        };

        for (int i = 0; i < 9; ++i) {
            auto sprite = ButtonSprite::create(
                names[i], 88, true, "bigFont.fnt",
                i == m_category ? "GJ_button_02.png" : "GJ_button_04.png",
                22.f, 0.48f
            );
            auto item = CCMenuItemSpriteExtra::create(
                sprite, this, menu_selector(NovaPopup::onCategory)
            );
            item->setTag(i);
            item->setPosition({0.f, -i * 26.f});
            categories->addChild(item);
        }

        m_heading = CCLabelBMFont::create("LEVEL", "bigFont.fnt");
        m_heading->setAnchorPoint({0.f, 0.5f});
        m_heading->setScale(0.58f);
        m_heading->setPosition({143.f, 251.f});
        m_mainLayer->addChild(m_heading);

        m_status = CCLabelBMFont::create(
            "FPS overlay is enabled", "bigFont.fnt"
        );
        m_status->setAnchorPoint({0.f, 0.5f});
        m_status->setScale(0.34f);
        m_status->setPosition({143.f, 30.f});
        m_mainLayer->addChild(m_status);

        m_options = CCMenu::create();
        m_options->setPosition({0.f, 0.f});
        m_mainLayer->addChild(m_options, 3);

        buildOptions();
        return true;
    }

    void addRow(const char* title, int column, int row, bool checked,
                int tag, bool enabled = false) {
        const float x = column == 0 ? 151.f : 307.f;
        const float y = 218.f - row * 24.f;

        auto boxSprite = ButtonSprite::create(
            checked ? "✓" : "", 23, true, "bigFont.fnt",
            checked ? "GJ_button_02.png" : "GJ_button_04.png",
            20.f, 0.58f
        );
        auto box = CCMenuItemSpriteExtra::create(
            boxSprite, this, menu_selector(NovaPopup::onOption)
        );
        box->setTag(tag);
        box->setPosition({x, y});
        m_options->addChild(box);

        auto label = CCLabelBMFont::create(title, "bigFont.fnt");
        label->setAnchorPoint({0.f, 0.5f});
        label->setScale(0.37f);
        label->setPosition({x + 16.f, y});
        label->setOpacity(enabled ? 255 : 205);
        m_options->addChild(label);
    }

    void buildOptions() {
        m_options->removeAllChildren();

        const char* names[] = {
            "Global", "Level", "Bypass", "Player", "Bot",
            "Creator", "Labels", "Shortcuts", "Interface"
        };
        m_heading->setString(names[m_category]);

        if (m_category == 0) {
            addRow("Show FPS overlay", 0, 0, Nova::fpsEnabled(), 100, true);
            addRow("Show NOVA button", 0, 1, true, 101, true);
            addRow("Remember position", 0, 2, true, 102, true);
            addRow("Compact mode", 1, 0, false, 199);
            addRow("Extra animations", 1, 1, false, 199);
            m_status->setString("FPS and overlay preferences are saved");
            return;
        }

        if (m_category == 1) {
            const char* left[] = {
                "0% Practice Completion", "All Platformer Modes",
                "Always Show Coins", "Auto Pickup Coins",
                "Auto Song Download", "Confirm Full Restart",
                "Confirm Restart", "Freeze Attempts", "Hide Pause Menu"
            };
            const char* right[] = {
                "Accurate Percentage", "All Passable", "Auto LDM",
                "Auto Practice Mode", "Checkpoint Delay",
                "Confirm Practice Mode", "Force Platformer",
                "Hide Complete VFX", "Hitbox Multiplier"
            };

            for (int i = 0; i < 9; ++i) {
                addRow(left[i], 0, i, false, 199);
                addRow(right[i], 1, i, false, 199);
            }
            m_status->setString("Level options not implemented yet");
            return;
        }

        if (m_category == 4) {
            addRow("Recording", 0, 0, false, 199);
            addRow("Playback", 0, 1, false, 199);
            addRow("Save replay", 1, 0, false, 199);
            addRow("Load replay", 1, 1, false, 199);
            m_status->setString("Bot replay engine not implemented");
            return;
        }

        addRow("Feature unavailable", 0, 0, false, 199);
        addRow("Not implemented yet", 0, 1, false, 199);
        m_status->setString("These controls are placeholders");
    }

    void onCategory(CCObject* sender) {
        m_category = static_cast<CCNode*>(sender)->getTag();
        buildOptions();
    }

    void onOption(CCObject* sender) {
        int tag = static_cast<CCNode*>(sender)->getTag();

        if (m_category == 0 && tag == 100) {
            Nova::setFpsEnabled(!Nova::fpsEnabled());
            buildOptions();
            return;
        }

        if (tag == 101 || tag == 102) {
            FLAlertLayer::create(
                "NOVA",
                "This control is not connected yet.",
                "OK"
            )->show();
            return;
        }

        FLAlertLayer::create(
            "NOVA",
            "This feature is not implemented yet.",
            "OK"
        )->show();
    }

public:
    static NovaPopup* create() {
        auto ret = new NovaPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

class $modify(NovaMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        auto sprite = ButtonSprite::create(
            "NOVA", 60, true, "goldFont.fnt",
            "GJ_button_04.png", 30.f, 0.8f
        );
        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(NovaMenuLayer::onNova)
        );
        auto menu = CCMenu::create();
        menu->setID("nova-qol-menu");
        menu->setPosition({42.f, 80.f});
        menu->addChild(button);
        this->addChild(menu, 1000);
        return true;
    }

    void onNova(CCObject*) {
        auto popup = NovaPopup::create();
        if (popup)
            popup->show();
    }
};

class $modify(NovaPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* fps = nullptr;
        CCMenu* overlay = nullptr;
        int frames = 0;
        float elapsed = 0.f;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        auto size = CCDirector::sharedDirector()->getWinSize();

        auto fps = CCLabelBMFont::create("FPS: --", "bigFont.fnt");
        if (fps) {
            fps->setAnchorPoint({0.f, 1.f});
            fps->setPosition({10.f, size.height - 10.f});
            fps->setScale(0.48f);
            fps->setID("nova-fps-overlay");
            fps->setVisible(Nova::fpsEnabled());
            this->addChild(fps, 999);
            m_fields->fps = fps;
        }

        auto sprite = ButtonSprite::create(
            "NOVA", 54, true, "goldFont.fnt",
            "GJ_button_04.png", 28.f, 0.75f
        );
        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(NovaPlayLayer::onNova)
        );

        auto menu = CCMenu::create();
        menu->setID("nova-game-overlay-menu");

        float x = Nova::savedX();
        float y = Nova::savedY();
        if (x < 0.f || y < 0.f) {
            x = size.width - 48.f;
            y = size.height * 0.62f;
        }
        menu->setPosition({x, y});
        menu->addChild(button);
        this->addChild(menu, 1000);
        m_fields->overlay = menu;

        return true;
    }

    void onNova(CCObject*) {
        auto popup = NovaPopup::create();
        if (popup)
            popup->show();
    }

    void update(float dt) {
        PlayLayer::update(dt);

        auto label = m_fields->fps;
        if (!label)
            return;

        label->setVisible(Nova::fpsEnabled());
        if (!Nova::fpsEnabled() || dt <= 0.f)
            return;

        ++m_fields->frames;
        m_fields->elapsed += dt;

        if (m_fields->elapsed >= 0.5f) {
            int value = static_cast<int>(
                m_fields->frames / m_fields->elapsed + 0.5f
            );
            m_fields->frames = 0;
            m_fields->elapsed = 0.f;
            auto text = fmt::format("FPS: {}", value);
            label->setString(text.c_str());
        }
    }
};
