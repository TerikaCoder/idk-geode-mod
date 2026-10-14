#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

namespace Nova {
    static bool fpsEnabled() {
        return Mod::get()->getSavedValue<bool>("show-fps", true);
    }

    static void setFpsEnabled(bool enabled) {
        Mod::get()->setSavedValue<bool>("show-fps", enabled);
    }
}

class NovaPopup final : public Popup {
protected:
    CCLabelBMFont* m_content = nullptr;
    CCLabelBMFont* m_fpsStatus = nullptr;

    bool init() {
        if (!Popup::init(320.f, 220.f))
            return false;

        this->setTitle("NOVA QoL");

        auto subtitle = CCLabelBMFont::create(
            "QUALITY OF LIFE  /  ANDROID",
            "goldFont.fnt"
        );
        if (subtitle) {
            subtitle->setScale(0.38f);
            subtitle->setOpacity(190);
            subtitle->setPosition({160.f, 174.f});
            m_mainLayer->addChild(subtitle);
        }

        auto tabs = CCMenu::create();
        tabs->setPosition({160.f, 145.f});
        m_mainLayer->addChild(tabs);

        auto makeTab = [this, tabs](
            char const* title,
            SEL_MenuHandler callback,
            float x
        ) {
            auto sprite = ButtonSprite::create(
                title, 48, true, "goldFont.fnt",
                "GJ_button_04.png", 24.f, 0.65f
            );
            auto item = CCMenuItemSpriteExtra::create(
                sprite, this, callback
            );
            item->setPosition({x, 0.f});
            tabs->addChild(item);
        };

        makeTab("HOME", menu_selector(NovaPopup::onHome), -102.f);
        makeTab("QOL", menu_selector(NovaPopup::onQol), -34.f);
        makeTab("BOT", menu_selector(NovaPopup::onBot), 34.f);
        makeTab("SET", menu_selector(NovaPopup::onSettings), 102.f);

        m_content = CCLabelBMFont::create("", "bigFont.fnt");
        if (m_content) {
            m_content->setAnchorPoint({0.5f, 0.5f});
            m_content->setScale(0.43f);
            m_content->setPosition({160.f, 95.f});
            m_content->setWidth(650.f);
            m_mainLayer->addChild(m_content);
        }

        auto actions = CCMenu::create();
        actions->setPosition({160.f, 43.f});
        m_mainLayer->addChild(actions);

        auto fpsSprite = ButtonSprite::create(
            "Toggle FPS", 90, true, "goldFont.fnt",
            "GJ_button_01.png", 30.f, 0.7f
        );
        auto fpsButton = CCMenuItemSpriteExtra::create(
            fpsSprite, this, menu_selector(NovaPopup::onToggleFps)
        );
        fpsButton->setPosition({-67.f, 0.f});
        actions->addChild(fpsButton);

        auto botSprite = ButtonSprite::create(
            "Bot status", 90, true, "goldFont.fnt",
            "GJ_button_02.png", 30.f, 0.7f
        );
        auto botButton = CCMenuItemSpriteExtra::create(
            botSprite, this, menu_selector(NovaPopup::onBot)
        );
        botButton->setPosition({67.f, 0.f});
        actions->addChild(botButton);

        this->showHome();
        return true;
    }

    void setContent(char const* text) {
        if (m_content)
            m_content->setString(text);
    }

    void showHome() {
        setContent(
            "NOVA QoL\n"
            "A lightweight in-game control panel.\n\n"
            "Open QOL to configure the FPS overlay.\n"
            "Open BOT to view replay feature status."
        );
    }

    void onHome(CCObject*) {
        this->showHome();
    }

    void onQol(CCObject*) {
        this->refreshQol();
    }

    void refreshQol() {
        setContent(
            Nova::fpsEnabled()
                ? "QOL SETTINGS\n\nFPS overlay: ON\n\nUse Toggle FPS below to disable it."
                : "QOL SETTINGS\n\nFPS overlay: OFF\n\nUse Toggle FPS below to enable it."
        );
    }

    void onToggleFps(CCObject*) {
        Nova::setFpsEnabled(!Nova::fpsEnabled());
        this->refreshQol();
    }

    void onBot(CCObject*) {
        setContent(
            "BOT SHOWCASE\n\n"
            "Replay engine: NOT IMPLEMENTED\n\n"
            "This panel does not pretend to record or play macros yet."
        );
    }

    void onSettings(CCObject*) {
        setContent(
            "SETTINGS\n\n"
            "FPS preference is saved automatically.\n"
            "More appearance controls can be added here."
        );
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

        // A separate menu avoids depending on the game's internal menu layout.
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
        CCLabelBMFont* label = nullptr;
        int frames = 0;
        float elapsed = 0.f;
    };

    bool init(
        GJGameLevel* level,
        bool useReplay,
        bool dontCreateObjects
    ) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        if (!Nova::fpsEnabled())
            return true;

        auto label = CCLabelBMFont::create(
            "NOVA QoL  |  FPS: --",
            "bigFont.fnt"
        );

        if (!label)
            return true;

        auto size = CCDirector::sharedDirector()->getWinSize();
        label->setAnchorPoint({0.f, 1.f});
        label->setPosition({10.f, size.height - 10.f});
        label->setScale(0.42f);
        label->setOpacity(225);
        label->setID("nova-fps-overlay");

        this->addChild(label, 999);
        m_fields->label = label;

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        auto label = m_fields->label;
        if (!label)
            return;

        if (!Nova::fpsEnabled()) {
            label->setVisible(false);
            return;
        }

        label->setVisible(true);

        if (dt <= 0.f)
            return;

        m_fields->frames++;
        m_fields->elapsed += dt;

        if (m_fields->elapsed < 0.5f)
            return;

        int fps = static_cast<int>(
            m_fields->frames / m_fields->elapsed + 0.5f
        );

        m_fields->frames = 0;
        m_fields->elapsed = 0.f;

        auto text = fmt::format("NOVA QoL  |  FPS: {}", fps);
        label->setString(text.c_str());
    }
};
