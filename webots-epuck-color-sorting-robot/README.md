# Webots e-puck Color Sorting Robot

An autonomous **e-puck robot controller** developed in **C using Webots**.

The robot uses camera-based RGB detection and a finite-state machine to locate and process colored balls in wavelength order:

**Blue → Green → Orange → Red**

## Demo Video

A public demonstration video will be added when available.

## Project Overview

The controller enables an e-puck robot to:

- Search for a target colored ball.
- Detect the ball using the robot camera.
- Confirm and align the target near the center of the camera image.
- Approach the detected target.
- Identify the ball in the required wavelength order.
- Reverse and turn before searching for the next target.

## State Machine

```text
SEARCH
  ↓
CONFIRM
  ↓
APPROACH
  ↓
IDENTIFY
  ↓
BACKUP
  ↓
TURN
  ↓
SEARCH
```

The sequence repeats until all four target colors have been processed.

## Color Detection

Instead of relying on a single camera pixel, the controller averages RGB values from a **7 × 7 region** around the image center. This helps reduce noise and gives more stable color classification.

Current color rules identify:

- Blue
- Green
- Orange
- Red

Very bright background pixels and very dark/noisy readings are ignored.

## Technologies

- Webots
- C
- e-puck mobile robot
- Camera-based color detection
- Finite-state machine
- Differential-drive motor control

## Repository Structure

```text
webots-epuck-color-sorting-robot/
├── controllers/
│   └── epuck_controller/
│       └── epuck_controller.c
├── .gitignore
└── README.md
```

## Controller Logic

1. **SEARCH** — rotate until the target color is detected.
2. **CONFIRM** — align the target and confirm a stable detection.
3. **APPROACH** — drive toward the detected ball.
4. **IDENTIFY** — register the ball in the correct order.
5. **BACKUP** — reverse away from the ball.
6. **TURN** — rotate before beginning the next search.

## Target Order

1. Blue — shortest wavelength
2. Green
3. Orange
4. Red — longest wavelength

## Future Improvements

- More robust object-centering logic.
- Better separation between target objects and background colors.
- Using multiple proximity sensors for safer navigation.
- Obstacle avoidance.
- Dynamic color calibration.
- Host the demonstration video directly in the repository or via a public portfolio link.
- Add the Webots `.wbt` world file to make the full simulation reproducible.

## Author

**Hamza Al Mandhari**  
Mechatronics Engineering student

---

This repository documents an academic robotics project and its development in Webots.

