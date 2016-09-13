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

#include "graph_visualization.h"
#include <ostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <typeinfo>
#include <map>
#include "../node_graph.h"
#include "../processing/sync_node.h"
#include "../processing/async_node.h"
#include "../processing/sink_node.h"
#include "../multiplex/multiplex_node.h"

namespace mf { namespace flow {

using namespace std::literals;

namespace {	
	std::vector<std::string> html_colors_ {
		"blue",
		"red",
		"forestgreen",
		"orange",
		"deeppink",
		"turquoise",
		"salmon",
		"lightskyblue3",
		"wheat4",
		"yellowgreen",
		"gray33",
		"indigo"
	};
}


graph_visualization::graph_visualization(const node_graph& gr, std::ostream& output) :
	output_(output),
	graph_(gr) { }


std::string graph_visualization::thread_index_color_(thread_index tid) const {
	if(! thread_index_colors_) return "black";
	else if(tid == graph_.root_thread_index()) return "black";
	else if(tid == undefined_thread_index) return "lightgray";
	else return html_colors_.at((tid - 1) % html_colors_.size());
}


void graph_visualization::generate_node_dispatch_(const node& nd) {	
	if(typeid(nd) == typeid(sync_node))
		generate_processing_node_(static_cast<const processing_node&>(nd), false, false);
	else if(typeid(nd) == typeid(async_node))
		generate_processing_node_(static_cast<const processing_node&>(nd), true, false);
	else if(typeid(nd) == typeid(sink_node))
		generate_processing_node_(static_cast<const processing_node&>(nd), false, true);
	else if(typeid(nd) == typeid(multiplex_node))
		generate_multiplex_node_(static_cast<const multiplex_node&>(nd));
	else
		throw std::logic_error("unknown node type for visualization");
}


void graph_visualization::generate_processing_node_(const processing_node& nd, bool async, bool sink) {
	std::string node_id = uid_(nd, "node");
		
	std::ostringstream html;
	html << R"(<TABLE BORDER="0" CELLBORDER="0" CELLSPACING="0" CELLPADDING="0">)";
	
	if(nd.inputs_count() > 0) {
		html << R"(<TR>)";
		html << R"(<TD WIDTH="20"></TD>)";
		for(std::ptrdiff_t i = 0; i < nd.inputs_count(); ++i) {
			const node_input& in = nd.input_at(i);
			std::string in_col = thread_index_color_(in.reader_thread_index());
			std::string input_name = "in";
			std::string input_id = uid_(in, "in");
			html << R"(<TD BORDER="1" CELLPADDING="1" PORT=")" << input_id << R"(" COLOR=")" << in_col << R"(">)";
			html << R"(<FONT POINT-SIZE="10">)" << nd.input_at(i).name() << R"(</FONT>)";
			html << R"(</TD>)";
			html << R"(<TD WIDTH="20"></TD>)";
		}
		html << R"(</TR>)";
	}
	
	std::string col = thread_index_color_(nd.processing_thread_index());
	std::size_t colspan = 2*nd.inputs_count() + 1;
	html << R"(<TR>)";
	html << R"(<TD COLSPAN=")" << colspan << R"(" BORDER="1" STYLE="ROUNDED" CELLPADDING="4" COLOR=")" << col << R"(">)";
	html << nd.name();
	html << R"(<BR/><FONT POINT-SIZE="10">)";
	if(sink) html << "sink node";
	else if(async) html << "async node";
	else html << "sync node";
	if(async) {
		html << R"(<BR/>)";
		html << "prefetch = " << static_cast<const async_node&>(nd).prefetch_duration();
	}
	html << R"(</FONT>)";
	
	if(with_parameters_ && nd.parameters_count() + nd.input_parameters_count() > 0) {
		html << R"(<BR/><BR/>)";
		for(std::ptrdiff_t i = 0; i < nd.parameters_count(); ++i)
			html << R"(&#x25A0; <FONT POINT-SIZE="10">)" << nd.parameter_at(i).id() << R"( ()" << nd.parameter_at(i).name() << R"()</FONT><BR/>)";
		for(std::ptrdiff_t i = 0; i < nd.input_parameters_count(); ++i)
			html << R"(&#x25A1; <FONT POINT-SIZE="10">*)" << nd.input_parameter_at(i) << R"(</FONT><BR/>)";
	}
	
	if(with_state_) {
		html << R"(<BR/><BR/><FONT POINT-SIZE="10">)";
		time_unit t = nd.current_time();
		html << "t = " << t;
		html << R"(</FONT>)";
	}
	
	html << R"(</TD>)";
	html << R"(</TR>)";
	
	if(! sink) {
		std::string output_col = thread_index_color_(nd.output().reader_thread_index());
		html << R"(<TR>)";
		html << R"(<TD COLSPAN=")" << colspan << R"(" BORDER="0">)";
		html << R"(<TABLE BORDER="0" CELLBORDER="0" CELLSPACING="0" CELLPADDING="0">)";
		if(async) html << R"(<TR><TD></TD><TD BORDER="1" HEIGHT="3" COLOR=")" << col << R"("></TD><TD></TD></TR>)";
		html << R"(<TR>)";
		html << R"(<TD WIDTH="20"></TD>)";
		html << R"(<TD BORDER="1" CELLPADDING="3" PORT=")"
			<< uid_(nd.output(), "out") << R"(" COLOR=")" << output_col << R"(">)";
		html << R"(<TABLE BORDER="0" CELLSPACING="2">)";
		html << R"(<TR CELLPADDING="1">)";
		for(std::ptrdiff_t i = 0; i < nd.output_channels_count(); ++i) {
			const std::string& chan_name = nd.output_channel_at(i).name();
			if(i > 0) html << R"(<TD WIDTH="5"></TD>)";
			html << R"(<TD BORDER="1" CELLPADDING="1">)";
			html << R"(<FONT POINT-SIZE="10">)" << chan_name << R"(</FONT>)";
			html << R"(</TD>)";
		}
		html << R"(</TR>)";
		html << R"(</TABLE>)";
		html << R"(</TD>)";
		html << R"(<TD WIDTH="20"></TD>)";
		html << R"(</TR>)";
		html << R"(</TABLE>)";
		html << R"(</TD>)";
		html << R"(</TR>)";
	}
	
