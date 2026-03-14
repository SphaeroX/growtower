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

## Semi-Hydroponic Growing System

The GrowTower operates as a **semi-hydroponic system** using a drip irrigation method. Water is delivered directly to the plant's root zone through a drip system, providing precise moisture control while maintaining some of the benefits of traditional soil growing.

### Recommended Growing Medium

For optimal results and to prevent root rot in this semi-hydroponic setup, we highly recommend:

- **Composana Qualitätsblumenerde** mixed with **40% Perlite**
- This 1:1 mixture ensures excellent drainage and aeration
- The high perlite content prevents waterlogging and root rot, which is crucial when working with a drip system
- Avoid using regular potting soil without amendments, as it retains too much moisture

## Drying after Harvest

With the included **Dryer Module** (STL), the GrowTower can easily be converted into an odor-free drying chamber. The module is simply inserted between the top Inner Shell and the Filter Housing. You can hang and tie your harvested plant directly to this module. The active carbon filter ensures that the entire drying process remains completely odorless.

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

## 3D Printing Instructions

All STL files and detailed printing instructions are located in the [`stls/`](stls/) directory. This includes material recommendations, slicer settings, and assembly instructions.

For detailed printing instructions including material recommendations, light-proofing steps, and slicer settings for the Plant Pot, Internal Sieve, and Reservoir, please see [`stls/README.md`](stls/README.md).

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

- **Strain:** Purple Bud (Feminized Photoperiod)
- **Approach:** Completely skip the vegetative phase. Both plants will be placed directly into the flowering light cycle (12/12) immediately after germination
- **Goal:** Produce a single main cola (One-Bud). This minimizes lateral growth, saves significant space, and reduces the total cultivation time. The GrowTower's vertical design is exceptionally well-suited for this technique.

### Setup Changes for Grow #2

- **Growing Medium:** Composana Qualitätsblumenerde mixed with **40-50% Perlite**

- **Fertilizer:** Use **mineral Compo Blüte** fertilizer 

### Parallel Experiment: Two Towers Comparison

For Grow #2, I will run **two GrowTowers simultaneously** with Purple Bud (Photoperiod) to compare two different watering setups:

| Tower A | Tower B |
|---------|---------|
| Composana + 50% Perlite | **LECA** (Expanded Clay) + Wick |
| Bottom watering only | Water reservoir with wick system |
| Purple Bud | Purple Bud |

**Tower A (No Wick):**
- Classic bottom watering approach
- Direct control over water/nutrient delivery
- No risk of overwatering from reservoir

**Tower B (With Wick + LECA):**
- Bottom section (feet): Completely filled with LECA, no soil
- Wick embedded in the LECA layer for passive water transport
- Tests if LECA + wick provides the right moisture balance
- LECA ensures excellent drainage and aeration around the roots

This comparison will help determine which method works best for the GrowTower semi-hydroponic setup. 

### Optimized 12/12 Semi-Hydro Timeplan (One-Bud Method)

This schedule is specifically designed for the GrowTower using a 50/50 Soil-Perlite mix and the One-Bud technique (12/12 from seed).

#### Phase 0: Pre-Grow (Germination & Underground Growth)
**Duration:** Approx. 3 to 7 days (not counted as weeks).
- **Action:** 
  1. Soak seed in water for **24–36 hours**.
  2. Plant seed in 50/50 Soil-Perlite mix and water with **30 ml**.
  3. Carefully add **3 ml** with a pipette twice daily (morning and evening) directly onto the seed's position to keep it moist (not soaked).
- **Light:** Set to **12/12 cycle** immediately so the seedling adapts from day one.
- **Goal:** Phase 0 ends as soon as the seedling breaks the surface and stands upright (Day 1).

#### Week 1 (Day 1 to 7): Establishment (Seedling Phase)
- **Development:** Seedling stands up, cotyledons open, and the first pair of true (serrated) leaves forms.
- **Action:** Continue adding water only drop by drop (pipette) if the surface around the stem is extremely dry. The root is already searching for its way down into the 50/50 mix.
- **Nutrients:** Pure water only.

#### Week 2 (Day 8 to 14): First Flushing & Root Stretch
- **Development:** The first pair of true leaves is fully formed.
- **Action (Day 8-10):** Perform the first **full watering from above** until water runs out the bottom (**runoff**). The substrate must be completely saturated once.
- **Critical:** After this flushing, **do not water at all** until the pot feels significantly lighter. This drying period forces the roots to stretch to the bottom of the pot in search of moisture.

#### Week 3 (Day 15 to 21): Switch to Semi-Hydro (Bottom Watering)
- **Development:** Roots should now have reached the bottom.
- **The "Reservoir Test":** Fill the reservoir to approx. 20-30% with pure water. If the water level drops after 24–48 hours, the capillary action is working perfectly.
- **Action:** **Stop all top-watering.** From now on, only fill the bottom tank with pure water.
- **Nutrients:** Still pure water only! The nutrients in the 50% Compo Sana soil are sufficient.

#### Week 4 to 5 (Day 22 to 35): Pre-Flowering & Stretch
- **Development:** Despite the 12/12 cycle from Day 1, the plant needed this time to reach hormonal maturity. It will now begin to stretch rapidly and show the first white pistils.
- **Action:** Continue filling the reservoir with pure water. Water consumption will increase significantly.

#### Week 6 to 8 (Day 36 to 56): Flowering Phase (Nutrients)
- **Development:** Vertical stretch stops. Energy shifts entirely into bud production.
- **Action (Fertilizer):** Start adding **Compo Blüte** to the reservoir. Begin with **25% to max. 50%** of the manufacturer's recommendation. 
  - *Observation:* If leaves turn very dark green or show "clawing," reduce the dosage immediately.
- **Mandatory Task (Salt Flush):** Once in Week 6 and once in Week 8, water from above with pure water until significant runoff occurs. This washes out fertilizer salts that have accumulated at the surface due to capillary action. Discard the drainage water.

#### Week 9 to Harvest (Day 57 to End): Ripening & Final Flush
- **Development:** Buds swell, and resin glands (trichomes) become milky/cloudy.
- **Action (Final Flush):** Stop all fertilizer **10 to 14 days before harvest**. Fill the reservoir with pure water only.
- **Goal:** The plant will consume its remaining nutrient reserves from its leaves (leaves will yellow and fall off). This is intentional and ensures clean, smooth-tasting buds without harshness.

### Critical Watering Warning

**NEVER water from above later in the grow!**

When bottom watering with mineral fertilizer, water moves up through the soil and evaporates at the surface. The fertilizer salts accumulate in the top soil layer (often visible as white crust).

If you suddenly water from above, you flush these highly concentrated salts directly down into the roots - causing immediate, fatal nutrient shock.

**Always water from below throughout the entire grow.**

### pH Management

- Use proper pH-down products (e.g., Canna pH- Bloom)
- Avoid pH-down with citric acid for pH correction
- Target pH range: 6.0-6.3

