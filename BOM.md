# Bill of Materials (BOM)

Here is the list of parts required to build the GrowTower.

## Hardware & Electronics

| Component | Quantity | Description | Price (Est.) | Source | Notes |
| :--- | :---: | :--- | :---: | :--- | :--- |
| **Fan** | 1 | Arctic P14 PWM PST (140mm) | 9.99€ | Amazon | High Static Pressure Fan. |
| **Power Supply** | 1 | 24V DC, 2A | 13.00€ | Amazon | Power supply for LEDs and Fan. |
| **Step-Down Module** | 2 | DC-DC Buck Converter | ~3.40€ | Amazon | Calculated from 5-pack (8.40€). Used to lower voltage for fan/controller if needed. |
| **Microcontroller** | 1 | Seed Studio XIAO ESP32 C6 | 5.00€ | AliExpress | Control unit (WiFi/BLE). |
| **Relay Module** | 1 | Arduino Relay Module (1-Channel) | 1.70€ | AliExpress | To switch high voltage components (if needed). |
| **Wiring** | - | 22 AWG Silicone Wire (0.32mm²) | ~5.00€ | AliExpress | Soft silicone insulation recommended. |
| **Screws & Nuts** | 4x | M3 x 10-15mm + Nuts | ~2.00€ | Hardware Store | For mounting components. |
| **Grow Light** | 1 | Auxmer Low Power 2835 LED Strip | 27.00€ | AliExpress | 120 LEDs/m, 4000K Color Temp. Price includes shipping. |

## Consumables

| Component | Quantity | Description | Price (Est.) | Notes |
| :--- | :---: | :--- | :---: | :--- |
| **Filament** | ~1 kg | PETG White | 22.00€ | Recommended for durability and light reflection. |
| **Activated Carbon** | 1 kg | Pellets (3-4mm), CTC > 65% | 16.80€ | eBay (helo24) | High absorption. |
| **Black Spray Foil** | 1 Can | Liquid Rubber / PlastiDip | ~15.00€ | Amazon/Hardware Store | For perfect light blocking and gap filling. |
| **Pipettes** | 5-10 pcs | 3ml Plastic Pipettes | ~3.00€ | Amazon/Pharmacy | Essential for precise dosage of pH-Down and fertilizer. |

## Growing Mediums & Nutrients

These materials depend on the specific grow method being tested (Soil vs. Hydroponic).

| Component | Description | Price (Est.) | Notes |
| :--- | :--- | :---: | :--- |
| **Composana Soil** | Qualitätsblumenerde | ~12.00€ | Small "earth nest" at top of pot for germination and initial seedling nutrients. Only ~200ml needed per grow. |
| **Pumice (Bimsstein)** | Compo Sana Granulat | ~9.00€ | Main growing medium (inert, no CEC). Excellent aeration and wicking. Reusable indefinitely — rinse between grows. |
| **Wick System (Dochte)** | Nylon or Cotton Wicks | ~5.00€ | Passive irrigation for reservoir setups. |
| **Fertilizer** | Hy-Pro Terra (Single Base) | ~15.00€ | Complete 1-component mineral nutrient (NPK 2-5-5). For pumice: 50% start → 75-100% stretch → 100% flower → optional 125% peak. ~2.0–5.0ml per 800ml. Use a pipette for precision. |
| **pH-Down** | Canna pH- Bloom (Phosphoric Acid) | ~12.00€ | Critical for Hydro and Soil. Recommended: ~2.3ml per 800ml water (depends on local tap water). Use a pipette. |
| **CalMag** | Hy-Pro Cal-Mag (or equivalent) | ~12.00€ | Prevents Ca/Mg deficiencies in reused inert substrate. Add 0.5–1.0 ml/L to every 3rd reservoir refill. |  
## 3D Printed Parts

Most parts need to be printed in **PETG White** to ensure maximum light reflection inside the tower. However, to prevent light leaks and algae, specific parts (Filter, Plant Pot, Reservoir) should be printed in black, and the white outer shell must be post-processed to be light-proof from the outside. For detailed printing instructions and color choices, see [`stls/README.md`](stls/README.md).

The modular design consists of:

### Base & Reservoir
- **Plant Pot:** The bottom container for the plant.
- **Reservoir/Overpot:** Outer water container/shell that holds the pot.

### Shell Assembly
The 4-level structure is built from the following parts:
- **Inner Shell (4x):** Holds the plant/roots.
- **Outer Shell Bottom (1x):** The base segment of the outer shell.
- **Outer Shell (3x):** Standard segments stacked on top of the bottom shell.
- **Shell Connector (3x):** Outer rings used to connect the shell segments (glued).

### Filter Assembly (Top)
- **Filter Housing:** Container for the activated carbon.
- **Internal Sieve:** Sieve insert for inside the filter housing.
- **Fan Adapter:** Connects the 140mm fan to the filter housing.

### Accessories
- **Dryer Module:** An optional insert placed between the top Inner Shell and the Filter Housing. Used to hang and tie the plant for drying within the tower after harvest, utilizing the tower's active carbon filter for odor control.

*Note: STL files are located in the `stls/` directory (placeholder).*

## Tools Required

- **3D Printer:** (or access to one)
- **Soldering Iron:** For electrical connections.
- **Side Cutters / Wire Strippers:** For preparing cables.
- **Screwdriver:** For M3 screws.

## Total Estimated Cost

- **GrowTower Construction:** Approx. **~125.00€** (Hardware, Electronics, Filament, and Filter materials).
- **Consumables & Growing:** Approx. **~50.00€** (Pumice, Soil, Fertilizer, pH-Down, and Pipettes).

**Total Project Estimate: ~175.00€** (excluding seeds).
