#include "../pid.h"
#include <stdio.h>

int main() {
    PIDController pid;
    float target_speed = 40.0f;
    float current_speed = 0.0f;
    float dt = 0.1f;

    for (int i = 0; i < 100; ++i) {
        float output = PID(target_speed, current_speed, dt, &pid);
        printf("Time: %.1f s, Target Speed: %.2f m/s, Current Speed: %.2f m/s, PID Output: %.4f\n",
               i * dt, target_speed, current_speed, output);
        current_speed += output * dt;
    }
    target_speed = 80.0f;
    for (int i = 0; i < 100; ++i) {
        float output = PID(target_speed, current_speed, dt, &pid);
        printf("Time: %.1f s, Target Speed: %.2f m/s, Current Speed: %.2f m/s, PID Output: %.4f\n",
               i * dt, target_speed, current_speed, output);
        current_speed += output * dt;
    }
    target_speed = 120.0f;
    for (int i = 0; i < 100; ++i) {
        float output = PID(target_speed, current_speed, dt, &pid);
        printf("Time: %.1f s, Target Speed: %.2f m/s, Current Speed: %.2f m/s, PID Output: %.4f\n",
               i * dt, target_speed, current_speed, output);
        current_speed += output * dt;
    }
    target_speed = 20.0f;
    for (int i = 0; i < 100; ++i) {
        float output = PID(target_speed, current_speed, dt, &pid);
        printf("Time: %.1f s, Target Speed: %.2f m/s, Current Speed: %.2f m/s, PID Output: %.4f\n",
               i * dt, target_speed, current_speed, output);
        current_speed += output * dt;
    }
    return 0;
}