#include <stdio.h>
#include <string.h>

#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/distance_sensor.h>
#include <webots/camera.h>

#define TIME_STEP 64

static WbDeviceTag left_motor;
static WbDeviceTag right_motor;
static WbDeviceTag camera_device;
static WbDeviceTag front_sensor;

typedef enum {
  SEARCH,
  CONFIRM,
  APPROACH,
  IDENTIFY,
  BACKUP,
  TURN
} RobotState;

const char *order[] = {"BLUE", "GREEN", "ORANGE", "RED"};
int current_target = 0;

RobotState state = SEARCH;
int stable_count = 0;
int state_timer = 0;

void set_speed(double l, double r) {
  wb_motor_set_velocity(left_motor, l);
  wb_motor_set_velocity(right_motor, r);
}

const char *detect_color() {
  const unsigned char *img = wb_camera_get_image(camera_device);
  if (img == NULL)
    return "NONE";

  int width = wb_camera_get_width(camera_device);
  int height = wb_camera_get_height(camera_device);

  int cx = width / 2;
  int cy = height / 2;

  int total_r = 0, total_g = 0, total_b = 0;
  int count = 0;

  // Average 7x7 region around center
  for (int dx = -3; dx <= 3; dx++) {
    for (int dy = -3; dy <= 3; dy++) {
      int x = cx + dx;
      int y = cy + dy;

      if (x >= 0 && x < width && y >= 0 && y < height) {
        int r = wb_camera_image_get_red(img, width, x, y);
        int g = wb_camera_image_get_green(img, width, x, y);
        int b = wb_camera_image_get_blue(img, width, x, y);

        // Ignore very bright background
        if (r > 235 && g > 235 && b > 235)
          continue;

        total_r += r;
        total_g += g;
        total_b += b;
        count++;
      }
    }
  }

  if (count == 0)
    return "NONE";

  int r = total_r / count;
  int g = total_g / count;
  int b = total_b / count;

  printf("AVG RGB = %d %d %d\n", r, g, b);

  // Ignore very dark/noisy readings
  if (r < 35 && g < 35 && b < 35)
    return "NONE";

  // BLUE
  if (b > r + 35 && b > g + 35)
    return "BLUE";

  // GREEN
  if (g > r + 30 && g > b + 30)
    return "GREEN";

  // ORANGE: red high, green noticeable, blue low
  if (r > 150 && g > 95 && g < r && b < 65 && (r - g) < 80)
    return "ORANGE";

  // RED: red high, green low, blue low
  if (r > 145 && g < 65 && b < 60)
    return "RED";

  return "NONE";
}

int get_center_offset() {
  const unsigned char *img = wb_camera_get_image(camera_device);
  if (img == NULL)
    return 0;

  int width = wb_camera_get_width(camera_device);
  int height = wb_camera_get_height(camera_device);

  int cx = width / 2;
  int cy = height / 2;

  // Compare left and right side of center
  int left_r  = wb_camera_image_get_red(img, width, cx - 5, cy);
  int left_g  = wb_camera_image_get_green(img, width, cx - 5, cy);
  int left_b  = wb_camera_image_get_blue(img, width, cx - 5, cy);

  int right_r = wb_camera_image_get_red(img, width, cx + 5, cy);
  int right_g = wb_camera_image_get_green(img, width, cx + 5, cy);
  int right_b = wb_camera_image_get_blue(img, width, cx + 5, cy);

  int left_sum = left_r + left_g + left_b;
  int right_sum = right_r + right_g + right_b;

  return left_sum - right_sum;
}

int main() {
  wb_robot_init();

  left_motor = wb_robot_get_device("left wheel motor");
  right_motor = wb_robot_get_device("right wheel motor");

  wb_motor_set_position(left_motor, INFINITY);
  wb_motor_set_position(right_motor, INFINITY);

  camera_device = wb_robot_get_device("camera");
  wb_camera_enable(camera_device, TIME_STEP);

  front_sensor = wb_robot_get_device("GS0");
  wb_distance_sensor_enable(front_sensor, TIME_STEP);

  printf("Robot searching balls in wavelength order:\n");
  printf("BLUE -> GREEN -> ORANGE -> RED\n");

  while (wb_robot_step(TIME_STEP) != -1) {
    if (current_target >= 4) {
      set_speed(0.0, 0.0);
      printf("All balls processed successfully.\n");
      break;
    }

    const char *target = order[current_target];
    const char *seen_color = detect_color();
    double d = wb_distance_sensor_get_value(front_sensor);

    printf("State=%d | Target=%s | Seen=%s | Distance=%.2f\n",
           state, target, seen_color, d);

    switch (state) {
      case SEARCH:
        if (strcmp(seen_color, target) == 0) {
          stable_count = 0;
          state = CONFIRM;
          set_speed(0.0, 0.0);
        } else {
          // Fast search
          set_speed(-2.2, 2.2);
        }
        break;

      case CONFIRM:
        if (strcmp(seen_color, target) == 0) {
          int offset = get_center_offset();

          // Align target to center before confirming
          if (offset > 20) {
            set_speed(-0.6, 0.6);
            stable_count = 0;
          } else if (offset < -20) {
            set_speed(0.6, -0.6);
            stable_count = 0;
          } else {
            set_speed(0.0, 0.0);
            stable_count++;

            if (stable_count >= 5) {
              printf("Confirmed target color: %s\n", target);
              state = APPROACH;
            }
          }
        } else {
          stable_count = 0;
          state = SEARCH;
        }
        break;

      case APPROACH:
        // If target lost, go search again
        if (strcmp(seen_color, target) != 0) {
          printf("Lost target while approaching. Re-searching.\n");
          state = SEARCH;
          break;
        }

        // Keep approaching quickly
        if (d < 200) {
          set_speed(3.5, 3.5);
        } else {
          set_speed(0.0, 0.0);
          state = IDENTIFY;
        }
        break;

      case IDENTIFY:
        printf("Ball identified in correct order: %s\n", target);

        if (strcmp(target, "BLUE") == 0)
          printf("Shortest wavelength\n");
        else if (strcmp(target, "GREEN") == 0)
          printf("Second wavelength\n");
        else if (strcmp(target, "ORANGE") == 0)
          printf("Third wavelength\n");
        else if (strcmp(target, "RED") == 0)
          printf("Longest wavelength\n");

        state_timer = 6;
        state = BACKUP;
        break;

      case BACKUP:
        if (state_timer > 0) {
          set_speed(-3.0, -3.0);
          state_timer--;
        } else {
          state_timer = 12;
          state = TURN;
        }
        break;

      case TURN:
        if (state_timer > 0) {
          set_speed(-2.8, 2.8);
          state_timer--;
        } else {
          current_target++;
          stable_count = 0;
          state = SEARCH;
        }
        break;
    }
  }

  wb_robot_cleanup();
  return 0;
}

