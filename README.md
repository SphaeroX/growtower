# GrowTower

A minimalist, vertical grow tower designed for home cannabis cultivation. This project focuses on space efficiency, ease of use, and effective odor control, making it perfect for beginners and small living spaces.

## Features

- **Vertical Grow System:** Maximizes vertical space usage.
- **Integrated Odor Control:** Features a custom-designed activated carbon filter housing.
- **Silent & Efficient Cooling:**  Uses a standard Arctic P14 generic PC fan for both airflow and cooling.
- **Web-based Control:** Control your GrowTower from any browser at `http://growtower.local` - no app required!
- **Automatic Light Timer:** Configurable 24h light schedule with web interface.
- **Minimalist Design:** clean aesthetics that blend into a modern home environment.
- **Beginner Friendly:**  Simple assembly and maintenance. Hard to make mistakes.
- **3D Printable:**  Most structural components are designed to be 3D printed.

## Gallery

| | | |
|:---:|:---:|:---:|
| ![View 1](images/1.jpg) | ![View 2](images/2.jpg) | ![View 3](images/3.jpg) |
| *Front View* | *With Light Chamber* | *Inside* |

## How it Works

The GrowTower utilizes a forced-air system where the Arctic P14 fan pulls air through the tower, ensuring fresh CO2 for the plant and maintaining optimal temperatures. The air is then pushed through an activated carbon filter before exiting, neutralizing any odors. The vertical stacking design allows for modularity and easy height adjustment as the plant grows.

## Semi-Hydroponic Growing System

The GrowTower operates as a **semi-hydroponic system** using a drip irrigation method. Water is delivered directly to the plant's root zone through a drip system, providing precise moisture control while maintaining some of the benefits of traditional soil growing.

### Recommended Growing Medium

For optimal results and to prevent root rot in this semi-hydroponic setup, we highly recommend:

- **Composana Qualitätsblumenerde** mixed with **40% Perlite**
- This 1:1 mixture ensures excellent drainage and aeration
- The high perlite content prevents waterlogging and root rot, which is crucial when working with a drip system
- Avoid using regular potting soil without amendments, as it retains too much moisture

## Getting Started

See [BOM.md](BOM.md) for a full list of materials required to build your own GrowTower.

## Web Controller

The GrowTower includes a built-in web server for easy control via WiFi:

- **Access:** Open `http://growtower.local` in any browser after connecting to WiFi
- **Features:**
  - Real-time status dashboard (light, fan, timer)
  - Manual light control (ON/OFF)
  - PWM fan speed adjustment (0-100%) with configurable min/max range
  - 24-hour light timer with automatic scheduling
  - Configurable device hostname (e.g., `mytower.local`)
  - REST API for custom integrations

### Quick Start

1. Flash the ESP32 with the firmware from the `firmware/` directory
2. Configure WiFi credentials in `firmware/.env` (see [firmware/README.md](firmware/README.md))
3. Connect to power - the device will automatically connect to WiFi
4. Open `http://growtower.local` in your browser
5. Control your GrowTower from anywhere on your network!

For detailed firmware documentation, see [firmware/README.md](firmware/README.md).

## Plant Pot Printing Instructions

For optimal results without supports, the plant pot can be cut in the slicer and glued together after printing:

1. **Cut the Pot:** In your slicer, use the cut tool to split the pot horizontally into two parts.
2. **Print Both Parts:** Print both halves without supports, flat side down.
3. **Glue Together:** After printing, glue the two halves together using super glue or epoxy.

| | |
|:---:|:---:|
| ![Plant Pot Cut 1](images/plant_pot_slicer_1.png) | ![Plant Pot Cut 2](images/plant_pot_slicer_2.png) |
| *Upper Part* | *Lower Part* |

## Internal Sieve Printing Instructions

The Internal Sieve model is intentionally designed to be "closed". To print it correctly:

1.  **Open the Sieve:** In your slicer, use a modifier to remove the top and bottom solid layers.
2.  **Create the Mesh:** Set the infill density to approximately **30%** and use a **Rectilinear** pattern. This generates the necessary mesh structure for the filter.

| | |
|:---:|:---:|
| ![Slicer View 1](images/sieve_slicer_1.png) | ![Slicer View 2](images/sieve_slicer_2.png) |

## Current Experiments

1. **Stress Killer CBD (Automatic):**
   - Currently testing with an automatic strain.
   - *Note:* While autoflowers can be counterproductive in restricted spaces due to lack of control over size/timing, this run serves primarily as a system stress test.
   - **Results so far:** The wick system with no-name soil (without perlite) created too much moisture at the roots, which blocked nutrient uptake. The leaves are very yellow, and although the plant is flowering, it shouldn't be this harsh already. 
   - **The Good:** The buds are beautiful—large and dense!
   - **The Not-So-Good:** The lower part of the plant is bushy, which hopefully won't happen with the One-Bud method.
   - **Expected Harvest:** Beginning of March 2026 (if it survives that long).
   - **Lesson learned:** The wick system with regular soil without perlite was the problem. Too much moisture at the roots blocks nutrient uptake. If using the water reservoir, only add as much water as you would normally water with, OR mix in 50% perlite with the soil to maintain the reservoir feature without creating a swamp.

## Grow #2 - One-Bud Method with Improved Setup

For the second grow, I will implement the One-Bud technique with an improved setup based on lessons learned from the first grow and forum feedback.

### Technique

- **Strain Type:** Feminized seed (Photoperiod), NOT Automatic
- **Approach:** Completely skip the vegetative phase. The plant will be placed directly into the flowering light cycle (12/12) immediately after germination
- **Goal:** Produce a single main cola (One-Bud). This minimizes lateral growth, saves significant space, and reduces the total cultivation time. The GrowTower's vertical design is exceptionally well-suited for this technique.

### Setup Changes for Grow #2

- **Growing Medium:** Composana Qualitätsblumenerde mixed with **40% Perlite** (instead of regular soil without amendments)

- **Fertilizer Strategy (based on forum feedback):**
  - Use only **50% of manufacturer dosage** (reduce by half)
  - Target EC value: **max 1.5** (previously 1.8 was too high)
  - This prevents over-fertilization and nutrient burn
  - Monitor EC in drain water to check uptake
  
- **pH Management:**
  - Use proper pH-down products (e.g., Canna pH- Bloom)
  - Avoid pH-down with citric acid for pH correction
  - Target pH range: 6.0-6.3

