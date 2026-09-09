// NIGHTMARE Neural HD — Advanced Neural Reconstruction & Multi-Biome WebGPU Shader (WGSL)
// Optimized Version: Fused Multi-Pass Texture Lookups, Low-Overhead Normals & Quality-Tiered Shadows

struct Uniforms {
    renderMode: u32,             // 0: Original, 1: Enhanced, 2: Split A/B, 3: Depth, 4: Light, 5: Semantic, 6: Normals
    splitPos: f32,               // 0.0 to 1.0 for A/B Split slider
    lightIntensity: f32,         // Point light boost multiplier
    bonfireFlicker: f32,         // Animated flicker amplitude
    contactShadowStrength: f32,  // Contact shadow darkness
    waterSpecular: f32,          // Water/Liquid specular intensity
    time: f32,                   // Current time in seconds
    dungeonBiome: u32,           // 0: Town, 1: Cathedral, 2: Catacombs, 3: Caves, 4: Hell, 5: Crypt, 6: Hive
    resolution: vec2<f32>,       // Screen dimensions (640.0, 480.0)
    bonfirePos: vec2<f32>,       // Light source position
    qualityTier: u32,            // 0: Safe Baseline (60FPS), 1: Rich Atmospheric (Shadows, Heat, Mist)
    mistDensity: f32,            // Ground mist density
};

@group(0) @binding(0) var<uniform> u: Uniforms;
@group(0) @binding(2) var t_rgb: texture_2d<f32>;
@group(0) @binding(3) var t_depth: texture_2d<f32>;
@group(0) @binding(4) var t_light: texture_2d<f32>;
@group(0) @binding(5) var t_semantic: texture_2d<f32>;

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
};

@vertex
fn vs_main(@builtin(vertex_index) vertexIndex: u32) -> VertexOutput {
    var out: VertexOutput;
    var pos = array<vec2<f32>, 3>(
        vec2<f32>(-1.0, -1.0),
        vec2<f32>( 3.0, -1.0),
        vec2<f32>(-1.0,  3.0)
    );
    out.position = vec4<f32>(pos[vertexIndex], 0.0, 1.0);
    out.uv = vec2<f32>((pos[vertexIndex].x + 1.0) * 0.5, (1.0 - pos[vertexIndex].y) * 0.5);
    return out;
}

fn getSemanticColor(semId: u32) -> vec3<f32> {
    switch (semId) {
        case 0u: { return vec3<f32>(0.05, 0.05, 0.05); }    // Void
        case 1u: { return vec3<f32>(0.15, 0.45, 0.15); }    // Floor
        case 2u: { return vec3<f32>(0.60, 0.40, 0.25); }    // Wall
        case 3u: { return vec3<f32>(0.20, 0.50, 0.90); }    // Player
        case 4u: { return vec3<f32>(0.90, 0.70, 0.10); }    // NPC
        case 5u: { return vec3<f32>(0.10, 0.70, 0.80); }    // Water / Lava
        case 6u: { return vec3<f32>(0.90, 0.25, 0.20); }    // Fire
        case 7u: { return vec3<f32>(0.80, 0.10, 0.80); }    // Monsters
        case 8u: { return vec3<f32>(1.00, 0.90, 0.20); }    // Missiles
        case 9u: { return vec3<f32>(0.50, 0.35, 0.15); }    // Doors
        default: { return vec3<f32>(1.0, 1.0, 1.0); }
    }
}

fn hash21(p: vec2<f32>) -> f32 {
    let q = fract(p * vec2<f32>(123.34, 456.21));
    return fract(dot(q, q + 45.32));
}

