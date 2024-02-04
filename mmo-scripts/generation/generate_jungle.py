import twmap
from perlin_noise import PerlinNoise
import numpy as np
import time
import random
from vmath import *
from PIL import Image

from ctypes import CDLL, c_double, POINTER

lib = CDLL("C:\\Users\\nemo\\Desktop\\Тык\\mmo\\scripts\\perlin_noise.dll")
lib.perlin_noise.argtypes = [c_double]
lib.perlin_noise.restype = c_double
per_noise_1d = lib.perlin_noise

# === Config ===
# General
map_width = 1000
map_height = 250

# Landscape
map_flat_modificator = 100
map_minimum_height = 90  # %
map_hilly_modificator = 5  # %

# Caves
map_caves_scale_modificator = 5
map_caves_big_scale_modificator = 75


def print_bar(progress, total, length, text):
	filled_length = int(length * progress / total)
	bar = "=" * (filled_length - 1) + ">" + " " * (length - filled_length)
	print("\r%s [%s] %.1f%%" % (text, bar, progress / total * 100), end="", flush=True)


def get_time():
	return time.time()


def init():
	map = twmap.Map.empty("DDNet06")
	map.info.author = "Generated"
	map.info.license = "Huh"
	map.info.version = "v0.1"
	map.info.credits = "Myr"

	return map


def generate_landscape(tilemap, noise):
	for x in range(map_width):
		print_bar(x + 1, map_width, 30, "Landscape")

		value = noise(x / map_flat_modificator)
		value *= round(map_height / 100 * map_hilly_modificator)
		value += round(map_height / 100 * (100 - map_minimum_height))

		for y in range(map_height):
			if y > value:
				tilemap[y][x][0] = 1

				# if y > value + 10:
				# 	value2 = noise([x / map_caves_scale_modificator, y / map_caves_scale_modificator])
				# 	if value2 < 0:
				# 		tilemap[y][x][0] = 0

			# if tilemap[y][x][0] != 0:
			# 	value2 = noise([x / map_caves_big_scale_modificator, y / map_caves_big_scale_modificator])

			# 	if value2 > -0.1 and value2 < 0:
			# 		tilemap[y][x][0] = 0


def generate_caves(tilemap, noise):
	global map_caves_radius
	#print("")


def generate_load_resources(map):
	print("\nLoading resources...")

	img = Image.open("mapres/grass_main.png").convert("RGBA")
	map.images.new_from_data("Grass main", np.array(img))

	print("Done!")


def generate_texturing(group, tilemap, noise):
	# Create main layer
	jungle_main = group.layers.new_tiles(map_width, map_height)
	jungle_main.name = "Main"
	jungle_main.image = 0

	tiles = np.zeros((map_height, map_width, 2), dtype=np.uint8)


	def get_tile(x, y):
		return tilemap[y][x][0]


	def set_tile(x, y, _id):
		tiles[y][x][0] = _id


	def do_better(x, y):
		is_up_open = (y - 1 > 0) and (get_tile(x, y - 1) == 0)
		is_right_open = (x + 1 < map_width) and (get_tile(x + 1, y) == 0)
		is_left_open = (x - 1 > 0) and (get_tile(x - 1, y) == 0)

		is_up_left_open = (y - 1 > 0) and (x - 1 > 0) and (get_tile(x - 1, y - 1) == 0)
		is_up_right_open = (y - 1 > 0) and (x + 1 < map_width) and (get_tile(x + 1, y - 1) == 0)

		if is_up_open:
			set_tile(x, y, 16)

		if is_up_open and is_left_open:
			set_tile(x, y, 32)
		if is_up_open and is_right_open:
			set_tile(x, y, 33)

		if not is_up_open and not is_left_open and is_up_left_open:
			set_tile(x, y, 48)
		if not is_up_open and not is_right_open and is_up_right_open:
			set_tile(x, y, 49)


	for x in range(map_width):
		print_bar(x + 1, map_width, 30, "Texturing")

		is_collide = False

		for y in range(map_height):
			if get_tile(x, y) == 0:
				continue

			set_tile(x, y, 1)

			# Raycast from sky to ground for create grass
			if not is_collide and get_tile(x, y) != 0:
				do_better(x, y)
				do_better(x, y + 1)

				is_collide = True

	jungle_main.tiles = tiles


def generate():
	start_time = get_time()
	map = init()
	tilemap = np.zeros((map_height, map_width, 2), dtype=np.uint8)
	textures_group = map.groups.new_physics()
	textures_group.parallax_x = 100
	textures_group.parallax_y = 100

	noise = PerlinNoise(seed=get_time(), octaves=1)

	# Generate
	generate_landscape(tilemap, noise)
	generate_caves(tilemap, noise)
	generate_load_resources(map)
	generate_texturing(textures_group, tilemap, noise)

	# Create Game group
	physics_group = map.groups.new_physics()
	game_layer = physics_group.layers.new_game(map_width, map_height)
	game_layer.tiles = tilemap

	finish_time = get_time() - start_time
	print("\nTook %.2f seconds" % finish_time)

	return map


def main():
	map = generate()
	map.save("out/mmo_jungle.map")


if __name__ == "__main__":
	main()
