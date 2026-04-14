#include <../carla/actors/BlueprintLibrary.h>
#include <../carla/client/Client.h>
#include <../carla/client/Map.h>
#include <../carla/client/Sensor.h>
#include <../carla/client/TimeoutException.h>
#include <../carla/client/World.h>
#include <../carla/geom/Transform.h>
#include <../carla/sensor/data/Image.h>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <mutex>
#include <signal.h>
#include "../kuksa/val/v2/KuksaLib.hpp"
#include "piracer_config.hpp"
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <netinet/tcp.h>

using namespace std::chrono_literals;
namespace cc = carla::client;
namespace cg = carla::geom;
namespace csd = carla::sensor::data;

boost::shared_ptr<cc::Vehicle> g_vehicle;
boost::shared_ptr<cc::Sensor>  g_rgb_camera;
static int         g_sock    = -1;
static std::string RPI_IP    = "10.21.221.17";
static int         RPI_PORT  = 9999;

// Fila com no máximo 1 frame — descarta frames antigas automaticamente
static std::queue<std::vector<uint8_t>> g_frame_queue;
static std::mutex                        g_queue_mtx;
static std::condition_variable           g_queue_cv;
static std::atomic<bool>                 g_sender_running{true};

// ─── Sender thread ─────────────────────────────────────────────────────────────
void sender_thread_func(int sock) {
    while (g_sender_running) {
        std::vector<uint8_t> buf;
        uint32_t rows_net, cols_net;
        {
            std::unique_lock<std::mutex> lk(g_queue_mtx);
            g_queue_cv.wait(lk, [] {
                return !g_frame_queue.empty() || !g_sender_running;
            });
            if (!g_sender_running && g_frame_queue.empty()) break;

            // Descarta frames acumuladas; mantém só a mais recente
            while (g_frame_queue.size() > 1)
                g_frame_queue.pop();

            buf = std::move(g_frame_queue.front());
            g_frame_queue.pop();
        }

        size_t total = buf.size();
        size_t sent  = 0;
        while (sent < total) {
            ssize_t n = send(sock, buf.data() + sent, total - sent, MSG_NOSIGNAL);
            if (n <= 0) { g_sender_running = false; return; }
            sent += n;
        }
    }
}

void handle_sigint(int) {
    std::cout << "\nSIGINT: a destruir actores...\n";
    g_sender_running = false;
    g_queue_cv.notify_all();
    if (g_rgb_camera) g_rgb_camera->Destroy();
    if (g_vehicle)    g_vehicle->Destroy();
    if (g_sock >= 0)  close(g_sock);
    std::exit(0);
}

cc::World setup_world(cc::Client& client) {
    auto world    = client.GetWorld();
    auto settings = world.GetSettings();
    settings.synchronous_mode    = true;
    settings.fixed_delta_seconds = piracer::WORLD_DT_S;
    world.ApplySettings(settings, 10s);
    return world;
}

int connect_rpi(const std::string& ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int flag = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Erro ao conectar ao RPi!\n";
        close(sock);
        return -1;
    }
    std::cout << "Ligado ao RPi!\n";
    return sock;
}

boost::shared_ptr<cc::Vehicle> spawn_vehicle(cc::World& world) {
    auto blueprints  = world.GetBlueprintLibrary();
    auto vehicle_bp  = blueprints->Find("vehicle.nissan.micra");
    if (!vehicle_bp) throw std::runtime_error("blueprint do veículo não encontrado");

    auto spawn_points = world.GetMap()->GetRecommendedSpawnPoints();
    if (spawn_points.empty()) throw std::runtime_error("sem spawn points");

    auto actor   = world.SpawnActor(*vehicle_bp, spawn_points[1]);
    if (!actor)  throw std::runtime_error("falhou spawn do veículo");

    auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
    vehicle->ApplyPhysicsControl(vehicle->GetPhysicsControl());
    return vehicle;
}

