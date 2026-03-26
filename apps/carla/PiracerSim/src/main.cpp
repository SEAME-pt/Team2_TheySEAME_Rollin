#include <../carla/actors/BlueprintLibrary.h>
#include <../carla/client/Client.h>
#include <../carla/client/Map.h>
#include <../carla/client/Sensor.h>
#include <../carla/client/TimeoutException.h>
#include <../carla/client/World.h>
#include <../carla/geom/Transform.h>
#include <../carla/image/ImageIO.h>
#include <../carla/image/ImageView.h>
#include <../carla/sensor/data/Image.h>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <mutex>
#include <optional>
#include <signal.h>
#include "../kuksa/val/v2/KuksaLib.hpp"
#include "piracer_config.hpp"
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std::chrono_literals;
namespace cc = carla::client;
namespace cg = carla::geom;
namespace csd = carla::sensor::data;

// --- Globals ---
boost::shared_ptr<cc::Vehicle> g_vehicle;
static std::mutex               g_img_mtx;
static boost::shared_ptr<cc::Sensor> g_rgb_camera;
static std::optional<carla::SharedPtr<csd::Image>> g_last_rgb;
static int         g_sock   = -1;
static std::string RPI_IP   = "10.21.221.17";
static int         RPI_PORT = 22;

void handle_sigint(int) {
    std::cout << "\nSIGINT received: destroying vehicle...\n";
    if (g_rgb_camera) g_rgb_camera->Destroy();
    if (g_vehicle)    g_vehicle->Destroy();
    if (g_sock >= 0)  close(g_sock);
    std::exit(0);
}

static float Slew(float cur, float target, float max_delta) {
    float d = target - cur;
    if (d >  max_delta) return cur + max_delta;
    if (d < -max_delta) return cur - max_delta;
    return target;
}

cc::World setup_world(cc::Client& client) {
    auto world    = client.GetWorld();
    auto settings = world.GetSettings();
    settings.synchronous_mode   = true;
    settings.fixed_delta_seconds = piracer::WORLD_DT_S;
    world.ApplySettings(settings, 10s);
    return world;
}

int connect_rpi(const std::string& ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error connecting to RPi!\n";
        return -1;
    }
    std::cout << "Connected to RPi!\n";
    return sock;
}

boost::shared_ptr<cc::Vehicle> spawn_vehicle(cc::World& world) {
    auto blueprints = world.GetBlueprintLibrary();
    auto vehicle_bp = blueprints->Find("vehicle.nissan.micra");
    if (!vehicle_bp) throw std::runtime_error("vehicle blueprint not found");

    auto spawn_points = world.GetMap()->GetRecommendedSpawnPoints();
    if (spawn_points.empty()) throw std::runtime_error("no spawn points available");

    auto actor = world.SpawnActor(*vehicle_bp, spawn_points[1]);
    if (!actor) throw std::runtime_error("failed to spawn vehicle");

    auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
    vehicle->ApplyPhysicsControl(vehicle->GetPhysicsControl());
    return vehicle;
}

boost::shared_ptr<cc::Sensor> setup_camera(
    cc::World& world,
    boost::shared_ptr<cc::Vehicle> vehicle,
    int sock)
{
    auto blueprints = world.GetBlueprintLibrary();
    auto rgb_bp_ptr = blueprints->Find("sensor.camera.rgb");
    if (!rgb_bp_ptr) throw std::runtime_error("camera blueprint not found");

    auto rgb_bp = *rgb_bp_ptr;
    rgb_bp.SetAttribute("image_size_x", "1640");
    rgb_bp.SetAttribute("image_size_y", "1232");
    rgb_bp.SetAttribute("fov",          "62");
    rgb_bp.SetAttribute("sensor_tick",  "0.0");

    cg::Transform cam_transform(
        cg::Location{2.0f, 0.0f, 0.8f},
        cg::Rotation{-5.0f, 0.0f, 0.0f}
    );

    auto cam_actor = world.SpawnActor(rgb_bp, cam_transform, vehicle.get());
    auto camera    = boost::static_pointer_cast<cc::Sensor>(cam_actor);

    camera->Listen([sock](auto data) {
        auto img = boost::static_pointer_cast<csd::Image>(data);

        cv::Mat raw(img->GetHeight(), img->GetWidth(), CV_8UC4,
                    const_cast<void*>(static_cast<const void*>(img->data())));
        cv::Mat frame;
        cv::cvtColor(raw, frame, cv::COLOR_BGRA2BGR);
        cv::imshow("PiRacer Camera", frame);
        cv::waitKey(1);

        if (sock >= 0) {
            std::vector<uchar> buf;
            cv::imencode(".jpg", frame, buf, {cv::IMWRITE_JPEG_QUALITY, 80});
            uint32_t size = htonl(buf.size());
            send(sock, &size, 4, 0);
            send(sock, buf.data(), buf.size(), 0);
        }
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

void apply_control(
    boost::shared_ptr<cc::Vehicle> vehicle,
    kuksaLib& kuksa)
{
    carla::rpc::VehicleControl ctrl;

    float target_steer    = kuksa.getSteering()  / 5.0f;
    float target_throttle = kuksa.getThrottle()  / 100.0f;

    ctrl.throttle   = std::clamp(target_throttle, 0.0f, 1.0f);
    ctrl.steer      = std::clamp(target_steer,    -1.0f, 1.0f);
    ctrl.brake      = 0.0f;
    ctrl.hand_brake = false;
    ctrl.reverse    = (kuksa.getGear() == 2);

    vehicle->ApplyControl(ctrl);

    std::cout << "VehicleControl: "
              << "throttle=" << ctrl.throttle
              << ", steer="  << ctrl.steer
              << ", brake="  << ctrl.brake
              << ", reverse=" << (ctrl.reverse ? "true" : "false")
              << "\n";
}

// ---------------------------------------------------------------
// main
// ---------------------------------------------------------------
int main(int argc, char* argv[]) {
    bool autopilot_mode = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--auto")   autopilot_mode = true;
        if (std::string(argv[i]) == "--manual") autopilot_mode = false;
    }
    std::cout << "Mode: " << (autopilot_mode ? "AUTOPILOT" : "MANUAL") << "\n";
    signal(SIGINT, handle_sigint);

    try {
        kuksaLib kuksaCtrl;
        std::thread kuksaThread([&]{ kuksaCtrl.subscribeFromKuksa(); });
        kuksaThread.detach();

        cc::Client client("localhost", 2000);
        auto world = setup_world(client);

        g_sock = connect_rpi(RPI_IP, RPI_PORT);
        if (g_sock < 0) return 1;

        g_vehicle    = spawn_vehicle(world);
        g_rgb_camera = setup_camera(world, g_vehicle, g_sock);

        // Traffic Manager setup
        auto tm = client.GetInstanceTM(8000);
        tm.SetGlobalDistanceToLeadingVehicle(2.0f);
        tm.SetRandomLeftLaneChangePercentage(g_vehicle, 0);
        tm.SetSynchronousMode(true);
        g_vehicle->SetAutopilot(autopilot_mode, 8000);

        // Main loop
        while (true) {
            world.Tick(10s);
            update_spectator(world, g_vehicle);

            if (!autopilot_mode)
                apply_control(g_vehicle, kuksaCtrl);
        }

    } catch (const cc::TimeoutException& e) {
        std::cout << "\nTimeoutException: " << e.what() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cout << "\nException: " << e.what() << '\n';
        return 2;
    }
    return 0;
}