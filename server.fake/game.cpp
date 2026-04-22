#include "bmsg/rogue.hpp"
#include "modlib_mod.hpp"
#include "modlib/BmServerPlugin.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <unordered_map>

class Roguelike : public modlib::BmServerPlugin {
public:
    std::string_view id() const override { return "isd.bms.rogue"; }
    std::string_view brief() const override { return "A simple roguelike"; }

    struct MapTile {
        modlib::BmClient *player = nullptr;
        char ch;
    };

    struct PlayerInfo {
        size_t x, y;
        int hp;
    };

    std::vector<std::vector<MapTile>> m_map;
    std::unordered_map<modlib::BmClient*, PlayerInfo> m_playerInfo;

    size_t width() const { return m_map[0].size(); }
    size_t height() const { return m_map.size(); }

    void onSetup(modlib::BmServer *server) override {
        server->registerPrefix("rogue", this);
    }

    void onConnect(modlib::BmClient *client) override {
        while (1) {
            size_t x = rand() % width(), y = rand() % height();
            if (m_map[y][x].player != nullptr || m_map[y][x].ch != '.')
                continue;
            m_map[y][x].player = client;
            m_playerInfo[client] = PlayerInfo { .x = x, .y = y, .hp = 10 };
            break;
        }
    }

    void moveTo(modlib::BmClient *player, size_t x, size_t y) {
        auto &info = m_playerInfo[player];
        m_map[info.y][info.x].player = nullptr;
        m_map[y][x].player = player;
        info.x = x; info.y = y;
    }

    void onMessage(modlib::BmClient *cl, const bmsg::Message *msg, size_t len) override {
        if (!msg->hasPref("rogue"))
            return;
        if (msg->hasType("move")) {
            auto ch = bmsg::CL_Rogue_Move::decode(msg, len);
            if (!ch) return;
            auto info = m_playerInfo[cl];
            if (info.hp == 0) return;
            if (abs(ch->dx) > 1 || abs(ch->dy) > 1) return;
            info.x += ch->dx;
            info.y += ch->dy;
            if (m_map[info.x][info.y].ch != '.') return;
            if (m_map[info.x][info.y].player) {
                auto enemy = m_map[info.x][info.y].player;
                int hp = --m_playerInfo[enemy].hp;
                if (hp == 0) {
                    cl->send(bmsg::CL_Rogue_Message(""))
                    moveTo(cl, info.x, info.y);
                }
            } else {
                moveTo(cl, info.x, info.y);
            }
        }
    }
};
