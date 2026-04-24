// carla_sender.cpp — YOLOv8-seg pipeline

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
#include <onnxruntime_cxx_api.h>

using namespace std::chrono_literals;
namespace cc = carla::client;
namespace cg = carla::geom;
namespace csd = carla::sensor::data;

Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "YOLO");
Ort::SessionOptions session_options;
Ort::Session session(env, "best.onnx", session_options);
Ort::AllocatorWithDefaultOptions allocator;

auto input_name_alloc   = session.GetInputNameAllocated(0, allocator);
auto output_name_alloc  = session.GetOutputNameAllocated(0, allocator);
auto output_name_alloc2 = session.GetOutputNameAllocated(1, allocator);

const char* input_names[]  = { input_name_alloc.get() };
const char* output_names[] = { output_name_alloc.get(), output_name_alloc2.get() };

// ─── Globals ──────────────────────────────────────────────────────────────────
boost::shared_ptr<cc::Vehicle> g_vehicle;
boost::shared_ptr<cc::Sensor>  g_rgb_camera;
static int         g_sock   = -1;
static std::string RPI_IP   = "10.21.221.17";
static int         RPI_PORT = 5005;

static std::queue<std::vector<uint8_t>> g_frame_queue;
static std::mutex                        g_queue_mtx;
static std::condition_variable           g_queue_cv;
static std::atomic<bool>                 g_sender_running{true};

