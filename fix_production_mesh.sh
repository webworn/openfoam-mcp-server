#!/bin/bash
# Fix the 2D RDE mesh generation issue

echo "🔧 Fixing 2D RDE mesh generation..."

cd production_2d_rde

# Create a corrected blockMeshDict with proper vertex ordering
cat > system/blockMeshDict << 'EOF'
/*--------------------------------*- C++ -*----------------------------------*\
| =========                 |                                                 |
| \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |
|  \\    /   O peration     | Version:  12                                     |
|   \\  /    A nd           | Web:      www.OpenFOAM.org                      |
|    \\/     M anipulation  |                                                 |
\*---------------------------------------------------------------------------*/
FoamFile
{
    version     2.0;
    format      ascii;
    class       dictionary;
    object      blockMeshDict;
}

// Simplified 2D RDE Annular Geometry
// Inner radius: 50mm, Outer radius: 80mm, Length: 150mm

convertToMeters 0.001; // millimeters

vertices
(
    // Bottom face (z=0) - 8 points around annulus
    // Inner radius points
    (50.0   0.0   0)    // 0
    (35.36  35.36 0)    // 1  
    (0.0    50.0  0)    // 2
    (-35.36 35.36 0)    // 3
    (-50.0  0.0   0)    // 4
    (-35.36 -35.36 0)   // 5
    (0.0    -50.0 0)    // 6
    (35.36  -35.36 0)   // 7
    
    // Outer radius points
    (80.0   0.0   0)    // 8
    (56.57  56.57 0)    // 9
    (0.0    80.0  0)    // 10
    (-56.57 56.57 0)    // 11
    (-80.0  0.0   0)    // 12
    (-56.57 -56.57 0)   // 13
    (0.0    -80.0 0)    // 14
    (56.57  -56.57 0)   // 15
    
    // Top face (z=150) - same pattern
    // Inner radius points
    (50.0   0.0   150)  // 16
    (35.36  35.36 150)  // 17
    (0.0    50.0  150)  // 18
    (-35.36 35.36 150)  // 19
    (-50.0  0.0   150)  // 20
    (-35.36 -35.36 150) // 21
    (0.0    -50.0 150)  // 22
    (35.36  -35.36 150) // 23
    
    // Outer radius points
    (80.0   0.0   150)  // 24
    (56.57  56.57 150)  // 25
    (0.0    80.0  150)  // 26
    (-56.57 56.57 150)  // 27
    (-80.0  0.0   150)  // 28
    (-56.57 -56.57 150) // 29
    (0.0    -80.0 150)  // 30
    (56.57  -56.57 150) // 31
);

blocks
(
    // 8 annular sectors - correct vertex ordering (right-hand rule)
    hex (0 8 9 1 16 24 25 17) (20 15 60) simpleGrading (1 1 1)    // Sector 1
    hex (1 9 10 2 17 25 26 18) (20 15 60) simpleGrading (1 1 1)   // Sector 2  
    hex (2 10 11 3 18 26 27 19) (20 15 60) simpleGrading (1 1 1)  // Sector 3
    hex (3 11 12 4 19 27 28 20) (20 15 60) simpleGrading (1 1 1)  // Sector 4
    hex (4 12 13 5 20 28 29 21) (20 15 60) simpleGrading (1 1 1)  // Sector 5
    hex (5 13 14 6 21 29 30 22) (20 15 60) simpleGrading (1 1 1)  // Sector 6
    hex (6 14 15 7 22 30 31 23) (20 15 60) simpleGrading (1 1 1)  // Sector 7
    hex (7 15 8 0 23 31 24 16) (20 15 60) simpleGrading (1 1 1)   // Sector 8
);

edges
(
    // Inner cylinder arcs (bottom)
    arc 0 1 (43.30 25.0 0)
    arc 1 2 (25.0 43.30 0)
    arc 2 3 (-25.0 43.30 0)
    arc 3 4 (-43.30 25.0 0)
    arc 4 5 (-43.30 -25.0 0)
    arc 5 6 (-25.0 -43.30 0)
    arc 6 7 (25.0 -43.30 0)
    arc 7 0 (43.30 -25.0 0)
    
    // Outer cylinder arcs (bottom)
    arc 8 9 (69.28 40.0 0)
    arc 9 10 (40.0 69.28 0)
    arc 10 11 (-40.0 69.28 0)
    arc 11 12 (-69.28 40.0 0)
    arc 12 13 (-69.28 -40.0 0)
    arc 13 14 (-40.0 -69.28 0)
    arc 14 15 (40.0 -69.28 0)
    arc 15 8 (69.28 -40.0 0)
    
    // Inner cylinder arcs (top)
    arc 16 17 (43.30 25.0 150)
    arc 17 18 (25.0 43.30 150)
    arc 18 19 (-25.0 43.30 150)
    arc 19 20 (-43.30 25.0 150)
    arc 20 21 (-43.30 -25.0 150)
    arc 21 22 (-25.0 -43.30 150)
    arc 22 23 (25.0 -43.30 150)
    arc 23 16 (43.30 -25.0 150)
    
    // Outer cylinder arcs (top)
    arc 24 25 (69.28 40.0 150)
    arc 25 26 (40.0 69.28 150)
    arc 26 27 (-40.0 69.28 150)
    arc 27 28 (-69.28 40.0 150)
    arc 28 29 (-69.28 -40.0 150)
    arc 29 30 (-40.0 -69.28 150)
    arc 30 31 (40.0 -69.28 150)
    arc 31 24 (69.28 -40.0 150)
);

boundary
(
    inlet
    {
        type patch;
        faces
        (
            (8 24 25 9)   // Injection at outer radius
            (9 25 26 10)
        );
    }
    
    outlet
    {
        type patch;
        faces
        (
            (12 28 29 13) // Outlet opposite to inlet
            (13 29 30 14)
        );
    }
    
    walls
    {
        type wall;
        faces
        (
            // Inner cylinder
            (0 16 17 1) (1 17 18 2) (2 18 19 3) (3 19 20 4)
            (4 20 21 5) (5 21 22 6) (6 22 23 7) (7 23 16 0)
            
            // Outer cylinder (except inlet/outlet)
            (10 26 27 11) (11 27 28 12) (14 30 31 15) (15 31 24 8)
        );
    }
    
    front
    {
        type empty;
        faces
        (
            (0 1 9 8) (1 2 10 9) (2 3 11 10) (3 4 12 11)
            (4 5 13 12) (5 6 14 13) (6 7 15 14) (7 0 8 15)
        );
    }
    
    back
    {
        type empty;
        faces
        (
            (16 24 25 17) (17 25 26 18) (18 26 27 19) (19 27 28 20)
            (20 28 29 21) (21 29 30 22) (22 30 31 23) (23 31 24 16)
        );
    }
);
EOF

echo "✅ Fixed blockMeshDict created"

# Generate mesh with corrected geometry
echo "🔧 Generating corrected mesh..."
blockMesh > blockMesh_fixed.log 2>&1

if [ $? -eq 0 ]; then
    echo "✅ Mesh generated successfully!"
    echo "📊 Mesh statistics:"
    checkMesh -noTopology | grep -E "(cells|points|faces)"
    
    echo ""
    echo "🎬 Mesh ready for video generation!"
    echo "📁 Check mesh quality: paraFoam"
    echo "🚀 Continue with: rhoReactingFoam"
    
else
    echo "❌ Mesh still failed. Check blockMesh_fixed.log"
    echo "🔍 Common issues:"
    echo "  • Vertex ordering (right-hand rule)"
    echo "  • Arc point calculations"
    echo "  • Boundary face definitions"
fi