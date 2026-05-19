import os
import shutil
import json
import argparse
from pathlib import Path
from sklearn.model_selection import train_test_split

class TuSimpleConverter:
	def __init__(self, tusimple_dir, output_dir, val_size=0.2, random_state=42):
		self.tusimple_dir = tusimple_dir
		self.output_dir = output_dir
		self.target_width = 1280
		self.target_height = 864
		self.val_size = val_size
		self.random_state = random_state
		self.val_clips = set()

	def create_yaml(self):
		yaml_content = "\n".join([
            f"path: {self.output_dir}",
            "train: images/train",
            "val: images/val",
            "nc: 1",
            "names: ['lane']",
		]) + "\n"
		with open(os.path.join(self.output_dir, "data.yaml"), 'w', encoding='utf-8') as yaml_file:
			yaml_file.write(yaml_content)

	def split_clips(self, data):
		"""Extract unique clips and split them into train/val by clip (not by frame)."""
		# A clip is identified by (sequence, clip_id), e.g. ("0313-1", "0")
		clips = set()
		for img in data:
			parts = Path(img['raw_file']).parts
			clip_id = (parts[1], parts[2])  # e.g. ("0313-1", "0")
			clips.add(clip_id)

		clips = sorted(clips)
		train_clips, val_clips = train_test_split(clips, test_size=self.val_size, random_state=self.random_state)
		print(f"  Train clips: {len(train_clips)}, Val clips: {len(val_clips)}")
		return set(val_clips)

	def get_split(self, parts):
		"""Return 'val' if this frame belongs to a val clip, else 'train'."""
		clip_id = (parts[1], parts[2])
		return "val" if clip_id in self.val_clips else "train"

	def create_txt(self, img, parts, split):
		safe_name = f"{parts[1]}_{parts[2]}_{parts[3].replace('.jpg', '.txt')}"
		new_img_txt = os.path.join(self.output_dir, "labels", split, safe_name)
		os.makedirs(os.path.dirname(new_img_txt), exist_ok=True)

		h_heights = img['h_samples']
		lanes = img['lanes']
		with open(new_img_txt, 'w') as label_file:
			for lane in lanes:
				valid_points = [
					(x, y) for x, y in zip(lane, h_heights) if x != -2
				]
				if len(valid_points) < 2:
					continue  # skip invalid lanes
				coords = " ".join(
					f"{x / self.target_width:.6f} {y / self.target_height:.6f}"
					for x, y in valid_points
				)
				label_file.write(f"0 {coords}\n")

	def convert_data(self, data):
		for img in data:
			print("Processing image:", img['raw_file'])
			raw_file = img['raw_file']
			src = os.path.join(self.tusimple_dir, "train_set", raw_file)
			parts = Path(raw_file).parts

			split = self.get_split(parts)

			# Copy image into correct split folder
			safe_img_name = f"{parts[1]}_{parts[2]}_{parts[3]}"
			dest = os.path.join(self.output_dir, "images", split, safe_img_name)
			os.makedirs(os.path.dirname(dest), exist_ok=True)
			shutil.copy(src, dest)

			self.create_txt(img, parts, split)

	def run(self):
		train_dir = os.path.join(self.tusimple_dir, "train_set")
		file_list = list(os.listdir(train_dir))
		json_files = [f for f in file_list if f.endswith('.json')]
		print("Found JSON files in", train_dir, ":", json_files)

		# Load all data first so we can split clips across all JSON files
		all_data = []
		for json_file in json_files:
			with open(os.path.join(train_dir, json_file), 'r') as f:
				all_data.extend([json.loads(line) for line in f])

		print(f"Total images: {len(all_data)}")

		# Split clips once across the full dataset
		self.val_clips = self.split_clips(all_data)

		print("Starting conversion of TuSimple dataset...")
		self.convert_data(all_data)

		self.create_yaml()
		print("Done! YAML written to", os.path.join(self.output_dir, "data.yaml"))


def parse_args():
	parser = argparse.ArgumentParser(description="Convert TuSimple dataset to YOLO format with train/val split")
	parser.add_argument('--i', type=str, default='../../../datasets/TuSimple/TUSimple/', help='Path to TuSimple dataset directory')
	parser.add_argument('--o', type=str, default='../../../datasets/convertedYolo', help='Path to output directory for converted data')
	parser.add_argument('--val_size', type=float, default=0.2, help='Fraction of clips to use for validation (default: 0.2)')
	parser.add_argument('--seed', type=int, default=42, help='Random seed for reproducibility')
	return parser.parse_args()

def main():
	args = parse_args()
	print("Input directory:", args.i)
	print("Output directory:", args.o)
	converter = TuSimpleConverter(args.i, args.o, val_size=args.val_size, random_state=args.seed)
	converter.run()

if __name__ == "__main__":
	main()