	html << R"(</TABLE>)";
	
	
	output_ << '\t' << node_id << " [shape=plaintext label=<\n" << html.str() << "\n>];\n";
}


void graph_visualization::generate_multiplex_node_(const multiplex_node& nd) {
	std::string node_id = uid_(nd, "node");
	
	std::ostringstream html;
	html << R"(<TABLE BORDER="0" CELLBORDER="0" CELLSPACING="0" CELLPADDING="0">)";
	
	std::string input_id = uid_(nd.input(), "in");
	std::string in_col = thread_index_color_(nd.input().reader_thread_index());
	html << R"(<TR>)";
	html << R"(<TD WIDTH="20"></TD>)";
	html << R"(<TD BORDER="1" CELLPADDING="1" PORT=")" << input_id << R"(" COLOR=")" << in_col << R"(">)";
	html << R"(<FONT POINT-SIZE="10">)" << nd.input().name() << R"(</FONT>)";
	html << R"(</TD>)";
	html << R"(<TD WIDTH="20"></TD>)";
	html << R"(</TR>)";
	
	std::string col = thread_index_color_(nd.loader_thread_index());
	html << R"(<TR>)";
	html << R"(<TD COLSPAN="3" BORDER="1" STYLE="ROUNDED" CELLPADDING="4" COLOR=")" << col << R"(">)";
	html << nd.name();
	if(nd.is_async()) html << " (async)";
	else html << " (sync)";
	html << R"(</TD>)";
	html << R"(</TR>)";
	
	html << R"(<TR>)";
	html << R"(<TD COLSPAN="3" BORDER="0">)";
	html << R"(<TABLE BORDER="0" CELLBORDER="0" CELLSPACING="0" CELLPADDING="0">)";

	if(nd.is_async()) {
		std::size_t colspan = 2*nd.outputs_count() - 1;
		html << R"(<TR><TD></TD><TD BORDER="1" HEIGHT="3" COLSPAN=")"
			<< colspan << R"(" COLOR=")" << col << R"("></TD><TD></TD></TR>)";
	}

	html << R"(<TR>)";
	html << R"(<TD WIDTH="20"></TD>)";
	
	for(std::ptrdiff_t i = 0; i < nd.outputs_count(); ++i) {
		std::string output_id = uid_(nd.output_at(i), "out");
		std::string output_col = thread_index_color_(nd.output_at(i).reader_thread_index());
		html << R"(<TD BORDER="1" CELLPADDING="3" PORT=")" << output_id << R"(" COLOR=")" << output_col << R"(">)";
		html << R"(<TABLE BORDER="0" CELLSPACING="2">)";
		html << R"(<TR CELLPADDING="1">)";
		html << R"(<TD BORDER="1" CELLPADDING="1">)";
		html << R"(<FONT POINT-SIZE="10">)" << nd.output_at(i).name() << R"(</FONT>)";
		html << R"(</TD>)";
		html << R"(</TR>)";
		html << R"(</TABLE>)";
		html << R"(</TD>)";
		html << R"(<TD WIDTH="20"></TD>)";
	}
	
	html << R"(</TR>)";
	html << R"(</TABLE>)";
	html << R"(</TD>)";
	html << R"(</TR>)";
	
	html << R"(</TABLE>)";
	
	
	output_ << '\t' << node_id << " [shape=plaintext label=<\n" << html.str() << "\n>];\n";
}


