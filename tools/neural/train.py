"""
NIGHTMARE Neural HD — Phase 4 Training Pipeline
Supervised training of TinyNeuralRenderer using Tristram dataset pairs.
"""

import os
import glob
import argparse
import numpy as np
import torch
from torch.utils.data import Dataset, DataLoader

from model import TinyNeuralRenderer, SilhouetteInvarianceLoss

class TristramDataset(Dataset):
    """Loads 6-channel input tuples and 2x target images."""
    def __init__(self, input_dir="dataset_tristram", target_dir="dataset_tristram_2x", width=640, height=480):
        self.input_dir = input_dir
        self.target_dir = target_dir
        self.width = width
        self.height = height
        
        self.frame_dirs = sorted(glob.glob(os.path.join(input_dir, "frame_*")))
        
    def __len__(self):
        return max(len(self.frame_dirs), 1)

    def __getitem__(self, idx):
        if not self.frame_dirs:
            # Synthetic fallback sample for dummy validation
            inp = torch.zeros((6, self.height, self.width), dtype=torch.float32)
            tgt = torch.zeros((3, self.height * 2, self.width * 2), dtype=torch.float32)
            return inp, tgt

        f_dir = self.frame_dirs[idx % len(self.frame_dirs)]
        f_name = os.path.basename(f_dir)
        
        # Load 4 input channels
        rgb_raw = np.fromfile(os.path.join(f_dir, "rgb.raw"), dtype=np.uint8).reshape((self.height, self.width, 4))[:, :, :3]
        depth_raw = np.fromfile(os.path.join(f_dir, "depth.raw"), dtype=np.float32).reshape((self.height, self.width))
        light_raw = np.fromfile(os.path.join(f_dir, "light.raw"), dtype=np.uint8).reshape((self.height, self.width))
        sem_raw = np.fromfile(os.path.join(f_dir, "semantic.raw"), dtype=np.uint8).reshape((self.height, self.width))
        
        # Stack into 6-channel tensor
        rgb = np.transpose(rgb_raw.astype(np.float32) / 255.0, (2, 0, 1)) # (3, H, W)
        depth = depth_raw[np.newaxis, :, :] # (1, H, W)
        light = (light_raw.astype(np.float32) / 255.0)[np.newaxis, :, :] # (1, H, W)
        sem = (sem_raw.astype(np.float32) / 255.0)[np.newaxis, :, :] # (1, H, W)
        
        inp_tensor = np.concatenate([rgb, depth, light, sem], axis=0) # (6, H, W)
        
        # Load 2x target
        target_path = os.path.join(self.target_dir, f_name, "target_2x.raw")
        if os.path.exists(target_path):
            tgt_raw = np.fromfile(target_path, dtype=np.uint8).reshape((self.height * 2, self.width * 2, 3))
            tgt_tensor = np.transpose(tgt_raw.astype(np.float32) / 255.0, (2, 0, 1))
        else:
            tgt_tensor = np.repeat(np.repeat(rgb, 2, axis=1), 2, axis=2)
            
        return torch.from_numpy(inp_tensor), torch.from_numpy(tgt_tensor)

def train_model(epochs=10, batch_size=4, lr=1e-3, output_path="nightmare_neural_hd.pth"):
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"[NIGHTMARE Train] Initializing training on device: {device}")

    dataset = TristramDataset()
    dataloader = DataLoader(dataset, batch_size=batch_size, shuffle=True)

    model = TinyNeuralRenderer().to(device)
    criterion = SilhouetteInvarianceLoss().to(device)
    optimizer = torch.optim.AdamW(model.parameters(), lr=lr, weight_decay=1e-4)

    model.train()
    for epoch in range(epochs):
        epoch_loss = 0.0
        for i, (inp, tgt) in enumerate(dataloader):
            inp = inp.to(device)
            tgt = tgt.to(device)

            optimizer.zero_grad()
            pred = model(inp)
            loss, metrics = criterion(pred, tgt, inp)
            loss.backward()
            optimizer.step()

            epoch_loss += loss.item()

        avg_loss = epoch_loss / max(len(dataloader), 1)
        print(f"[Epoch {epoch+1}/{epochs}] Loss: {avg_loss:.5f} | Pixel: {metrics['loss_pixel']:.4f} | Edge: {metrics['loss_edge']:.4f} | Sil: {metrics['loss_sil']:.4f}")

    torch.save(model.state_dict(), output_path)
    print(f"[NIGHTMARE Train] Model saved to {output_path}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Train NIGHTMARE TinyNeuralRenderer")
    parser.add_argument("--epochs", type=int, default=5)
    parser.add_argument("--batch-size", type=int, default=2)
    parser.add_argument("--lr", type=float, default=1e-3)
    parser.add_argument("--out", default="nightmare_neural_hd.pth")
    args = parser.parse_args()

    train_model(epochs=args.epochs, batch_size=args.batch_size, lr=args.lr, output_path=args.out)
