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
- **Odor-Free Drying:** Convert the tower into a drying chamber after harvest using the included Dryer Module, utilizing the carbon filter.

## Gallery

| | | |
|:---:|:---:|:---:|
| ![View 1](images/1.jpg) | ![View 2](images/2.jpg) | ![View 3](images/3.jpg) |
| *Front View* | *With Light Chamber* | *Inside* |

### Controller & Electronics

![Controller](images/controller.jpg)
*Wired ESP32 Controller with Fan, Light Relay and Power Supply*

## How it Works

The GrowTower utilizes a forced-air system where the Arctic P14 fan pulls air through the tower, ensuring fresh CO2 for the plant and maintaining optimal temperatures. The air is then pushed through an activated carbon filter before exiting, neutralizing any odors. The vertical stacking design allows for modularity and easy height adjustment as the plant grows.

## Growing System

The GrowTower is designed for a **Full-Hydroponic Wick System** (using Pumice) or a **Semi-Hydroponic** setup. It utilizes passive irrigation to provide precise moisture control while maintaining high aeration for the roots.

For detailed, step-by-step instructions on the recommended **12/12 One-Bud method**, please refer to the [**GROW_GUIDE.md**](GROW_GUIDE.md).

## Drying after Harvest

With the included **Dryer Module** (STL), the GrowTower can easily be converted into an odor-free drying chamber. The module is simply inserted between the top Inner Shell and the Filter Housing. You can hang and tie your harvested plant directly to this module. The active carbon filter ensures that the entire drying process remains completely odorless.

## Getting Started

1. **Build your Tower:** See [BOM.md](BOM.md) for a full list of materials required.
2. **Follow the Grow Guide:** Use the [**GROW_GUIDE.md**](GROW_GUIDE.md) as your primary resource for successful cultivation. It contains the complete timeline, nutrient dosages, and substrate setup.
3. **Setup Controller:** Flash the ESP32 with the firmware from the `firmware/` directory. See [firmware/README.md](firmware/README.md) for details.

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

## 3D Printing Instructions

All STL files and detailed printing instructions are located in the [`stls/`](stls/) directory. This includes material recommendations, slicer settings, and assembly instructions. For detailed post-processing (light-proofing), see [`stls/README.md`](stls/README.md).