void graph_visualization::generate_node_input_connections_(const node& nd) {
	for(std::ptrdiff_t i = 0; i < nd.inputs_count(); ++i) {
		const node_input& in = nd.input_at(i);
		if(! in.is_connected()) continue;
		const node_output& out = in.connected_output();

		std::string in_col = thread_index_color_(in.reader_thread_index());
		
		std::string label;
		time_unit p = in.past_window_duration();
		time_unit f = in.future_window_duration();
		if(p > 0 && f > 0) label = "[-"s + std::to_string(p) + ", +" + std::to_string(f) + "]     ";
		else if(p > 0) label = "[-"s + std::to_string(p) + "]  ";
		else if(f > 0) label = "[+" + std::to_string(f) + "]  ";
		
		std::string arrow_shape, style;
		if(!with_state_ || in.is_activated()) {
			arrow_shape = "normal";
			style = "";
		} else {
			arrow_shape = "nonetee";
			style = "dotted";
		}
		
		if(with_parameters_ && out.propagated_parameters_count() > 0) {
			std::string pars;
			for(std::ptrdiff_t j = 0; j < out.propagated_parameters_count(); ++j) {
				if(j > 0) pars += ", ";
				pars += R"(&#x25A1; )" + std::to_string(out.propagated_parameter_at(j));
			}
			label = pars + R"(<BR/>)" + label;
		}
				
		output_
			<< '\t' << uid_(out.this_node(), "node") << ':' << uid_(out, "out")
			<< " -> " << uid_(nd, "node") << ':' << uid_(in, "in") << " ["
			<< "style=\"" << style << "\", "
			<< "arrowhead=\"" << arrow_shape << "\", "
			<< "color=" << in_col << ", "
			<< "headlabel=<" << label << ">, "
			<< "fontsize=10, "
			<< "labelangle=45, "
			<< "labeldistance=2.0, "
			<< "labelfontcolor=" << in_col
			<< "];\n";
	}
}


void graph_visualization::generate_ranks_() {
	output_ << "\t{rank=source;";
	for(std::ptrdiff_t i = 0; i < graph_.nodes_count(); ++i) {
		const node& nd = graph_.node_at(i);
		if(nd.inputs_count() == 0) output_ << ' ' << uid_(nd, "node");
	}
	output_ << "}\n";
	
	const node& nd = graph_.sink();
	output_ << "\t{rank=sink; " << uid_(nd, "node") << "}\n";
}


void graph_visualization::generate() {
	output_ << "digraph " << graph_id_ << "{\n";
	output_ << "\trankdir=TB\n";

	for(std::ptrdiff_t i = 0; i < graph_.nodes_count(); ++i) generate_node_dispatch_(graph_.node_at(i));
	for(std::ptrdiff_t i = 0; i < graph_.nodes_count(); ++i) generate_node_input_connections_(graph_.node_at(i));
	generate_ranks_();	
			
	output_ << '}' << std::flush;
}


void export_graph_visualization(const node_graph& gr, const std::string& filename) {
	std::ofstream fstr(filename);
	graph_visualization vis(gr, fstr);
	vis.generate();
}

}}
