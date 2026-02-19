# 3D Printing Instructions

This directory contains all STL files for the GrowTower project.

## General Printing Recommendations

- **Material:** PETG White (recommended for durability and light reflection)
- **Nozzle:** 0.4mm standard
- **Support:** Generally not required for most parts

---

## Plant Pot Printing Instructions

For optimal results without supports, the plant pot can be cut in the slicer and glued together after printing:

1. **Cut the Pot:** In your slicer, use the cut tool to split the pot horizontally into two parts.
2. **Print Both Parts:** Print both halves without supports, flat side down.
3. **Glue Together:** After printing, glue the two halves together using super glue or epoxy.

---

## Internal Sieve Printing Instructions

The Internal Sieve model is intentionally designed to be "closed". To print it correctly:

1.  **Open the Sieve:** In your slicer, use a modifier to remove the top and bottom solid layers.
2.  **Create the Mesh:** Set the infill density to approximately **30%** and use a **Rectilinear** pattern. This generates the necessary mesh structure for the filter.

---

## Reservoir 850 Vase Printing Instructions

**File:** `Reservoir 850 vase.stl`

For optimal printing results and watertightness, use these settings:

1. **Vase Mode:** Enable "Spiralize" or "Vase Mode" in your slicer.
2. **Bottom Layers:** Use at least **4 bottom layers** for a stable and leak-proof base.
3. **Pattern Design:** Ensure the upper pattern differs from the lower pattern to create a distinct transition and visual appeal.
4. **Layer Height:** Use a **small layer height** (e.g., 0.1-0.2mm) to ensure the print is watertight.
5. **Flow Rate:** Increase flow rate by at least **5%** compared to your standard settings for better layer adhesion and watertightness.
6. **Ironing:** Enable ironing to re-weld the top layer for a smoother and more sealed surface.
7. **Infill/Wall Overlap (OrcaSlicer):** Set BOTH values to **40% to 50%**:
   - `InfillWall-Overlap`
   - `infill-wall-overlap`

### Troubleshooting Leaks

- **Water Test:** Always perform a water test before using the reservoir to check for leaks.
- **Common Issue:** Pay special attention to the transition from the bottom to the walls, as this is where leaks most commonly occur.
- **Repair:** If leaks are found at the bottom-wall transition, they can often be fixed by welding the spots with a soldering iron. Don't give up - somehow it can always be made watertight!

---

## Other Parts

The remaining parts (Inner Shell, Outer Shell, Outer Shell Bottom, Shell Connector, Filter Housing, Fan Adapter) can be printed with standard settings:

- **Infill:** 15-20%
- **Supports:** Not required
- **Orientation:** Print as oriented in the STL file

---

## Images

See the main [README.md](../README.md) for slicer screenshots and assembly photos.