boost::shared_ptr<cc::Sensor> setup_camera(
    cc::World& world,
    boost::shared_ptr<cc::Vehicle> vehicle,
    int sock)
{
    auto blueprints  = world.GetBlueprintLibrary();
    auto rgb_bp_ptr  = blueprints->Find("sensor.camera.rgb");
    if (!rgb_bp_ptr) throw std::runtime_error("blueprint câmera não encontrado");

    auto rgb_bp = *rgb_bp_ptr;
    rgb_bp.SetAttribute("image_size_x", "320");
    rgb_bp.SetAttribute("image_size_y", "240");
    rgb_bp.SetAttribute("fov",          "62");
    rgb_bp.SetAttribute("sensor_tick",  "0.05");

    cg::Transform cam_transform(
        cg::Location{2.0f, 0.0f, 0.8f},
        cg::Rotation{-5.0f, 0.0f, 0.0f}
    );

    auto cam_actor = world.SpawnActor(rgb_bp, cam_transform, vehicle.get());
    auto camera    = boost::static_pointer_cast<cc::Sensor>(cam_actor);

    camera->Listen([sock](auto data) {
        auto img = boost::static_pointer_cast<csd::Image>(data);

        const uint32_t H = img->GetHeight();
        const uint32_t W = img->GetWidth();

        cv::Mat raw(H, W, CV_8UC4,
                    const_cast<void*>(static_cast<const void*>(img->data())));
        cv::Mat frame;
        cv::cvtColor(raw, frame, cv::COLOR_BGRA2BGR);

        cv::imshow("PiRacer Camera", frame);
        cv::waitKey(1);

        if (sock < 0) return;

        const size_t pixel_bytes = (size_t)H * W * 3;
        std::vector<uint8_t> buf(8 + pixel_bytes);

        uint32_t rows_net = htonl(H);
        uint32_t cols_net = htonl(W);
        std::memcpy(buf.data() + 0, &rows_net, 4);
        std::memcpy(buf.data() + 4, &cols_net, 4);
        std::memcpy(buf.data() + 8, frame.data, pixel_bytes);

        {
            std::lock_guard<std::mutex> lk(g_queue_mtx);
            if (!g_frame_queue.empty()) g_frame_queue.pop();
            g_frame_queue.push(std::move(buf));
        }
        g_queue_cv.notify_one();
    });

    return camera;
}

void update_spectator(cc::World& world, boost::shared_ptr<cc::Vehicle> vehicle) {
    static carla::geom::Location smooth_loc = vehicle->GetTransform().location;
    auto vt  = vehicle->GetTransform();
    auto fwd = vt.GetForwardVector();

    carla::geom::Location target = vt.location;
    target.x -= fwd.x * 6.0f;
    target.y -= fwd.y * 6.0f;
    target.z += 2.5f;

    const float alpha = 0.1f;
    smooth_loc.x = (1 - alpha) * smooth_loc.x + alpha * target.x;
    smooth_loc.y = (1 - alpha) * smooth_loc.y + alpha * target.y;
    smooth_loc.z = (1 - alpha) * smooth_loc.z + alpha * target.z;

    carla::geom::Rotation cam_rot = vt.rotation;
    cam_rot.pitch = -12.0f;
    world.GetSpectator()->SetTransform({smooth_loc, cam_rot});
}

void apply_control(boost::shared_ptr<cc::Vehicle> vehicle, kuksaLib& kuksa) {
    carla::rpc::VehicleControl ctrl;
    ctrl.throttle   = std::clamp(kuksa.getThrottle() / 100.0f, 0.0f, 1.0f);
    ctrl.steer      = std::clamp(kuksa.getSteering() / 5.0f,  -1.0f, 1.0f);
    ctrl.brake      = 0.0f;
    ctrl.hand_brake = false;
    ctrl.reverse    = (kuksa.getGear() == 2);
    vehicle->ApplyControl(ctrl);
    std::cout << "throttle=" << ctrl.throttle
              << " steer="   << ctrl.steer
              << " rev="     << ctrl.reverse << "\n";
}
int main(int argc, char* argv[]) {
    bool autopilot_mode = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--auto")   autopilot_mode = true;
        if (std::string(argv[i]) == "--manual") autopilot_mode = false;
    }
    std::cout << "Modo: " << (autopilot_mode ? "AUTOPILOT" : "MANUAL") << "\n";
    signal(SIGINT, handle_sigint);

    try {
        kuksaLib kuksaCtrl;
        std::thread kuksaThread([&] { kuksaCtrl.subscribeFromKuksa(); });
        kuksaThread.detach();

        cc::Client client("localhost", 2000);
        auto world = setup_world(client);

        g_sock = connect_rpi(RPI_IP, RPI_PORT);
        if (g_sock < 0) return 1;

        g_vehicle    = spawn_vehicle(world);
        g_rgb_camera = setup_camera(world, g_vehicle, g_sock);

        auto tm = client.GetInstanceTM(8000);
        tm.SetGlobalDistanceToLeadingVehicle(2.0f);
        tm.SetRandomLeftLaneChangePercentage(g_vehicle, 0);
        tm.SetSynchronousMode(true);
        g_vehicle->SetAutopilot(autopilot_mode, 8000);

        std::thread sender(sender_thread_func, g_sock);

        while (true) {
            world.Tick(10s);
            update_spectator(world, g_vehicle);
            if (!autopilot_mode)
                apply_control(g_vehicle, kuksaCtrl);
        }

    } catch (const cc::TimeoutException& e) {
        std::cout << "TimeoutException: " << e.what() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << '\n';
        return 2;
    }
    return 0;
}