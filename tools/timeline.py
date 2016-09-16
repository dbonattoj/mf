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
	row_background_pad = 3
	
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


	# compute jobs offset (max width of labels)
	jobs_offset = 0
	for nd in root['nodes']:
		label = node_label(nd)
		_, _, w, _, _, _ = ctx.text_extents(label)
		jobs_offset = max(w, jobs_offset)	


	# draw jobs for each node
	y = 0
	for nd in root['nodes']:
		label = node_label(nd)
		draw_text_in_box(label, label_space, y, jobs_offset - 2*label_space, row_height, 'right')
		
		ctx.rectangle(jobs_offset, y - row_background_pad, width - jobs_offset, row_height + 2*row_background_pad)
		ctx.set_source_rgb(0.8, 0.8, 0.8)
		ctx.fill()

		for jb in nd['jobs']:
			x0 = jobs_offset + jb['from']*point_per_usec
			x1 = jobs_offset + jb['to']*point_per_usec
			ctx.rectangle(x0, y, x1 - x0, row_height)
			ctx.set_source_rgb(0.0, 0.0, 0.0);
			ctx.stroke()
			ctx.rectangle(x0, y, x1 - x0, row_height)
			ctx.set_source_rgb(1.0, 1.0, 1.0);
			ctx.fill()
			ctx.set_source_rgb(0.0, 0.0, 0.0);
			draw_text_in_box(str(jb['t']), x0, y, x1 - x0, row_height, 'center')

		y += row_height + row_space

