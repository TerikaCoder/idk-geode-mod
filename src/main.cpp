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

    bool init() {
        if (!Popup::init(340.f, 240.f))
            return false;

        this->setTitle("NOVA QoL");

        auto subtitle = CCLabelBMFont::create(
            "IN-GAME CONTROL PANEL",
            "goldFont.fnt"
        );
        if (subtitle) {
            subtitle->setScale(0.38f);
            subtitle->setOpacity(190);
            subtitle->setPosition({170.f, 190.f});
            m_mainLayer->addChild(subtitle);
        }

        auto tabs = CCMenu::create();
        tabs->setPosition({170.f, 157.f});
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

        makeTab("HOME", menu_selector(NovaPopup::onHome), -112.f);
        makeTab("QOL", menu_selector(NovaPopup::onQol), -38.f);
        makeTab("BOT", menu_selector(NovaPopup::onBot), 38.f);
        makeTab("SET", menu_selector(NovaPopup::onSettings), 112.f);

        m_content = CCLabelBMFont::create("", "bigFont.fnt");
        if (m_content) {
            m_content->setAnchorPoint({0.5f, 0.5f});
            m_content->setScale(0.43f);
            m_content->setPosition({170.f, 105.f});
            m_content->setWidth(680.f);
            m_mainLayer->addChild(m_content);
        }

        auto actions = CCMenu::create();
        actions->setPosition({170.f, 45.f});
        m_mainLayer->addChild(actions);

        auto fpsSprite = ButtonSprite::create(
            Nova::fpsEnabled() ? "FPS: ON" : "FPS: OFF",
            95, true, "goldFont.fnt",
            "GJ_button_01.png", 30.f, 0.7f
        );
        auto fpsButton = CCMenuItemSpriteExtra::create(
            fpsSprite, this, menu_selector(NovaPopup::onToggleFps)
        );
        fpsButton->setPosition({-65.f, 0.f});
        actions->addChild(fpsButton);

        auto botSprite = ButtonSprite::create(
            "Bot status", 95, true, "goldFont.fnt",
            "GJ_button_02.png", 30.f, 0.7f
        );
        auto botButton = CCMenuItemSpriteExtra::create(
            botSprite, this, menu_selector(NovaPopup::onBot)
        );
        botButton->setPosition({65.f, 0.f});
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
            "In-game control panel\n\n"
            "QOL: FPS overlay controls\n"
            "BOT: replay feature status\n"
            "SET: saved preferences"
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
                ? "QOL SETTINGS\n\nFPS overlay: ON\n\nTap FPS: ON below to disable."
                : "QOL SETTINGS\n\nFPS overlay: OFF\n\nTap FPS: OFF below to enable."
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
            "Recording and playback are not available yet."
        );
    }

    void onSettings(CCObject*) {
        setContent(
            "SETTINGS\n\n"
            "FPS preference is saved automatically.\n"
            "More controls require implementation."
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

        auto size = CCDirector::sharedDirector()->getWinSize();

        // Always create the FPS label so it can be enabled mid-level.
        auto label = CCLabelBMFont::create(
            "FPS: --",
            "bigFont.fnt"
        );

        if (label) {
            label->setAnchorPoint({0.f, 1.f});
            label->setPosition({10.f, size.height - 10.f});
            label->setScale(0.48f);
            label->setOpacity(235);
            label->setID("nova-fps-overlay");
            label->setVisible(Nova::fpsEnabled());
            this->addChild(label, 999);
            m_fields->label = label;
        }

        // Separate overlay menu: visible during gameplay.
        auto buttonSprite = ButtonSprite::create(
            "NOVA", 54, true, "goldFont.fnt",
            "GJ_button_04.png", 28.f, 0.75f
        );

        auto button = CCMenuItemSpriteExtra::create(
            buttonSprite, this, menu_selector(NovaPlayLayer::onNova)
        );

        auto menu = CCMenu::create();
        menu->setID("nova-game-overlay-menu");
        menu->setPosition({size.width - 48.f, size.height * 0.62f});
        button->setPosition({0.f, 0.f});
        menu->addChild(button);
        this->addChild(menu, 1000);

        return true;
    }

    void onNova(CCObject*) {
        auto popup = NovaPopup::create();
        if (popup)
            popup->show();
    }

    void update(float dt) {
        PlayLayer::update(dt);

        auto label = m_fields->label;
        if (!label)
            return;

        label->setVisible(Nova::fpsEnabled());
        if (!Nova::fpsEnabled() || dt <= 0.f)
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

        auto text = fmt::format("FPS: {}", fps);
        label->setString(text.c_str());
    }
};
