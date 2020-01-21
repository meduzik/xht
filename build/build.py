import os
import sys


HEADER_LIST = [
	"config.hpp",
	"feature.hpp",
	"core.hpp",
	"intrin.hpp",
	"traits.hpp",
	"intlog.hpp",
	"compare.hpp",
	"key.hpp",
	"simplify.hpp",
	"hashtable.hpp",
	"fnv1ahash.hpp",
	"hash.hpp",
	"hashmap.hpp"
]

HEADER_PATH = "include/xht/"

SOURCE_LIST = [
	"hashtable.cpp"
]

SOURCE_PATH = "source/xht/"


root_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")


def open_create(file):
	os.makedirs(os.path.dirname(file), exist_ok=True)
	return open(file, 'w')
	
	
def is_line_local_header(line):
	return line.startswith("#include \"") or line.startswith("#include <xht/")


def write_headers(out_file):
	for header in HEADER_LIST:
		with open(os.path.join(root_dir, HEADER_PATH, header), 'r') as in_file:
			out_file.write(f"// #### BEGIN {header} #### \n")
			for line in in_file.readlines():
				if line.strip() == "#pragma once" or is_line_local_header(line):
					continue
				out_file.write(line)
			out_file.write(f"// #### END {header} #### \n")

def write_sources(out_file):
	for source in SOURCE_LIST:		
		with open(os.path.join(root_dir, SOURCE_PATH, source), 'r') as in_file:
			out_file.write(f"// #### BEGIN {source} #### \n")
			for line in in_file.readlines():
				if is_line_local_header(line):
					continue
				out_file.write(line)
			out_file.write(f"// #### END {source} #### \n")
				

def generate_unity_file(path):
	with open_create(path) as out_file:
		out_file.write(f"#if !defined(XHT_UNITY_HEADER)\n")
		out_file.write(f"#define XHT_UNITY_HEADER\n")
		write_headers(out_file)
		
		out_file.write(f"// ##### BEGIN IMPLEMENTATION #### \n")
		out_file.write(f"#if defined(XHT_INCLUDE_IMPLEMENTATION)\n")
		write_sources(out_file)
		out_file.write(f"#endif\n")
		out_file.write(f"// ##### END IMPLEMENTATION #### \n")
		
		out_file.write(f"#endif\n")
		
		
def generate_library(path):
	with open_create(os.path.join(path, "include/xht/xht.hpp")) as out_file:
		out_file.write(f"#if !defined(XHT_UNITY_HEADER)\n")
		out_file.write(f"#define XHT_UNITY_HEADER\n")
		write_headers(out_file)
		out_file.write(f"#endif\n")
		
	with open_create(os.path.join(path, "source/xht/xht.cpp")) as out_file:
		out_file.write(f"#include <xht/xht.hpp>\n")
		write_sources(out_file)


if __name__ == "__main__":
	
	if len(sys.argv) > 1:
		out_dir = sys.argv[1]
	else:
		out_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "products")
		generate_unity_file(os.path.join(out_dir, "header_only/xht.hpp"))
		generate_library(os.path.join(out_dir, "lib"))
