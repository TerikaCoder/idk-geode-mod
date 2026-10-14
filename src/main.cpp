#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <chrono>

using namespace geode::prelude;

namespace Nova {
    static bool fpsEnabled() {
        return Mod::get()->getSavedValue<bool>("show-fps", true);
    }

    static void setFPS(bool v) {
        Mod::get()->setSavedValue<bool>("show-fps", v);
    }

    static int theme() {
        return Mod::get()->getSavedValue<int>("theme", 0);
    }

    static ccColor4B accent() {
        switch (theme()) {
            case 1: return {  0, 210, 170, 255};
            case 2: return { 80, 150, 255, 255};
            case 3: return {190, 100, 255, 255};
            case 4: return {255, 140,  70, 255};
            default:return { 40, 220, 200, 255};
        }
    }

    static CCPoint savedPosition() {
        float x = Mod::get()->getSavedValue<float>("overlay-x", -1.f);
        float y = Mod::get()->getSavedValue<float>("overlay-y", -1.f);
        return {x, y};
    }

    static void savePosition(CCPoint p) {
        Mod::get()->setSavedValue<float>("overlay-x", p.x);
        Mod::get()->setSavedValue<float>("overlay-y", p.y);
    }
}

class NovaPopup final : public Popup {
    CCLabelBMFont* m_heading = nullptr;
    CCLabelBMFont* m_status = nullptr;
    CCMenu* m_options = nullptr;
    int m_tab = 0;

    bool init() {
        if (!Popup::init(430.f, 285.f)) return false;
        this->setTitle("NOVA");

        auto bg = CCLayerColor::create({18, 23, 38, 250}, 410.f, 250.f);
        bg->setPosition({10.f, 12.f});
        m_mainLayer->addChild(bg, -1);

        auto side = CCLayerColor::create({27, 35, 53, 255}, 108.f, 236.f);
        side->setPosition({17.f, 19.f});
        m_mainLayer->addChild(side);

        auto tabs = CCMenu::create();
        tabs->setPosition({71.f, 240.f});
        m_mainLayer->addChild(tabs, 3);

        const char* names[] = {
            "Global", "Level", "Bypass", "Player", "Bot",
            "Creator", "Labels", "Shortcuts", "Interface"
        };

        for (int i = 0; i < 9; ++i) {
            auto spr = ButtonSprite::create(
                names[i], 88, true, "bigFont.fnt",
                i == m_tab ? "GJ_button_02.png" : "GJ_button_04.png",
                22.f, 0.46f
            );
            auto item = CCMenuItemSpriteExtra::create(
                spr, this, menu_selector(NovaPopup::onTab)
            );
            item->setTag(i);
            item->setPosition({0.f, -i * 25.f});
            tabs->addChild(item);
        }

        m_heading = CCLabelBMFont::create("Global", "bigFont.fnt");
        m_heading->setAnchorPoint({0.f, .5f});
        m_heading->setPosition({137.f, 242.f});
        m_heading->setScale(.62f);
        m_mainLayer->addChild(m_heading);

        m_status = CCLabelBMFont::create("NOVA settings", "bigFont.fnt");
        m_status->setAnchorPoint({0.f, .5f});
        m_status->setPosition({137.f, 28.f});
        m_status->setScale(.32f);
        m_mainLayer->addChild(m_status);

        m_options = CCMenu::create();
        m_options->setPosition({0.f, 0.f});
        m_mainLayer->addChild(m_options, 5);

        buildTab();
        return true;
    }

