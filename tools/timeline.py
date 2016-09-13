/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#!/usr/bin/python

import cairo
import json
import sys

if __name__ == '__main__':
	json_filename = sys.argv[1]
	pdf_filename = sys.argv[2]

	with open(json_filename, 'r') as f:
		root = json.load(f)	
	
	point_per_usec = 0.00015

	row_height = 20
	row_space = 10
	jobs_offset = 150
	label_space = 15
	
	def node_label(nd):
		return nd['name'] + ' (' + nd['type'] + ')'
	
	

	width = (root['duration'] * point_per_usec) + jobs_offset
	height = len(root['nodes']) * (row_height + row_space)

	surface = cairo.PDFSurface(pdf_filename, width, height)
	ctx = cairo.Context(surface)

	def draw_text_in_box(txt, box_x, box_y, box_w, box_h, align):
		x_bearing, y_bearing, width, height, x_advance, y_advance = ctx.text_extents(txt)
		y = box_y + box_h/2.0 - height/2.0 - y_bearing
		if align == 'left':
			x = box_x + x_bearing
		elif align == 'center':
			x = box_x + box_w/2.0 - width/2.0 - x_bearing
		elif align == 'right':
			x = box_x + box_w - width - x_bearing
		ctx.move_to(x, y)
		ctx.show_text(txt)


	y = 0
	for nd in root['nodes']:
		label = nd['name'] + ' (' + nd['type'] + ')'
		draw_text_in_box(label, label_space, y, jobs_offset - 2*label_space, row_height, 'right')
		for jb in nd['jobs']:
			x0 = jobs_offset + jb['from']*point_per_usec
			x1 = jobs_offset + jb['to']*point_per_usec
			ctx.rectangle(x0, y, x1 - x0, row_height)
			ctx.stroke()
			draw_text_in_box(str(jb['t']), x0, y, x1 - x0, row_height, 'center')

		y += row_height + row_space