// Subpixel Edge Reconstruction, Legacy Rain Eradicator & Heat Shimmer (Optimized)
fn sampleReconstructedRGB(coords: vec2<i32>) -> vec4<f32> {
    let dim = vec2<i32>(u.resolution);
    var sampleCoords = coords;

    // Atmospheric Heat Shimmer (Town Bonfire only)
    if (u.qualityTier > 0u && u.dungeonBiome == 0u) {
        let dyFire = u.bonfirePos.y - f32(coords.y);
        let dxFire = abs(f32(coords.x) - u.bonfirePos.x);
        if (dyFire > 6.0 && dyFire < 80.0 && dxFire < 38.0) {
            let heatFactor = (1.0 - (dxFire / 38.0)) * (1.0 - (dyFire / 80.0));
            let shimmerX = sin(f32(coords.y) * 0.16 + u.time * 8.5) * (heatFactor * 1.6);
            let shimmerY = cos(f32(coords.x) * 0.18 + u.time * 7.0) * (heatFactor * 0.8);
            sampleCoords = clamp(coords + vec2<i32>(i32(shimmerX), i32(shimmerY)), vec2<i32>(0), dim - 1);
        }
    }

    let c = textureLoad(t_rgb, clamp(sampleCoords, vec2<i32>(0), dim - 1), 0);
    let cL = textureLoad(t_rgb, clamp(sampleCoords + vec2<i32>(-1, 0), vec2<i32>(0), dim - 1), 0);
    let cR = textureLoad(t_rgb, clamp(sampleCoords + vec2<i32>(1, 0), vec2<i32>(0), dim - 1), 0);
    let cU = textureLoad(t_rgb, clamp(sampleCoords + vec2<i32>(0, -1), vec2<i32>(0), dim - 1), 0);
    let cD = textureLoad(t_rgb, clamp(sampleCoords + vec2<i32>(0, 1), vec2<i32>(0), dim - 1), 0);

    let lC = dot(c.rgb, vec3<f32>(0.299, 0.587, 0.114));
    let lL = dot(cL.rgb, vec3<f32>(0.299, 0.587, 0.114));
    let lR = dot(cR.rgb, vec3<f32>(0.299, 0.587, 0.114));
    let lU = dot(cU.rgb, vec3<f32>(0.299, 0.587, 0.114));
    let lD = dot(cD.rgb, vec3<f32>(0.299, 0.587, 0.114));

    // Legacy Rain Eradicator in Town
    if (u.dungeonBiome == 0u) {
        if (lC < lL - 0.04 && lC < lR - 0.04 && abs(lC - lU) < 0.05 && abs(lC - lD) < 0.05) {
            return mix(c, (cL + cR) * 0.5, 0.85);
        }
    }

    // Subpixel Edge Smoothing (DLSS-style)
    let edgeH = abs(lL - lR);
    let edgeV = abs(lU - lD);

    if (max(edgeH, edgeV) > 0.08) {
        let blend = (c + cL + cR + cU + cD) * 0.2;
        return mix(c, blend, 0.42);
    }
    return c;
}