    void addOption(const char* title, int x, int y, int tag) {
        auto spr = ButtonSprite::create(
            title, 138, true, "bigFont.fnt",
            "GJ_button_04.png", 22.f, .40f
        );
        auto item = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(NovaPopup::onOption)
        );
        item->setTag(tag);
        item->setPosition({static_cast<float>(x), static_cast<float>(y)});
        m_options->addChild(item);
    }

    void buildTab() {
        m_options->removeAllChildren();

        const char* names[] = {
            "Global", "Level", "Bypass", "Player", "Bot",
            "Creator", "Labels", "Shortcuts", "Interface"
        };
        m_heading->setString(names[m_tab]);

        if (m_tab == 0) {
            addOption(
                Nova::fpsEnabled() ? "FPS: ON" : "FPS: OFF",
                215, 205, 1
            );
            m_status->setString("FPS overlay preference saved");
        } else if (m_tab == 8) {
            const char* colors[] = {
                "Teal", "Mint", "Blue", "Purple", "Orange"
            };
            int current = Nova::theme();
            char text[80];
            snprintf(text, sizeof(text), "Accent: %s",
                colors[(current >= 0 && current < 5) ? current : 0]);
            addOption(text, 215, 205, 2);
            m_status->setString("Tap to cycle accent color");
        } else {
            addOption("Coming in next implementation", 230, 205, 99);
            m_status->setString("This tab is not implemented yet");
        }
    }

    void onTab(CCObject* sender) {
        m_tab = static_cast<CCNode*>(sender)->getTag();
        buildTab();
    }

    void onOption(CCObject* sender) {
        int tag = static_cast<CCNode*>(sender)->getTag();

        if (tag == 1) {
            Nova::setFPS(!Nova::fpsEnabled());
            buildTab();
        } else if (tag == 2) {
            int next = (Nova::theme() + 1) % 5;
            Mod::get()->setSavedValue<int>("theme", next);
            buildTab();
        } else {
            FLAlertLayer::create("NOVA", "This option is not implemented yet.", "OK")->show();
        }
    }

