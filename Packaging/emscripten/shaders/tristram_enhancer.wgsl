// NIGHTMARE Neural HD — Advanced Neural Reconstruction & Multi-Biome WebGPU Shader (WGSL)
// Features: Subpixel Edge Reconstruction (DLSS-Style), Volumetric Light Scattering, 2.5D Pseudo-Normals, Wet Surface Shimmer, Material Physiology & Silhouette Invariance

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
};

@group(0) @binding(0) var<uniform> u: Uniforms;
@group(0) @binding(1) var s_sampler: sampler;
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

// Subpixel Edge Reconstruction & Legacy Rain Eradicator
fn sampleReconstructedRGB(coords: vec2<i32>) -> vec4<f32> {
    let dim = vec2<i32>(u.resolution);
    let c = textureLoad(t_rgb, clamp(coords, vec2<i32>(0), dim - 1), 0);
    let cL = textureLoad(t_rgb, clamp(coords + vec2<i32>(-1, 0), vec2<i32>(0), dim - 1), 0);
    let cR = textureLoad(t_rgb, clamp(coords + vec2<i32>(1, 0), vec2<i32>(0), dim - 1), 0);
    let cU = textureLoad(t_rgb, clamp(coords + vec2<i32>(0, -1), vec2<i32>(0), dim - 1), 0);
    let cD = textureLoad(t_rgb, clamp(coords + vec2<i32>(0, 1), vec2<i32>(0), dim - 1), 0);

    let lC = dot(c.rgb, vec3<f32>(0.299, 0.587, 0.114));
    let lL = dot(cL.rgb, vec3<f32>(0.299, 0.587, 0.114));
    let lR = dot(cR.rgb, vec3<f32>(0.299, 0.587, 0.114));
    let lU = dot(cU.rgb, vec3<f32>(0.299, 0.587, 0.114));
    let lD = dot(cD.rgb, vec3<f32>(0.299, 0.587, 0.114));

    // Legacy C++ Rain Eradicator: The old rain draws 1-pixel wide vertical dark streaks.
    // If the current pixel is significantly darker than both left and right neighbors,
    // AND forms a vertical line (similar luminance to above/below), we erase it by blending!
    if (u.dungeonBiome == 0u) {
        if (lC < lL - 0.04 && lC < lR - 0.04 && abs(lC - lU) < 0.05 && abs(lC - lD) < 0.05) {
            // It's an old rain streak! Erase it with horizontal median interpolation
            return mix(c, (cL + cR) * 0.5, 0.85);
        }
    }

    // Standard Subpixel Edge Smoothing (DLSS-style)
    let edgeH = abs(lL - lR);
    let edgeV = abs(lU - lD);

    if (max(edgeH, edgeV) > 0.08) {
        let blend = (c + cL + cR + cU + cD) * 0.2;
        return mix(c, blend, 0.42);
    }
    return c;
}

