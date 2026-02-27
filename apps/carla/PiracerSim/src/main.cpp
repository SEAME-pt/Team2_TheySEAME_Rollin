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

using namespace std::chrono_literals;


namespace cc = carla::client;
namespace cg = carla::geom;
namespace csd = carla::sensor::data;

// Variável global para o veículo
boost::shared_ptr<cc::Vehicle> g_vehicle;

// Handler para SIGINT (Ctrl+C)
void handle_sigint(int) {
  if (g_vehicle) {
    std::cout << "\nSIGINT received: destroying vehicle..." << std::endl;
    g_vehicle->Destroy();
  }
  std::exit(0);
}

static std::mutex g_mtx;
static std::optional<carla::SharedPtr<csd::Image>> g_last_image;
  
static float Slew(float cur, float target, float max_delta) {
  float d = target - cur;
  if (d >  max_delta) return cur + max_delta;
  if (d < -max_delta) return cur - max_delta;
  return target;
}

int main() {
  signal(SIGINT, handle_sigint);
  try {
    kuksaLib kuksaCtrl;
    std::thread kuksaThread([&](){kuksaCtrl.subscribeFromKuksa();});
    kuksaThread.detach();

    cc::Client client("localhost", 2000);
    // client.SetTimeout(40s);

    auto world = client.GetWorld();

    // Synchronous mode (very important)
    auto settings = world.GetSettings();
    settings.synchronous_mode = true;
    settings.fixed_delta_seconds = piracer::WORLD_DT_S;
    world.ApplySettings(settings, 10s);

    // Base vehicle (small)
    auto blueprints = world.GetBlueprintLibrary();

    
    auto vehicle_bp = blueprints->Find("vehicle.nissan.micra");
    if (!vehicle_bp) { std::cerr << "vehicle bp not found\n"; return 1; }

    auto spawn_points = world.GetMap()->GetRecommendedSpawnPoints();
    if (spawn_points.empty()) { std::cerr << "no spawn points\n"; return 1; }

    auto actor = world.SpawnActor(*vehicle_bp, spawn_points[1]);
    if (!actor) { std::cerr << "failed to spawn vehicle\n"; return 1; }
    auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
    g_vehicle = vehicle;

    // Basic physical adjustment (mass; you can also modify wheels)
    auto phys = vehicle->GetPhysicsControl();

    phys.mass = (float)piracer::MASS_KG;

    phys.drag_coefficient = 0.9f;

    phys.center_of_mass = carla::geom::Vector3D(0.0f, 0.0f, -0.10f);

    vehicle->ApplyPhysicsControl(phys);

    // // RGB camera setup
    // auto cam_bp_opt = blueprints->Find("sensor.camera.rgb");
    // if (!cam_bp_opt) { std::cerr << "camera bp not found\n"; return 1; }
    // carla::actors::ActorBlueprint cam_bp = *cam_bp_opt;

    // cam_bp.SetAttribute("image_size_x", "640");
    // cam_bp.SetAttribute("image_size_y", "480");
    // cam_bp.SetAttribute("fov", "90");
    // cam_bp.SetAttribute("sensor_tick", std::to_string(piracer::CAM_TICK_S));

    // cg::Transform cam_tf(
    //   cg::Location(piracer::CAM_X, piracer::CAM_Y, piracer::CAM_Z),
    //   cg::Rotation(piracer::CAM_PITCH_DEG, 0.0, 0.0)
    // );

    // auto cam_actor = world.SpawnActor(cam_bp, cam_tf, vehicle.get());
    // auto cam = boost::static_pointer_cast<cc::Sensor>(cam_actor);

    // cam->Listen([](auto data) {
    //   auto img = boost::static_pointer_cast<csd::Image>(data);
    //   std::lock_guard<std::mutex> lk(g_mtx);
    //   g_last_image = img;
    // });

    carla::rpc::VehicleControl carlaCtrl;
    float cur_throttle = 0.0f;
    float cur_steer = 0.0f;

    while (1)
    {
      // 1) advance simulation
      world.Tick(10s);

      // // 2) read the latest image (if needed)
      // {
      //   std::lock_guard<std::mutex> lk(g_mtx);
      //   if (g_last_image) {
      //     // Here we would call our ADAS with the image
      //     // (*g_last_image)->GetWidth(), GetHeight(), raw data, etc.
      //   }
      // }

      // 3) placeholder: target command from your ADAS
      float target_throttle = kuksaCtrl.getThrottle() / 100.0f;
      float target_steer = kuksaCtrl.getSteering() / 30.0f;

      // cur_throttle = Slew(cur_throttle, target_throttle, (float)piracer::MAX_THROTTLE_SLEW_PER_TICK);
      // cur_steer    = Slew(cur_steer,    target_steer,    (float)piracer::MAX_STEER_SLEW_PER_TICK);
      
      carlaCtrl.throttle = std::clamp(target_throttle, 0.0f, 1.0f);
      carlaCtrl.steer = std::clamp(target_steer, -1.0f, 1.0f);

      carlaCtrl.brake = 0.0f;
      carlaCtrl.hand_brake = false;
      
      carlaCtrl.reverse = (kuksaCtrl.getGear() == 2);
      
      vehicle->ApplyControl(carlaCtrl);
      std::cout << "VehicleControl: "
            << "throttle=" << carlaCtrl.throttle
            << ", steer=" << carlaCtrl.steer
            << ", brake=" << carlaCtrl.brake
            << ", hand_brake=" << (carlaCtrl.hand_brake ? "true" : "false")
            << ", reverse=" << (carlaCtrl.reverse ? "true" : "false")
            << ", manual_gear_shift=" << (carlaCtrl.manual_gear_shift ? "true" : "false")
            << ", gear=" << carlaCtrl.gear
            << std::endl;
    }

    // Stop and destroy camera and vehicle actors
    // cam->Stop();
    // cam->Destroy();
    // vehicle->Destroy();
  } catch (const cc::TimeoutException &e) {
    std::cout << '\n' << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cout << "\nException: " << e.what() << std::endl;
    return 2;
  }
  return 0;
}