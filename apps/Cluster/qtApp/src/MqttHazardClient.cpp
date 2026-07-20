#include "MqttHazardClient.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <chrono>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace {

constexpr int kKeepAliveSec = 60;

bool writeAll(int sock, const uint8_t *data, size_t len)
{
    size_t sent = 0;
    while (sent < len) {
        const ssize_t n = ::send(sock, data + sent, len - sent, 0);
        if (n <= 0)
            return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

bool readByte(int sock, uint8_t &out)
{
    const ssize_t n = ::recv(sock, &out, 1, 0);
    return n == 1;
}

bool decodeRemainingLength(int sock, uint32_t &value)
{
    value = 0;
    uint32_t multiplier = 1;
    for (int i = 0; i < 4; ++i) {
        uint8_t encoded = 0;
        if (!readByte(sock, encoded))
            return false;
        value += static_cast<uint32_t>(encoded & 0x7F) * multiplier;
        if ((encoded & 0x80) == 0)
            return true;
        multiplier *= 128;
    }
    return false;
}

std::vector<uint8_t> encodeRemainingLength(uint32_t length)
{
    std::vector<uint8_t> out;
    do {
        uint8_t encoded = static_cast<uint8_t>(length % 128);
        length /= 128;
        if (length > 0)
            encoded |= 0x80;
        out.push_back(encoded);
    } while (length > 0);
    return out;
}

void appendUtf8String(std::vector<uint8_t> &buf, const std::string &text)
{
    const uint16_t len = static_cast<uint16_t>(text.size());
    buf.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>(len & 0xFF));
    buf.insert(buf.end(), text.begin(), text.end());
}

std::vector<uint8_t> buildConnect(const std::string &clientId)
{
    std::vector<uint8_t> payload;
    appendUtf8String(payload, "MQTT");
    payload.push_back(4); // protocol level 3.1.1
    payload.push_back(0x02); // clean session
    payload.push_back(static_cast<uint8_t>((kKeepAliveSec >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(kKeepAliveSec & 0xFF));
    appendUtf8String(payload, clientId);

    std::vector<uint8_t> packet;
    packet.push_back(0x10);
    const auto rem = encodeRemainingLength(static_cast<uint32_t>(payload.size()));
    packet.insert(packet.end(), rem.begin(), rem.end());
    packet.insert(packet.end(), payload.begin(), payload.end());
    return packet;
}

std::vector<uint8_t> buildSubscribe(uint16_t packetId, const std::vector<std::string> &topics)
{
    std::vector<uint8_t> payload;
    payload.push_back(static_cast<uint8_t>((packetId >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(packetId & 0xFF));
    for (const auto &topic : topics) {
        appendUtf8String(payload, topic);
        payload.push_back(0); // QoS 0
    }

    std::vector<uint8_t> packet;
    packet.push_back(0x82);
    const auto rem = encodeRemainingLength(static_cast<uint32_t>(payload.size()));
    packet.insert(packet.end(), rem.begin(), rem.end());
    packet.insert(packet.end(), payload.begin(), payload.end());
    return packet;
}

std::vector<uint8_t> buildPingResp()
{
    return {0xD0, 0x00};
}

std::vector<uint8_t> buildPubAck(uint16_t packetId)
{
    return {0x40, 0x02,
            static_cast<uint8_t>((packetId >> 8) & 0xFF),
            static_cast<uint8_t>(packetId & 0xFF)};
}

bool readExact(int sock, std::vector<uint8_t> &buf, size_t len)
{
    buf.resize(len);
    size_t got = 0;
    while (got < len) {
        const ssize_t n = ::recv(sock, buf.data() + got, len - got, 0);
        if (n <= 0)
            return false;
        got += static_cast<size_t>(n);
    }
    return true;
}

uint16_t readUint16(const uint8_t *data)
{
    return static_cast<uint16_t>((data[0] << 8) | data[1]);
}

std::string readUtf8At(const uint8_t *data, size_t len, size_t &offset)
{
    if (offset + 2 > len)
        return {};
    const uint16_t strLen = readUint16(data + offset);
    offset += 2;
    if (offset + strLen > len)
        return {};
    std::string out(reinterpret_cast<const char *>(data + offset), strLen);
    offset += strLen;
    return out;
}

} // namespace

MqttHazardClient::~MqttHazardClient()
{
    stop();
}

bool MqttHazardClient::start(const std::string &host, int port, const std::string &clientId, MessageCallback callback)
{
    if (_running)
        return true;

    _host = host;
    _port = port;
    _clientId = clientId;
    _callback = std::move(callback);
    _running = true;

    _thread = std::thread([this]() {
        while (_running) {
            addrinfo hints{};
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;

            addrinfo *result = nullptr;
            const std::string portStr = std::to_string(_port);
            if (getaddrinfo(_host.c_str(), portStr.c_str(), &hints, &result) != 0) {
                std::cerr << "[MQTT] Failed to resolve broker host " << _host << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
                continue;
            }

            int sock = -1;
            for (addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
                sock = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if (sock < 0)
                    continue;
                if (::connect(sock, rp->ai_addr, rp->ai_addrlen) == 0)
                    break;
                ::close(sock);
                sock = -1;
            }
            freeaddrinfo(result);

            if (sock < 0) {
                std::cerr << "[MQTT] Failed to connect to " << _host << ":" << _port << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
                continue;
            }

            if (!connectAndSubscribe(sock, _clientId)) {
                std::cerr << "[MQTT] Handshake/subscribe failed" << std::endl;
                ::close(sock);
                std::this_thread::sleep_for(std::chrono::seconds(3));
                continue;
            }

            std::cout << "[MQTT] Connected to " << _host << ":" << _port << std::endl;
            runLoop(sock);
            ::close(sock);

            if (_running)
                std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    });

    return true;
}

void MqttHazardClient::stop()
{
    _running = false;
    if (_thread.joinable())
        _thread.join();
}

bool MqttHazardClient::connectAndSubscribe(int sock, const std::string &clientId)
{
    const auto connectPkt = buildConnect(clientId);
    if (!writeAll(sock, connectPkt.data(), connectPkt.size()))
        return false;

    uint8_t type = 0;
    if (!readByte(sock, type) || (type & 0xF0) != 0x20)
        return false;

    uint32_t rem = 0;
    if (!decodeRemainingLength(sock, rem))
        return false;

    std::vector<uint8_t> connack;
    if (!readExact(sock, connack, rem) || connack.empty() || connack[0] != 0x00)
        return false;

    const std::vector<std::string> topics = {"/incidents", "hazards/#"};
    const auto subPkt = buildSubscribe(1, topics);
    if (!writeAll(sock, subPkt.data(), subPkt.size()))
        return false;

    if (!readByte(sock, type) || (type & 0xF0) != 0x90)
        return false;
    if (!decodeRemainingLength(sock, rem))
        return false;
    std::vector<uint8_t> suback;
    if (!readExact(sock, suback, rem) || suback.size() < 3)
        return false;
    for (size_t i = 2; i < suback.size(); ++i) {
        if (suback[i] > 0x02)
            return false;
    }
    return true;
}

void MqttHazardClient::runLoop(int sock)
{
    while (_running) {
        uint8_t header = 0;
        if (!readByte(sock, header))
            break;

        uint32_t rem = 0;
        if (!decodeRemainingLength(sock, rem))
            break;

        std::vector<uint8_t> payload;
        if (!readExact(sock, payload, rem))
            break;

        const uint8_t type = static_cast<uint8_t>(header & 0xF0);
        if (type == 0xC0) {
            const auto pingResp = buildPingResp();
            writeAll(sock, pingResp.data(), pingResp.size());
            continue;
        }
        if (type != 0x30)
            continue;

        size_t offset = 0;
        const std::string topic = readUtf8At(payload.data(), payload.size(), offset);
        const uint8_t qos = static_cast<uint8_t>((header >> 1) & 0x03);
        uint16_t packetId = 0;
        if (qos > 0) {
            if (offset + 2 > payload.size())
                continue;
            packetId = readUint16(payload.data() + offset);
            offset += 2;
        }

        if (offset > payload.size())
            continue;

        const std::string body(reinterpret_cast<const char *>(payload.data() + offset),
                               payload.size() - offset);

        if (qos > 0) {
            const auto pubAck = buildPubAck(packetId);
            writeAll(sock, pubAck.data(), pubAck.size());
        }

        std::cout << "[MQTT] Received on " << topic << ": " << body << std::endl;
        if (_callback)
            _callback(topic, body);
    }
}