// Compute 2.5D Pseudo-Normals with Surface Relief
fn computeMultiBiomeNormal(coords: vec2<i32>, semId: u32, depthVal: f32) -> vec3<f32> {
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
            // Micro-relief extracted from sprite luminance gradients (clothing folds, armor plates, muscles)
            let cL = textureLoad(t_rgb, clamp(coords + vec2<i32>(-1, 0), vec2<i32>(0), texDim - 1), 0).rgb;
            let cR = textureLoad(t_rgb, clamp(coords + vec2<i32>( 1, 0), vec2<i32>(0), texDim - 1), 0).rgb;
            let cU = textureLoad(t_rgb, clamp(coords + vec2<i32>(0, -1), vec2<i32>(0), texDim - 1), 0).rgb;
            let cD = textureLoad(t_rgb, clamp(coords + vec2<i32>(0,  1), vec2<i32>(0), texDim - 1), 0).rgb;
            let lumL = dot(cL, vec3<f32>(0.299, 0.587, 0.114));
            let lumR = dot(cR, vec3<f32>(0.299, 0.587, 0.114));
            let lumU = dot(cU, vec3<f32>(0.299, 0.587, 0.114));
            let lumD = dot(cD, vec3<f32>(0.299, 0.587, 0.114));
            let spriteGradX = (lumR - lumL) * 3.2;
            let spriteGradY = (lumD - lumU) * 3.2;

            // Head / Cranial curvature detection (~top 14px of character entity)
            let semUp12 = u32(round(textureLoad(t_semantic, clamp(coords + vec2<i32>(0, -12), vec2<i32>(0), texDim - 1), 0).r * 255.0));
            let semDn12 = u32(round(textureLoad(t_semantic, clamp(coords + vec2<i32>(0, 12), vec2<i32>(0), texDim - 1), 0).r * 255.0));
            var headBowing = vec2<f32>(0.0, 0.0);
            if (semUp12 != semId && semDn12 == semId) {
                // Spherical dome curvature around the skull / helmet / face
                headBowing = vec2<f32>(spriteGradX * 0.35, -0.22);
            }

            n = normalize(vec3<f32>(-dz_dx * 1.6 - spriteGradX + headBowing.x, -dz_dy * 1.6 - spriteGradY + headBowing.y, 0.70));
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

    let normal = computeMultiBiomeNormal(screenPixel, semId, depthVal);

    if (u.renderMode == 6u) {
        return vec4<f32>(normal * 0.5 + 0.5, 1.0);
    }

    // Dynamic Point Light Radiance
    let pixelPos = vec2<f32>(screenPixel);
    let distToBonfire = length(pixelPos - u.bonfirePos);
    let lightDir = normalize(vec3<f32>(u.bonfirePos.x - pixelPos.x, u.bonfirePos.y - pixelPos.y, 44.0));
    let viewDir = normalize(vec3<f32>(0.0, 0.6, 0.8));
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

    // Material Specular & Wetness Shimmer
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

    // Head / Facial Region Detection (~top 14px of character entity)
    let isEntity = (semId == 3u || semId == 4u || semId == 7u);
    var isHeadRegion = false;
    if (isEntity) {
        let semUp14 = u32(round(textureLoad(t_semantic, clamp(screenPixel + vec2<i32>(0, -14), vec2<i32>(0), vec2<i32>(u.resolution) - 1), 0).r * 255.0));
        let semDn14 = u32(round(textureLoad(t_semantic, clamp(screenPixel + vec2<i32>(0, 14), vec2<i32>(0), vec2<i32>(u.resolution) - 1), 0).r * 255.0));
        if (semUp14 != semId && semDn14 == semId) {
            isHeadRegion = true;
        }
    }

    switch (semId) {
        case 1u: {
            // Wet ground shimmer in Town
            let wetSpec = pow(NdotH, 18.0) * 0.35 * attenuation;
            specularContribution = vec3<f32>(1.0, 0.85, 0.6) * wetSpec;
            diffuseFactor = max(NdotL * 0.82 + 0.18, 0.0);
        }
        case 2u, 9u: {
            diffuseFactor = pow(NdotL, 1.22);
        }
        case 3u, 4u: {
            // Player Hero (3u) & Town NPCs (4u):
            // Differentiate Skin (Face/Beard) vs Metal (Helmet/Armor) vs Cloth (Hood/Robe)
            let isSkinTone = (origColor.r > origColor.g && origColor.g > origColor.b && origColor.b > 0.12 && (origColor.r - origColor.b) > 0.09);
            let isGoldMetal = (origColor.r > 0.38 && origColor.g > 0.26 && origColor.b < origColor.g * 0.82 && lum > 0.25);
            let isSteelMetal = (sat < 0.28 && lum > 0.30);
            let isMetal = isGoldMetal || isSteelMetal;

            if (isHeadRegion && isSkinTone) {
                // 10% Realism: Subsurface Scattering & Soft Facial Contouring
                // Human skin gently diffuses firelight with a warm micro-subsurface glow
                let sssColor = vec3<f32>(1.08, 0.94, 0.84);
                let faceWrap = max(NdotL * 0.65 + 0.35, 0.0);
                diffuseFactor = faceWrap;
                // Soft skin micro-sheen (forehead/cheek highlight)
                let skinSheen = pow(NdotH, 18.0) * 0.32;
                specularContribution = sssColor * skinSheen * (mainRadiance + vec3<f32>(0.10));
                // Soft rim-lighting outlining the face/jawline
                specularContribution += sssColor * (rimFactor * 0.32 * (mainRadiance + vec3<f32>(0.10)));
                // Subtle warmth infusion
                emissiveLight = origColor.rgb * sssColor * 0.08 * attenuation;
            } else if (isMetal) {
                // High-reflectance metallic armor / helmet / shield / sword
                let metalPower = select(36.0, 48.0, isSteelMetal);
                // On the helmet crown, focus an apex specular glint (cranial curvature)
                let helmetBoost = select(1.0, 1.25, isHeadRegion);
                let metalSpec = pow(NdotH, metalPower) * (2.8 * helmetBoost);
                let metalColor = select(vec3<f32>(1.0, 0.84, 0.46), vec3<f32>(0.92, 0.96, 1.0), isSteelMetal);
                // Directional point light glint + ambient sheen
                specularContribution = metalColor * (metalSpec * mainRadiance + metalSpec * 0.25);
                // Crisp rim lighting along armor bevels, visor slit, and helmet crest
                specularContribution += metalColor * (rimFactor * 0.60 * (mainRadiance + vec3<f32>(0.18)));
                diffuseFactor = pow(NdotL, 0.95);
            } else {
                // Cloth / Robes / Leather / Tunic / Cowl / Hair:
                // Soft fabric wrap-lighting with velvet micro-sheen on folds and hood
                diffuseFactor = max(NdotL * 0.72 + 0.28, 0.0);
                let clothSpec = pow(NdotH, 12.0) * 0.40;
                specularContribution = origColor.rgb * clothSpec * mainRadiance;
                // Soft rim-light along fabric folds and cloak silhouettes
                specularContribution += origColor.rgb * (rimFactor * 0.40 * (mainRadiance + vec3<f32>(0.12)));
            }
        }
        case 5u: {
            // Liquid Water / Molten Lava
            if (u.dungeonBiome == 3u || u.dungeonBiome == 4u) {
                emissiveLight = vec3<f32>(1.0, 0.42, 0.06) * 0.8;
            } else {
                let fresnel = fresnelSchlick(NdotV, 0.02) * u.waterSpecular;
                let liquidSpec = pow(NdotH, 30.0) * 2.4 * u.waterSpecular;
                specularContribution = (vec3<f32>(0.7, 0.85, 1.0) * liquidSpec + vec3<f32>(0.2, 0.4, 0.6) * fresnel) * attenuation;
            }
            diffuseFactor = diffuseFactor * 0.5 + 0.5;
        }
        case 7u: {
            // Monsters & Bosses (Enemies):
            // Armored demons (Knights), Skeletons (Bone/Plate), Overlords (Leathery hide/horns)
            let isMonsterArmor = (sat < 0.30 && lum > 0.26) || (origColor.r > 0.40 && origColor.g > 0.28 && origColor.b < 0.25);
            if (isMonsterArmor) {
                // Sharp sinister gleam on enemy armor, shields, weapons, and cranial horns/bone
                let hornBoost = select(1.0, 1.30, isHeadRegion);
                let armorSpec = pow(NdotH, 44.0) * (2.6 * hornBoost);
                let armorTint = select(vec3<f32>(0.95, 0.85, 0.6), vec3<f32>(0.85, 0.92, 1.0), sat < 0.22);
                specularContribution = armorTint * (armorSpec * mainRadiance + armorSpec * 0.22);
                // Edge rim-light to cut through the dungeon dark
                specularContribution += vec3<f32>(0.95, 0.75, 0.65) * (rimFactor * 0.58 * (mainRadiance + vec3<f32>(0.16)));
                diffuseFactor = pow(NdotL, 1.05);
            } else {
                // Demonic flesh / scales / dark hides
                diffuseFactor = pow(NdotL, 1.12) * 0.95;
                let skinSpec = pow(NdotH, 16.0) * 0.38;
                specularContribution = vec3<f32>(0.75, 0.45, 0.35) * skinSpec * mainRadiance;
                // Eerie red-amber rim light on demonic silhouettes
                let rimTint = vec3<f32>(1.0, 0.45, 0.18);
                specularContribution += rimTint * (rimFactor * 0.48 * (mainRadiance + vec3<f32>(0.14)));
                // 10% Realism: In head region, give demonic eye sockets a subtle menacing ember
                if (isHeadRegion && lum > 0.32 && sat > 0.40) {
                    emissiveLight += origColor.rgb * 0.30;
                }
            }
        }
        case 8u: {
            emissiveLight = origColor.rgb * 1.5;
        }
        default: {}
    }

    // Volumetric Atmospheric Glow (subtle scattering near fire)
    let volumetricGlow = biomeLightTint * pow(attenuation, 1.5) * 0.15;

    // Composition & Neural Tone Curve
    let ambientBase = max(lightVal, 0.24);
    let dynamicDiffuse = mainRadiance * diffuseFactor;
    let totalLight = vec3<f32>(ambientBase) + dynamicDiffuse + volumetricGlow;

    var enhanced = (origColor.rgb * totalLight * contactShadow) + specularContribution + emissiveLight;

    // Rich Shadow Depth & Highlight Saturation Curve
    enhanced = pow(enhanced, vec3<f32>(0.96)); // Tone curve lift

    // Silhouette Invariance Masking
    enhanced = mix(origColor.rgb, enhanced, origColor.a);

    // Modes
    if (u.renderMode == 0u) {
        return origColor;
    } else if (u.renderMode == 1u) {
        return vec4<f32>(enhanced, origColor.a);
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
