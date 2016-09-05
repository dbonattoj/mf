#!/usr/bin/python

import cairo
import json
import sys

if __name__ == '__main__':
	json_filename = sys.argv[1]
	pdf_filename = sys.argv[2]

	with open(json_filename, 'r') as f:
		root = json.load(f)
	
	for nd in root['nodes']:
		print nd['name']