// Compute 2.5D Pseudo-Normals with Surface Relief (Optimized Texture Access)
fn computeMultiBiomeNormal(coords: vec2<i32>, semId: u32, depthVal: f32, isHeadRegion: bool) -> vec3<f32> {
    let texDim = vec2<i32>(u.resolution);
    
    let dL = textureLoad(t_depth, clamp(coords + vec2<i32>(-1, 0), vec2<i32>(0), texDim - 1), 0).r;
    let dR = textureLoad(t_depth, clamp(coords + vec2<i32>( 1, 0), vec2<i32>(0), texDim - 1), 0).r;
    let dU = textureLoad(t_depth, clamp(coords + vec2<i32>(0, -1), vec2<i32>(0), texDim - 1), 0).r;
    let dD = textureLoad(t_depth, clamp(coords + vec2<i32>(0,  1), vec2<i32>(0), texDim - 1), 0).r;

    let dz_dx = (dR - dL) * 28.0;
    let dz_dy = (dD - dU) * 28.0;

    var n = vec3<f32>(-dz_dx, -dz_dy, 1.0);
    let p = vec2<f32>(coords);

    switch (semId) {
        case 1u: {
            // Floor / Mud (Slight grain relief + wetness)
            let grain = (hash21(p) - 0.5) * 0.09;
            n = normalize(vec3<f32>(-dz_dx * 0.45 + grain, 0.68 - dz_dy * 0.22 + grain, 0.73));
        }
        case 2u: {
            // Walls & Columns (Stone relief)
            let mortarX = sin(p.x * 0.38) * 0.14;
            let mortarY = sin(p.y * 0.28) * 0.14;
            n = normalize(vec3<f32>(-dz_dx * 2.6 + mortarX, -dz_dy * 2.6 + mortarY, 0.32));
        }
        case 3u, 4u, 7u: {
            // Characters (Player 3u, NPCs 4u, Monsters/Enemies 7u):
            // 4-tap 2D sprite luminance gradient for micro-relief, muscles and curved plate armor
            let cL = textureLoad(t_rgb, clamp(coords + vec2<i32>(-1, 0), vec2<i32>(0), texDim - 1), 0).rgb;
            let cR = textureLoad(t_rgb, clamp(coords + vec2<i32>( 1, 0), vec2<i32>(0), texDim - 1), 0).rgb;
            let cU = textureLoad(t_rgb, clamp(coords + vec2<i32>(0, -1), vec2<i32>(0), texDim - 1), 0).rgb;
            let cD = textureLoad(t_rgb, clamp(coords + vec2<i32>(0,  1), vec2<i32>(0), texDim - 1), 0).rgb;
            let lumL = dot(cL, vec3<f32>(0.299, 0.587, 0.114));
            let lumR = dot(cR, vec3<f32>(0.299, 0.587, 0.114));
            let lumU = dot(cU, vec3<f32>(0.299, 0.587, 0.114));
            let lumD = dot(cD, vec3<f32>(0.299, 0.587, 0.114));
            let spriteGradX = (lumR - lumL) * 3.6;
            let spriteGradY = (lumD - lumU) * 3.6;

            var headBowing = vec2<f32>(0.0, 0.0);
            if (isHeadRegion) {
                headBowing = vec2<f32>(spriteGradX * 0.35, -0.28);
            }

            n = normalize(vec3<f32>(-dz_dx * 1.8 - spriteGradX + headBowing.x, -dz_dy * 1.8 - spriteGradY + headBowing.y, 0.65));
        }
        case 5u: {
            // Water / Lava waves
            if (u.dungeonBiome == 3u || u.dungeonBiome == 4u) {
                let lavaWave = sin(p.x * 0.06 + u.time * 0.9) * 0.06 + cos(p.y * 0.06 + u.time * 0.7) * 0.06;
                n = normalize(vec3<f32>(lavaWave, 0.8, 0.6));
            } else {
                let w1 = sin(p.x * 0.14 + u.time * 3.4) * 0.10;
                let w2 = cos(p.y * 0.16 + u.time * 2.6) * 0.10;
                n = normalize(vec3<f32>(w1, 0.72 + w2, 0.68));
            }
        }
        default: {
            n = normalize(n);
        }
    }

    return n;
}

