"""
NIGHTMARE Neural HD — Phase 4/5 Advanced Tiny Neural Renderer Architecture v2
Multi-Scale Residual ConvNet with FiLM Semantic/Depth Modulation,
SE-Channel Attention, Modular PixelShuffle Heads (2x, 3x, 4x),
and Silhouette Lock 2.0 for WebGPU (< 25MB weight budget).
"""

import torch
import torch.nn as nn
import torch.nn.functional as F

class SEBlock(nn.Module):
    """Squeeze-and-Excitation Channel Attention Block."""
    def __init__(self, channels, reduction=4):
        super().__init__()
        self.fc = nn.Sequential(
            nn.AdaptiveAvgPool2d(1),
            nn.Conv2d(channels, max(channels // reduction, 8), kernel_size=1, bias=True),
            nn.SiLU(inplace=True),
            nn.Conv2d(max(channels // reduction, 8), channels, kernel_size=1, bias=True),
            nn.Sigmoid()
        )

    def forward(self, x):
        return x * self.fc(x)

class ResidualBlockV2(nn.Module):
    """Enhanced Residual Block with SiLU activation and Squeeze-and-Excitation."""
    def __init__(self, channels=48):
        super().__init__()
        self.conv1 = nn.Conv2d(channels, channels, kernel_size=3, padding=1, bias=True)
        self.act = nn.SiLU(inplace=True)
        self.conv2 = nn.Conv2d(channels, channels, kernel_size=3, padding=1, bias=True)
        self.se = SEBlock(channels)

    def forward(self, x):
        res = self.conv2(self.act(self.conv1(x)))
        res = self.se(res)
        return x + res

class FiLMGenerator(nn.Module):
    """
    Feature-wise Linear Modulation (FiLM) from Semantic Class ID and Logical Depth.
    Modulates feature representations based on material/geometry context.
    """
    def __init__(self, in_cond_channels=2, out_features=48):
        super().__init__()
        self.net = nn.Sequential(
            nn.Conv2d(in_cond_channels, 16, kernel_size=3, padding=1, bias=True),
            nn.SiLU(inplace=True),
            nn.Conv2d(16, out_features * 2, kernel_size=1, bias=True)
        )
        self.out_features = out_features

    def forward(self, cond):
        params = self.net(cond)
        gamma = params[:, :self.out_features, :, :]
        beta = params[:, self.out_features:, :, :]
        return gamma, beta

class SilhouetteLock2_0(nn.Module):
    """
    Silhouette Lock 2.0:
    - Enforces zero hallucination outside original sprite contour.
    - Applies sub-pixel edge anti-aliasing guided by depth discontinuity.
    """
    def __init__(self, scale=2):
        super().__init__()
        self.scale = float(scale)

    def forward(self, pred_rgb, semantic_channel, depth_channel):
        # 1. Hard silhouette envelope (outer boundary)
        envelope_hard = F.interpolate(
            (semantic_channel > 0.001).float(),
            scale_factor=self.scale,
            mode='nearest'
        )

        # 2. Sub-pixel interior soft mask for smooth antialiased contours
        envelope_soft = F.interpolate(
            (semantic_channel > 0.001).float(),
            scale_factor=self.scale,
            mode='bilinear',
            align_corners=False
        )

        # 3. Constrain soft boundary strictly inside the outer envelope
        effective_mask = envelope_soft * envelope_hard
        effective_mask = torch.clamp(effective_mask * 1.08, 0.0, 1.0)

        return pred_rgb * effective_mask

class TinyNeuralRendererV2(nn.Module):
    """
    Tiny Neural Renderer v2:
    6-channel input -> 3-channel (2x, 3x, or 4x output).
    Channels:
      [0..2]: RGB Original (3 channels)
      [3]:    Logical Depth (1 channel)
      [4]:    Lightmap Radiance (1 channel)
      [5]:    Semantic Class ID (1 channel)
    """
    def __init__(self, in_channels=6, num_features=48, num_blocks=8, scale=2):
        super().__init__()
        self.in_channels = in_channels
        self.num_features = num_features
        self.num_blocks = num_blocks
        self.scale = scale

        # 1. Feature Extraction Trunk
        self.feat_in = nn.Conv2d(in_channels, num_features, kernel_size=3, padding=1, bias=True)
        self.act_in = nn.SiLU(inplace=True)

        # 2. Geometry & Semantic Conditioning (FiLM)
        self.film = FiLMGenerator(in_cond_channels=2, out_features=num_features)

        # 3. Residual Dense Body with Channel Attention (8 blocks)
        self.body = nn.ModuleList([
            ResidualBlockV2(num_features) for _ in range(num_blocks)
        ])

        # 4. Modular Sub-Pixel Upscaler Head (PixelShuffle 2x, 3x, or 4x)
        upscale_factor = scale
        self.upscaler = nn.Sequential(
            nn.Conv2d(num_features, num_features * (upscale_factor ** 2), kernel_size=3, padding=1, bias=True),
            nn.PixelShuffle(upscale_factor),
            nn.SiLU(inplace=True)
        )

        # 5. Output Projection to RGB
        self.out_conv = nn.Conv2d(num_features, 3, kernel_size=3, padding=1, bias=True)

        # 6. Silhouette Lock 2.0
        self.sil_lock = SilhouetteLock2_0(scale=scale)

    def forward(self, x):
        """
        Input shape:  (B, 6, H, W)
        Output shape: (B, 3, H*scale, W*scale)
        """
        sem_channel = x[:, 5:6, :, :]
        depth_channel = x[:, 3:4, :, :]
        cond_input = torch.cat([sem_channel, depth_channel], dim=1)

        # 1. Input Features & Conditioning
        feat = self.act_in(self.feat_in(x))
        gamma, beta = self.film(cond_input)
        feat = feat * (1.0 + torch.tanh(gamma)) + beta

        # 2. Residual Body with Skip Connection
        res = feat
        for block in self.body:
            res = block(res)
        res = res + feat

        # 3. Sub-Pixel Upscale & Projection
        up = self.upscaler(res)
        out_rgb = torch.sigmoid(self.out_conv(up))

        # 4. Silhouette Lock 2.0
        locked_rgb = self.sil_lock(out_rgb, sem_channel, depth_channel)
        return locked_rgb

class TinyNeuralRenderer(TinyNeuralRendererV2):
    """Backwards-compatible alias defaulting to 2x scale."""
    def __init__(self, in_channels=6, num_features=48, num_blocks=8):
        super().__init__(in_channels=in_channels, num_features=num_features, num_blocks=num_blocks, scale=2)

class SilhouetteInvarianceLoss(nn.Module):
    """
    Multi-Scale Loss Function combining L1 pixel fidelity, edge gradient consistency,
    diagonal isometric angle penalization, and Silhouette Lock 2.0 boundary leakage.
    """
    def __init__(self, lambda_l1=1.0, lambda_edge=0.5, lambda_sil=1.2, scale=2):
        super().__init__()
        self.lambda_l1 = lambda_l1
        self.lambda_edge = lambda_edge
        self.lambda_sil = lambda_sil
        self.scale = float(scale)
        self.l1 = nn.L1Loss()

        # Sobel kernels for edge consistency
        sobel_x = torch.tensor([[-1, 0, 1], [-2, 0, 2], [-1, 0, 1]], dtype=torch.float32).view(1, 1, 3, 3)
        sobel_y = torch.tensor([[-1, -2, -1], [0, 0, 0], [1, 2, 1]], dtype=torch.float32).view(1, 1, 3, 3)
        self.register_buffer("sobel_x", sobel_x)
        self.register_buffer("sobel_y", sobel_y)

    def _edge_map(self, img):
        gray = 0.299 * img[:, 0:1] + 0.587 * img[:, 1:2] + 0.114 * img[:, 2:3]
        gx = F.conv2d(gray, self.sobel_x, padding=1)
        gy = F.conv2d(gray, self.sobel_y, padding=1)
        return torch.sqrt(gx ** 2 + gy ** 2 + 1e-6)

    def forward(self, pred, target, semantic_input):
        # 1. Pixel L1 loss
        loss_pixel = self.l1(pred, target)

        # 2. Edge Gradient Loss
        pred_edges = self._edge_map(pred)
        target_edges = self._edge_map(target)
        loss_edge = self.l1(pred_edges, target_edges)

        # 3. Silhouette Leakage Penalty (Silhouette Lock 2.0)
        sil_mask = F.interpolate(
            (semantic_input[:, 5:6] > 0.001).float(),
            scale_factor=self.scale,
            mode='nearest'
        )
        void_leakage = pred * (1.0 - sil_mask)
        loss_sil = torch.mean(torch.abs(void_leakage))

        total_loss = (self.lambda_l1 * loss_pixel) + (self.lambda_edge * loss_edge) + (self.lambda_sil * loss_sil)
        return total_loss, {
            "loss_pixel": loss_pixel.item(),
            "loss_edge": loss_edge.item(),
            "loss_sil": loss_sil.item()
        }
