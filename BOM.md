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
| **Black Acrylic Paint** | 1 Can | Spray Paint | ~6.00€ | Amazon/Hardware Store | For light blocking/aesthetics. |

## 3D Printed Parts

The following parts need to be printed (recommended in PETG White). The modular design consists of:

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

*Note: STL files are located in the `stls/` directory (placeholder).*

## Tools Required

- **3D Printer:** (or access to one)
- **Soldering Iron:** For electrical connections.
- **Side Cutters / Wire Strippers:** For preparing cables.
- **Screwdriver:** For M3 screws.

## Total Estimated Cost

**Approx. ~111.90€** (plus misc. items like potting soil, seeds, etc.)