public:
    static NovaPopup* create() {
        auto ret = new NovaPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

// Floating circular N button. Drag to move; tap to open NOVA.
class NovaOrb final : public CCLayer {
    CCDrawNode* m_circle = nullptr;
    CCLabelBMFont* m_letter = nullptr;
    CCPoint m_touchStart = {0.f, 0.f};
    CCPoint m_startPosition = {0.f, 0.f};
    bool m_dragging = false;
    bool m_moved = false;
    float m_radius = 24.f;

    void redraw() {
        if (!m_circle) return;
        m_circle->clear();
        auto c = Nova::accent();
        ccColor4F fill = {
            c.r / 255.f, c.g / 255.f, c.b / 255.f, .96f
        };
        ccColor4F edge = {.85f, .98f, 1.f, 1.f};
        m_circle->drawDot({0.f, 0.f}, m_radius, fill);
        m_circle->drawSegment(
            {-m_radius + 2.f, 0.f},
            {m_radius - 2.f, 0.f},
            1.f, edge
        );
    }

    bool init() {
        if (!CCLayer::init()) return false;

        auto size = CCDirector::sharedDirector()->getWinSize();
        this->setContentSize(size);
        this->setTouchEnabled(true);
        this->setTouchMode(kCCTouchesOneByOne);
        this->setTouchPriority(-500);

        m_circle = CCDrawNode::create();
        this->addChild(m_circle);
        redraw();

        m_letter = CCLabelBMFont::create("N", "goldFont.fnt");
        m_letter->setScale(.72f);
        this->addChild(m_letter);

        auto p = Nova::savedPosition();
        if (p.x < 0.f || p.y < 0.f)
            p = CCPoint(size.width - 36.f, size.height * .62f);

        setOrbPosition(p);
        return true;
    }

    void setOrbPosition(CCPoint p) {
        auto size = CCDirector::sharedDirector()->getWinSize();
        p.x = std::max(m_radius, std::min(size.width - m_radius, p.x));
        p.y = std::max(m_radius, std::min(size.height - m_radius, p.y));
        m_circle->setPosition(p);
        m_letter->setPosition(p);
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent*) {
        CCPoint p = this->convertToNodeSpace(touch->getLocation());
        CCPoint center = m_circle->getPosition();
        float dx = p.x - center.x;
        float dy = p.y - center.y;
        if (dx * dx + dy * dy > (m_radius + 8.f) * (m_radius + 8.f))
            return false;

        m_dragging = true;
        m_moved = false;
        m_touchStart = p;
        m_startPosition = center;
        return true;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent*) {
        if (!m_dragging) return;
        CCPoint p = this->convertToNodeSpace(touch->getLocation());
        float dx = p.x - m_touchStart.x;
        float dy = p.y - m_touchStart.y;

        if (dx * dx + dy * dy > 36.f) m_moved = true;
        if (m_moved)
            setOrbPosition({m_startPosition.x + dx, m_startPosition.y + dy});
    }

    void ccTouchEnded(CCTouch*, CCEvent*) {
        if (!m_dragging) return;
        m_dragging = false;

        if (m_moved) {
            Nova::savePosition(m_circle->getPosition());
            return;
        }

        auto popup = NovaPopup::create();
        if (popup) popup->show();
    }

    void registerWithTouchDispatcher() override {
        CCTouchDispatcher::get()->addTargetedDelegate(this, -500, true);
    }

public:
    static NovaOrb* create() {
        auto ret = new NovaOrb();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

class $modify(NovaMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto sprite = ButtonSprite::create(
            "NOVA", 60, true, "goldFont.fnt",
            "GJ_button_04.png", 30.f, .8f
        );
        auto item = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(NovaMenuLayer::onNova)
        );
        auto menu = CCMenu::create();
        menu->setID("nova-qol-menu");
        menu->setPosition({42.f, 80.f});
        menu->addChild(item);
        this->addChild(menu, 1000);
        return true;
    }

    void onNova(CCObject*) {
        auto popup = NovaPopup::create();
        if (popup) popup->show();
    }
};

class $modify(NovaPlayLayer, PlayLayer) {
    struct Fields {
        NovaOrb* orb = nullptr;
        CCLabelBMFont* fps = nullptr;
        int fpsFrames = 0;
        bool fpsWasEnabled = false;
        std::chrono::steady_clock::time_point fpsSampleStart{};
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        auto size = CCDirector::sharedDirector()->getWinSize();
        auto orb = NovaOrb::create();
        if (orb) {
            this->addChild(orb, 1000);
            m_fields->orb = orb;
        }

        auto fps = CCLabelBMFont::create("FPS: --", "bigFont.fnt");
        if (fps) {
            fps->setAnchorPoint({0.f, 1.f});
            fps->setPosition({10.f, size.height - 10.f});
            fps->setScale(.48f);
            fps->setID("nova-fps-overlay");
            fps->setVisible(Nova::fpsEnabled());
            this->addChild(fps, 999);
            m_fields->fps = fps;
        }
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        auto label = m_fields->fps;
        if (!label) return;

        const bool enabled = Nova::fpsEnabled();
        label->setVisible(enabled);

        if (!enabled) {
            m_fields->fpsFrames = 0;
            m_fields->fpsWasEnabled = false;
            return;
        }

        const auto now = std::chrono::steady_clock::now();

        if (!m_fields->fpsWasEnabled) {
            m_fields->fpsWasEnabled = true;
            m_fields->fpsFrames = 0;
            m_fields->fpsSampleStart = now;
            label->setString("FPS: ...");
            return;
        }

        ++m_fields->fpsFrames;

        const auto elapsed = std::chrono::duration<double>(
            now - m_fields->fpsSampleStart
        ).count();

        if (elapsed >= 0.5) {
            const int fps = static_cast<int>(
                m_fields->fpsFrames / elapsed + 0.5
            );

            auto text = fmt::format("FPS: {}", fps);
            label->setString(text.c_str());

            m_fields->fpsFrames = 0;
            m_fields->fpsSampleStart = now;
        }
    }
};