// ─── Sender thread ─────────────────────────────────────────────────────────────
void sender_thread_func(int sock) {
    while (g_sender_running) {
        std::vector<uint8_t> buf;
        {
            std::unique_lock<std::mutex> lk(g_queue_mtx);
            g_queue_cv.wait(lk, [] {
                return !g_frame_queue.empty() || !g_sender_running;
            });
            if (!g_sender_running && g_frame_queue.empty()) break;

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

// ─── Signal ────────────────────────────────────────────────────────────────────
void handle_sigint(int) {
    std::cout << "\nSIGINT: a destruir actores...\n";
    g_sender_running = false;
    g_queue_cv.notify_all();
    if (g_rgb_camera) g_rgb_camera->Destroy();
    if (g_vehicle)    g_vehicle->Destroy();
    if (g_sock >= 0)  close(g_sock);
    std::exit(0);
}

// ─── World setup ───────────────────────────────────────────────────────────────
cc::World setup_world(cc::Client& client) {
    auto world    = client.GetWorld();
    auto settings = world.GetSettings();
    settings.synchronous_mode    = true;
    settings.fixed_delta_seconds = piracer::WORLD_DT_S;
    world.ApplySettings(settings, 10s);
    return world;
}

// ─── TCP connect ───────────────────────────────────────────────────────────────
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

// ─── Spawn vehicle ─────────────────────────────────────────────────────────────
boost::shared_ptr<cc::Vehicle> spawn_vehicle(cc::World& world) {
    auto blueprints  = world.GetBlueprintLibrary();
    auto vehicle_bp  = blueprints->Find("vehicle.nissan.micra");
    if (!vehicle_bp) throw std::runtime_error("blueprint do veículo não encontrado");

    auto spawn_points = world.GetMap()->GetRecommendedSpawnPoints();
    if (spawn_points.empty()) throw std::runtime_error("sem spawn points");

    auto actor  = world.SpawnActor(*vehicle_bp, spawn_points[1]);
    if (!actor) throw std::runtime_error("falhou spawn do veículo");

    auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
    vehicle->ApplyPhysicsControl(vehicle->GetPhysicsControl());
    return vehicle;
}

// ─── Camera setup ──────────────────────────────────────────────────────────────
boost::shared_ptr<cc::Sensor> setup_camera(
    cc::World& world,
    boost::shared_ptr<cc::Vehicle> vehicle,
    int sock)
{
    auto blueprints = world.GetBlueprintLibrary();
    auto rgb_bp_ptr = blueprints->Find("sensor.camera.rgb");
    if (!rgb_bp_ptr) throw std::runtime_error("blueprint câmera não encontrado");

    auto rgb_bp = *rgb_bp_ptr;
    rgb_bp.SetAttribute("image_size_x", "640");
    rgb_bp.SetAttribute("image_size_y", "640");
    rgb_bp.SetAttribute("fov",          "62");
    rgb_bp.SetAttribute("sensor_tick",  "0.05");

    cg::Transform cam_transform(
        cg::Location{2.0f, 0.0f, 0.8f},
        cg::Rotation{-5.0f, 0.0f, 0.0f}
    );

    auto cam_actor = world.SpawnActor(rgb_bp, cam_transform, vehicle.get());
    auto camera    = boost::static_pointer_cast<cc::Sensor>(cam_actor);

    static const std::vector<std::string> CLASS_NAMES = {
        "center_continuous_lane",
        "center_dashed_lane",
        "crosswalk",
        "left_lane",
        "right_lane"
    };

    static const std::vector<cv::Scalar> COLORS = {
        {255, 0,   0  },  // center_continuous_lane — azul
        {0,   255, 255},  // center_dashed_lane     — amarelo
        {0,   0,   255},  // crosswalk              — vermelho
        {255, 255, 0  },  // left_lane              — ciano
        {255, 0,   255},  // right_lane             — magenta
    };

    const float CONF_THRESH = 0.5f;
    const int   NUM_CLASSES = 5;
    const int   NUM_MASKS   = 32;

    camera->Listen([sock, CONF_THRESH, NUM_CLASSES, NUM_MASKS](auto data) {
        auto img = boost::static_pointer_cast<csd::Image>(data);

        const int H = (int)img->GetHeight();
        const int W = (int)img->GetWidth();

        cv::Mat raw(H, W, CV_8UC4,
            const_cast<void*>(static_cast<const void*>(img->data())));

        cv::Mat frame;
        cv::cvtColor(raw, frame, cv::COLOR_BGRA2BGR);
        cv::Mat display = frame.clone();

        // ── Preprocess ─────────────────────────────────────────
        cv::Mat resized;
        cv::resize(frame, resized, cv::Size(640, 640));
        cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);

        std::vector<float> input_tensor_values(1 * 3 * 640 * 640);
        for (int c = 0; c < 3; c++)
            for (int i = 0; i < 640; i++)
                for (int j = 0; j < 640; j++)
                    input_tensor_values[c * 640 * 640 + i * 640 + j] =
                        resized.at<cv::Vec3b>(i, j)[c] / 255.0f;

        auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        std::array<int64_t, 4> input_shape = {1, 3, 640, 640};

        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info,
            input_tensor_values.data(),
            input_tensor_values.size(),
            input_shape.data(),
            input_shape.size()
        );

        auto output = session.Run(
            Ort::RunOptions{nullptr},
            input_names, &input_tensor, 1,
            output_names, 2
        );

        auto& out_tensor = output[0];
        auto shape       = out_tensor.GetTensorTypeAndShapeInfo().GetShape();
        const float* det = out_tensor.GetTensorData<float>();
        const int num_anchors = (int)shape[2]; // 8400

        auto& proto_tensor = output[1];
        auto proto_shape   = proto_tensor.GetTensorTypeAndShapeInfo().GetShape();
        const float* protos = proto_tensor.GetTensorData<float>();
        const int PH = (int)proto_shape[2]; // 160
        const int PW = (int)proto_shape[3]; // 160

        std::vector<cv::Rect>            boxes;
        std::vector<float>               scores;
        std::vector<int>                 class_ids;
        std::vector<std::vector<float>>  mask_coefs;

        // ── Decode ─────────────────────────────────────────────
        for (int a = 0; a < num_anchors; a++) {
            float max_score = 0.0f;
            int best_cls = 0;
            for (int c = 0; c < NUM_CLASSES; c++) {
                float s = det[(4 + c) * num_anchors + a];
                if (s > max_score) { max_score = s; best_cls = c; }
            }
            if (max_score < CONF_THRESH) continue;

            float cx = det[0 * num_anchors + a];
            float cy = det[1 * num_anchors + a];
            float bw = det[2 * num_anchors + a];
            float bh = det[3 * num_anchors + a];

            int x = (int)((cx - bw / 2.0f) * W / 640.0f);
            int y = (int)((cy - bh / 2.0f) * H / 640.0f);
            int w = (int)(bw * W / 640.0f);
            int h = (int)(bh * H / 640.0f);

            boxes.emplace_back(x, y, w, h);
            scores.push_back(max_score);
            class_ids.push_back(best_cls);

            std::vector<float> coefs(NUM_MASKS);
            for (int m = 0; m < NUM_MASKS; m++)
                coefs[m] = det[(4 + NUM_CLASSES + m) * num_anchors + a];
            mask_coefs.push_back(coefs);
        }

        // ── NMS ────────────────────────────────────────────────
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, scores, CONF_THRESH, 0.4f, indices);

        for (int idx : indices) {
            int cls      = class_ids[idx];
            float score  = scores[idx];
            cv::Rect box = boxes[idx];
            auto& coefs  = mask_coefs[idx];
            cv::Scalar color = COLORS[cls % (int)COLORS.size()];

            cv::Mat mask_proto(PH, PW, CV_32F, cv::Scalar(0));
            for (int m = 0; m < NUM_MASKS; m++)
                for (int py = 0; py < PH; py++)
                    for (int px = 0; px < PW; px++)
                        mask_proto.at<float>(py, px) +=
                            coefs[m] * protos[m * PH * PW + py * PW + px];

            cv::Mat mask_sig;
            cv::exp(-mask_proto, mask_sig);
            mask_sig = 1.0f / (1.0f + mask_sig);

            cv::Mat mask_full;
            cv::resize(mask_sig, mask_full, cv::Size(W, H));

            cv::Mat mask_bin = (mask_full > 0.5f);
            cv::Mat roi_mask = cv::Mat::zeros(H, W, CV_8U);

            int x1 = std::max(box.x, 0);
            int y1 = std::max(box.y, 0);
            int x2 = std::min(box.x + box.width,  W);
            int y2 = std::min(box.y + box.height, H);

            if (x2 > x1 && y2 > y1) {
                cv::Rect safe_box(x1, y1, x2 - x1, y2 - y1);
                mask_bin(safe_box).copyTo(roi_mask(safe_box));
            }

            cv::Mat colored(H, W, CV_8UC3, cv::Scalar(0, 0, 0));
            colored.setTo(color, roi_mask);
            cv::addWeighted(display, 1.0f, colored, 0.4f, 0, display);

            cv::putText(display, label,
                cv::Point(box.x, std::max(box.y - 5, 0)),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 1);

        }

        cv::imshow("PiRacer Camera", display);
        cv::waitKey(1);
    });

    return camera;
}

// ─── Spectator ─────────────────────────────────────────────────────────────────
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

// ─── Control ───────────────────────────────────────────────────────────────────
void apply_control(boost::shared_ptr<cc::Vehicle> vehicle, kuksaLib& kuksa) {
    carla::rpc::VehicleControl ctrl;
    ctrl.throttle   = std::clamp(kuksa.getThrottle() / 100.0f, 0.0f, 1.0f);
    ctrl.steer      = std::clamp(kuksa.getSteering() / 5.0f,  -1.0f, 1.0f);
    ctrl.brake      = 0.0f;
    ctrl.hand_brake = false;
    ctrl.reverse    = (kuksa.getGear() == 3);
    vehicle->ApplyControl(ctrl);
    std::cout << "throttle=" << ctrl.throttle
              << " steer="   << ctrl.steer
              << " rev="     << ctrl.reverse << "\n";
}

// ─── main ──────────────────────────────────────────────────────────────────────
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
        if (g_sock < 0) std::cerr << "continuing without connection RPi...\n";

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