fn fresnelSchlick(cosTheta: f32, F0: f32) -> f32 {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    let screenPixel = vec2<i32>(in.uv * u.resolution);
    let origColor = sampleReconstructedRGB(screenPixel);
    let depthVal = textureLoad(t_depth, screenPixel, 0).r;
    let lightVal = textureLoad(t_light, screenPixel, 0).r;
    let semRaw = textureLoad(t_semantic, screenPixel, 0).r;
    let semId = u32(round(semRaw * 255.0));

    // Hard Silhouette Invariance: Void / Black pixels unmodified
    if (semId == 0u || (origColor.r == 0.0 && origColor.g == 0.0 && origColor.b == 0.0)) {
        return origColor;
    }

    // Diagnostics Modes
    if (u.renderMode == 3u) {
        return vec4<f32>(vec3<f32>(depthVal), 1.0);
    } else if (u.renderMode == 4u) {
        return vec4<f32>(lightVal, lightVal * 0.88, lightVal * 0.7, 1.0);
    } else if (u.renderMode == 5u) {
        return vec4<f32>(getSemanticColor(semId), 1.0);
    }

    // Single-tap Head / Facial Region Detection (~top 12px border of character entity)
    let isEntity = (semId == 3u || semId == 4u || semId == 7u);
    var isHeadRegion = false;
    if (isEntity) {
        let semUp12 = u32(round(textureLoad(t_semantic, clamp(screenPixel + vec2<i32>(0, -12), vec2<i32>(0), vec2<i32>(u.resolution) - 1), 0).r * 255.0));
        if (semUp12 != semId) {
            isHeadRegion = true;
        }
    }

    let normal = computeMultiBiomeNormal(screenPixel, semId, depthVal, isHeadRegion);

    if (u.renderMode == 6u) {
        return vec4<f32>(normal * 0.5 + 0.5, 1.0);
    }

    // Dynamic Point Light Radiance with Virtual 3D Z-Elevation
    let pixelPos = vec2<f32>(screenPixel);
    let distToBonfire = length(pixelPos - u.bonfirePos);
    let lightDir = normalize(vec3<f32>(u.bonfirePos.x - pixelPos.x, u.bonfirePos.y - pixelPos.y, 58.0));
    let viewDir = normalize(vec3<f32>(0.0, 0.25, 0.95));
    let halfVec = normalize(lightDir + viewDir);

    let NdotL = max(dot(normal, lightDir), 0.0);
    let NdotV = max(dot(normal, viewDir), 0.0);
    let NdotH = max(dot(normal, halfVec), 0.0);

    // Warm Gothic Color Tone
    var biomeLightTint = vec3<f32>(1.0, 0.64, 0.26); // Fire Amber
    if (u.dungeonBiome == 1u) {
        biomeLightTint = vec3<f32>(1.0, 0.80, 0.42); // Cathedral Gold
    } else if (u.dungeonBiome == 2u) {
        biomeLightTint = vec3<f32>(0.85, 0.38, 0.18); // Catacombs Torch
    } else if (u.dungeonBiome == 3u || u.dungeonBiome == 4u) {
        biomeLightTint = vec3<f32>(1.0, 0.32, 0.10); // Infernal Lava
    } else if (u.dungeonBiome == 5u) {
        biomeLightTint = vec3<f32>(0.42, 0.75, 1.0);  // Crypt Ghostly Blue
    }

    let flicker = 1.0 + (sin(u.time * 8.2) * 0.07 + cos(u.time * 15.8) * 0.04) * u.bonfireFlicker;
    let attenuation = 1.0 / (1.0 + (distToBonfire * 0.0052) + (distToBonfire * distToBonfire * 0.000032));
    let mainRadiance = biomeLightTint * u.lightIntensity * flicker * attenuation;

    // Contact Shadows (Darker ground near walls/characters)
    var contactShadow = 1.0;
    if (semId == 1u) {
        let semAbove = u32(round(textureLoad(t_semantic, clamp(screenPixel + vec2<i32>(0, -2), vec2<i32>(0), vec2<i32>(u.resolution) - 1), 0).r * 255.0));
        if (semAbove >= 2u && semAbove <= 9u) {
            contactShadow = 1.0 - (0.42 * u.contactShadowStrength);
        }
    }

    // Directional Cast Shadows from Point Light (Optimized 2-Tap Trace)
    var directionalShadow = 1.0;
    if (u.qualityTier > 0u && semId == 1u && distToBonfire > 36.0 && distToBonfire < 300.0) {
        let traceDir = normalize(u.bonfirePos - pixelPos);
        let s1 = u32(round(textureLoad(t_semantic, clamp(screenPixel + vec2<i32>(traceDir * 9.0), vec2<i32>(0), vec2<i32>(u.resolution) - 1), 0).r * 255.0));
        let s2 = u32(round(textureLoad(t_semantic, clamp(screenPixel + vec2<i32>(traceDir * 20.0), vec2<i32>(0), vec2<i32>(u.resolution) - 1), 0).r * 255.0));

        var occlude = 0.0;
        if (s1 >= 2u && s1 <= 7u && s1 != 5u) { occlude += 0.55; }
        if (s2 >= 2u && s2 <= 7u && s2 != 5u) { occlude += 0.45; }

        directionalShadow = clamp(1.0 - (occlude * 0.50 * u.contactShadowStrength), 0.40, 1.0);
    }
    let totalGroundShadow = contactShadow * directionalShadow;

    // Material Specular, Cloth Volumetrics & Character Rim Shimmer
    var diffuseFactor = NdotL;
    var specularContribution = vec3<f32>(0.0);
    var emissiveLight = vec3<f32>(0.0);

    // Dynamic Color & Surface Analysis
    let lum = dot(origColor.rgb, vec3<f32>(0.299, 0.587, 0.114));
    let maxC = max(origColor.r, max(origColor.g, origColor.b));
    let minC = min(origColor.r, min(origColor.g, origColor.b));
    let sat = select(0.0, (maxC - minC) / max(maxC, 0.001), maxC > 0.01);
    let rimFactor = pow(clamp(1.0 - NdotV, 0.0, 1.0), 3.2);

    switch (semId) {
        case 1u: {
            // Soil / Mud / Stone Path: Damp ground sheen & micro-relief
            let wetSpec = pow(NdotH, 22.0) * 0.42 * attenuation;
            let pathSpecColor = vec3<f32>(0.92, 0.95, 1.05); // Cool moonlight sheen
            specularContribution = pathSpecColor * wetSpec;
            diffuseFactor = max(NdotL * 0.85 + 0.15, 0.0);
        }
        case 2u, 9u: {
            // Architecture: Stone Masons & Wooden Beams (Roughness 0.85)
            // Enhanced mortar crease shadow and wood grain relief
            diffuseFactor = pow(NdotL, 1.30);
            let stoneGrain = sin(f32(screenPixel.x) * 0.5) * cos(f32(screenPixel.y) * 0.5) * 0.08;
            diffuseFactor = clamp(diffuseFactor + stoneGrain, 0.0, 1.5);
        }
        case 3u, 4u: {
            // Characters (Player Hero 3u & NPCs 4u):
            // Distinct Material Breakdown: Metal Plate vs Gold Trim vs Red Cloth/Tunic vs Skin
            let isSkinTone = (origColor.r > origColor.g && origColor.g > origColor.b && origColor.b > 0.10 && (origColor.r - origColor.b) > 0.08);
            let isGoldMetal = (origColor.r > 0.35 && origColor.g > 0.22 && origColor.b < origColor.g * 0.78 && lum > 0.22);
            let isSteelMetal = (sat < 0.25 && lum > 0.28);
            let isMetal = isGoldMetal || isSteelMetal;
            let isRedCloth = (origColor.r > 0.32 && origColor.g < 0.22 && origColor.b < 0.22);

            if (isHeadRegion && isSkinTone) {
                // Head / Visor / Face: Subsurface warmth and gentle volume contour
                let sssColor = vec3<f32>(1.06, 0.92, 0.82);
                let faceWrap = max(NdotL * 0.60 + 0.40, 0.0);
                diffuseFactor = faceWrap;
                let skinSheen = pow(NdotH, 16.0) * 0.28;
                specularContribution = sssColor * skinSheen * (mainRadiance + vec3<f32>(0.12));
                specularContribution += sssColor * (rimFactor * 0.35 * (mainRadiance + vec3<f32>(0.12)));
                emissiveLight = origColor.rgb * sssColor * 0.06 * attenuation;
            } else if (isMetal) {
                // Metallic Armor / Helmet / Pauldrons / Sword (Roughness 0.22)
                // Crisp high-intensity highlights, metallic specularity and edge rim
                let metalPower = select(64.0, 96.0, isSteelMetal);
                let metalSpec = pow(NdotH, metalPower) * 3.8;
                let metalColor = select(vec3<f32>(1.0, 0.88, 0.52), vec3<f32>(0.94, 0.97, 1.10), isSteelMetal);
                specularContribution = metalColor * (metalSpec * mainRadiance + metalSpec * 0.35);
                specularContribution += metalColor * (rimFactor * 0.70 * (mainRadiance + vec3<f32>(0.22)));
                diffuseFactor = pow(NdotL, 0.90);
            } else if (isRedCloth) {
                // Tunic / Robes / Cloth (Roughness 0.92): Micro-fold shadows and fabric separation
                let foldShadow = max(NdotL * 0.68 + 0.32, 0.0);
                diffuseFactor = foldShadow;
                let clothVelvet = pow(rimFactor, 2.5) * 0.25;
                specularContribution = origColor.rgb * clothVelvet * mainRadiance;
            } else {
                // Leather / Boots / Belt: Satin sheen
                diffuseFactor = max(NdotL * 0.75 + 0.25, 0.0);
                let leatherSpec = pow(NdotH, 14.0) * 0.38;
                specularContribution = origColor.rgb * leatherSpec * mainRadiance;
                specularContribution += origColor.rgb * (rimFactor * 0.38 * (mainRadiance + vec3<f32>(0.10)));
            }
        }
        case 5u: {
            // Liquid River / Water: Specular reflections
            if (u.dungeonBiome == 3u || u.dungeonBiome == 4u) {
                emissiveLight = vec3<f32>(1.0, 0.42, 0.06) * 0.8;
            } else {
                let fresnel = fresnelSchlick(NdotV, 0.02) * u.waterSpecular;
                var ripple = 0.0;
                if (u.qualityTier > 0u) {
                    let pf = vec2<f32>(screenPixel);
                    let r1 = sin(length(pf - vec2<f32>(310.0, 195.0)) * 0.50 - u.time * 4.5);
                    let r2 = cos(length(pf - vec2<f32>(335.0, 210.0)) * 0.60 - u.time * 5.2);
                    ripple = (r1 + r2) * 0.12;
                }
                let liquidSpec = pow(clamp(NdotH + ripple, 0.0, 1.0), 32.0) * 2.8 * u.waterSpecular;
                specularContribution = (vec3<f32>(0.7, 0.85, 1.0) * liquidSpec + vec3<f32>(0.2, 0.4, 0.6) * fresnel) * attenuation;
            }
            diffuseFactor = diffuseFactor * 0.5 + 0.5;
        }
        case 7u: {
            // Monsters & Bosses: Horns, scales, demon flesh
            let isMonsterArmor = (sat < 0.28 && lum > 0.24) || (origColor.r > 0.38 && origColor.g > 0.26 && origColor.b < 0.24);
            if (isMonsterArmor) {
                let hornBoost = select(1.0, 1.35, isHeadRegion);
                let armorSpec = pow(NdotH, 44.0) * (2.8 * hornBoost);
                let armorTint = select(vec3<f32>(0.95, 0.85, 0.6), vec3<f32>(0.88, 0.94, 1.05), sat < 0.22);
                specularContribution = armorTint * (armorSpec * mainRadiance + armorSpec * 0.24);
                specularContribution += vec3<f32>(0.95, 0.75, 0.65) * (rimFactor * 0.60 * (mainRadiance + vec3<f32>(0.18)));
                diffuseFactor = pow(NdotL, 1.05);
            } else {
                diffuseFactor = pow(NdotL, 1.10) * 0.95;
                let skinSpec = pow(NdotH, 18.0) * 0.40;
                specularContribution = vec3<f32>(0.75, 0.45, 0.35) * skinSpec * mainRadiance;
                let rimTint = vec3<f32>(1.0, 0.45, 0.18);
                specularContribution += rimTint * (rimFactor * 0.50 * (mainRadiance + vec3<f32>(0.15)));
                if (isHeadRegion && lum > 0.32 && sat > 0.40) {
                    emissiveLight += origColor.rgb * 0.35;
                }
            }
        }
        case 8u: {
            // Emissive Magic, Orbs, Fire: Pure incandescent radiance
            emissiveLight = origColor.rgb * 1.6;
        }
        default: {}
    }

    // Volumetric Atmospheric Glow & Low Ground Mist
    let volumetricGlow = biomeLightTint * pow(attenuation, 1.3) * 0.32;

    // Composition & Neural Tone Curve
    var ambientBase = max(lightVal, 0.20);
    var ambientTint = vec3<f32>(1.0);
    if (u.dungeonBiome == 0u) {
        ambientBase = max(lightVal * 0.72, 0.14);
        ambientTint = vec3<f32>(0.70, 0.78, 0.95) * 0.72; // Deep, atmospheric gothic night in town
    }

    // Multi-Octave Atmospheric Ground Mist 2.5D
    var groundMist = vec3<f32>(0.0);
    if (u.qualityTier > 0u && (semId == 1u || semId == 0u)) {
        let pWorld = vec2<f32>(screenPixel);
        // Multi-octave wind-driven low mist
        let wind1 = vec2<f32>(u.time * 0.09, u.time * 0.03);
        let wind2 = vec2<f32>(-u.time * 0.05, u.time * 0.07);
        let n1 = sin(pWorld.x * 0.024 + wind1.x) * cos(pWorld.y * 0.028 + wind1.y);
        let n2 = sin(pWorld.x * 0.048 + wind2.x) * cos(pWorld.y * 0.042 + wind2.y) * 0.5;
        let mistNoise = clamp((n1 + n2) * 0.5 + 0.5, 0.0, 1.0);
        
        let fireClear = clamp((distToBonfire - 35.0) / 100.0, 0.1, 1.0);
        let mistFactor = mistNoise * fireClear * (u.mistDensity * 0.70);
        
        // Cold moonlight mist tint in town, blood mist in invasion
        var mistColor = vec3<f32>(0.62, 0.76, 0.94); // Cold Gothic Slate Blue Mist
        if (u.renderMode == 7u) {
            mistColor = vec3<f32>(0.92, 0.30, 0.20); // Crimson invasion mist
        } else if (u.dungeonBiome == 5u) {
            mistColor = vec3<f32>(0.40, 0.75, 0.85); // Crypt ethereal mist
        }
        groundMist = mistColor * mistFactor * (ambientBase + 0.25);
    }

    let dynamicDiffuse = mainRadiance * diffuseFactor * 1.25;
    let totalLight = (ambientTint * ambientBase) + dynamicDiffuse + volumetricGlow;

    var enhanced = (origColor.rgb * totalLight * totalGroundShadow) + (specularContribution * 1.35) + emissiveLight + groundMist;

    // Cinematic Color Grading & Split Toning (Cold Shadows / Warm Golden Highlights)
    let lumVal = dot(enhanced, vec3<f32>(0.299, 0.587, 0.114));
    let shadowTone = vec3<f32>(0.74, 0.86, 1.18); // Cold Gothic Slate in deep shadows
    let highlightTone = vec3<f32>(1.22, 0.98, 0.76); // Warm Amber / Torch Gold in lights
    let splitTone = mix(shadowTone, highlightTone, smoothstep(0.12, 0.68, lumVal));
    enhanced = enhanced * splitTone;

    // S-Curve Contrast Enhancement (Rich, Deep Inky Blacks)
    let contrastS = enhanced * enhanced * (3.0 - 2.0 * clamp(enhanced, vec3<f32>(0.0), vec3<f32>(1.0)));
    enhanced = mix(enhanced, contrastS, 0.56);

    // Subtle Vignette in Shader
    let vigCoord = (in.uv - vec2<f32>(0.5)) * vec2<f32>(1.15, 0.96);
    let vig = 1.0 - smoothstep(0.35, 0.94, length(vigCoord)) * 0.38;
    enhanced = enhanced * vig;

    // Silhouette Invariance Masking
    enhanced = mix(origColor.rgb, enhanced, origColor.a);

    // Modes & Presets
    if (u.renderMode == 0u) {
        return origColor; // Original 1996 Pure
    } else if (u.renderMode == 1u || u.renderMode == 7u) {
        return vec4<f32>(enhanced, origColor.a); // 1: Cinematic Grimdark, 7: La Caída de Tristram
    } else if (u.renderMode == 2u) {
        if (in.uv.x < u.splitPos) {
            return origColor;
        } else if (abs(in.uv.x - u.splitPos) < 0.002) {
            return vec4<f32>(1.0, 0.84, 0.22, 1.0);
        } else {
            return vec4<f32>(enhanced, origColor.a);
        }
    }

    return vec4<f32>(enhanced, origColor.a);
}
