---
name: validate-stl
description: Validate and check 3D model STL files for printing. Includes special handling for Internal Sieve mesh generation and print settings verification.
license: MIT
compatibility: opencode
metadata:
  project: growtower
  component: hardware
  files: stl
  action: validate
---

## When to Use

- Before printing any STL file
- After modifying or exporting STL files
- Checking for manifold geometry issues
- Verifying dimensions match hardware components
- Preparing Internal Sieve for mesh printing

## STL Files Location

All models in `stls/` directory:
- `Internal Sieve.stl` - Carbon filter mesh (special handling required)
- `Fan Adapter.stl` - Arctic P14 mounting
- `Outer Shell.stl` - Main tower body
- `Inner Shell.stl` - Light chamber
- `plant pot.stl` - Plant container
- `Filter Housing.stl` - Carbon filter case
- `Reservoir 850 vase.stl` - Water reservoir
- `Shell Connector.stl` - Modular connector
- `Outer Shell Bottom.stl` - Base piece

## Instructions

1. Open STL in slicer software
2. Check for errors (non-manifold edges, holes)
3. Verify dimensions against hardware specs
4. Apply appropriate print settings
5. Slice and inspect preview

## Special: Internal Sieve Printing

The Internal Sieve requires special slicer settings:

1. **Remove solid layers:**
   - Use slicer modifier to remove top and bottom layers
   - This creates openings for the mesh

2. **Set infill:**
   - Density: **30%**
   - Pattern: **Rectilinear**
   - This generates the carbon filter mesh structure

3. **Result:** Grid-like mesh perfect for activated carbon

## Recommended Print Settings

**Material:** PETG (white recommended for light reflection)
**Infill:** 20-30% depending on part strength needs
**Layer height:** 0.2mm for good detail
**Supports:** Minimal, mostly self-supporting designs

## Hardware Dimensions Reference

- **Arctic P14 fan:** 140mm mounting holes
- **LED strips:** Channel width for Auxmer 2835
- **Carbon filter:** Fits in Filter Housing with Internal Sieve

## Tools

- **PrusaSlicer** - Recommended, good error detection
- **Cura** - Alternative with good supports
- **Bambu Studio** - Newer option with excellent defaults

## Troubleshooting

- **Non-manifold edges:** Use slicer's fix feature or repair in Meshmixer
- **Wrong scale:** Check STL units (should be in mm)
- **Internal Sieve solid:** Verify top/bottom layers removed
- **Fit issues:** Measure hardware, compare to STL dimensions
