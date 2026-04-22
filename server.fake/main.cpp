#include "bmsg.hpp"
#include "modlib/BmServerPlugin.hpp"
#include "bmsg/rogue.hpp"
#include "modlib_manager.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <cstring>
#include <unordered_set>

class FakeServer;

class FakeClient : public modlib::BmClient {
    friend class FakeServer;
    FakeServer *m_server;
    size_t m_id;
public:

    FakeServer *server() const {
        return m_server;
    }

    virtual size_t id() const {
        return m_id;
    }

    // this just handles the message
    virtual void send(const bmsg::Message *msg, size_t len) = 0;

    virtual void process() = 0;
};

class FakeServer : public modlib::BmServer {
    std::unordered_map<uint32_t, FakeClient*> m_clients;
    std::unordered_map<uint64_t, modlib::BmServerModule*> m_prefMapping;
    std::unordered_set<modlib::BmServerModule*> m_plugins;
    size_t m_lastId = 0;
public:

    void registerPrefix(std::string_view pref, modlib::BmServerModule *plugin) override {
        assert(pref.size() <= 8);
        union { char asChar64[8]; uint64_t asUint64; };
        asUint64 = 0;
        memcpy(asChar64, pref.data(), pref.size());
        m_prefMapping[asUint64] = plugin;
    }

    std::vector<modlib::BmClient*> allClients() override {
        std::vector<modlib::BmClient*> res;
        res.reserve(m_clients.size());
        for (auto [id, i] : m_clients)
            res.push_back(i);
        return res;
    }

    void addPlugin(modlib::BmServerModule *plugin) {
        m_plugins.insert(plugin);
        plugin->onSetup(this);
    }

    void addClient(FakeClient *client) {
        size_t id = ++m_lastId;
        m_clients[id] = client;
        client->m_server = this;
        client->m_id = id;
        for (auto i : m_plugins)
            i->onConnect(client);
    }

    void toPlugin(FakeClient *from, const bmsg::Message *msg, size_t size) {
        if (!m_prefMapping.count(msg->pref_asU64))
            return;
        m_prefMapping[msg->pref_asU64]->onMessage(from, msg, size);
    }

    template<typename T>
    void toPlugin(FakeClient *from, const T& msg) {
        std::ostringstream oss;
        msg.encode(oss);
        toPlugin(from, (const bmsg::Message *) oss.str().data(), oss.str().size());
    }
};

class ServerApi : public modlib::BmServerModule {
public:
    std::string_view id() const override { return "-"; }
    std::string_view brief() const override { return "Builtin server (srv:*) API"; }

    void onSetup(modlib::BmServer *server) override {
        server->registerPrefix("srv", this);
    }

    void onConnect(modlib::BmClient *client) override {}

    void onMessage(modlib::BmClient *cl, const bmsg::Message *msg, size_t len) override {
        return;
    }
};

class HelloClient : public FakeClient {
public:
    virtual void send(const bmsg::Message *msg, size_t len) override {
        if (!msg->hasPref("hello") || !msg->hasType("simple"))
            return;
        auto ch = bmsg::CL_Rogue_Move::decode(msg, len);
        if (!ch) return;
    }

    void process() override {
        // ...
    }
};

int main() {
    ModManager mm;
    mm.loadAllFromDir(".");
    mm.initLoaded();

    FakeServer server;

    auto sps = mm.allOfType<modlib::BmServerModule>();
    for (auto i : sps)
      server.addPlugin(i);

    HelloClient cl;
    server.addClient(&cl);
}
