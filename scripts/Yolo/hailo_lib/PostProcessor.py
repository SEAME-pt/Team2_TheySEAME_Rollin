import os
import subprocess
import sys
import struct
import traceback
import argparse
import threading
import queue

import numpy as np

		
def apply_sigmoid(tensor):
	tensor = 1 / (1 + np.exp(-tensor))  # Sigmoid
	return tensor

def apply_threshold(tensor, threshold):
	tensor = (tensor > threshold).astype(np.float32)
	return tensor

def polyfit_lines(tensor):
	if tensor.ndim != 4 or tensor.shape[0] == 0:
		return np.empty((0, 1, 2), dtype=np.int32)

	lane_mask = tensor[0, :, :, 0] == 1
	y_coords, x_coords = np.where(lane_mask)

	if x_coords.size < 3:
		return np.empty((0, 1, 2), dtype=np.int32)

	coefficients = np.polyfit(y_coords, x_coords, 2)
	y_values = np.linspace(0, lane_mask.shape[0] - 1, 100)
	x_values = coefficients[0] * y_values**2 + coefficients[1] * y_values + coefficients[2]
	x_values = np.clip(x_values, 0, lane_mask.shape[1] - 1)

	points = np.column_stack((x_values, y_values)).astype(np.int32)
	return points.reshape(-1, 1, 2)
