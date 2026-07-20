#pragma once

#include <atomic>
#include <functional>
#include <string>
#include <thread>

// Minimal MQTT 3.1.1 subscriber for mobility hazard topics (/incidents, hazards/#).
class MqttHazardClient
{
public:
    using MessageCallback = std::function<void(const std::string &topic, const std::string &payload)>;

    MqttHazardClient() = default;
    ~MqttHazardClient();

    bool start(const std::string &host, int port, const std::string &clientId, MessageCallback callback);
    void stop();

private:
    bool connectAndSubscribe(int sock, const std::string &clientId);
    void runLoop(int sock);

    std::thread _thread;
    std::atomic_bool _running{false};
    MessageCallback _callback;
    std::string _host;
    int _port = 1883;
    std::string _clientId;
};
