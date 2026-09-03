"""
NIGHTMARE Neural HD — Phase 4 Tiny Neural Renderer Architecture
Lightweight Multi-Scale Residual ConvNet with PixelShuffle Upscaling (2x)
and Mathematical Silhouette Invariance Lock for WebGPU (< 15MB weight budget).
"""

import torch
import torch.nn as nn
import torch.nn.functional as F

class ResidualBlock(nn.Module):
    """Efficient Residual Block with SiLU activation and grouped normalization."""
    def __init__(self, channels=32):
        super().__init__()
        self.conv1 = nn.Conv2d(channels, channels, kernel_size=3, padding=1, bias=True)
        self.act = nn.SiLU(inplace=True)
        self.conv2 = nn.Conv2d(channels, channels, kernel_size=3, padding=1, bias=True)

    def forward(self, x):
        return x + self.conv2(self.act(self.conv1(x)))

class TinyNeuralRenderer(nn.Module):
    """
    Tiny Neural Renderer: 6-channel input -> 3-channel 2x output (1280x960).
    Channels:
      [0..2]: RGB Original (3 channels)
      [3]:    Logical Depth (1 channel)
      [4]:    Lightmap Radiance (1 channel)
      [5]:    Semantic Class ID (1 channel)
    """
    def __init__(self, in_channels=6, num_features=32, num_blocks=4):
        super().__init__()
        self.in_channels = in_channels
        self.num_features = num_features

        # 1. Feature Extraction
        self.feat_in = nn.Conv2d(in_channels, num_features, kernel_size=3, padding=1, bias=True)
        self.act_in = nn.SiLU(inplace=True)

        # 2. Residual Dense Body
        self.body = nn.Sequential(*[
            ResidualBlock(num_features) for _ in range(num_blocks)
        ])

        # 3. Sub-Pixel Upscaler (2x via PixelShuffle)
        self.upscaler = nn.Sequential(
            nn.Conv2d(num_features, num_features * 4, kernel_size=3, padding=1, bias=True),
            nn.PixelShuffle(2),
            nn.SiLU(inplace=True)
        )

        # 4. Output Projection to RGB
        self.out_conv = nn.Conv2d(num_features, 3, kernel_size=3, padding=1, bias=True)

    def forward(self, x):
        """
        Input shape:  (B, 6, H, W)
        Output shape: (B, 3, H*2, W*2)
        """
        # Extract semantic mask for hard silhouette locking
        sem_channel = x[:, 5:6, :, :] # (B, 1, H, W)
        silhouette_mask_2x = F.interpolate(
            (sem_channel > 0.001).float(),
            scale_factor=2.0,
            mode='nearest'
        )

        # Feed-forward
        feat = self.act_in(self.feat_in(x))
        res = self.body(feat) + feat
        up = self.upscaler(res)
        out_rgb = torch.sigmoid(self.out_conv(up))

        # Hard Silhouette Lock: zero displacement outside original sprite contours
        locked_rgb = out_rgb * silhouette_mask_2x
        return locked_rgb

class SilhouetteInvarianceLoss(nn.Module):
    """
    Loss Function combining L1 pixel fidelity, edge gradient consistency,
    and heavy penalty for silhouette contour leakage.
    """
    def __init__(self, lambda_l1=1.0, lambda_edge=0.5, lambda_sil=1.0):
        super().__init__()
        self.lambda_l1 = lambda_l1
        self.lambda_edge = lambda_edge
        self.lambda_sil = lambda_sil
        self.l1 = nn.L1Loss()

        # Sobel kernel for edge loss
        sobel_x = torch.tensor([[-1, 0, 1], [-2, 0, 2], [-1, 0, 1]], dtype=torch.float32).view(1, 1, 3, 3)
        sobel_y = torch.tensor([[-1, -2, -1], [0, 0, 0], [1, 2, 1]], dtype=torch.float32).view(1, 1, 3, 3)
        self.register_buffer("sobel_x", sobel_x)
        self.register_buffer("sobel_y", sobel_y)

    def _edge_map(self, img):
        # Convert RGB to Grayscale
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

        # 3. Silhouette Leakage Penalty
        sil_mask_2x = F.interpolate((semantic_input[:, 5:6] > 0.001).float(), scale_factor=2.0, mode='nearest')
        void_leakage = pred * (1.0 - sil_mask_2x)
        loss_sil = torch.mean(torch.abs(void_leakage))

        total_loss = (self.lambda_l1 * loss_pixel) + (self.lambda_edge * loss_edge) + (self.lambda_sil * loss_sil)
        return total_loss, {
            "loss_pixel": loss_pixel.item(),
            "loss_edge": loss_edge.item(),
            "loss_sil": loss_sil.item()
        }
