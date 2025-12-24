# Hybrid Force-Position Control

Support SDK V1.11.0+ version, algorithm principle can be followed from HPMicro's KB.

## Overview

The **hybrid_force_position** project demonstrates hybrid force-position control for BLDC motors.

- Uses **FOC** (Field Oriented Control) algorithm
- Implements hybrid force-position control with PD controller
- Supports both hardware and software current loop implementation
- Supports ABZ encoder for position detection
- Features speed filtering and torque limiting capabilities

## Key Features

### Hybrid Control Algorithm
- **Position Control**: PD controller with configurable kp and kd gains
- **Force Control**: Torque output with adjustable limits
- **Speed Filtering**: Low-pass filter with deadzone to reduce noise
- **Compliant Control**: Combines position stiffness with force feedback

### Control Parameters
- `kp`: Position proportional gain (default: 0.06)
- `kd`: Velocity damping gain (default: 0.001429)
- `tau_limit`: Torque output limits (default: ±0.5 Nm)
- `speed_lpf_alpha`: Speed filter coefficient (default: 0.003)
- `speed_deadzone`: Speed filter deadzone (default: 0.1 rad/s)

## Hardware Configuration

### Motor Specification
This demo uses the **BLM57050-1000** brushless motor from Leisai Intelligence. Please refer to the [Leisai Intelligence website](https://leisai.com/) for detailed motor specifications.

### Board Settings
Refer to the development board documentation for :ref:`Motor Pin <board_resource>` configuration:

- Ensure PWM frequency is set correctly (default: 20kHz)
- Ensure motor pole pairs is configured correctly
- Ensure ADC sampling configuration is correct

### Peripheral Configuration

#### PWM Configuration
- Uses BLDC PWM module
- Configures dead time to prevent shoot-through
- Configures complementary PWM output
- Supports three-phase six-channel PWM output
- Supports PWM synchronized ADC sampling trigger
- Uses shadow register to update PWM duty cycle
- Supports PWM fault protection

#### ADC Configuration
- Uses two ADC modules to sample U-phase and V-phase current
- Configures ADC reference voltage (3.3V)
- Configures op-amp gain (10x)
- Configures sampling precision (12-bit)
- Supports DMA transfer
- Supports ADC synchronized sampling
- Supports ADC trigger source configuration
- Supports ADC sampling interrupt

#### Encoder Configuration
- Supports ABZ encoder and QEI encoder
- Configures encoder precision
- Configures encoder direction
- Configures encoder sampling period
- Supports encoder position counting
- Supports encoder speed calculation

#### Timer Configuration
- Uses GPTMR as timer
- Configures current loop sampling period (50us)
- Configures speed loop sampling period (250us)
- Configures position loop sampling period (1ms)
- Supports timer interrupt

## Current Loop Performance

Hardware current loop shows near-zero computation time. Software current loop computation time is around 1us, with up to 25% variation depending on angle.

To measure the current loop time:
- Enable only `MCL_EN_LOOP_TIME_COUNT` macro in `mcl_app_config.h`
- Compile with `flash_xip_release` option

## Running the Demo

### Initialization Sequence

1. Motor performs alignment action on power-up
2. System initializes hybrid control parameters
3. Motor enters hybrid force-position control mode
4. Console displays current control parameters

### User Input

The motor can be controlled via serial console by entering target positions:

**Position Input**: Integer value in degrees
- Range: Practical range depends on application requirements
- Unit: degrees
- Positive value: Clockwise direction
- Negative value: Counterclockwise direction

### Console Output

```console
Hybrid force-position control mode.

Hybrid control mode
kp=0.060, kd=0.001, tau_limit=0.500
Speed filter: alpha=0.003, deadzone=0.10 rad/s
Target position: 0 degrees
Input position:
90
Pos: 90 deg, Tau: 0.1234 Nm, Pos_err: 0.0123, Speed: 0.234 rad/s
Input position:
-45
Pos: -45 deg, Tau: -0.0987 Nm, Pos_err: -0.0087, Speed: -0.156 rad/s
Input position:
```

### Status Information

For each position command, the system displays:
- **Pos**: Target position in degrees
- **Tau**: Output torque in Newton-meters
- **Pos_err**: Position error in radians
- **Speed**: Current angular velocity in rad/s

## Safety Warnings

- **Alignment Phase**: Do not disturb the motor during the initial alignment action to avoid jitter
- **Current Monitoring**: Always monitor current levels and be ready to cut power in case of abnormalities
- **Input Format**: Press Enter after entering position values to confirm input
- **Initial Testing**: Start with small position changes for first-time operation
- **Force Feedback**: The hybrid controller allows compliant motion - the motor will respond to external forces based on the configured stiffness and damping parameters

## Application Notes

### Tuning Guidelines

1. **Increase Stiffness**: Increase `kp` value for firmer position holding
2. **Increase Damping**: Increase `kd` value to reduce oscillations
3. **Adjust Compliance**: Decrease `kp` and `kd` for more compliant behavior
4. **Torque Limiting**: Adjust `tau_limit` based on application safety requirements
5. **Noise Reduction**: Tune `speed_lpf_alpha` and `speed_deadzone` to filter sensor noise

### Typical Applications

- Collaborative robotics (cobots)
- Haptic feedback devices
- Precision assembly with force control
- Compliant motion in constrained environments
- Human-robot interaction scenarios

