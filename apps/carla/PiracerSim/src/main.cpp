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

boost::shared_ptr<cc::Vehicle> g_vehicle;

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

    auto world = client.GetWorld();

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

    auto phys = vehicle->GetPhysicsControl();

    vehicle->ApplyPhysicsControl(phys);

    carla::rpc::VehicleControl carlaCtrl;
    float cur_throttle = 0.0f;
    float cur_steer = 0.0f;

    while (1)
    {
      // advance simulation
      world.Tick(10s);
      static carla::geom::Location smooth_loc = vehicle->GetTransform().location;

      auto vt  = vehicle->GetTransform();
      auto fwd = vt.GetForwardVector();

      carla::geom::Location target = vt.location;
      target.x -= fwd.x * 6.0f;
      target.y -= fwd.y * 6.0f;
      target.z += 2.5f;

      float alpha = 0.1f;
      smooth_loc.x = (1 - alpha) * smooth_loc.x + alpha * target.x;
      smooth_loc.y = (1 - alpha) * smooth_loc.y + alpha * target.y;
      smooth_loc.z = (1 - alpha) * smooth_loc.z + alpha * target.z;

      carla::geom::Rotation cam_rot = vt.rotation;
      cam_rot.pitch = -12.0f;

      world.GetSpectator()->SetTransform({smooth_loc, cam_rot});

      // placeholder: target command from our ADAS
      float target_throttle = kuksaCtrl.getThrottle() / 100.0f;
      float target_steer = kuksaCtrl.getSteering() / 5.0f;
      
      carlaCtrl.throttle = std::clamp(target_throttle, 0.0f, 1.0f);
      carlaCtrl.steer = std::clamp(target_steer, -1.0f, 1.0f);

      carlaCtrl.brake = 0.0f;
      carlaCtrl.hand_brake = false;
      
      carlaCtrl.reverse = (kuksaCtrl.getGear() == 2);
      
      auto vel = vehicle->GetVelocity();
      float speed_ms = std::sqrt(vel.x*vel.x + vel.y*vel.y + vel.z*vel.z);

      const float V_MAX = 6.0f;
      const float V_HYST = 0.3f;
      const float KP_BRAKE = 0.25f;

      float throttle_cmd = std::clamp(target_throttle, 0.0f, 1.0f);
      float brake_cmd = 0.0f;

      carlaCtrl.throttle = throttle_cmd;
      carlaCtrl.brake = brake_cmd;

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
  } catch (const cc::TimeoutException &e) {
    std::cout << '\n' << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cout << "\nException: " << e.what() << std::endl;
    return 2;
  }
  return 0;
}